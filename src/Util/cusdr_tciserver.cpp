/**
 * @file  cusdr_tciserver.cpp
 * @brief ExpertSDR TCI WebSocket server — Phase 1 control commands
 *
 * Implements a subset of the TCI text protocol for remote rig control:
 *   VFO, DDS, IF, MODULATION, RX_FILTER_BAND, TRX, DRIVE, TUNE, RX_SMETER,
 *   SPLIT_ENABLE / RIT_ENABLE / XIT_ENABLE / MUTE / LOCK (compat no-ops),
 *   SQL/ANF/NB/BIN/MON/AGC_AUTO_EX / RX_VOLUME / TUNE_DRIVE (compat stubs),
 *   TX_PROFILES_EX / TX_PROFILE_EX / TX_STREAM_AUDIO_BUFFERING / TX_SENSORS_ENABLE,
 *   TX_SENSORS / TX_POWER / TX_SWR (PA telemetry from RadioTelemetry),
 *   AUDIO_START/STOP (binary RX audio), START, STOP,
 *   TX_CHRONO (binary type 3) to clock client TX audio during MOX
 *
 * Transport: WebSocket text frames, commands NAME:arg1,arg2;
 * cudaSDR acts as server; clients receive an init burst on connect plus live push updates.
 */

#include "cusdr_tciserver.h"
#include "Util/tci_protocol_utils.h"
#include "DataEngine/protocol_boundary_utils.h"
#include "cusdr_settings.h"
#include "cusdr_hamDatabase.h"
#include "Settings/SettingsTypes.h"
#include "Models/RadioModel.h"
#include "Models/RadioTelemetry.h"
#include "Models/SliceModel.h"

#include <QWebSocketServer>
#include <QWebSocket>
#include <QHostAddress>
#include <QStringList>
#include <QDebug>
#include <QDateTime>
#include <QtEndian>
#include <QMetaType>
#include <cmath>
#include <cstring>
#include <cstdlib>

using namespace TciProtocol;

TciServer::TciServer(QObject *parent)
    : QObject(parent)
    , m_server(new QWebSocketServer(QStringLiteral("cudaSDR TCI"),
                                    QWebSocketServer::NonSecureMode, this))
    , m_watchdog(new QTimer(this))
    , m_txChronoTimer(new QTimer(this))
    , m_settings(Settings::instance())
{
    qRegisterMetaType<QVector<float>>("QVector<float>");

    connect(m_server, &QWebSocketServer::newConnection, this, &TciServer::onNewConnection);

    m_watchdog->setSingleShot(true);
    m_watchdog->setInterval(WATCHDOG_TIMEOUT_MS);
    connect(m_watchdog, &QTimer::timeout, this, &TciServer::onWatchdogTimeout);

    m_txChronoTimer->setTimerType(Qt::PreciseTimer);
    m_txChronoTimer->setInterval(kTxChronoPollMs);
    connect(m_txChronoTimer, &QTimer::timeout, this, &TciServer::onTxChronoTick);

    connect(m_settings, &Settings::vfoFrequencyChanged,
            this, &TciServer::onVfoFrequencyChanged);
    connect(m_settings, &Settings::ctrFrequencyChanged,
            this, &TciServer::onCtrFrequencyChanged);
    connect(m_settings, &Settings::ncoFrequencyChanged,
            this, &TciServer::onNcoFrequencyChanged);
    connect(m_settings, &Settings::dspModeChanged,
            this, &TciServer::onDspModeChanged);
    connect(m_settings, &Settings::filterFrequenciesChanged,
            this, &TciServer::onFilterFrequenciesChanged);
    connect(m_settings, &Settings::radioStateChanged,
            this, &TciServer::onRadioStateChanged);
    connect(m_settings, &Settings::driveLevelChanged,
            this, &TciServer::onDriveLevelChanged);
    connect(m_settings, &Settings::tciServerEnabledChanged,
            this, &TciServer::onTciServerEnabledChanged);
    connect(m_settings, &Settings::systemStateChanged, this,
            [this](QSDR::_Error, QSDR::_HWInterfaceMode, QSDR::_ServerMode,
                   QSDR::_DataEngineState state) {
        if (state != QSDR::DataEngineUp && state != QSDR::DataEngineDown)
            return;
        const bool powerOn = (state == QSDR::DataEngineUp);
        if (powerOn == m_advertisedPowerOn)
            return;
        m_advertisedPowerOn = powerOn;
        // Keep TCI clients in sync when the user starts/stops from the UI (or
        // when a TCI START succeeds after the data engine comes up).
        broadcast(tciMessage(powerOn ? QStringLiteral("START")
                                     : QStringLiteral("STOP")));
    });

    m_advertisedPowerOn = (m_settings->getDataEngineState() == QSDR::DataEngineUp);
}

TciServer::~TciServer()
{
    stopListening();
}

bool TciServer::startListening(quint16 port)
{
    if (m_server->isListening())
        return true;

    // Bind on both IPv4 and IPv6 stacks. Browsers resolve "localhost" to the
    // IPv6 loopback (::1) first, so an IPv4-only bind (AnyIPv4) refuses those
    // ws://localhost connections. QHostAddress::Any listens dual-stack.
    if (!m_server->listen(QHostAddress::Any, port)) {
        TCI_WARN << "Failed to listen on port" << port << ":" << m_server->errorString();
        return false;
    }

    TCI_DEBUG << "Listening on port" << port;
    emit connectionStatusChanged();
    return true;
}

void TciServer::stopListening()
{
    stopTxChrono();
    const bool hadClients = !m_clients.isEmpty();
    for (QWebSocket *client : std::as_const(m_clients)) {
        client->close();
    }
    m_clients.clear();
    m_clientStates.clear();
    m_server->close();
    if (m_settings)
        m_settings->setTciIqActive(false);
    if (hadClients)
        emit remoteControlChanged(false);
    emit connectionStatusChanged();
}

bool TciServer::isListening() const
{
    return m_server->isListening();
}

quint16 TciServer::port() const
{
    return m_server->serverPort();
}

void TciServer::setRxGain(float gain)
{
    const float clamped = qBound(0.0f, gain, 2.0f);
    if (std::abs(m_rxGain - clamped) < 1e-6f)
        return;
    m_rxGain = clamped;
    emit rxGainChanged(m_rxGain);
}

void TciServer::setTxGain(float gain)
{
    const float clamped = qBound(0.0f, gain, 2.0f);
    if (std::abs(m_txGain - clamped) < 1e-6f)
        return;
    m_txGain = clamped;
    emit txGainChanged(m_txGain);
}

QString TciServer::connectionStatusText() const
{
    if (!m_settings || !m_settings->getTciServerEnabled())
        return QStringLiteral("Disabled");
    if (!isListening())
        return QStringLiteral("Not listening");

    QString text;
    const int n = m_clients.size();
    if (n == 0)
        text = QStringLiteral("Listening — no clients");
    else if (n == 1)
        text = QStringLiteral("Connected — 1 client");
    else
        text = QStringLiteral("Connected — %1 clients").arg(n);

    if (m_txChronoClient)
        text += QStringLiteral(" (TX)");
    return text;
}

void TciServer::sendToClient(QWebSocket *client, const QString &message)
{
    if (!client || client->state() != QAbstractSocket::ConnectedState)
        return;

    TCI_TRACE << "TX ->" << client->peerAddress().toString() << message;
    client->sendTextMessage(message);
}

void TciServer::broadcast(const QString &message)
{
    if (m_clients.isEmpty())
        return;
    TCI_TRACE << "TX *" << message;
    for (QWebSocket *client : std::as_const(m_clients))
        sendToClient(client, message);
}

QString TciServer::formatVfo(int trx, int channel, qint64 frequency) const
{
    return tciMessage(QStringLiteral("VFO"),
                      {QString::number(trx), QString::number(channel), QString::number(frequency)});
}

/**
 * cudaSDR extension: which VFO memory the RX dial is sitting on. TCI 1.5 has no
 * verb for this — channel 0/1 only address the two frequency memories — so the
 * A/B switch would otherwise be local to whichever end the operator touched.
 */
QString TciServer::formatActiveVfo(int trx, int channel) const
{
    return tciMessage(QStringLiteral("ACTIVE_VFO"),
                      {QString::number(trx), QString::number(channel)});
}

