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

#include <QObject>
#include <QTimer>
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

public slots:
    /** RX audio from SliceProcessor (queued to GUI thread). */
    void onRxAudioSamples(int rx, QVector<float> stereoInterleaved, int sampleRate);

signals:
    void remoteControlChanged(bool active);

private slots:
    void onNewConnection();
    void onClientTextMessage(const QString &message);
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

private:
    void sendToClient(QWebSocket *client, const QString &message);
    void broadcast(const QString &message);
    void sendInitState(QWebSocket *client);
    void handleCommand(QWebSocket *client, const QString &commandLine);
    void flushClientAudio(QWebSocket *client, int rx, bool forcePartial = false);

    struct TciClientState {
        QSet<int> audioEnabledReceivers;
        int audioChannels = 2;
        int audioFormat = 3;          // FLOAT32
        int audioSamplesPerPacket = 512;
        int audioSampleRate = 48000;
        QHash<int, QVector<float>> pendingAudio;
    };

    TciClientState *clientState(QWebSocket *client);
    const TciClientState *clientState(QWebSocket *client) const;
    void sendAudioPacket(QWebSocket *client, const TciClientState &state, int rx,
                         const float *stereoInterleaved, int stereoFloatCount);
    int stereoFloatsNeeded(const TciClientState &state) const;
    int parseAudioFormat(const QString &value) const;

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

    static constexpr int WATCHDOG_TIMEOUT_MS = 30000;
    static constexpr int kStreamHeaderBytes = 64;
    static constexpr uint32_t kRxAudioStreamType = 1;

    QWebSocketServer *m_server   = nullptr;
    QTimer           *m_watchdog = nullptr;
    QList<QWebSocket *> m_clients;
    QHash<QWebSocket *, TciClientState> m_clientStates;
    Settings         *m_settings = nullptr;
    RadioModel       *m_radioModel = nullptr;
    QList<QMetaObject::Connection> m_sliceConnections;
};

#endif // CUSDR_TCISERVER_H
