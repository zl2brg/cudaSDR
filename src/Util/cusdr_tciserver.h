/**
 * @file  cusdr_tciserver.h
 * @brief ExpertSDR TCI (Transceiver Control Interface) WebSocket server
 */

#ifndef CUSDR_TCISERVER_H
#define CUSDR_TCISERVER_H

#define LOG_TCI_SERVER

#ifdef LOG_TCI_SERVER
#   define TCI_DEBUG qDebug().nospace() << "TciServer::\t"
#   define TCI_WARN  qWarning().nospace() << "TciServer::\t"
#else
#   define TCI_DEBUG nullDebug()
#   define TCI_WARN  nullDebug()
#endif

// Per-command traffic dump. Off by default — enable with CUSDR_TCI_TRACE=1.
#define TCI_TRACE \
    if (qEnvironmentVariableIntValue("CUSDR_TCI_TRACE") == 0) {} \
    else TCI_DEBUG

#include "cusdr_hamDatabase.h"
#include "Settings/SettingsTypes.h"
#include "Util/cusdr_queue.h"
#include "Util/tci_protocol_utils.h"

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QList>
#include <QHash>
#include <QSet>
#include <QVector>

class QWebSocketServer;
class QWebSocket;
class Settings;
class RadioModel;

class TciServer : public QObject {
    Q_OBJECT

public:
    explicit TciServer(QObject *parent = nullptr);
    ~TciServer() override;

    bool startListening(quint16 port = 50001);
    void stopListening();
    bool isListening() const;
    quint16 port() const;
    bool hasClients() const { return !m_clients.isEmpty(); }

    /** True while a TCI client is keyed and we are clocking TX_CHRONO.
     *  During this window TX mic audio must come from the network queue only. */
    bool isTxChronoActive() const { return m_txChronoClient != nullptr; }

    /** Connect to SliceModel S-meter updates (call after RadioModel is ready). */
    void bindSlices(RadioModel *radioModel);

    /** Hand the transmit path's network-mic queue to the server so received
     *  TX-audio frames can be enqueued for the DSP transmit path. The queue is
     *  owned by TransmitAudioInput and is thread-safe (QHQueue). */
    void setTransmitAudioQueue(QHQueue<QVector<double>> *queue) { m_txAudioQueue = queue; }

public slots:
    /** RX audio from SliceProcessor (queued to GUI thread). */
    void onRxAudioSamples(int rx, QVector<float> stereoInterleaved, int sampleRate);

    /** RX IQ from SliceProcessor (queued to GUI thread). Raw input I/Q at the
     *  radio's actual RX sample rate, for both HPSDR and SoapySDR. */
    void onRxIqSamples(int rx, QVector<float> iqInterleaved, int sampleRate);

signals:
    void remoteControlChanged(bool active);
    /** Request MainWindow to start the data engine (same path as UI Start). */
    void startRequested();
    /** Request MainWindow to stop the data engine (same path as UI Stop). */
    void stopRequested();

private slots:
    void onNewConnection();
    void onClientTextMessage(const QString &message);
    void onClientBinaryMessage(const QByteArray &message);
    void onClientDisconnected();
    void onWatchdogTimeout();

    void onVfoFrequencyChanged(int mode, int rx, qint64 frequency);
    void onCtrFrequencyChanged(int mode, int rx, qint64 frequency);
    void onNcoFrequencyChanged(int rx, qint64 frequency);
    void onDspModeChanged(int rx, DSPMode mode);
    void onFilterFrequenciesChanged(int rx, qreal low, qreal high);
    void onRadioStateChanged(RadioState state);
    void onDriveLevelChanged(int level);
    void onForwardPowerChanged(qreal watts);
    void onReversePowerChanged(qreal watts);
    void onSwrChanged(qreal swr);
    void onSMeterValueChanged(int rx, double rawValue);
    void onTciServerEnabledChanged(bool enabled);

private:
    void sendToClient(QWebSocket *client, const QString &message);
    void broadcast(const QString &message);
    void sendInitState(QWebSocket *client);
    void handleCommand(QWebSocket *client, const QString &commandLine);

    // WSJT-X / ExpertSDR3 clients only emit TX audio in response to TX_CHRONO
    // (stream type 3) timing frames. Drive those while a TCI client is keyed.
    void startTxChrono(QWebSocket *client, int trx);
    void stopTxChrono();
    void sendTxChronoFrame(QWebSocket *client);
    void onTxChronoTick();

    struct TciClientState {
        QSet<int> audioEnabledReceivers;
        int audioChannels = 2;
        int audioFormat = 3;          // FLOAT32
        int audioSamplesPerPacket = 512;
        int audioSampleRate = 48000;

        QSet<int> iqEnabledReceivers;
        int iqChannels = 2;
        int iqFormat = 3;             // FLOAT32
        int iqSamplesPerPacket = 512;
        int iqSampleRate = 48000;     // advertised IQ rate (≤ audio doubled, e.g. 48k→96k)

        // ExpertSDR TX_SENSORS_ENABLE — gate TX power / SWR push updates.
        bool txSensorsEnabled = false;
        int txSensorsIntervalMs = 200;
        qint64 txSensorsLastSendMs = 0;