QString TciServer::formatDds(int trx, int channel, qint64 frequency) const
{
    return tciMessage(QStringLiteral("DDS"),
                      {QString::number(trx), QString::number(channel), QString::number(frequency)});
}

QString TciServer::formatIf(int trx, int channel, qint64 offset) const
{
    return tciMessage(QStringLiteral("IF"),
                      {QString::number(trx), QString::number(channel), QString::number(offset)});
}

QString TciServer::formatModulation(int trx, DSPMode mode) const
{
    return tciMessage(QStringLiteral("MODULATION"),
                      {QString::number(trx), dspModeToTci(mode)});
}

QString TciServer::formatRxFilterBand(int trx, qreal low, qreal high) const
{
    return tciMessage(QStringLiteral("RX_FILTER_BAND"),
                      {QString::number(trx),
                       QString::number(static_cast<qint64>(low)),
                       QString::number(static_cast<qint64>(high))});
}

QString TciServer::formatTrx(int trx, RadioState state) const
{
    return tciMessage(QStringLiteral("TRX"),
                      {QString::number(trx), isTrxActive(state) ? QStringLiteral("true") : QStringLiteral("false")});
}

QString TciServer::formatDrive(int trx, int level) const
{
    return tciMessage(QStringLiteral("DRIVE"),
                      {QString::number(trx), QString::number(level)});
}

QString TciServer::formatTune(int trx, bool enabled) const
{
    return tciMessage(QStringLiteral("TUNE"),
                      {QString::number(trx), enabled ? QStringLiteral("true") : QStringLiteral("false")});
}

QString TciServer::formatRxSMeter(int trx, int channel, double dbm) const
{
    return tciMessage(QStringLiteral("RX_SMETER"),
                      {QString::number(trx),
                       QString::number(channel),
                       QString::number(dbm, 'f', 1)});
}

QString TciServer::formatTxSensors(int trx) const
{
    // ExpertSDR: trx, mic_dBm, fwd_W (RMS), peak_W, SWR.
    // Mic level is not available yet — report 0. Peak falls back to forward.
    const qreal swr = effectiveSwr();
    return tciMessage(QStringLiteral("TX_SENSORS"),
                      {QString::number(trx),
                       QStringLiteral("0.0"),
                       QString::number(m_fwdPowerWatts, 'f', 1),
                       QString::number(m_fwdPowerWatts, 'f', 1),
                       QString::number(swr, 'f', 2)});
}

QString TciServer::formatTxPower(int trx, qreal watts) const
{
    return tciMessage(QStringLiteral("TX_POWER"),
                      {QString::number(trx), QString::number(watts, 'f', 1)});
}

QString TciServer::formatTxSwr(int trx, qreal swr) const
{
    return tciMessage(QStringLiteral("TX_SWR"),
                      {QString::number(trx), QString::number(swr, 'f', 2)});
}

qreal TciServer::effectiveSwr() const
{
    if (m_swrValid)
        return m_swr;
    const bool tx = m_settings && acceptsTxAudio(m_settings->getRadioState());
    return ProtocolBoundaryUtils::swrFromFwdRevWatts(m_fwdPowerWatts, m_revPowerWatts, tx);
}

void TciServer::maybeBroadcastTxSensors(bool force)
{
    if (m_clients.isEmpty() || !m_settings)
        return;

    const bool transmitting = isTrxActive(m_settings->getRadioState());
    if (!transmitting && !force)
        return;

    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    const QString sensors = formatTxSensors(0);
    const QString power = formatTxPower(0, m_fwdPowerWatts);
    const QString swr = formatTxSwr(0, effectiveSwr());

    for (QWebSocket *client : std::as_const(m_clients)) {
        TciClientState *state = clientState(client);
        if (!state || !state->txSensorsEnabled)
            continue;
        if (!force && state->txSensorsLastSendMs > 0
            && (nowMs - state->txSensorsLastSendMs) < state->txSensorsIntervalMs) {
            continue;
        }
        state->txSensorsLastSendMs = nowMs;
        sendToClient(client, sensors);
        sendToClient(client, power);
        sendToClient(client, swr);
    }
}

void TciServer::scheduleTxSensorsBroadcast()
{
    // Coalesce forward/reverse/SWR updates that arrive in the same C&C frame.
    if (m_txSensorsFlushPending)
        return;
    m_txSensorsFlushPending = true;
    QTimer::singleShot(0, this, [this]() {
        m_txSensorsFlushPending = false;
        maybeBroadcastTxSensors();
    });
}

QString TciServer::dspModeToTci(DSPMode mode) const
{
    return TciProtocol::dspModeToTci(mode);
}

DSPMode TciServer::tciModeToDsp(const QString &mode) const
{
    return TciProtocol::tciModeToDsp(mode);
}

bool TciServer::parseBoolArg(const QString &value) const
{
    return TciProtocol::parseBoolArg(value);
}

void TciServer::parseTrxEnableArgs(const QStringList &args, int &enableTrx,
                                   bool &enableValue, bool &hasValue) const
{
    enableTrx = 0;
    enableValue = false;
    hasValue = false;

    if (args.isEmpty())
        return;

    if (args.size() == 1) {
        bool ok = false;
        const int asInt = args.at(0).toInt(&ok);
        // Pure integer → query for that TRX; otherwise treat as bool set
        // (e.g. WSJT-X: split_enable:false / mon_enable:true).
        if (ok && args.at(0).trimmed() == QString::number(asInt)) {
            enableTrx = asInt;
            return;
        }
        enableValue = parseBoolArg(args.at(0));
        hasValue = true;
        return;
    }

    bool ok = false;
    enableTrx = args.at(0).toInt(&ok);
    if (!ok)
        enableTrx = 0;
    // VFO_LOCK:trx,channel,bool — take the last token as the bool.
    enableValue = parseBoolArg(args.last());
    hasValue = true;
}

int TciServer::ifOffsetHz(int rx) const
{
    return static_cast<int>(m_settings->getVfoFrequency(rx) - m_settings->getCtrFrequency(rx));
}

double TciServer::smeterDbmFromRaw(double rawValue) const
{
    // Raw WDSP RXA_S_AV meter — no panadapter display offset.
    return rawValue;
}

double TciServer::smeterDbmForRx(int rx) const
{
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size()) {
        if (SliceModel *slice = m_radioModel->slices().at(rx))
            return smeterDbmFromRaw(slice->sMeterValue());
    }
    return smeterDbmFromRaw(-140.0);
}

int TciServer::nativeIqSampleRate() const
{
    const int rate = m_settings ? m_settings->getSampleRate() : 0;
    return effectiveIqSampleRate(rate > 0 ? rate : 48000);
}

int TciServer::effectiveIqSampleRate(int actualRate) const
{
    static const int override = qEnvironmentVariableIntValue("CUSDR_TCI_IQ_RATE");
    return TciProtocol::effectiveIqSampleRate(actualRate, static_cast<int>(kRxAudioRateHz), override);
}

void TciServer::updateIqActiveHint()
{
    bool anyIq = false;
    if (m_settings && m_settings->getTciServerEnabled()) {
        for (auto it = m_clientStates.cbegin(); it != m_clientStates.cend(); ++it) {
            if (!it->iqEnabledReceivers.isEmpty()) {
                anyIq = true;
                break;
            }
        }
    }
    if (m_settings)
        m_settings->setTciIqActive(anyIq);
}

