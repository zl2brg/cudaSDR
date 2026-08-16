#ifndef DXCLUSTERCLIENT_H
#define DXCLUSTERCLIENT_H

#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <QWebSocket>
#include <QTimer>
#include <QUrl>

/**
 * Background Telnet / WebSocket DX Cluster & RBN (Reverse Beacon Network) client.
 * Connects to DX skimmers/clusters, parses live spot streams, and emits spot signals.
 */
class DxClusterClient : public QObject {
    Q_OBJECT

public:
    enum ConnectionType {
        Telnet,
        WebSocket
    };
    Q_ENUM(ConnectionType)

    enum State {
        Disconnected,
        Connecting,
        WaitingForLogin,
        Connected
    };
    Q_ENUM(State)

    explicit DxClusterClient(QObject *parent = nullptr);
    ~DxClusterClient() override;

    // Connection Control
    void connectTelnet(const QString &host, quint16 port, const QString &callsign = QString());
    void connectWebSocket(const QUrl &url, const QString &callsign = QString());
    void disconnectFromCluster();

    bool isConnected() const { return m_state == Connected; }
    State state() const { return m_state; }
    ConnectionType connectionType() const { return m_type; }
    QString host() const { return m_host; }
    quint16 port() const { return m_port; }
    QUrl url() const { return m_url; }
    QString callsign() const { return m_callsign; }
    void setCallsign(const QString &callsign);

    // Auto-reconnect settings
    bool autoReconnect() const { return m_autoReconnect; }
    void setAutoReconnect(bool enable) { m_autoReconnect = enable; }
    int reconnectIntervalMs() const { return m_reconnectIntervalMs; }
    void setReconnectIntervalMs(int ms) { m_reconnectIntervalMs = ms; }

    // Send custom command to DX cluster
    void sendCommand(const QString &cmd);

    // Static parser helper for parsing a single DX/RBN line
    static bool parseDxLine(const QString &line,
                            qint64 &freqHz,
                            QString &dxCall,
                            QString &mode,
                            int &snr,
                            int &wpm,
                            QString &spotter,
                            QString &comment,
                            QString &timeUtc);

signals:
    void stateChanged(DxClusterClient::State state);
    void connected();
    void disconnected();
    void errorOccurred(const QString &errorMessage);
    void rawLineReceived(const QString &line);

    /**
     * Emitted whenever a valid DX or RBN spot is parsed.
     * @param freqHz Frequency in Hz
     * @param dxCall Spotted station callsign (e.g. "ZL2BRG")
     * @param mode Operating mode (e.g. "CW", "FT8", "SSB", "RTTY")
     * @param snr Signal to noise ratio in dB (or 0 if not present)
     * @param wpm CW speed in WPM (or 0 if not present)
     * @param spotter Spotter / Skimmer callsign (e.g. "K3LR-#")
     * @param comment Additional remarks / text
     */
    void spotReceived(qint64 freqHz,
                      const QString &dxCall,
                      const QString &mode,
                      int snr,
                      int wpm,
                      const QString &spotter,
                      const QString &comment);

private slots:
    // Telnet Socket Slots
    void onTcpConnected();
    void onTcpDisconnected();
    void onTcpReadyRead();
    void onTcpError(QAbstractSocket::SocketError socketError);

    // WebSocket Slots
    void onWsConnected();
    void onWsDisconnected();
    void onWsTextMessageReceived(const QString &message);
    void onWsError(QAbstractSocket::SocketError error);

    // Reconnect Slot
    void onReconnectTimeout();

private:
    void setState(State state);
    void processLine(const QString &line);
    void handleLoginPrompt(const QString &text);

    ConnectionType m_type = Telnet;
    State m_state = Disconnected;

    QString m_host;
    quint16 m_port = 7000;
    QUrl m_url;
    QString m_callsign;

    bool m_autoReconnect = true;
    int m_reconnectIntervalMs = 10000;
    bool m_userInitiatedDisconnect = false;

    QTcpSocket *m_tcpSocket = nullptr;
    QWebSocket *m_webSocket = nullptr;
    QTimer *m_reconnectTimer = nullptr;

    QByteArray m_incomingBuffer;
};

#endif // DXCLUSTERCLIENT_H
