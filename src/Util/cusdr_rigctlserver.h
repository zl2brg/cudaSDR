/**
* @file  cusdr_rigctlserver.h
* @brief rigctld-compatible TCP server for WSJT-X / third-party rig control
*/

#ifndef CUSDR_RIGCTLSERVER_H
#define CUSDR_RIGCTLSERVER_H

#define LOG_RIGCTL_SERVER

#ifdef LOG_RIGCTL_SERVER
#   define RIGCTL_DEBUG qDebug().nospace() << "RigCtlServer::\t"
#   define RIGCTL_WARN  qWarning().nospace() << "RigCtlServer::\t"
#else
#   define RIGCTL_DEBUG nullDebug()
#   define RIGCTL_WARN  nullDebug()
#endif

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QList>

class Settings;

class RigCtlServer : public QObject {
    Q_OBJECT

public:
    explicit RigCtlServer(QObject *parent = nullptr);
    ~RigCtlServer() override;

    bool startListening(quint16 port = 4532);
    void stopListening();
    bool isListening() const;
    quint16 port() const;
    bool hasClients() const { return !m_clients.isEmpty(); }

signals:
    // Emitted when the first client connects or the last client disconnects.
    void remoteControlChanged(bool active);

private slots:
    void onNewConnection();
    void onClientReadyRead();
    void onClientDisconnected();
    void onWatchdogTimeout();

private:
    QString processCommand(const QString &cmd);
    QString dumpState() const;
    QString dspModeToRigctlMode(int dspMode) const;
    int rigctlModeToDsp(const QString &mode) const;

    static constexpr int WATCHDOG_TIMEOUT_MS = 30000;

    QTcpServer  *m_server   = nullptr;
    QTimer      *m_watchdog = nullptr;
    QList<QTcpSocket *> m_clients;
    Settings    *m_settings = nullptr;
};

#endif // CUSDR_RIGCTLSERVER_H