void TciServer::bindSlices(RadioModel *radioModel)
{
    if (!radioModel || m_radioModel == radioModel)
        return;

    for (const QMetaObject::Connection &conn : std::as_const(m_sliceConnections))
        disconnect(conn);
    m_sliceConnections.clear();

    m_radioModel = radioModel;
    for (SliceModel *slice : radioModel->slices()) {
        if (!slice)
            continue;
        const int rx = slice->id();
        m_sliceConnections.append(
            connect(slice, &SliceModel::sMeterValueChanged, this,
                    [this, rx](double value) { onSMeterValueChanged(rx, value); }));
        // Channel 0/1 track the A/B memories (the dial writes through to the
        // active one), so clients see local A/B edits, copies and swaps.
        m_sliceConnections.append(
            connect(slice, &SliceModel::vfoAFrequencyChanged, this,
                    [this, rx](qint64 hz) {
                        if (rx == rxSliceIdForTrx(0))
                            broadcast(formatVfo(0, 0, hz));
                    }));
        m_sliceConnections.append(
            connect(slice, &SliceModel::vfoBFrequencyChanged, this,
                    [this, rx](qint64 hz) {
                        if (rx == rxSliceIdForTrx(0))
                            broadcast(formatVfo(0, 1, hz));
                    }));
        m_sliceConnections.append(
            connect(slice, &SliceModel::activeVfoChanged, this,
                    [this, rx](SliceModel::ActiveVfo) { onActiveVfoChanged(rx); }));
    }

    if (RadioTelemetry *tel = radioModel->telemetry()) {
        m_sliceConnections.append(
            connect(tel, &RadioTelemetry::forwardPowerChanged, this,
                    &TciServer::onForwardPowerChanged));
        m_sliceConnections.append(
            connect(tel, &RadioTelemetry::reversePowerChanged, this,
                    &TciServer::onReversePowerChanged));
        m_sliceConnections.append(
            connect(tel, &RadioTelemetry::swrChanged, this,
                    &TciServer::onSwrChanged));
    }
}

void TciServer::sendInitState(QWebSocket *client)
{
    const int trxCount = 1;
    const int rxCount = qMax(1, m_settings->getNumberOfReceivers());
    const bool receiveOnly = !m_settings->getTxAllowed();
    const int rx0 = rxSliceIdForTrx(0);

    // Identity: protocol "ExpertSDR3" sets WSJT-X's ESDR3 flag (command formats
    // + full TX audio gain). See WSJTX/wsjtx Transceiver/TCITransceiver.cpp.
    sendToClient(client, tciMessage(QStringLiteral("DEVICE"), {QStringLiteral("cudaSDR")}));
    sendToClient(client, tciMessage(QStringLiteral("PROTOCOL"),
                                    {QStringLiteral("ExpertSDR3"), QStringLiteral("1.5")}));
    sendToClient(client, tciMessage(QStringLiteral("RECEIVE_ONLY"),
                                    {receiveOnly ? QStringLiteral("true") : QStringLiteral("false")}));
    sendToClient(client, tciMessage(QStringLiteral("TRX_COUNT"), {QString::number(trxCount)}));
    // ExpertSDR / WSJT-X: two VFO channels (A=RX, B=TX route) per TRX.
    sendToClient(client, tciMessage(QStringLiteral("CHANNELS_COUNT"), {QStringLiteral("2")}));
    sendToClient(client, tciMessage(QStringLiteral("CHANNEL_COUNT"), {QString::number(rxCount)}));
    sendToClient(client, tciMessage(QStringLiteral("MODULATIONS_LIST"),
                                    {QStringLiteral("lsb,usb,am,fm,nfm,digu,digl,cw,cwl,sam,fdv")}));
    sendToClient(client, tciMessage(QStringLiteral("VFO_LIMITS"),
                                    {QString::number(kVfoMinHz), QString::number(kVfoMaxHz)}));
    sendToClient(client, tciMessage(QStringLiteral("IF_LIMITS"),
                                    {QStringLiteral("-48000"), QStringLiteral("48000")}));

    if (m_radioModel && rx0 >= 0 && rx0 < m_radioModel->slices().size()
        && m_radioModel->slices().at(rx0)) {
        sendToClient(client, formatVfo(0, 0, m_radioModel->slices().at(rx0)->vfoAFrequency()));
        sendToClient(client, formatVfo(0, 1, m_radioModel->slices().at(rx0)->vfoBFrequency()));
    } else {
        sendToClient(client, formatVfo(0, 0, m_settings->getVfoFrequency(rx0)));
        sendToClient(client, formatVfo(0, 1, vfoBFrequencyHz()));
    }
    sendToClient(client, formatActiveVfo(0, activeVfoChannel(0)));
    sendToClient(client, formatDds(0, 0, m_settings->getCtrFrequency(rx0)));
    sendToClient(client, formatIf(0, 0, ifOffsetHz(rx0)));
    sendToClient(client, formatModulation(0, m_settings->getDSPMode(rx0)));
    sendToClient(client, formatRxFilterBand(0,
                                            m_settings->getFilterLo(rx0),
                                            m_settings->getFilterHi(rx0)));
    sendToClient(client, formatRxSMeter(0, 0, smeterDbmForRx(rx0)));
    sendToClient(client, tciMessage(QStringLiteral("RX_ENABLE"),
                                    {QStringLiteral("0"), QStringLiteral("true")}));

    sendToClient(client, formatTrx(0, m_settings->getRadioState()));
    sendToClient(client, formatDrive(0, m_settings->getDriveLevel()));
    sendToClient(client, formatTune(0, m_settings->getRadioState() == RadioState::TUNE));
    sendToClient(client, tciMessage(QStringLiteral("SPLIT_ENABLE"),
                                    {QStringLiteral("0"),
                                     m_routingState.splitRequested() ? QStringLiteral("true")
                                                                     : QStringLiteral("false")}));
    sendToClient(client, tciMessage(QStringLiteral("RIT_ENABLE"),
                                    {QStringLiteral("0"), QStringLiteral("false")}));
    sendToClient(client, tciMessage(QStringLiteral("XIT_ENABLE"),
                                    {QStringLiteral("0"), QStringLiteral("false")}));
    sendToClient(client, tciMessage(QStringLiteral("MUTE"), {QStringLiteral("false")}));
    sendToClient(client, tciMessage(QStringLiteral("AUDIO_SAMPLERATE"), {QStringLiteral("48000")}));
    sendToClient(client, tciMessage(QStringLiteral("AUDIO_STREAM_SAMPLE_TYPE"), {QStringLiteral("float32")}));
    sendToClient(client, tciMessage(QStringLiteral("AUDIO_STREAM_CHANNELS"), {QStringLiteral("2")}));
    sendToClient(client, tciMessage(QStringLiteral("AUDIO_STREAM_SAMPLES"),
                                    {QString::number(kTxChronoSamples)}));
    sendToClient(client, tciMessage(QStringLiteral("IQ_SAMPLERATE"),
                                    {QString::number(nativeIqSampleRate())}));
    sendToClient(client, tciMessage(QStringLiteral("TX_PROFILES_EX"),
                                    {QStringLiteral("Default")}));
    sendToClient(client, tciMessage(QStringLiteral("READY")));

    if (m_settings->getDataEngineState() == QSDR::DataEngineUp)
        sendToClient(client, tciMessage(QStringLiteral("START")));
    else
        sendToClient(client, tciMessage(QStringLiteral("STOP")));
}

TciServer::TciClientState *TciServer::clientState(QWebSocket *client)
{
    if (!client)
        return nullptr;
    return &m_clientStates[client];
}

const TciServer::TciClientState *TciServer::clientState(QWebSocket *client) const
{
    const auto it = m_clientStates.constFind(client);
    return (it == m_clientStates.cend()) ? nullptr : &(*it);
}

int TciServer::parseAudioFormat(const QString &value) const
{
    return TciProtocol::parseAudioFormat(value);
}

void TciServer::sendAudioPacket(QWebSocket *client, const TciClientState &state, int rx,
                                const float *stereoInterleaved, int stereoFloatCount)
{
    if (!client)
        return;

    const QByteArray frame = buildRxAudioFrame(rx, state.audioSampleRate, state.audioFormat,
                                               state.audioChannels, stereoInterleaved,
                                               stereoFloatCount);
    if (frame.isEmpty())
        return;

    client->sendBinaryMessage(frame);
}

void TciServer::onRxAudioSamples(int rx, QVector<float> stereoInterleaved, int sampleRate)
{
    Q_UNUSED(sampleRate)

    if (stereoInterleaved.isEmpty() || !m_settings || !m_settings->getTciServerEnabled())
        return;

    const float *samples = stereoInterleaved.constData();
    int sampleCount = stereoInterleaved.size();
    QVector<float> scaled;
    if (std::abs(m_rxGain - 1.0f) > 1e-6f) {
        scaled = stereoInterleaved;
        for (float &s : scaled)
            s *= m_rxGain;
        samples = scaled.constData();
        sampleCount = scaled.size();
    }

    // Send each DSP audio block straight to the socket, in its natural cadence
    // (~21 ms at 48k pan, ~5 ms at 192k) — exactly like the local soundcard
    // path. The client's own jitter buffer absorbs the block granularity. RX
    // audio is never dropped: it is small and any gap makes the client reset.
    for (QWebSocket *client : std::as_const(m_clients)) {
        if (client->state() != QAbstractSocket::ConnectedState)
            continue;

        TciClientState *state = clientState(client);
        if (!state || !state->audioEnabledReceivers.contains(rx))
            continue;

        sendAudioPacket(client, *state, rx, samples, sampleCount);
    }
}

