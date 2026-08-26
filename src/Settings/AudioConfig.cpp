#include "AudioConfig.h"
#include <QSettings>
#include "Util/settings_utils.h"

AudioConfig::AudioConfig(QObject *parent)
    : QObject(parent)
    , m_rxEqEnabled(false)
    , m_rxEqBands(11, 0)
    , m_rxEqCurveDeg(0)
    , m_emnrPost2Enabled(false)
    , m_emnrPost2Factor(15.0)
    , m_emnrPost2Nlevel(15.0)
    , m_emnrPost2Taper(12.0)
    , m_emnrPost2Rate(5.0)
    , m_mainVolume(0.1f)
{
}

void AudioConfig::setRxEqEnabled(bool enabled) {
    if (m_rxEqEnabled != enabled) {
        m_rxEqEnabled = enabled;
        emit rxEqEnabledChanged(m_rxEqEnabled);
    }
}

void AudioConfig::setRxEqBands(const QVector<int> &bands) {
    QVector<int> next = bands;
    if (next.size() < 11)
        next.resize(11);
    for (int &g : next)
        g = SettingsUtils::clampEqBandDb(g);
    if (next != m_rxEqBands) {
        m_rxEqBands = next;
        emit rxEqBandsChanged();
    }
}

void AudioConfig::setRxEqBand(int index, int gainDb) {
    if (index < 0 || index >= 11)
        return;
    const int g = SettingsUtils::clampEqBandDb(gainDb);
    if (m_rxEqBands.size() < 11)
        m_rxEqBands.resize(11);
    if (m_rxEqBands[index] != g) {
        m_rxEqBands[index] = g;
        emit rxEqBandsChanged();
    }
}

void AudioConfig::setRxEqCurveDeg(int deg) {
    deg = SettingsUtils::clampEqDeg(deg);
    if (m_rxEqCurveDeg != deg) {
        m_rxEqCurveDeg = deg;
        emit rxEqCurveDegChanged(m_rxEqCurveDeg);
    }
}

void AudioConfig::setEmnrPost2Enabled(bool enabled) {
    if (m_emnrPost2Enabled != enabled) {
        m_emnrPost2Enabled = enabled;
        emit emnrPost2Changed();
    }
}

void AudioConfig::setEmnrPost2Factor(double pct) {
    pct = SettingsUtils::clampBoundedDouble(pct, 0.0, 100.0);
    if (m_emnrPost2Factor != pct) {
        m_emnrPost2Factor = pct;
        emit emnrPost2Changed();
    }
}

void AudioConfig::setEmnrPost2Nlevel(double pct) {
    pct = SettingsUtils::clampBoundedDouble(pct, 0.0, 100.0);
    if (m_emnrPost2Nlevel != pct) {
        m_emnrPost2Nlevel = pct;
        emit emnrPost2Changed();
    }
}

void AudioConfig::setEmnrPost2Taper(double pct) {
    pct = SettingsUtils::clampBoundedDouble(pct, 0.0, 100.0);
    if (m_emnrPost2Taper != pct) {
        m_emnrPost2Taper = pct;
        emit emnrPost2Changed();
    }
}

void AudioConfig::setEmnrPost2Rate(double seconds) {
    seconds = SettingsUtils::clampBoundedDouble(seconds, 0.2, 20.0);
    if (m_emnrPost2Rate != seconds) {
        m_emnrPost2Rate = seconds;
        emit emnrPost2Changed();
    }
}

void AudioConfig::setMainVolume(float vol) {
    if (m_mainVolume != vol) {
        m_mainVolume = vol;
        emit mainVolumeChanged(m_mainVolume);
    }
}

