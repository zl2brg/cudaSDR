#include "NetworkConfig.h"

NetworkConfig::NetworkConfig(QObject *parent)
    : QObject(parent)
    , m_serverAddress("127.0.0.1")
    , m_localAddress("127.0.0.1")
    , m_serverPort(52685)
    , m_listenPort(11000)
    , m_audioPort(15000)
    , m_metisPort(1024)
    , m_socketBufferSize(32)
{}

void NetworkConfig::setServerAddress(const QString &addr) {
    if (m_serverAddress != addr) {
        m_serverAddress = addr;
        emit serverAddressChanged(m_serverAddress);
    }
}

void NetworkConfig::setLocalAddress(const QString &addr) {
    if (m_localAddress != addr) {
        m_localAddress = addr;
        emit localAddressChanged(m_localAddress);
    }
}

void NetworkConfig::setServerPort(quint16 port) {
    if (m_serverPort != port) {
        m_serverPort = port;
        emit serverPortChanged(m_serverPort);
    }
}

void NetworkConfig::setListenPort(quint16 port) {
    if (m_listenPort != port) {
        m_listenPort = port;
        emit listenPortChanged(m_listenPort);
    }
}

void NetworkConfig::setAudioPort(quint16 port) {
    if (m_audioPort != port) {
        m_audioPort = port;
        emit audioPortChanged(m_audioPort);
    }
}

void NetworkConfig::setMetisPort(quint16 port) {
    if (m_metisPort != port) {
        m_metisPort = port;
        emit metisPortChanged(m_metisPort);
    }
}

void NetworkConfig::setSocketBufferSize(int size) {
    if (m_socketBufferSize != size) {
        m_socketBufferSize = size;
        emit socketBufferSizeChanged(m_socketBufferSize);
    }
}

void NetworkConfig::load(const QJsonObject &json) {
    if (json.contains("serverAddress")) m_serverAddress = json["serverAddress"].toString();
    if (json.contains("localAddress")) m_localAddress = json["localAddress"].toString();
    if (json.contains("serverPort")) m_serverPort = static_cast<quint16>(json["serverPort"].toInt());
    if (json.contains("listenPort")) m_listenPort = static_cast<quint16>(json["listenPort"].toInt());
    if (json.contains("audioPort")) m_audioPort = static_cast<quint16>(json["audioPort"].toInt());
    if (json.contains("metisPort")) m_metisPort = static_cast<quint16>(json["metisPort"].toInt());
    if (json.contains("socketBufferSize")) m_socketBufferSize = json["socketBufferSize"].toInt();
}

void NetworkConfig::save(QJsonObject &json) const {
    json["serverAddress"] = m_serverAddress;
    json["localAddress"] = m_localAddress;
    json["serverPort"] = m_serverPort;
    json["listenPort"] = m_listenPort;
    json["audioPort"] = m_audioPort;
    json["metisPort"] = m_metisPort;
    json["socketBufferSize"] = m_socketBufferSize;
}