void TciServer::onRxIqSamples(int rx, QVector<float> iqInterleaved, int sampleRate)
{
    if (iqInterleaved.isEmpty() || sampleRate <= 0 || !m_settings || !m_settings->getTciServerEnabled())
        return;

    for (QWebSocket *client : std::as_const(m_clients)) {
        if (client->state() != QAbstractSocket::ConnectedState)
            continue;

        TciClientState *state = clientState(client);
        if (!state || !state->iqEnabledReceivers.contains(rx))
            continue;

        // Advertise effective (legacy-doubled) rate; send raw DSP samples as-is.
        state->iqSampleRate = effectiveIqSampleRate(sampleRate);

        // Panadapter IQ is best-effort and droppable. If the socket already has
        // a write backlog the link is congested — skip this frame so IQ never
        // builds latency on the socket shared with RX audio (audio priority).
        if (client->bytesToWrite() > kIqBacklogDropBytes) {
            if ((++state->iqFramesDropped % 200) == 1)
                TCI_WARN << "IQ backpressure: dropping panadapter frame (link congested), total drops"
                         << state->iqFramesDropped;
            continue;
        }

        sendIqPacket(client, *state, rx, iqInterleaved.constData(), iqInterleaved.size());
    }
}

void TciServer::sendIqPacket(QWebSocket *client, const TciClientState &state, int rx,
                             const float *iqInterleaved, int iqFloatCount)
{
    if (!client)
        return;

    const QByteArray frame = buildIqFrame(rx, state.iqSampleRate, state.iqFormat, state.iqChannels,
                                          iqInterleaved, iqFloatCount);
    if (frame.isEmpty())
        return;

    client->sendBinaryMessage(frame);
}

void TciServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QWebSocket *client = m_server->nextPendingConnection();
        connect(client, &QWebSocket::textMessageReceived,
                this, &TciServer::onClientTextMessage);
        connect(client, &QWebSocket::binaryMessageReceived,
                this, &TciServer::onClientBinaryMessage);
        connect(client, &QWebSocket::disconnected,
                this, &TciServer::onClientDisconnected);

        const bool wasEmpty = m_clients.isEmpty();
        m_clients.append(client);
        m_watchdog->stop();

        TCI_DEBUG << "Client connected from" << client->peerAddress().toString();
        sendInitState(client);

        if (wasEmpty) {
            emit remoteControlChanged(true);
        }
        emit connectionStatusChanged();
    }
}

void TciServer::onClientTextMessage(const QString &message)
{
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    if (!client)
        return;

    TCI_TRACE << "RX <-" << client->peerAddress().toString() << message;

    const QStringList commands = message.split(';', Qt::SkipEmptyParts);
    for (const QString &rawCommand : commands)
        handleCommand(client, rawCommand.trimmed());
}

void TciServer::onClientBinaryMessage(const QByteArray &message)
{
    if (!m_settings || !m_settings->getTciServerEnabled())
        return;

    StreamHeader hdr;
    if (!parseStreamHeader(message, hdr))
        return;

    // We only accept the client→server TX (mic) audio stream here. Any other
    // binary type is silently ignored so unrelated frames never disturb TX.
    if (hdr.streamType != kTxAudioStreamType)
        return;

    // Network mic audio is only fed to the transmitter while the radio is
    // actually in a TX state. Outside TX we drop it (and reset the accumulator)
    // so nothing is buffered up to leak into a later transmission.
    const RadioState state = m_settings->getRadioState();
    if (!acceptsTxAudio(state)) {
        m_txAudioResidual.clear();
        return;
    }
    if (!m_txAudioQueue)
        return;

    const QByteArray payload = message.mid(kStreamHeaderBytes);
    const int payloadBytes = payload.size();
    if (payloadBytes <= 0)
        return;

    // Decode float32 TX audio. ExpertSDR-family clients send stereo TX (mono
    // sources as L=R). WSJT-X oversizes the payload (length*8 bytes) and only
    // fills the first `length` floats — geometry in resolveTxAudioFloat32Layout,
    // not L≈R sample matching.
    int channels = 1;
    int floatsUsed = 0;
    QByteArray decodePayload = payload;
    if (hdr.format == kStreamFormatFloat32) {
        const int availFloats = payloadBytes / static_cast<int>(sizeof(float));
        if (availFloats <= 0)
            return;

        const float *samples = reinterpret_cast<const float *>(payload.constData());
        const TxAudioFloatLayout layout = resolveTxAudioFloat32Layout(
            samples, availFloats, static_cast<int>(hdr.length), static_cast<int>(hdr.channels));
        if (!layout.ok || layout.useFloats <= 0)
            return;

        channels = layout.channels;
        floatsUsed = layout.useFloats;
        decodePayload = payload.left(layout.useFloats * static_cast<int>(sizeof(float)));
    } else if (hdr.channels >= 2) {
        channels = 2;
    }

    QVector<double> decoded;
    if (!decodeTxAudioMonoSamples(decodePayload, static_cast<int>(hdr.format), channels, decoded))
        return;

    // Integer-factor resample into the 48 kHz TX mic clock when the client
    // declares a different rate (only the newly decoded samples).
    const int srcRate = (hdr.sampleRate == 8000 || hdr.sampleRate == 12000
                         || hdr.sampleRate == 24000 || hdr.sampleRate == 48000)
                            ? static_cast<int>(hdr.sampleRate)
                            : 48000;
    if (srcRate != 48000 && !decoded.isEmpty()) {
        QVector<double> resampled;
        if (srcRate < 48000 && (48000 % srcRate) == 0) {
            const int factor = 48000 / srcRate;
            resampled.reserve(decoded.size() * factor);
            for (double s : std::as_const(decoded)) {
                for (int k = 0; k < factor; ++k)
                    resampled.append(s);
            }
        } else if (srcRate > 48000 && (srcRate % 48000) == 0) {
            const int factor = srcRate / 48000;
            resampled.reserve(decoded.size() / factor + 1);
            for (int i = 0; i + factor - 1 < decoded.size(); i += factor)
                resampled.append(decoded.at(i));
        }
        if (!resampled.isEmpty())
            decoded = std::move(resampled);
    }

    if (std::abs(m_txGain - 1.0f) > 1e-6f) {
        for (double &s : decoded)
            s *= static_cast<double>(m_txGain);
    }

    m_txAudioResidual.append(decoded);

    // Enqueue exactly DSP_SAMPLE_SIZE mono blocks — the same granularity the
    // local soundcard mic uses (fetch_MicData truncates to DSP_SAMPLE_SIZE).
    static quint64 txAudioBlockCounter = 0;
    static quint64 txAudioDropWarnCounter = 0;
    const int queueBefore = m_txAudioQueue->count();
    const TxAudioChunkResult chunked = chunkTxAudioResidual(
        m_txAudioResidual,
        DSP_SAMPLE_SIZE,
        kTxAudioMaxQueueBlocks,
        queueBefore);
    m_txAudioResidual = chunked.residual;

    for (const QVector<double> &block : chunked.blocks)
        m_txAudioQueue->enqueue(block);

    if (!chunked.blocks.isEmpty() && (++txAudioBlockCounter % 50) == 1) {
        TCI_DEBUG << "TX audio from client: enqueued" << chunked.blocks.size()
                  << "block(s), queue=" << m_txAudioQueue->count()
                  << "ch=" << channels << "fmt=" << hdr.format
                  << "rate=" << hdr.sampleRate
                  << "len=" << hdr.length << "payload=" << payloadBytes
                  << "floatsUsed=" << floatsUsed
                  << "monoOut=" << decoded.size();
    }

    if (chunked.droppedBlocks > 0) {
        // Client is producing faster than the TX path drains — drop to keep
        // TX latency bounded. Rate-limit the WARN; sustained backlog is noisy.
        if ((++txAudioDropWarnCounter % 200) == 1) {
            TCI_WARN << "TX audio backlog: dropping" << chunked.droppedBlocks
                     << "client mic block(s) (TX drain slower than input)";
        }
    }
}