void AudioConfig::load(const QJsonObject &json) {
    if (json.contains("rxEqEnabled")) setRxEqEnabled(json["rxEqEnabled"].toBool());
    if (json.contains("rxEqBands") && json["rxEqBands"].isArray())
        setRxEqBands(SettingsUtils::jsonArrayToVector<int>(json, QStringLiteral("rxEqBands"), 11));
    if (json.contains("rxEqCurveDeg")) setRxEqCurveDeg(json["rxEqCurveDeg"].toInt());
    if (json.contains("emnrPost2Enabled")) setEmnrPost2Enabled(json["emnrPost2Enabled"].toBool());
    if (json.contains("emnrPost2Factor")) setEmnrPost2Factor(json["emnrPost2Factor"].toDouble());
    if (json.contains("emnrPost2Nlevel")) setEmnrPost2Nlevel(json["emnrPost2Nlevel"].toDouble());
    if (json.contains("emnrPost2Taper")) setEmnrPost2Taper(json["emnrPost2Taper"].toDouble());
    if (json.contains("emnrPost2Rate")) setEmnrPost2Rate(json["emnrPost2Rate"].toDouble());
    if (json.contains("mainVolume")) setMainVolume(static_cast<float>(json["mainVolume"].toDouble()));
}

void AudioConfig::save(QJsonObject &json) const {
    json["rxEqEnabled"] = m_rxEqEnabled;
    json["rxEqBands"] = SettingsUtils::toJsonArray(m_rxEqBands);
    json["rxEqCurveDeg"] = m_rxEqCurveDeg;
    json["emnrPost2Enabled"] = m_emnrPost2Enabled;
    json["emnrPost2Factor"] = m_emnrPost2Factor;
    json["emnrPost2Nlevel"] = m_emnrPost2Nlevel;
    json["emnrPost2Taper"] = m_emnrPost2Taper;
    json["emnrPost2Rate"] = m_emnrPost2Rate;
    json["mainVolume"] = static_cast<double>(m_mainVolume);
}

void AudioConfig::loadIni(QSettings *settings) {
    setRxEqEnabled(settings->value("rx_eq_enabled", false).toBool());
    {
        QVector<int> bands(11, 0);
        for (int i = 0; i < 11; ++i)
            bands[i] = settings->value(QStringLiteral("rx_eq_band_%1").arg(i), 0).toInt();
        setRxEqBands(bands);
    }
    setRxEqCurveDeg(settings->value("rx_eq_curve_deg", 0).toInt());

    setEmnrPost2Enabled(settings->value("emnr_post2_enabled", false).toBool());
    setEmnrPost2Factor(settings->value("emnr_post2_factor", 15.0).toDouble());
    setEmnrPost2Nlevel(settings->value("emnr_post2_nlevel", 15.0).toDouble());
    setEmnrPost2Taper(settings->value("emnr_post2_taper", 12.0).toDouble());
    setEmnrPost2Rate(settings->value("emnr_post2_rate", 5.0).toDouble());

    int volVal = settings->value("server/mainVolume", 10).toInt();
    if (volVal < 0) volVal = 0;
    if (volVal > 100) volVal = 100;
    setMainVolume(volVal / 100.0f);
}

void AudioConfig::saveIni(QSettings *settings) const {
    settings->setValue("rx_eq_enabled", m_rxEqEnabled);
    for (int i = 0; i < m_rxEqBands.size() && i < 11; ++i)
        settings->setValue(QStringLiteral("rx_eq_band_%1").arg(i), m_rxEqBands.at(i));
    settings->setValue("rx_eq_curve_deg", m_rxEqCurveDeg);
    settings->setValue("emnr_post2_enabled", m_emnrPost2Enabled);
    settings->setValue("emnr_post2_factor", m_emnrPost2Factor);
    settings->setValue("emnr_post2_nlevel", m_emnrPost2Nlevel);
    settings->setValue("emnr_post2_taper", m_emnrPost2Taper);
    settings->setValue("emnr_post2_rate", m_emnrPost2Rate);
    settings->setValue("server/mainVolume", static_cast<int>(m_mainVolume * 100));
}
