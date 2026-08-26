#include "NetworkConfig.h"
#include <QSettings>
#include "Util/settings_utils.h"

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
    if (json.contains("serverAddress")) setServerAddress(json["serverAddress"].toString());
    if (json.contains("localAddress")) setLocalAddress(json["localAddress"].toString());
    if (json.contains("serverPort")) setServerPort(static_cast<quint16>(json["serverPort"].toInt()));
    if (json.contains("listenPort")) setListenPort(static_cast<quint16>(json["listenPort"].toInt()));
    if (json.contains("audioPort")) setAudioPort(static_cast<quint16>(json["audioPort"].toInt()));
    if (json.contains("metisPort")) setMetisPort(static_cast<quint16>(json["metisPort"].toInt()));
    if (json.contains("socketBufferSize")) setSocketBufferSize(json["socketBufferSize"].toInt());
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

void NetworkConfig::loadIni(QSettings *settings) {
    setServerAddress(SettingsUtils::stripSurroundingQuotes(settings->value("network/server_ipAddress", "127.0.0.1").toString()));
    setLocalAddress(SettingsUtils::stripSurroundingQuotes(settings->value("network/hpsdr_local_ipAddress", "127.0.0.1").toString()));
    setServerPort(static_cast<quint16>(SettingsUtils::clampNetworkPort(settings->value("network/server_port", 52685).toInt(), 52685)));
    setListenPort(static_cast<quint16>(SettingsUtils::clampNetworkPort(settings->value("network/listen_port", 11000).toInt(), 11000)));
    setAudioPort(static_cast<quint16>(SettingsUtils::clampNetworkPort(settings->value("network/audio_port", 15000).toInt(), 15000)));
    setMetisPort(static_cast<quint16>(SettingsUtils::clampNetworkPort(settings->value("network/metis_port", 1024).toInt(), 1024)));
    setSocketBufferSize(SettingsUtils::clampSocketBufferSizeKb(settings->value("network/socketBufferSize", 32).toInt()));
}

void NetworkConfig::saveIni(QSettings *settings) const {
    settings->setValue("network/server_ipAddress", m_serverAddress);
    settings->setValue("network/hpsdr_local_ipAddress", m_localAddress);
    settings->setValue("network/server_port", m_serverPort);
    settings->setValue("network/listen_port", m_listenPort);
    settings->setValue("network/audio_port", m_audioPort);
    settings->setValue("network/metis_port", m_metisPort);
    settings->setValue("network/socketBufferSize", m_socketBufferSize);
}