void TciServer::onClientDisconnected()
{
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    if (!client)
        return;

    TCI_DEBUG << "Client disconnected";
    if (client == m_txChronoClient)
        stopTxChrono();

    m_clients.removeAll(client);
    m_clientStates.remove(client);
    client->deleteLater();
    updateIqActiveHint();

    if (m_clients.isEmpty()) {
        emit remoteControlChanged(false);
        // Last client gone: drop TCI-created TX designation but keep VFO-B memory.
        if (m_routingState.ownsRoute())
            m_routingState.clearTciRoute();
        m_routingState.setSplitRequested(false);
        syncTxSliceForSplit();
        if (m_settings->getRadioState() != RadioState::RX) {
            TCI_WARN << "Client disconnected while TX active — watchdog started (30s)";
            m_watchdog->start();
        }
    }
    emit connectionStatusChanged();
}

void TciServer::onWatchdogTimeout()
{
    TCI_WARN << "Watchdog timeout: client disconnected while TX, forcing RX";
    m_settings->setRadioState(RadioState::RX);
}

void TciServer::handleCommand(QWebSocket *client, const QString &commandLine)
{
    if (commandLine.isEmpty())
        return;

    const QString reply = m_commandHandler.handleCommand(commandLine);
    if (!reply.isEmpty())
        sendToClient(client, reply);

    applyPendingHandlerEffects(client, m_commandHandler);

    if (m_commandHandler.lastCommandNeedsServer())
        handleServerCommand(client, m_commandHandler.lastCommandName(),
                            m_commandHandler.lastCommandArgs());
}

void TciServer::applyPendingHandlerEffects(QWebSocket *client, TciCommandHandler &handler)
{
    const QString note = handler.pendingNotification();
    if (!note.isEmpty())
        broadcast(note);

    if (const auto req = handler.takeVfoRequest())
        handleVfoRequest(client, *req);
    if (const auto req = handler.takeSplitRequest())
        handleSplitRequest(client, *req);
    if (const auto req = handler.takeTrxRequest())
        handleTrxRequest(client, *req);

    if (const auto req = handler.takeTuneRequest()) {
        m_settings->setRadioState(req->enabled ? RadioState::TUNE : RadioState::RX);
    }
    if (const auto req = handler.takeDriveRequest()) {
        if (req->isSet)
            m_settings->setDriveLevel(req->level);
    }
    if (const auto req = handler.takeModulationRequest()) {
        const int rx = rxSliceIdForTrx(req->trx);
        m_settings->setDSPMode(rx, req->mode);
    }
    if (const auto req = handler.takeFilterRequest()) {
        const int rx = rxSliceIdForTrx(req->trx);
        m_settings->setRXFilter(rx, req->lo, req->hi);
    }
    if (const auto req = handler.takeDdsRequest()) {
        const int rx = rxSliceIdForTrx(req->trx);
        const qint64 vfo = m_settings->getVfoFrequency(rx);
        m_settings->setCtrFrequency(0, rx, req->frequencyHz);
        m_settings->setNCOFrequency(true, rx, vfo - req->frequencyHz);
    }
    if (const auto req = handler.takeIfRequest()) {
        const int rx = rxSliceIdForTrx(req->trx);
        m_settings->setVFOFrequency(0, rx, m_settings->getCtrFrequency(rx) + req->offsetHz);
    }
    if (const auto req = handler.takeStartStopRequest()) {
        if (req->start) {
            if (m_settings->getDataEngineState() == QSDR::DataEngineUp)
                sendToClient(client, tciMessage(QStringLiteral("START")));
            else
                emit startRequested();
        } else {
            if (m_settings->getDataEngineState() != QSDR::DataEngineDown)
                emit stopRequested();
            sendToClient(client, tciMessage(QStringLiteral("STOP")));
        }
    }
}

QVector<TciSliceEndpoint> TciServer::routingEndpoints() const
{
    QVector<TciSliceEndpoint> out;
    if (!m_radioModel)
        return out;

    const QList<SliceModel*> slices = m_radioModel->slices();
    const int active = qBound(1, m_radioModel->activeReceivers(), slices.size());
    const int designatedTx = m_radioModel->txSliceIndex();

    for (int i = 0; i < active; ++i)
        out.push_back({i, designatedTx >= 0 && i == designatedTx});

    // Include a TCI-bound VFO-B slice even when it is outside the active DDC set.
    const int routedTx = m_routingState.txSliceId();
    if (routedTx >= 0 && routedTx < slices.size()) {
        bool found = false;
        for (TciSliceEndpoint &ep : out) {
            if (ep.sliceId == routedTx) {
                ep.isTx = (designatedTx == routedTx) || m_routingState.splitRequested();
                found = true;
                break;
            }
        }
        if (!found) {
            out.push_back({routedTx,
                           designatedTx == routedTx || m_routingState.splitRequested()});
        }
    }
    return out;
}

int TciServer::rxSliceIdForTrx(int trx) const
{
    Q_UNUSED(trx)
    // Phase-1: single TCI TRX maps to the current / primary receiver.
    if (m_settings)
        return qBound(0, m_settings->getCurrentReceiver(),
                       qMax(0, m_settings->getNumberOfReceivers() - 1));
    return 0;
}

int TciServer::allocateVfoBSlice(int rxSliceId) const
{
    if (!m_radioModel)
        return -1;
    const int n = m_radioModel->slices().size();
    // Prefer a spare pool slice (typically index 1) so VFO-B does not steal an
    // operator's independent active receiver.
    for (int i = 0; i < n; ++i) {
        if (i == rxSliceId)
            continue;
        if (i >= m_radioModel->activeReceivers())
            return i;
    }
    for (int i = 0; i < n; ++i) {
        if (i != rxSliceId)
            return i;
    }
    return -1;
}

void TciServer::syncTxSliceForSplit()
{
    if (!m_radioModel)
        return;
    if (m_routingState.splitRequested() && m_routingState.txSliceId() >= 0)
        m_radioModel->setTxSliceIndex(m_routingState.txSliceId());
    else
        m_radioModel->setTxSliceIndex(-1);
    applyEffectiveTxFrequency();
}

void TciServer::tuneRxVfo(int rx, qint64 frequencyHz)
{
    // WSJT-X / ExpertSDR CAT-style VFO sets dial frequency and moves LO with it.
    m_settings->setCtrFrequency(1, rx, frequencyHz);
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size()
        && m_radioModel->slices().at(rx)) {
        m_radioModel->slices().at(rx)->setFrequency(frequencyHz);
        m_radioModel->slices().at(rx)->setCenterFrequency(frequencyHz);
    }
    applyEffectiveTxFrequency();
}

void TciServer::tuneTxVfo(int txSliceId, qint64 frequencyHz)
{
    if (!m_radioModel || txSliceId < 0 || txSliceId >= m_radioModel->slices().size())
        return;
    SliceModel *slice = m_radioModel->slices().at(txSliceId);
    if (!slice)
        return;
    slice->setFrequency(frequencyHz);
    // Keep Settings VFO list in sync when the TX slice is also an active RX.
    if (txSliceId < m_settings->getNumberOfReceivers())
        m_settings->setVFOFrequency(0, txSliceId, frequencyHz);
    applyEffectiveTxFrequency();
    broadcast(formatVfo(0, 1, frequencyHz));
}

qint64 TciServer::vfoAFrequencyHz(int trx) const
{
    if (const SliceModel *slice = rxSliceForTrx(trx))
        return slice->vfoAFrequency();
    return m_settings->getVfoFrequency(rxSliceIdForTrx(trx));
}

qint64 TciServer::vfoBFrequencyHz() const
{
    const int rx = rxSliceIdForTrx(0);
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size()
        && m_radioModel->slices().at(rx)) {
        return m_radioModel->slices().at(rx)->vfoBFrequency();
    }
    if (m_radioModel && m_routingState.txSliceId() >= 0
        && m_routingState.txSliceId() < m_radioModel->slices().size()
        && m_radioModel->slices().at(m_routingState.txSliceId())) {
        return m_radioModel->slices().at(m_routingState.txSliceId())->frequency();
    }
    return m_settings->getVfoFrequency(rx);
}

