#ifndef NETWORKCONFIG_H
#define NETWORKCONFIG_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QHostAddress>
#include <QNetworkInterface>

class QSettings;

class NetworkConfig : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString serverAddress READ serverAddress WRITE setServerAddress NOTIFY serverAddressChanged)
    Q_PROPERTY(QString localAddress READ localAddress WRITE setLocalAddress NOTIFY localAddressChanged)
    Q_PROPERTY(quint16 serverPort READ serverPort WRITE setServerPort NOTIFY serverPortChanged)
    Q_PROPERTY(quint16 listenPort READ listenPort WRITE setListenPort NOTIFY listenPortChanged)
    Q_PROPERTY(quint16 audioPort READ audioPort WRITE setAudioPort NOTIFY audioPortChanged)
    Q_PROPERTY(quint16 metisPort READ metisPort WRITE setMetisPort NOTIFY metisPortChanged)
    Q_PROPERTY(int socketBufferSize READ socketBufferSize WRITE setSocketBufferSize NOTIFY socketBufferSizeChanged)

public:
    explicit NetworkConfig(QObject *parent = nullptr);

    QString serverAddress() const { return m_serverAddress; }
    void setServerAddress(const QString &addr);

    QString localAddress() const { return m_localAddress; }
    void setLocalAddress(const QString &addr);

    quint16 serverPort() const { return m_serverPort; }
    void setServerPort(quint16 port);

    quint16 listenPort() const { return m_listenPort; }
    void setListenPort(quint16 port);

    quint16 audioPort() const { return m_audioPort; }
    void setAudioPort(quint16 port);

    quint16 metisPort() const { return m_metisPort; }
    void setMetisPort(quint16 port);

    int socketBufferSize() const { return m_socketBufferSize; }
    void setSocketBufferSize(int size);

    void load(const QJsonObject &json);
    void save(QJsonObject &json) const;

    void loadIni(QSettings *settings);
    void saveIni(QSettings *settings) const;

signals:
    void serverAddressChanged(const QString &addr);
    void localAddressChanged(const QString &addr);
    void serverPortChanged(quint16 port);
    void listenPortChanged(quint16 port);
    void audioPortChanged(quint16 port);
    void metisPortChanged(quint16 port);
    void socketBufferSizeChanged(int size);

private:
    QString m_serverAddress;
    QString m_localAddress;
    quint16 m_serverPort;
    quint16 m_listenPort;
    quint16 m_audioPort;
    quint16 m_metisPort;
    int m_socketBufferSize;
};

#endif // NETWORKCONFIG_H
