/**
 * @file  cusdr_tciserver.cpp
 * @brief ExpertSDR TCI WebSocket server — Phase 1 control commands
 *
 * Implements a subset of the TCI text protocol for remote rig control:
 *   VFO, DDS, IF, MODULATION, RX_FILTER_BAND, TRX, DRIVE, TUNE, RX_SMETER,
 *   AUDIO_START/STOP (binary RX audio), START, STOP
 *
 * Transport: WebSocket text frames, commands NAME:arg1,arg2;
 * cudaSDR acts as server; clients receive an init burst on connect plus live push updates.
 */

#include "cusdr_tciserver.h"
#include "Util/tci_protocol_utils.h"
#include "cusdr_settings.h"
#include "cusdr_hamDatabase.h"
#include "Settings/SettingsTypes.h"
#include "Models/RadioModel.h"
#include "Models/SliceModel.h"

#include <QWebSocketServer>
#include <QWebSocket>
#include <QHostAddress>
#include <QStringList>
#include <QDebug>
#include <QtEndian>
#include <QMetaType>
#include <cstring>
#include <cstdlib>

using namespace TciProtocol;

TciServer::TciServer(QObject *parent)
    : QObject(parent)
    , m_server(new QWebSocketServer(QStringLiteral("cudaSDR TCI"),
                                    QWebSocketServer::NonSecureMode, this))
    , m_watchdog(new QTimer(this))
    , m_settings(Settings::instance())
{
    qRegisterMetaType<QVector<float>>("QVector<float>");

    connect(m_server, &QWebSocketServer::newConnection, this, &TciServer::onNewConnection);

    m_watchdog->setSingleShot(true);
    m_watchdog->setInterval(WATCHDOG_TIMEOUT_MS);
    connect(m_watchdog, &QTimer::timeout, this, &TciServer::onWatchdogTimeout);

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
    for (QWebSocket *client : std::as_const(m_clients)) {
        client->close();
    }
    m_clients.clear();
    m_server->close();
    if (m_settings)
        m_settings->setTciIqActive(false);
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

    TCI_DEBUG << "TX ->" << client->peerAddress().toString() << message;
    client->sendTextMessage(message);
}

void TciServer::broadcast(const QString &message)
{
    if (m_clients.isEmpty())
        return;
    TCI_DEBUG << "TX *" << message;
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
}

void TciServer::sendInitState(QWebSocket *client)
{
    const int trxCount = 1;
    const int channelCount = qMax(1, m_settings->getNumberOfReceivers());
    const bool receiveOnly = !m_settings->getTxAllowed();

    sendToClient(client, tciMessage(QStringLiteral("DEVICE"), {QStringLiteral("cudaSDR")}));
    sendToClient(client, tciMessage(QStringLiteral("PROTOCOL"), {QStringLiteral("1"), QStringLiteral("0"), QStringLiteral("0")}));
    sendToClient(client, tciMessage(QStringLiteral("RECEIVE_ONLY"),
                                    {receiveOnly ? QStringLiteral("true") : QStringLiteral("false")}));
    sendToClient(client, tciMessage(QStringLiteral("TRX_COUNT"), {QString::number(trxCount)}));
    sendToClient(client, tciMessage(QStringLiteral("CHANNEL_COUNT"), {QString::number(channelCount)}));
    sendToClient(client, tciMessage(QStringLiteral("MODULATIONS_LIST"),
                                    {QStringLiteral("LSB,USB,AM,FM,NFM,DIGU,DIGL,CW,CWL,SAM,FDV")}));
    sendToClient(client, tciMessage(QStringLiteral("VFO_LIMITS"),
                                    {QString::number(kVfoMinHz), QString::number(kVfoMaxHz)}));

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

    sendToClient(client, formatTrx(0, m_settings->getRadioState()));
    sendToClient(client, formatDrive(0, m_settings->getDriveLevel()));
    sendToClient(client, formatTune(0, m_settings->getRadioState() == RadioState::TUNE));
    sendToClient(client, tciMessage(QStringLiteral("AUDIO_SAMPLERATE"), {QStringLiteral("48000")}));
    sendToClient(client, tciMessage(QStringLiteral("IQ_SAMPLERATE"),
                                    {QString::number(nativeIqSampleRate())}));
    sendToClient(client, tciMessage(QStringLiteral("READY")));
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

        // Send the radio's actual RX rate as-is (no decimation); the frame
        // header advertises the rate so the client's FFT scales correctly.
        sendIqPacket(client, *state, rx, iqInterleaved.constData(),
                     iqInterleaved.size());
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

    TCI_DEBUG << "RX <-" << client->peerAddress().toString() << message;

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

    const int channels = (hdr.channels >= 1) ? static_cast<int>(hdr.channels) : 1;
    const QByteArray payload = message.mid(kStreamHeaderBytes);
    if (!decodeTxAudioMonoSamples(payload, static_cast<int>(hdr.format), channels, m_txAudioResidual))
        return;

    // Enqueue exactly DSP_SAMPLE_SIZE mono blocks — the same granularity the
    // local soundcard mic uses (fetch_MicData truncates to DSP_SAMPLE_SIZE).
    static quint64 txAudioBlockCounter = 0;
    const TxAudioChunkResult chunked = chunkTxAudioResidual(
        m_txAudioResidual,
        DSP_SAMPLE_SIZE,
        kTxAudioMaxQueueBlocks,
        m_txAudioQueue->count());
    m_txAudioResidual = chunked.residual;

    for (const QVector<double> &block : chunked.blocks)
        m_txAudioQueue->enqueue(block);

    if (chunked.droppedBlocks > 0 && (++txAudioBlockCounter % 200) == 1) {
        // Client is producing faster than the TX path drains — drop to keep
        // TX latency bounded. Never block the socket thread on the queue.
        TCI_WARN << "TX audio backlog: dropping client mic block (TX drain slower than input)";
    }
}