SliceModel *TciServer::rxSliceForTrx(int trx) const
{
    if (!m_radioModel)
        return nullptr;
    const int rx = rxSliceIdForTrx(trx);
    if (rx < 0 || rx >= m_radioModel->slices().size())
        return nullptr;
    return m_radioModel->slices().at(rx);
}

int TciServer::activeVfoChannel(int trx) const
{
    const SliceModel *slice = rxSliceForTrx(trx);
    return (slice && slice->activeVfo() == SliceModel::VfoB) ? 1 : 0;
}

void TciServer::applyActiveVfo(int trx, int channel)
{
    SliceModel *slice = rxSliceForTrx(trx);
    if (!slice)
        return;

    const SliceModel::ActiveVfo target = (channel == 1) ? SliceModel::VfoB : SliceModel::VfoA;
    if (slice->activeVfo() == target)
        return;

    slice->setActiveVfo(target);
    // Move the hardware with the dial; recentres only if the memory is off-span.
    m_settings->setVfoFrequencyVisible(slice->id(), slice->frequency());
    applyEffectiveTxFrequency();
}

void TciServer::onActiveVfoChanged(int rx)
{
    if (rx != rxSliceIdForTrx(0))
        return;
    broadcast(formatActiveVfo(0, activeVfoChannel(0)));
}

void TciServer::applyEffectiveTxFrequency()
{
    if (!m_radioModel)
        return;
    const qint64 hz = m_radioModel->effectiveTxFrequency();
    if (hz > 0)
        m_radioModel->txParams().txFrequency = hz;
}

void TciServer::handleVfoRequest(QWebSocket *client, const TciCommandHandler::VfoRequest &request)
{
    Q_UNUSED(client)
    const int rxSliceId = rxSliceIdForTrx(request.trx);
    SliceModel *rxSlice = nullptr;
    if (m_radioModel && rxSliceId >= 0 && rxSliceId < m_radioModel->slices().size())
        rxSlice = m_radioModel->slices().at(rxSliceId);

    if (request.channel == 0) {
        // VFO-A lives on the same RX slice; make A active so dial write-through stays consistent.
        if (rxSlice) {
            if (rxSlice->activeVfo() != SliceModel::VfoA)
                rxSlice->setActiveVfo(SliceModel::VfoA);
            rxSlice->setVfoAFrequency(request.frequencyHz);
        }
        tuneRxVfo(rxSliceId, request.frequencyHz);
        return;
    }

    // VFO-B memory on the same RX slice (operator A/B store).
    if (rxSlice) {
        rxSlice->setVfoBFrequency(request.frequencyHz);
        // B is the live dial: the slice write alone moves the display, not the DDC.
        if (rxSlice->activeVfo() == SliceModel::VfoB)
            m_settings->setVfoFrequencyVisible(rxSlice->id(), request.frequencyHz);
    }

    // Digi split path (unchanged): resolve / create a TX route spare slice, then tune it.
    const TciRoutingState::RouteDecision route =
        m_routingState.resolveVfoB(rxSliceId, routingEndpoints());

    int txSliceId = route.txSliceId;
    if (route.action == TciRoutingState::RouteAction::Create
        || route.action == TciRoutingState::RouteAction::Unavailable) {
        txSliceId = allocateVfoBSlice(rxSliceId);
        if (txSliceId < 0) {
            TCI_WARN << "VFO-B unavailable: no spare slice for trx" << request.trx;
            broadcast(formatVfo(0, 1, vfoBFrequencyHz()));
            return;
        }
        // Seed from RX dial so an untouched VFO-B is not 0 Hz.
        if (m_radioModel && m_radioModel->slices().at(txSliceId)
            && m_radioModel->slices().at(txSliceId)->frequency() <= 0) {
            qint64 seed = m_settings->getVfoFrequency(rxSliceId);
            if (rxSlice)
                seed = rxSlice->vfoBFrequency();
            m_radioModel->slices().at(txSliceId)->setFrequency(seed);
        }
        m_routingState.bindCreatedRoute(rxSliceId, txSliceId);
    } else if (route.action == TciRoutingState::RouteAction::PromoteExisting
               || route.action == TciRoutingState::RouteAction::UseExisting) {
        txSliceId = route.txSliceId;
        if (route.action == TciRoutingState::RouteAction::PromoteExisting)
            m_routingState.bindCreatedRoute(rxSliceId, txSliceId);
    }

    if (txSliceId < 0) {
        broadcast(formatVfo(0, 1, vfoBFrequencyHz()));
        return;
    }

    syncTxSliceForSplit();
    tuneTxVfo(txSliceId, request.frequencyHz);
}

void TciServer::handleSplitRequest(QWebSocket *client, const TciCommandHandler::SplitRequest &request)
{
    Q_UNUSED(client)
    const bool wasSplit = m_routingState.splitRequested();
    const bool changed = m_routingState.setSplitRequested(request.enabled);
    const QString confirmation = tciMessage(QStringLiteral("SPLIT_ENABLE"),
                                            {QString::number(request.trx),
                                             request.enabled ? QStringLiteral("true")
                                                             : QStringLiteral("false")});

    if (request.enabled) {
        const int rxSliceId = rxSliceIdForTrx(request.trx);
        const TciRoutingState::RouteDecision route =
            m_routingState.resolveVfoB(rxSliceId, routingEndpoints());
        if (route.action == TciRoutingState::RouteAction::Create
            || route.action == TciRoutingState::RouteAction::Unavailable) {
            const int txSliceId = allocateVfoBSlice(rxSliceId);
            if (txSliceId < 0) {
                m_routingState.setSplitRequested(false);
                broadcast(tciMessage(QStringLiteral("SPLIT_ENABLE"),
                                     {QString::number(request.trx), QStringLiteral("false")}));
                return;
            }
            if (m_radioModel && m_radioModel->slices().at(txSliceId)
                && m_radioModel->slices().at(txSliceId)->frequency() <= 0) {
                qint64 seed = m_settings->getVfoFrequency(rxSliceId);
                if (rxSliceId >= 0 && rxSliceId < m_radioModel->slices().size()
                    && m_radioModel->slices().at(rxSliceId))
                    seed = m_radioModel->slices().at(rxSliceId)->vfoBFrequency();
                m_radioModel->slices().at(txSliceId)->setFrequency(seed);
            }
            m_routingState.bindCreatedRoute(rxSliceId, txSliceId);
        } else if (route.action == TciRoutingState::RouteAction::PromoteExisting) {
            m_routingState.bindCreatedRoute(rxSliceId, route.txSliceId);
        }
        syncTxSliceForSplit();
        broadcast(confirmation);
        broadcast(formatVfo(0, 1, vfoBFrequencyHz()));
        return;
    }

    // WSJT-X sends a steady split_enable:false before programming VFO-B — do
    // not discard the route on a no-op. Only reclaim on a true→false edge.
    if (changed && wasSplit && m_routingState.ownsRoute()) {
        // Keep the spare-slice frequency (VFO-B memory); just stop using it for TX.
        m_routingState.clearTciRoute();
    }
    syncTxSliceForSplit();
    broadcast(confirmation);
}

void TciServer::handleTrxRequest(QWebSocket *client, const TciCommandHandler::TrxRequest &request)
{
    if (request.transmitting)
        m_watchdog->stop();

    const int rxSliceId = rxSliceIdForTrx(request.trx);
    const int pttSlice = m_routingState.resolvePttSlice(rxSliceId, routingEndpoints());
    if (request.transmitting && m_routingState.splitRequested() && pttSlice >= 0
        && m_radioModel) {
        m_radioModel->setTxSliceIndex(pttSlice);
        applyEffectiveTxFrequency();
    } else if (!request.transmitting) {
        syncTxSliceForSplit();
    } else {
        applyEffectiveTxFrequency();
    }

    m_settings->setRadioState(request.transmitting ? RadioState::MOX : RadioState::RX);
    if (request.transmitting)
        startTxChrono(client, request.trx);
    else if (client == m_txChronoClient)
        stopTxChrono();
}

