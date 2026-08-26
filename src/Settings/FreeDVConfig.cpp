#include "FreeDVConfig.h"
#include <QSettings>
#include "Util/settings_utils.h"

FreeDVConfig::FreeDVConfig(QObject *parent)
    : QObject(parent)
    , m_defaultMode(0) // FREEDV_MODE_1600
    , m_sqThreshold(0.0f)
    , m_autoSync(true)
    , m_clipAudio(true)
    , m_txBandpass(true)
{
    for (int i = 0; i < kMaxReceivers; ++i) {
        m_rxModes.append(0);
    }
}

void FreeDVConfig::setDefaultMode(int mode) {
    if (m_defaultMode != mode) {
        m_defaultMode = mode;
        emit defaultModeChanged(m_defaultMode);
    }
}

void FreeDVConfig::setSqThreshold(float threshold) {
    if (m_sqThreshold != threshold) {
        m_sqThreshold = threshold;
        emit sqThresholdChanged(m_sqThreshold);
    }
}

void FreeDVConfig::setAutoSync(bool enabled) {
    if (m_autoSync != enabled) {
        m_autoSync = enabled;
        emit autoSyncChanged(m_autoSync);
    }
}

void FreeDVConfig::setClipAudio(bool enabled) {
    if (m_clipAudio != enabled) {
        m_clipAudio = enabled;
        emit clipAudioChanged(m_clipAudio);
    }
}

void FreeDVConfig::setTxBandpass(bool enabled) {
    if (m_txBandpass != enabled) {
        m_txBandpass = enabled;
        emit txBandpassChanged(m_txBandpass);
    }
}

int FreeDVConfig::rxMode(int rx) const {
    if (rx >= 0 && rx < m_rxModes.size())
        return m_rxModes.at(rx);
    return m_defaultMode;
}

void FreeDVConfig::setRxMode(int rx, int mode) {
    if (rx >= 0 && rx < m_rxModes.size()) {
        if (m_rxModes.at(rx) != mode) {
            m_rxModes[rx] = mode;
            emit rxModeChanged(rx, mode);
        }
    }
}

void FreeDVConfig::setRxModes(const QList<int> &modes) {
    m_rxModes = modes;
    while (m_rxModes.size() < kMaxReceivers)
        m_rxModes.append(0);
}

void FreeDVConfig::load(const QJsonObject &json) {
    if (json.contains("defaultMode")) setDefaultMode(json["defaultMode"].toInt());
    if (json.contains("sqThreshold")) setSqThreshold(static_cast<float>(json["sqThreshold"].toDouble()));
    if (json.contains("autoSync")) setAutoSync(json["autoSync"].toBool());
    if (json.contains("clipAudio")) setClipAudio(json["clipAudio"].toBool());
    if (json.contains("txBandpass")) setTxBandpass(json["txBandpass"].toBool());

    SettingsUtils::applyJsonArray<int>(json, QStringLiteral("rxModes"), m_rxModes.size(),
                                      [this](int i, int mode) { setRxMode(i, mode); });
}

void FreeDVConfig::save(QJsonObject &json) const {
    json["defaultMode"] = m_defaultMode;
    json["sqThreshold"] = static_cast<double>(m_sqThreshold);
    json["autoSync"] = m_autoSync;
    json["clipAudio"] = m_clipAudio;
    json["txBandpass"] = m_txBandpass;

    json["rxModes"] = SettingsUtils::toJsonArray(m_rxModes);
}

void FreeDVConfig::loadIni(QSettings *settings) {
    setDefaultMode(settings->value("freedv/default_mode", 0).toInt());
    setSqThreshold(settings->value("freedv/sq_threshold", 0.0f).toFloat());
    setAutoSync(settings->value("freedv/auto_sync", true).toBool());
    setClipAudio(settings->value("freedv/clip_audio", true).toBool());
    setTxBandpass(settings->value("freedv/tx_bandpass", true).toBool());

    for (int i = 0; i < kMaxReceivers; ++i) {
        const QString k = QStringLiteral("freedv/rx%1_mode").arg(i);
        if (settings->contains(k))
            setRxMode(i, settings->value(k).toInt());
    }
}

void FreeDVConfig::saveIni(QSettings *settings) const {
    settings->setValue("freedv/default_mode", m_defaultMode);
    settings->setValue("freedv/sq_threshold", m_sqThreshold);
    settings->setValue("freedv/auto_sync", m_autoSync);
    settings->setValue("freedv/clip_audio", m_clipAudio);
    settings->setValue("freedv/tx_bandpass", m_txBandpass);

    for (int i = 0; i < kMaxReceivers; ++i) {
        settings->setValue(QStringLiteral("freedv/rx%1_mode").arg(i), m_rxModes.value(i));
    }
}
