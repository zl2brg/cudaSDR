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
}

bool TciServer::isListening() const
{
    return m_server->isListening();
}

quint16 TciServer::port() const
{
    return m_server->serverPort();
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
    const int channelCount = qMax(1, m_settings->getNumberOfReceivers());
    const bool receiveOnly = !m_settings->getTxAllowed();

    // Identity: protocol "ExpertSDR3" sets WSJT-X's ESDR3 flag (command formats
    // + full TX audio gain). See WSJTX/wsjtx Transceiver/TCITransceiver.cpp.
    sendToClient(client, tciMessage(QStringLiteral("DEVICE"), {QStringLiteral("cudaSDR")}));
    sendToClient(client, tciMessage(QStringLiteral("PROTOCOL"),
                                    {QStringLiteral("ExpertSDR3"), QStringLiteral("1.5")}));
    sendToClient(client, tciMessage(QStringLiteral("RECEIVE_ONLY"),
                                    {receiveOnly ? QStringLiteral("true") : QStringLiteral("false")}));
    sendToClient(client, tciMessage(QStringLiteral("TRX_COUNT"), {QString::number(trxCount)}));
    // Plural form is what WSJT-X / eesdr-tci recognise; keep singular for PDF-spec clients.
    sendToClient(client, tciMessage(QStringLiteral("CHANNELS_COUNT"), {QString::number(1)}));
    sendToClient(client, tciMessage(QStringLiteral("CHANNEL_COUNT"), {QString::number(channelCount)}));
    sendToClient(client, tciMessage(QStringLiteral("MODULATIONS_LIST"),
                                    {QStringLiteral("lsb,usb,am,fm,nfm,digu,digl,cw,cwl,sam,fdv")}));
    sendToClient(client, tciMessage(QStringLiteral("VFO_LIMITS"),
                                    {QString::number(kVfoMinHz), QString::number(kVfoMaxHz)}));
    sendToClient(client, tciMessage(QStringLiteral("IF_LIMITS"),
                                    {QStringLiteral("-48000"), QStringLiteral("48000")}));

    for (int rx = 0; rx < channelCount; ++rx) {
        sendToClient(client, formatVfo(0, rx, m_settings->getVfoFrequency(rx)));
        sendToClient(client, formatDds(0, rx, m_settings->getCtrFrequency(rx)));
        sendToClient(client, formatIf(0, rx, ifOffsetHz(rx)));
        sendToClient(client, formatModulation(0, m_settings->getDSPMode(rx)));
        sendToClient(client, formatRxFilterBand(0,
                                                m_settings->getFilterLo(rx),
                                                m_settings->getFilterHi(rx)));
        sendToClient(client, formatRxSMeter(0, rx, smeterDbmForRx(rx)));
    }
    sendToClient(client, tciMessage(QStringLiteral("RX_ENABLE"),
                                    {QStringLiteral("0"), QStringLiteral("true")}));

    sendToClient(client, formatTrx(0, m_settings->getRadioState()));
    sendToClient(client, formatDrive(0, m_settings->getDriveLevel()));
    sendToClient(client, formatTune(0, m_settings->getRadioState() == RadioState::TUNE));
    // WSJT-X / ExpertSDR clients expect these enable flags during handshake.
    sendToClient(client, tciMessage(QStringLiteral("SPLIT_ENABLE"),
                                    {QStringLiteral("0"),
                                     m_splitEnabled ? QStringLiteral("true") : QStringLiteral("false")}));
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

    // Advertise honest device power from data-engine state. Device START/STOP
    // is distinct from IQ_START. Tradeoff: WSJT-X do_start() sets _power_ from
    // a "start;" notification (else "TCI SDR is not switched on" unless the
    // client option that sends start itself / rig_power is enabled). When the
    // radio is stopped we send "stop;" so clients see real state; start the
    // engine (UI or TCI START) before expecting WSJT-X without that option.
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

        sendAudioPacket(client, *state, rx, stereoInterleaved.constData(),
                        stereoInterleaved.size());
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
        if (m_settings->getRadioState() != RadioState::RX) {
            TCI_WARN << "Client disconnected while TX active — watchdog started (30s)";
            m_watchdog->start();
        }
    }
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

    const int colon = commandLine.indexOf(':');
    const QString name = (colon >= 0 ? commandLine.left(colon) : commandLine).trimmed().toUpper();
    QStringList args;
    if (colon >= 0)
        args = commandLine.mid(colon + 1).split(',', Qt::KeepEmptyParts);

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

    const int maxRx = qMax(0, m_settings->getNumberOfReceivers() - 1);
    const int rx = qBound(0, channel, maxRx);

    // Device power (ExpertSDR START/STOP) — not the same as IQ stream control.
    // START/STOP request MainWindow (via signals) to start/stop the data engine
    // like the UI Start/Stop button. "start;" / "stop;" are advertised when the
    // engine state changes (onSystemStateChanged), and echoed immediately when
    // already in the requested state.
    if (name == QLatin1String("START")) {
        if (m_settings->getDataEngineState() == QSDR::DataEngineUp) {
            sendToClient(client, tciMessage(QStringLiteral("START")));
            return;
        }
        emit startRequested();
        return;
    }

    if (name == QLatin1String("STOP")) {
        if (m_settings->getDataEngineState() != QSDR::DataEngineDown)
            emit stopRequested();
        sendToClient(client, tciMessage(QStringLiteral("STOP")));
        return;
    }

    if (name == QLatin1String("IQ_START")) {
        // Test toggle: set CUSDR_TCI_NO_IQ=1 to refuse IQ subscriptions so the
        // panadapter stream stays off while RX audio keeps running. Lets us
        // isolate whether audio choppiness is caused by the IQ stream.
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
        // Advertise effective IQ rate (≤ audio doubled, e.g. 48k→96k).
        sendToClient(client, tciMessage(QStringLiteral("IQ_SAMPLERATE"),
                                        {QString::number(nativeIqSampleRate())}));
        return;
    }

    if (name == QLatin1String("IQ_STOP")) {
        if (TciClientState *state = clientState(client))
            state->iqEnabledReceivers.remove(rx);
        updateIqActiveHint();
        return;
    }

    if (name == QLatin1String("IQ_SAMPLERATE")) {
        // Client-requested rates are ignored. We report the effective
        // advertised rate (≤ audio doubled, e.g. 48k→96k); binary IQ remains
        // at the actual DSP sample rate (legacy rate/label mismatch).
        sendToClient(client, tciMessage(QStringLiteral("IQ_SAMPLERATE"),
                                        {QString::number(nativeIqSampleRate())}));
        return;
    }

    if (name == QLatin1String("IQ_STREAM_SAMPLE_TYPE")) {
        const QString fmtArg = (args.size() >= 2) ? args.at(1) : (args.isEmpty() ? QString() : args.at(0));
        if (!fmtArg.isEmpty() && clientState(client))
            clientState(client)->iqFormat = parseAudioFormat(fmtArg);
        return;
    }

    if (name == QLatin1String("IQ_STREAM_CHANNELS")) {
        const QString chArg = (args.size() >= 2) ? args.at(1) : (args.isEmpty() ? QString() : args.at(0));
        if (!chArg.isEmpty()) {
            bool ok = false;
            const int channels = chArg.toInt(&ok);
            if (ok && clientState(client))
                clientState(client)->iqChannels = (channels == 1) ? 1 : 2;
        }
        return;
    }

    if (name == QLatin1String("IQ_STREAM_SAMPLES")) {
        const QString samplesArg = (args.size() >= 2) ? args.at(1) : (args.isEmpty() ? QString() : args.at(0));
        if (!samplesArg.isEmpty()) {
            bool ok = false;
            const int samples = samplesArg.toInt(&ok);
            if (ok && clientState(client) && samples > 0)
                clientState(client)->iqSamplesPerPacket = samples;
        }
        return;
    }

    if (name == QLatin1String("VFO")) {
        if (args.size() >= 3) {
            bool ok = false;
            const qint64 freq = args.at(2).toLongLong(&ok);
            if (!ok || !isValidVfoHz(freq))
                return;
            // WSJT-X / ExpertSDR CAT-style VFO sets the dial frequency. Mode-0
            // (NCO-only) leaves CTR on the previous band (e.g. 14.074 → 7.074
            // yields NCO = −7 MHz). Move LO with VFO so digi band hops land
            // with CTR≈VFO and NCO≈0 — same contract as rigctld set_freq.
            m_settings->setCtrFrequency(1, rx, freq);
            return;
        }

        sendToClient(client, formatVfo(trx, channel, m_settings->getVfoFrequency(rx)));
        return;
    }

    if (name == QLatin1String("DDS")) {
        // Classic TCI is DDS:trx,freq (2 args). ExpertSDR3 / some clients also
        // send DDS:trx,channel,freq (3 args). Accept either; frequency is last.
        if (args.size() >= 2) {
            bool ok = false;
            const qint64 freq = args.last().toLongLong(&ok);
            if (!ok || !isValidVfoHz(freq))
                return;
            // Panadapter center only — keep dial VFO, refresh IF/NCO.
            const qint64 vfo = m_settings->getVfoFrequency(rx);
            m_settings->setCtrFrequency(0, rx, freq);
            m_settings->setNCOFrequency(true, rx, vfo - freq);
            return;
        }

        sendToClient(client, formatDds(trx, channel, m_settings->getCtrFrequency(rx)));
        return;
    }

    if (name == QLatin1String("IF")) {
        if (args.size() >= 3) {
            bool ok = false;
            const qint64 offset = args.at(2).toLongLong(&ok);
            if (!ok)
                return;
            // IF is the dial offset from DDS/CTR; keep VFO = CTR + IF.
            m_settings->setVFOFrequency(0, rx, m_settings->getCtrFrequency(rx) + offset);
            return;
        }

        sendToClient(client, formatIf(trx, channel, ifOffsetHz(rx)));
        return;
    }

    if (name == QLatin1String("MODULATION")) {
        if (args.size() >= 2 && !args.at(1).trimmed().isEmpty()) {
            m_settings->setDSPMode(rx, tciModeToDsp(args.at(1)));
            return;
        }

        sendToClient(client, formatModulation(trx, m_settings->getDSPMode(rx)));
        return;
    }

    if (name == QLatin1String("RX_FILTER_BAND")) {
        if (args.size() >= 3) {
            bool okLo = false;
            bool okHi = false;
            const qreal lo = args.at(1).toDouble(&okLo);
            const qreal hi = args.at(2).toDouble(&okHi);
            if (!okLo || !okHi)
                return;
            m_settings->setRXFilter(rx, lo, hi);
            return;
        }

        sendToClient(client, formatRxFilterBand(trx,
                                                m_settings->getFilterLo(rx),
                                                m_settings->getFilterHi(rx)));
        return;
    }

    if (name == QLatin1String("TRX") || name == QLatin1String("TX_ENABLE")) {
        if (args.size() >= 2) {
            const bool txOn = parseBoolArg(args.at(1));
            if (txOn)
                m_watchdog->stop();
            m_settings->setRadioState(txOn ? RadioState::MOX : RadioState::RX);
            // ExpertSDR3 / WSJT-X only send TX audio after TX_CHRONO clocks.
            // Third arg may be "tci" / "dax" / omitted — all still need chrono.
            if (txOn)
                startTxChrono(client, trx);
            else if (client == m_txChronoClient)
                stopTxChrono();
            return;
        }

        sendToClient(client, formatTrx(trx, m_settings->getRadioState()));
        return;
    }

    if (name == QLatin1String("TUNE")) {
        if (args.size() >= 2) {
            m_settings->setRadioState(parseBoolArg(args.at(1)) ? RadioState::TUNE : RadioState::RX);
            return;
        }

        sendToClient(client, formatTune(trx, m_settings->getRadioState() == RadioState::TUNE));
        return;
    }

    if (name == QLatin1String("DRIVE")) {
        if (args.size() >= 2) {
            bool ok = false;
            const int level = args.at(1).toInt(&ok);
            if (!ok)
                return;
            m_settings->setDriveLevel(qBound(0, level, 100));
            return;
        }

        sendToClient(client, formatDrive(trx, m_settings->getDriveLevel()));
        return;
    }

    // Bidirectional enable flags. cudaSDR is single-VFO (no real split/RIT/XIT);
    // accept and echo so WSJT-X TCI clients stay connected.
    if (name == QLatin1String("SPLIT_ENABLE")
        || name == QLatin1String("RIT_ENABLE")
        || name == QLatin1String("XIT_ENABLE")
        || name == QLatin1String("MUTE")
        || name == QLatin1String("LOCK")
        || name == QLatin1String("VFO_LOCK")) {
        int enableTrx = 0;
        bool enableValue = false;
        bool hasValue = false;
        parseTrxEnableArgs(args, enableTrx, enableValue, hasValue);

        if (hasValue) {
            if (name == QLatin1String("SPLIT_ENABLE"))
                m_splitEnabled = enableValue;
            // Always echo accepted state (ExpertSDR sync style).
            if (name == QLatin1String("MUTE")) {
                broadcast(tciMessage(name, {enableValue ? QStringLiteral("true")
                                                        : QStringLiteral("false")}));
            } else if (name == QLatin1String("VFO_LOCK") && args.size() >= 3) {
                broadcast(tciMessage(name,
                                     {QString::number(enableTrx),
                                      args.at(1),
                                      enableValue ? QStringLiteral("true") : QStringLiteral("false")}));
            } else {
                broadcast(tciMessage(name,
                                     {QString::number(enableTrx),
                                      enableValue ? QStringLiteral("true") : QStringLiteral("false")}));
            }
            return;
        }

        // Query
        if (name == QLatin1String("MUTE")) {
            sendToClient(client, tciMessage(name, {QStringLiteral("false")}));
        } else if (name == QLatin1String("SPLIT_ENABLE")) {
            sendToClient(client,
                         tciMessage(name,
                                    {QString::number(enableTrx),
                                     m_splitEnabled ? QStringLiteral("true") : QStringLiteral("false")}));
        } else {
            sendToClient(client,
                         tciMessage(name,
                                    {QString::number(enableTrx), QStringLiteral("false")}));
        }
        return;
    }

    // DSP / audio enable stubs — ExpertSDR clients query these during handshake.
    // No real DSP wiring; GET returns defaults, SET echoes so clients stay happy.
    if (name == QLatin1String("SQL_ENABLE")
        || name == QLatin1String("RX_ANF_ENABLE")
        || name == QLatin1String("MON_ENABLE")
        || name == QLatin1String("RX_NB_ENABLE")
        || name == QLatin1String("RX_NB2_ENABLE")
        || name == QLatin1String("RX_BIN_ENABLE")
        || name == QLatin1String("AGC_AUTO_EX")) {
        int enableTrx = 0;
        bool enableValue = false;
        bool hasValue = false;
        parseTrxEnableArgs(args, enableTrx, enableValue, hasValue);

        const bool defaultOn = (name == QLatin1String("AGC_AUTO_EX"));
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

    if (name == QLatin1String("SQL_LEVEL") || name == QLatin1String("RX_VOLUME")) {
        // GET: name:trx  → name:trx,0
        // SET: name:trx,value → echo
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

    if (name == QLatin1String("TUNE_DRIVE")) {
        // Mirror DRIVE. Single pure-int arg is a GET for that TRX (client: tune_drive:0).
        if (args.size() >= 2) {
            bool ok = false;
            const int level = args.at(1).toInt(&ok);
            if (!ok)
                return;
            m_settings->setDriveLevel(qBound(0, level, 100));
            return;
        }

        sendToClient(client, tciMessage(QStringLiteral("TUNE_DRIVE"),
                                        {QString::number(trx),
                                         QString::number(m_settings->getDriveLevel())}));
        return;
    }

    if (name == QLatin1String("TX_PROFILES_EX")) {
        sendToClient(client, tciMessage(QStringLiteral("TX_PROFILES_EX"),
                                        {QStringLiteral("Default")}));
        return;
    }

    if (name == QLatin1String("TX_PROFILE_EX")) {
        if (!args.isEmpty() && !args.at(0).trimmed().isEmpty()) {
            broadcast(tciMessage(QStringLiteral("TX_PROFILE_EX"), {args.at(0).trimmed()}));
            return;
        }
        sendToClient(client, tciMessage(QStringLiteral("TX_PROFILE_EX"),
                                        {QStringLiteral("Default")}));
        return;
    }

    if (name == QLatin1String("TX_STREAM_AUDIO_BUFFERING")) {
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

    if (name == QLatin1String("RX_SMETER")) {
        sendToClient(client, formatRxSMeter(trx, channel, smeterDbmForRx(rx)));
        return;
    }

    // Accept enable command from TCI clients; cudaSDR always pushes S-meter updates.
    // TX_SENSORS_ENABLE gates TX_SENSORS / TX_POWER / TX_SWR push updates.
    if (name == QLatin1String("RX_SENSORS_ENABLE")) {
        return;
    }

    if (name == QLatin1String("TX_SENSORS_ENABLE")) {
        TciClientState *state = clientState(client);
        if (!state)
            return;

        // Forms: tx_sensors_enable:true[ ,interval_ms]
        //        tx_sensors_enable:0,true[ ,interval_ms]  (trx-prefixed, rare)
        bool enable = false;
        int intervalMs = state->txSensorsIntervalMs;
        if (args.isEmpty()) {
            return;
        }
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
        // ExpertSDR pushes TX sensors while transmitting; snapshot only if already TX.
        if (enable && isTrxActive(m_settings->getRadioState()))
            maybeBroadcastTxSensors(true);
        return;
    }

    if (name == QLatin1String("TX_SENSORS")
        || name == QLatin1String("TX_POWER")
        || name == QLatin1String("POWER")
        || name == QLatin1String("TX_SWR")
        || name == QLatin1String("SWR")) {
        // Push-only sensors; honour queries with the last known values.
        if (name == QLatin1String("TX_SENSORS")) {
            sendToClient(client, formatTxSensors(trx));
        } else if (name == QLatin1String("TX_SWR") || name == QLatin1String("SWR")) {
            sendToClient(client, formatTxSwr(trx, effectiveSwr()));
        } else {
            sendToClient(client, formatTxPower(trx, m_fwdPowerWatts));
        }
        return;
    }

    if (name == QLatin1String("AUDIO_START") || name == QLatin1String("LINE_OUT_START")) {
        TciClientState *state = clientState(client);
        if (!state)
            return;
        state->audioEnabledReceivers.insert(rx);
        // WSJT-X sets stream_audio_ only when it receives audio_start:<rx>;
        // echo the enable so "TCI Audio could not be switched on" is avoided.
        sendToClient(client, tciMessage(QStringLiteral("AUDIO_START"), {QString::number(rx)}));
        sendToClient(client, tciMessage(QStringLiteral("AUDIO_SAMPLERATE"),
                                        {QString::number(state->audioSampleRate)}));
        return;
    }

    if (name == QLatin1String("AUDIO_STOP") || name == QLatin1String("LINE_OUT_STOP")) {
        if (TciClientState *state = clientState(client))
            state->audioEnabledReceivers.remove(rx);
        sendToClient(client, tciMessage(QStringLiteral("AUDIO_STOP"), {QString::number(rx)}));
        return;
    }

    if (name == QLatin1String("AUDIO_SAMPLERATE")) {
        const QString rateArg = (args.size() >= 2) ? args.at(1) : (args.isEmpty() ? QString() : args.at(0));
        if (!rateArg.isEmpty()) {
            bool ok = false;
            const int rate = rateArg.toInt(&ok);
            if (ok && clientState(client))
                clientState(client)->audioSampleRate = rate;
            return;
        }
        sendToClient(client, tciMessage(QStringLiteral("AUDIO_SAMPLERATE"), {QStringLiteral("48000")}));
        return;
    }

    if (name == QLatin1String("AUDIO_STREAM_SAMPLE_TYPE")) {
        const QString fmtArg = (args.size() >= 2) ? args.at(1) : (args.isEmpty() ? QString() : args.at(0));
        if (!fmtArg.isEmpty() && clientState(client))
            clientState(client)->audioFormat = parseAudioFormat(fmtArg);
        return;
    }

    if (name == QLatin1String("AUDIO_STREAM_CHANNELS")) {
        const QString chArg = (args.size() >= 2) ? args.at(1) : (args.isEmpty() ? QString() : args.at(0));
        if (!chArg.isEmpty()) {
            bool ok = false;
            const int channels = chArg.toInt(&ok);
            if (ok && clientState(client))
                clientState(client)->audioChannels = (channels == 1) ? 1 : 2;
        }
        return;
    }

    if (name == QLatin1String("AUDIO_STREAM_SAMPLES")) {
        const QString samplesArg = (args.size() >= 2) ? args.at(1) : (args.isEmpty() ? QString() : args.at(0));
        if (!samplesArg.isEmpty()) {
            bool ok = false;
            const int samples = samplesArg.toInt(&ok);
            if (ok && clientState(client) && samples > 0)
                clientState(client)->audioSamplesPerPacket = samples;
        }
        return;
    }

    if (name == QLatin1String("QPING") || name == QLatin1String("KEEPALIVE")) {
        // Client keep-alive / RTT probe. Echo qping with the same timestamp so
        // latency-aware clients (e.g. TCI Remote) stay happy; keepalive is silent.
        if (name == QLatin1String("QPING"))
            sendToClient(client, tciMessage(QStringLiteral("QPING"), args));
        return;
    }

    // Newer ExpertSDR clients send many optional verbs we do not implement yet.
    // Keep at TRACE (CUSDR_TCI_TRACE=1) so they do not spam WARN; real failures
    // (listen/bind, TX backlog, watchdog) still use TCI_WARN above.
    TCI_TRACE << "Unknown command:" << commandLine;
}

void TciServer::onVfoFrequencyChanged(int mode, int rx, qint64 frequency)
{
    Q_UNUSED(mode)
    broadcast(formatVfo(0, rx, frequency));
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