void TciServer::onClientDisconnected()
{
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    if (!client)
        return;

    TCI_DEBUG << "Client disconnected";
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

    if (name == QLatin1String("START") || name == QLatin1String("IQ_START")) {
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
        // Advertise the radio's actual RX rate so the client scales its FFT.
        sendToClient(client, tciMessage(QStringLiteral("IQ_SAMPLERATE"),
                                        {QString::number(nativeIqSampleRate())}));
        return;
    }

    if (name == QLatin1String("STOP") || name == QLatin1String("IQ_STOP")) {
        if (TciClientState *state = clientState(client))
            state->iqEnabledReceivers.remove(rx);
        updateIqActiveHint();
        return;
    }

    if (name == QLatin1String("IQ_SAMPLERATE")) {
        // cudaSDR streams the radio's actual RX rate (no decimation), so a
        // client-requested rate is ignored; we always report the native rate.
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
            m_settings->setVFOFrequency(0, rx, freq);
            return;
        }

        sendToClient(client, formatVfo(trx, channel, m_settings->getVfoFrequency(rx)));
        return;
    }

    if (name == QLatin1String("DDS")) {
        if (args.size() >= 3) {
            bool ok = false;
            const qint64 freq = args.at(2).toLongLong(&ok);
            if (!ok || !isValidVfoHz(freq))
                return;
            m_settings->setCtrFrequency(0, rx, freq);
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
            m_settings->setNCOFrequency(true, rx, offset);
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
            if (parseBoolArg(args.at(1)))
                m_watchdog->stop();
            m_settings->setRadioState(parseBoolArg(args.at(1)) ? RadioState::MOX : RadioState::RX);
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

    if (name == QLatin1String("RX_SMETER")) {
        sendToClient(client, formatRxSMeter(trx, channel, smeterDbmForRx(rx)));
        return;
    }

    // Accept enable command from TCI clients; cudaSDR always pushes S-meter updates.
    if (name == QLatin1String("RX_SENSORS_ENABLE")) {
        return;
    }

    if (name == QLatin1String("AUDIO_START") || name == QLatin1String("LINE_OUT_START")) {
        TciClientState *state = clientState(client);
        if (!state)
            return;
        state->audioEnabledReceivers.insert(rx);
        sendToClient(client, tciMessage(QStringLiteral("AUDIO_SAMPLERATE"),
                                        {QString::number(state->audioSampleRate)}));
        return;
    }

    if (name == QLatin1String("AUDIO_STOP") || name == QLatin1String("LINE_OUT_STOP")) {
        if (TciClientState *state = clientState(client))
            state->audioEnabledReceivers.remove(rx);
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

    TCI_WARN << "Unknown command:" << commandLine;
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
}

void TciServer::onDriveLevelChanged(int level)
{
    broadcast(formatDrive(0, level));
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