        // Best-effort IQ drop counter (diagnostics). The panadapter IQ stream is
        // shed when the socket write backlog is large so it never builds latency
        // on the socket shared with RX audio (audio priority). RX audio is never
        // dropped.
        qint64 iqFramesDropped = 0;
    };

    TciClientState *clientState(QWebSocket *client);
    const TciClientState *clientState(QWebSocket *client) const;
    void sendAudioPacket(QWebSocket *client, const TciClientState &state, int rx,
                         const float *stereoInterleaved, int stereoFloatCount);
    void sendIqPacket(QWebSocket *client, const TciClientState &state, int rx,
                      const float *iqInterleaved, int iqFloatCount);
    int parseAudioFormat(const QString &value) const;

    // Recompute and publish the "any client wants IQ" hint to Settings so the
    // DSP thread can gate the per-block IQ emission. Call whenever an IQ
    // subscription changes (start/stop/disconnect).
    void updateIqActiveHint();

    QString formatVfo(int trx, int channel, qint64 frequency) const;
    QString formatDds(int trx, int channel, qint64 frequency) const;
    QString formatIf(int trx, int channel, qint64 offset) const;
    QString formatModulation(int trx, DSPMode mode) const;
    QString formatRxFilterBand(int trx, qreal low, qreal high) const;
    QString formatTrx(int trx, RadioState state) const;
    QString formatDrive(int trx, int level) const;
    QString formatTune(int trx, bool enabled) const;
    QString formatRxSMeter(int trx, int channel, double dbm) const;
    QString formatTxSensors(int trx) const;
    QString formatTxPower(int trx, qreal watts) const;
    QString formatTxSwr(int trx, qreal swr) const;
    void maybeBroadcastTxSensors(bool force = false);
    void scheduleTxSensorsBroadcast();
    qreal effectiveSwr() const;

    QString dspModeToTci(DSPMode mode) const;
    DSPMode tciModeToDsp(const QString &mode) const;

    bool parseBoolArg(const QString &value) const;
    /** Parse trx[,bool] enable args. Pure integer single arg = GET for that TRX. */
    void parseTrxEnableArgs(const QStringList &args, int &enableTrx,
                            bool &enableValue, bool &hasValue) const;
    int ifOffsetHz(int rx) const;
    double smeterDbmFromRaw(double rawValue) const;
    double smeterDbmForRx(int rx) const;
    int nativeIqSampleRate() const;
    int effectiveIqSampleRate(int actualRate) const;

    static constexpr int WATCHDOG_TIMEOUT_MS = 30000;

    // Bound the transmit-mic queue backlog (in DSP blocks) so a client sending
    // TX audio faster than the DSP transmit path drains can never build TX
    // latency nor block the socket thread. Keep in sync with TX_MIC_QUEUE_MAX_BLOCKS.
    static constexpr int kTxAudioMaxQueueBlocks = 16;

    // TX_CHRONO steady-state period matches one WSJT geometric TX reply at 48 kHz:
    // length=2048 float stereo pairs → 1024 mono samples → 1024/48000 s ≈ 21.3 ms.
    // Priming burst in startTxChrono() is separate; do not shorten this period.
    static constexpr qint64 kTxChronoPeriodNs =
        (static_cast<qint64>(TciProtocol::kTxChronoStereoFrames) * 1000000000LL) / 48000LL;
    static constexpr int kTxChronoPollMs = 5;

    // IQ backpressure threshold (socket bytesToWrite backlog). The panadapter
    // IQ stream is best-effort: it is shed at a very small backlog so it can
    // never build socket latency that would delay RX audio on the shared
    // socket. A dropped panadapter frame is invisible; late audio makes the
    // client hard-reset its buffer. RX audio itself is never dropped.
    static constexpr qint64 kIqBacklogDropBytes = 32 * 1024;

    QWebSocketServer *m_server   = nullptr;
    QTimer           *m_watchdog = nullptr;
    QTimer           *m_txChronoTimer = nullptr;
    QList<QWebSocket *> m_clients;
    QHash<QWebSocket *, TciClientState> m_clientStates;
    Settings         *m_settings = nullptr;
    RadioModel       *m_radioModel = nullptr;
    QList<QMetaObject::Connection> m_sliceConnections;

    // Transmit (mic) audio received from clients. The queue is owned by
    // TransmitAudioInput; the residual accumulates decoded mono samples so we
    // enqueue exactly DSP_SAMPLE_SIZE blocks (matching the local mic path).
    QHQueue<QVector<double>> *m_txAudioQueue = nullptr;
    QVector<double>          m_txAudioResidual;
    bool                     m_splitEnabled = false;

    QWebSocket   *m_txChronoClient = nullptr;
    int           m_txChronoTrx = 0;
    QElapsedTimer m_txChronoClock;
    qint64        m_txChronoAccumNs = 0;

    // Last start/stop power state advertised to clients (avoids duplicate
    // broadcasts when systemStateChanged fires for unrelated field changes).
    bool m_advertisedPowerOn = false;

    // Cached PA telemetry from RadioTelemetry (Hermes / Alex C&C).
    qreal m_fwdPowerWatts = 0.0;
    qreal m_revPowerWatts = 0.0;
    qreal m_swr = 1.0;
    bool m_swrValid = false;
    bool m_txSensorsFlushPending = false;
};

#endif // CUSDR_TCISERVER_H