void TciServer::handleServerCommand(QWebSocket *client, const QString &name, const QStringList &args)
{
    // Remaining verbs that need per-client stream state or live Settings reads.
    int trx = 0;
    int channel = 0;
    if (!args.isEmpty()) {
        bool ok = false;
        trx = args.at(0).toInt(&ok);
        if (!ok)
            trx = 0;
    }
    if (args.size() >= 2) {
        bool ok = false;
        const int ch = args.at(1).toInt(&ok);
        if (ok)
            channel = ch;
    }

    // IQ / audio subscriptions: ExpertSDR uses trx as the receiver index.
    const int maxRx = qMax(0, m_settings->getNumberOfReceivers() - 1);
    const int rx = qBound(0, trx, maxRx);

    if (name == QLatin1String("vfo")) {
        if (channel == 1)
            sendToClient(client, formatVfo(trx, 1, vfoBFrequencyHz()));
        else
            sendToClient(client, formatVfo(trx, 0, vfoAFrequencyHz(trx)));
        return;
    }
    if (name == QLatin1String("active_vfo")) {
        if (args.size() >= 2)
            applyActiveVfo(trx, channel);
        else
            sendToClient(client, formatActiveVfo(trx, activeVfoChannel(trx)));
        return;
    }
    if (name == QLatin1String("dds")) {
        sendToClient(client, formatDds(trx, channel, m_settings->getCtrFrequency(rxSliceIdForTrx(trx))));
        return;
    }
    if (name == QLatin1String("if")) {
        sendToClient(client, formatIf(trx, channel, ifOffsetHz(rxSliceIdForTrx(trx))));
        return;
    }
    if (name == QLatin1String("modulation")) {
        sendToClient(client, formatModulation(trx, m_settings->getDSPMode(rxSliceIdForTrx(trx))));
        return;
    }
    if (name == QLatin1String("rx_filter_band")) {
        const int r = rxSliceIdForTrx(trx);
        sendToClient(client, formatRxFilterBand(trx, m_settings->getFilterLo(r), m_settings->getFilterHi(r)));
        return;
    }
    if (name == QLatin1String("trx") || name == QLatin1String("tx_enable")) {
        sendToClient(client, formatTrx(trx, m_settings->getRadioState()));
        return;
    }
    if (name == QLatin1String("tune")) {
        sendToClient(client, formatTune(trx, m_settings->getRadioState() == RadioState::TUNE));
        return;
    }
    if (name == QLatin1String("drive") || name == QLatin1String("tune_drive")) {
        sendToClient(client, tciMessage(name,
                                        {QString::number(trx),
                                         QString::number(m_settings->getDriveLevel())}));
        return;
    }

    if (name == QLatin1String("iq_start")) {
        static const bool iqDisabled = qEnvironmentVariableIntValue("CUSDR_TCI_NO_IQ") != 0;
        if (iqDisabled) {
            TCI_DEBUG << "IQ_START ignored (CUSDR_TCI_NO_IQ set)";
            sendToClient(client, tciMessage(QStringLiteral("IQ_STOP"), {QString::number(rx)}));
            return;
        }
        TciClientState *state = clientState(client);
        if (!state)
            return;
        state->iqEnabledReceivers.insert(rx);
        updateIqActiveHint();
        sendToClient(client, tciMessage(QStringLiteral("IQ_SAMPLERATE"),
                                        {QString::number(nativeIqSampleRate())}));
        return;
    }

    if (name == QLatin1String("iq_stop")) {
        if (TciClientState *state = clientState(client))
            state->iqEnabledReceivers.remove(rx);
        updateIqActiveHint();
        return;
    }

    if (name == QLatin1String("iq_samplerate")) {
        sendToClient(client, tciMessage(QStringLiteral("IQ_SAMPLERATE"),
                                        {QString::number(nativeIqSampleRate())}));
        return;
    }

    if (name == QLatin1String("iq_stream_sample_type")) {
        const QString fmtArg = (args.size() >= 2) ? args.at(1) : (args.isEmpty() ? QString() : args.at(0));
        if (!fmtArg.isEmpty() && clientState(client))
            clientState(client)->iqFormat = parseAudioFormat(fmtArg);
        return;
    }

    if (name == QLatin1String("iq_stream_channels")) {
        const QString chArg = (args.size() >= 2) ? args.at(1) : (args.isEmpty() ? QString() : args.at(0));
        if (!chArg.isEmpty()) {
            bool ok = false;
            const int channels = chArg.toInt(&ok);
            if (ok && clientState(client))
                clientState(client)->iqChannels = (channels == 1) ? 1 : 2;
        }
        return;
    }

    if (name == QLatin1String("iq_stream_samples")) {
        const QString samplesArg = (args.size() >= 2) ? args.at(1) : (args.isEmpty() ? QString() : args.at(0));
        if (!samplesArg.isEmpty()) {
            bool ok = false;
            const int samples = samplesArg.toInt(&ok);
            if (ok && clientState(client) && samples > 0)
                clientState(client)->iqSamplesPerPacket = samples;
        }
        return;
    }

    if (name == QLatin1String("sql_enable")
        || name == QLatin1String("rx_anf_enable")
        || name == QLatin1String("mon_enable")
        || name == QLatin1String("rx_nb_enable")
        || name == QLatin1String("rx_nb2_enable")
        || name == QLatin1String("rx_bin_enable")
        || name == QLatin1String("agc_auto_ex")) {
        int enableTrx = 0;
        bool enableValue = false;
        bool hasValue = false;
        parseTrxEnableArgs(args, enableTrx, enableValue, hasValue);
        const bool defaultOn = (name == QLatin1String("agc_auto_ex"));
        if (hasValue) {
            broadcast(tciMessage(name,
                                 {QString::number(enableTrx),
                                  enableValue ? QStringLiteral("true") : QStringLiteral("false")}));
            return;
        }
        sendToClient(client,
                     tciMessage(name,
                                {QString::number(enableTrx),
                                 defaultOn ? QStringLiteral("true") : QStringLiteral("false")}));
        return;
    }

    if (name == QLatin1String("sql_level") || name == QLatin1String("rx_volume")) {
        if (args.size() >= 2) {
            bool ok = false;
            const int level = args.at(1).toInt(&ok);
            if (!ok)
                return;
            broadcast(tciMessage(name, {QString::number(trx), QString::number(level)}));
            return;
        }
        sendToClient(client, tciMessage(name, {QString::number(trx), QStringLiteral("0")}));
        return;
    }

    if (name == QLatin1String("tx_profiles_ex")) {
        sendToClient(client, tciMessage(QStringLiteral("TX_PROFILES_EX"), {QStringLiteral("Default")}));
        return;
    }
    if (name == QLatin1String("tx_profile_ex")) {
        if (!args.isEmpty() && !args.at(0).isEmpty()) {
            broadcast(tciMessage(QStringLiteral("TX_PROFILE_EX"), {args.at(0)}));
            return;
        }
        sendToClient(client, tciMessage(QStringLiteral("TX_PROFILE_EX"), {QStringLiteral("Default")}));
        return;
    }
    if (name == QLatin1String("tx_stream_audio_buffering")) {
        if (!args.isEmpty()) {
            bool ok = false;
            const int ms = args.at(0).toInt(&ok);
            if (ok)
                broadcast(tciMessage(QStringLiteral("TX_STREAM_AUDIO_BUFFERING"),
                                     {QString::number(ms)}));
            return;
        }
        sendToClient(client, tciMessage(QStringLiteral("TX_STREAM_AUDIO_BUFFERING"),
                                        {QStringLiteral("50")}));
        return;
    }

    if (name == QLatin1String("rx_smeter")) {
        sendToClient(client, formatRxSMeter(trx, channel, smeterDbmForRx(rx)));
        return;
    }

    if (name == QLatin1String("rx_sensors_enable"))
        return;

    if (name == QLatin1String("tx_sensors_enable")) {
        TciClientState *state = clientState(client);
        if (!state || args.isEmpty())
            return;

        // Forms: tx_sensors_enable:true[,interval_ms]
        //        tx_sensors_enable:0,true[,interval_ms]
        bool enable = false;
        int intervalMs = state->txSensorsIntervalMs;
        if (args.size() == 1) {
            enable = parseBoolArg(args.at(0));
        } else {
            bool firstIsTrx = false;
            const int asInt = args.at(0).toInt(&firstIsTrx);
            Q_UNUSED(asInt)
            if (firstIsTrx && args.at(0).trimmed() == QString::number(asInt)
                && args.size() >= 2) {
                enable = parseBoolArg(args.at(1));
                if (args.size() >= 3) {
                    bool ok = false;
                    const int ms = args.at(2).toInt(&ok);
                    if (ok)
                        intervalMs = ms;
                }
            } else {
                enable = parseBoolArg(args.at(0));
                bool ok = false;
                const int ms = args.at(1).toInt(&ok);
                if (ok)
                    intervalMs = ms;
            }
        }
        state->txSensorsEnabled = enable;
        state->txSensorsIntervalMs = qBound(30, intervalMs, 1000);
        state->txSensorsLastSendMs = 0;
        if (enable && isTrxActive(m_settings->getRadioState()))
            maybeBroadcastTxSensors(true);
        return;
    }

    if (name == QLatin1String("tx_sensors")) {
        sendToClient(client, formatTxSensors(trx));
        return;
    }
    if (name == QLatin1String("tx_power")) {
        sendToClient(client, formatTxPower(trx, m_fwdPowerWatts));
        return;
    }
    if (name == QLatin1String("tx_swr") || name == QLatin1String("swr")) {
        sendToClient(client, formatTxSwr(trx, effectiveSwr()));
        return;
    }

    if (name == QLatin1String("audio_start") || name == QLatin1String("line_out_start")) {
        TciClientState *state = clientState(client);
        if (!state)
            return;
        state->audioEnabledReceivers.insert(rx);
        // WSJT-X sets stream_audio_ only when it receives audio_start:<rx>;
        sendToClient(client, tciMessage(QStringLiteral("AUDIO_START"), {QString::number(rx)}));
        sendToClient(client, tciMessage(QStringLiteral("AUDIO_SAMPLERATE"),
                                        {QString::number(state->audioSampleRate)}));
        return;
    }
    if (name == QLatin1String("audio_stop") || name == QLatin1String("line_out_stop")) {
        if (TciClientState *state = clientState(client))
            state->audioEnabledReceivers.remove(rx);
        sendToClient(client, tciMessage(QStringLiteral("AUDIO_STOP"), {QString::number(rx)}));
        return;
    }
    if (name == QLatin1String("audio_samplerate")) {
        if (!args.isEmpty() && clientState(client)) {
            bool ok = false;
            const int rate = args.at(0).toInt(&ok);
            if (ok && rate > 0)
                clientState(client)->audioSampleRate = rate;
            return;
        }
        sendToClient(client, tciMessage(QStringLiteral("AUDIO_SAMPLERATE"), {QStringLiteral("48000")}));
        return;
    }
    if (name == QLatin1String("audio_stream_sample_type")) {
        const QString fmtArg = (args.size() >= 2) ? args.at(1) : (args.isEmpty() ? QString() : args.at(0));
        if (!fmtArg.isEmpty() && clientState(client))
            clientState(client)->audioFormat = parseAudioFormat(fmtArg);
        return;
    }
    if (name == QLatin1String("audio_stream_channels")) {
        const QString chArg = (args.size() >= 2) ? args.at(1) : (args.isEmpty() ? QString() : args.at(0));
        if (!chArg.isEmpty()) {
            bool ok = false;
            const int channels = chArg.toInt(&ok);
            if (ok && clientState(client))
                clientState(client)->audioChannels = (channels == 1) ? 1 : 2;
        }
        return;
    }
    if (name == QLatin1String("audio_stream_samples")) {
        const QString samplesArg = (args.size() >= 2) ? args.at(1) : (args.isEmpty() ? QString() : args.at(0));
        if (!samplesArg.isEmpty()) {
            bool ok = false;
            const int samples = samplesArg.toInt(&ok);
            if (ok && clientState(client) && samples > 0)
                clientState(client)->audioSamplesPerPacket = samples;
        }
        return;
    }

    if (name == QLatin1String("qping") || name == QLatin1String("keepalive")) {
        if (name == QLatin1String("qping"))
            sendToClient(client, tciMessage(QStringLiteral("QPING"), args));
        return;
    }

    TCI_TRACE << "Unknown command:" << name << args;
}

