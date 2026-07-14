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
    void onSMeterValueChanged(int rx, double rawValue);
    void onTciServerEnabledChanged(bool enabled);

private:
    void sendToClient(QWebSocket *client, const QString &message);
    void broadcast(const QString &message);
    void sendInitState(QWebSocket *client);
    void handleCommand(QWebSocket *client, const QString &commandLine);

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
        int iqSampleRate = 48000;     // set to the radio's actual RX rate on each block

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

    QString dspModeToTci(DSPMode mode) const;
    DSPMode tciModeToDsp(const QString &mode) const;

    bool parseBoolArg(const QString &value) const;
    int ifOffsetHz(int rx) const;
    double smeterDbmFromRaw(double rawValue) const;
    double smeterDbmForRx(int rx) const;
    int nativeIqSampleRate() const;
    int effectiveIqSampleRate(int actualRate) const;

    static constexpr int WATCHDOG_TIMEOUT_MS = 30000;

    // Bound the transmit-mic queue backlog (in DSP blocks) so a client sending
    // TX audio faster than the DSP transmit path drains can never build TX
    // latency nor block the socket thread. ~48 blocks ≈ 1 s at 48 kHz/1024.
    static constexpr int kTxAudioMaxQueueBlocks = 48;

    // IQ backpressure threshold (socket bytesToWrite backlog). The panadapter
    // IQ stream is best-effort: it is shed at a very small backlog so it can
    // never build socket latency that would delay RX audio on the shared
    // socket. A dropped panadapter frame is invisible; late audio makes the
    // client hard-reset its buffer. RX audio itself is never dropped.
    static constexpr qint64 kIqBacklogDropBytes = 32 * 1024;

    QWebSocketServer *m_server   = nullptr;
    QTimer           *m_watchdog = nullptr;
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
};

#endif // CUSDR_TCISERVER_H