void TciServer::onVfoFrequencyChanged(int mode, int rx, qint64 frequency)
{
    Q_UNUSED(mode)
    // Slice-bound setups broadcast the A/B memories from the slice signals in
    // bindSlices; reporting the dial here as well would label a VFO-B retune as
    // channel 0. This is the pre-MVC fallback.
    if (rxSliceForTrx(0))
        return;
    if (rx == rxSliceIdForTrx(0))
        broadcast(formatVfo(0, 0, frequency));
}

void TciServer::onCtrFrequencyChanged(int mode, int rx, qint64 frequency)
{
    Q_UNUSED(mode)
    broadcast(formatDds(0, rx, frequency));
}

void TciServer::onNcoFrequencyChanged(int rx, qint64 frequency)
{
    broadcast(formatIf(0, rx, frequency));
}

void TciServer::onDspModeChanged(int rx, DSPMode mode)
{
    Q_UNUSED(rx)
    broadcast(formatModulation(0, mode));
}

void TciServer::onFilterFrequenciesChanged(int rx, qreal low, qreal high)
{
    Q_UNUSED(rx)
    broadcast(formatRxFilterBand(0, low, high));
}

void TciServer::onRadioStateChanged(RadioState state)
{
    broadcast(formatTrx(0, state));
    broadcast(formatTune(0, state == RadioState::TUNE));
    if (state == RadioState::RX)
        stopTxChrono();
    else if (isTrxActive(state))
        maybeBroadcastTxSensors(true);
}

void TciServer::startTxChrono(QWebSocket *client, int trx)
{
    if (!client)
        return;
    if (m_txChronoClient && m_txChronoClient != client)
        stopTxChrono();

    m_txChronoClient = client;
    m_txChronoTrx = trx;
    m_txChronoAccumNs = 0;
    m_txChronoClock.start();
    if (!m_txChronoTimer->isActive())
        m_txChronoTimer->start();
    // Kick several frames immediately so the client has chrono credits before
    // its TX modulator starts (WSJT-X consumes one chrono → one TX audio reply).
    sendTxChronoFrame(client);
    sendTxChronoFrame(client);
    sendTxChronoFrame(client);
    TCI_DEBUG << "TX_CHRONO started trx=" << trx;
    emit connectionStatusChanged();
}

void TciServer::stopTxChrono()
{
    if (!m_txChronoTimer->isActive() && !m_txChronoClient)
        return;

    m_txChronoTimer->stop();
    m_txChronoClient = nullptr;
    m_txChronoAccumNs = 0;
    m_txChronoClock.invalidate();
    m_txAudioResidual.clear();
    TCI_DEBUG << "TX_CHRONO stopped";
    emit connectionStatusChanged();
}

void TciServer::sendTxChronoFrame(QWebSocket *client)
{
    if (!client || client->state() != QAbstractSocket::ConnectedState)
        return;
    client->sendBinaryMessage(buildTxChronoFrame(m_txChronoTrx, 48000, kTxChronoSamples));
}

void TciServer::onTxChronoTick()
{
    QWebSocket *client = m_txChronoClient;
    if (!client || client->state() != QAbstractSocket::ConnectedState) {
        stopTxChrono();
        return;
    }
    if (!m_txChronoClock.isValid())
        m_txChronoClock.start();

    m_txChronoAccumNs += m_txChronoClock.nsecsElapsed();
    m_txChronoClock.restart();
    while (m_txChronoAccumNs >= kTxChronoPeriodNs) {
        sendTxChronoFrame(client);
        m_txChronoAccumNs -= kTxChronoPeriodNs;
    }
}

void TciServer::onDriveLevelChanged(int level)
{
    broadcast(formatDrive(0, level));
}

void TciServer::onForwardPowerChanged(qreal watts)
{
    m_fwdPowerWatts = watts;
    scheduleTxSensorsBroadcast();
}

void TciServer::onReversePowerChanged(qreal watts)
{
    m_revPowerWatts = watts;
    if (!m_swrValid)
        scheduleTxSensorsBroadcast();
}

void TciServer::onSwrChanged(qreal swr)
{
    m_swr = swr;
    m_swrValid = true;
    scheduleTxSensorsBroadcast();
}

void TciServer::onSMeterValueChanged(int rx, double rawValue)
{
    broadcast(formatRxSMeter(0, rx, smeterDbmFromRaw(rawValue)));
}

void TciServer::onTciServerEnabledChanged(bool enabled)
{
    if (!enabled) {
        stopListening();
    }
}
