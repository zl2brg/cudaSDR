#include "TransmitConfig.h"
#include <QSettings>
#include "Util/settings_utils.h"

namespace {
QVariant iniValue(QSettings *settings, const char *legacy, const char *alt, const QVariant &def)
{
    if (settings->contains(QLatin1String(legacy)))
        return settings->value(QLatin1String(legacy));
    if (alt && settings->contains(QLatin1String(alt)))
        return settings->value(QLatin1String(alt));
    return def;
}
}

TransmitConfig::TransmitConfig(QObject *parent)
    : QObject(parent)
    , m_micSource(1)
    , m_micInputDev(0)
    , m_digitalAudioInputDev(0)
    , m_micInputSourceName(QString())
    , m_digitalInputSourceName(QString())
    , m_micGain(10.0)
    , m_driveLevel(100)
    , m_tunePower(10)
    , m_paEnabled(true)
    , m_fmPreemphasis(1)
    , m_phaseRotator(1)
    , m_phaseRotatorAuto(false)
    , m_phaseRotatorStatus(QStringLiteral("Active"))
    , m_amCarrierLevel(0.5)
    , m_audioCompression(0)
    , m_fmDeviation(5000.0)
    , m_ctcssToneHz(0)
    , m_txEqEnabled(false)
    , m_txEqCurveDeg(0)
    , m_cfcEnabled(false)
    , m_cfcPeqEnabled(false)
    , m_cfcPrecomp(3.0)
    , m_cfcPrePeq(-9.0)
    , m_cfcCurveDeg(0)
    , m_txFullDuplex(true)
    , m_repeaterOffset(0.0)
    , m_txFilterLow(100)
    , m_txFilterHigh(2900)
    , m_txUseRxFilter(false)
{
    m_txEqBands.fill(0, kEqBands);
    ensureCfcDefaults();
}

void TransmitConfig::ensureCfcDefaults() {
    static const double kFreq[kCfcBands] = {
        50, 150, 300, 500, 750, 1250, 1750, 2300, 2800, 3100
    };
    static const double kLvl[kCfcBands] = {
        0, 0, 3, 3, 3, 6, 6, 6, 9, 9
    };
    if (m_cfcFreqs.size() != kCfcBands) {
        m_cfcFreqs.resize(kCfcBands);
        for (int i = 0; i < kCfcBands; ++i)
            m_cfcFreqs[i] = kFreq[i];
    }
    if (m_cfcLevels.size() != kCfcBands) {
        m_cfcLevels.resize(kCfcBands);
        for (int i = 0; i < kCfcBands; ++i)
            m_cfcLevels[i] = kLvl[i];
    }
    if (m_cfcPost.size() != kCfcBands) {
        m_cfcPost.fill(0.0, kCfcBands);
    }
}

void TransmitConfig::setMicSource(int source) {
    if (m_micSource != source) {
        m_micSource = source;
        emit micSourceChanged(m_micSource);
    }
}

void TransmitConfig::setMicInputDev(int dev) {
    if (m_micInputDev != dev) {
        m_micInputDev = dev;
        emit micInputDevChanged(m_micInputDev);
    }
}

void TransmitConfig::setDigitalAudioInputDev(int dev) {
    if (m_digitalAudioInputDev != dev) {
        m_digitalAudioInputDev = dev;
        emit digitalAudioInputDevChanged(m_digitalAudioInputDev);
    }
}

void TransmitConfig::setMicInputSourceName(const QString &name) {
    if (m_micInputSourceName != name) {
        m_micInputSourceName = name;
        emit micInputSourceNameChanged(m_micInputSourceName);
    }
}

void TransmitConfig::setDigitalInputSourceName(const QString &name) {
    if (m_digitalInputSourceName != name) {
        m_digitalInputSourceName = name;
        emit digitalInputSourceNameChanged(m_digitalInputSourceName);
    }
}

void TransmitConfig::setMicGain(double gain) {
    if (m_micGain != gain) {
        m_micGain = gain;
        emit micGainChanged(m_micGain);
    }
}

void TransmitConfig::setDriveLevel(int level) {
    if (m_driveLevel != level) {
        m_driveLevel = level;
        emit driveLevelChanged(m_driveLevel);
    }
}

void TransmitConfig::setTunePower(int power) {
    const int val = qBound(0, power, 100);
    if (m_tunePower != val) {
        m_tunePower = val;
        emit tunePowerChanged(m_tunePower);
    }
}

void TransmitConfig::setPaEnabled(bool enabled) {
    if (m_paEnabled != enabled) {
        m_paEnabled = enabled;
        emit paEnabledChanged(m_paEnabled);
    }
}

void TransmitConfig::setFmPreemphasis(int val) {
    if (m_fmPreemphasis != val) {
        m_fmPreemphasis = val;
        emit fmPreemphasisChanged(m_fmPreemphasis);
    }
}

void TransmitConfig::setPhaseRotator(int val) {
    const int clamped = (val != 0) ? 1 : 0;
    if (m_phaseRotator != clamped) {
        m_phaseRotator = clamped;
        emit phaseRotatorChanged(m_phaseRotator);
    }
}

void TransmitConfig::setPhaseRotatorAuto(bool enabled) {
    if (m_phaseRotatorAuto != enabled) {
        m_phaseRotatorAuto = enabled;
        emit phaseRotatorAutoChanged(m_phaseRotatorAuto);
    }
}

void TransmitConfig::setPhaseRotatorStatus(const QString &status) {
    if (m_phaseRotatorStatus != status) {
        m_phaseRotatorStatus = status;
        emit phaseRotatorStatusChanged(m_phaseRotatorStatus);
    }
}

void TransmitConfig::setAmCarrierLevel(double level) {
    const double val = qBound(0.0, level, 1.0);
    if (m_amCarrierLevel != val) {
        m_amCarrierLevel = val;
        emit amCarrierLevelChanged(m_amCarrierLevel);
    }
}

void TransmitConfig::setAudioCompression(int val) {
    if (m_audioCompression != val) {
        m_audioCompression = val;
        emit audioCompressionChanged(m_audioCompression);
    }
}

void TransmitConfig::setFmDeviation(double dev) {
    if (m_fmDeviation != dev) {
        m_fmDeviation = dev;
        emit fmDeviationChanged(m_fmDeviation);
    }
}

void TransmitConfig::setCtcssToneHz(int hz) {
    int clamped = hz;
    if (clamped < 0) clamped = 0;
    if (clamped > 1000) clamped = 1000;
    if (m_ctcssToneHz != clamped) {
        m_ctcssToneHz = clamped;
        emit ctcssToneHzChanged(m_ctcssToneHz);
    }
}

void TransmitConfig::setTxEqEnabled(bool enabled) {
    if (m_txEqEnabled != enabled) {
        m_txEqEnabled = enabled;
        emit txEqEnabledChanged(m_txEqEnabled);
    }
}

void TransmitConfig::setTxEqBands(const QVector<int> &bands) {
    QVector<int> next = bands;
    if (next.size() != kEqBands)
        next.resize(kEqBands);
    for (int &g : next)
        g = SettingsUtils::clampEqBandDb(g);
    if (next != m_txEqBands) {
        m_txEqBands = next;
        emit txEqBandsChanged();
    }
}

void TransmitConfig::setTxEqBand(int index, int gainDb) {
    if (index < 0 || index >= kEqBands)
        return;
    const int g = SettingsUtils::clampEqBandDb(gainDb);
    if (m_txEqBands.size() != kEqBands)
        m_txEqBands.resize(kEqBands);
    if (m_txEqBands.at(index) != g) {
        m_txEqBands[index] = g;
        emit txEqBandsChanged();
    }
}

void TransmitConfig::setTxEqCurveDeg(int deg) {
    deg = SettingsUtils::clampEqDeg(deg);
    if (m_txEqCurveDeg != deg) {
        m_txEqCurveDeg = deg;
        emit txEqCurveDegChanged(m_txEqCurveDeg);
    }
}

void TransmitConfig::setCfcEnabled(bool enabled) {
    if (m_cfcEnabled != enabled) {
        m_cfcEnabled = enabled;
        emit cfcEnabledChanged(m_cfcEnabled);
    }
}

void TransmitConfig::setCfcPeqEnabled(bool enabled) {
    if (m_cfcPeqEnabled != enabled) {
        m_cfcPeqEnabled = enabled;
        emit cfcPeqEnabledChanged(m_cfcPeqEnabled);
    }
}

void TransmitConfig::setCfcPrecomp(double db) {
    db = SettingsUtils::clampDb(db, -20.0, 20.0);
    if (m_cfcPrecomp != db) {
        m_cfcPrecomp = db;
        emit cfcPrecompChanged(m_cfcPrecomp);
    }
}

void TransmitConfig::setCfcPrePeq(double db) {
    db = SettingsUtils::clampDb(db, -20.0, 20.0);
    if (m_cfcPrePeq != db) {
        m_cfcPrePeq = db;
        emit cfcPrePeqChanged(m_cfcPrePeq);
    }
}

void TransmitConfig::setCfcLevels(const QVector<double> &levels) {
    QVector<double> next = levels;
    if (next.size() != kCfcBands)
        next.resize(kCfcBands);
    for (double &v : next)
        v = SettingsUtils::clampDb(v, -20.0, 20.0);
    m_cfcLevels = next;
    emit cfcLevelsChanged();
}

void TransmitConfig::setCfcLevel(int index, double db) {
    ensureCfcDefaults();
    if (index < 0 || index >= kCfcBands)
        return;
    db = SettingsUtils::clampDb(db, -20.0, 20.0);
    if (m_cfcLevels.at(index) != db) {
        m_cfcLevels[index] = db;
        emit cfcLevelsChanged();
    }
}

void TransmitConfig::setCfcPost(const QVector<double> &post) {
    QVector<double> next = post;
    if (next.size() != kCfcBands)
        next.resize(kCfcBands);
    for (double &v : next)
        v = SettingsUtils::clampDb(v, -20.0, 20.0);
    m_cfcPost = next;
    emit cfcPostChanged();
}

void TransmitConfig::setCfcPostBand(int index, double db) {
    ensureCfcDefaults();
    if (index < 0 || index >= kCfcBands)
        return;
    db = SettingsUtils::clampDb(db, -20.0, 20.0);
    if (m_cfcPost.at(index) != db) {
        m_cfcPost[index] = db;
        emit cfcPostChanged();
    }
}

void TransmitConfig::setCfcCurveDeg(int deg) {
    deg = SettingsUtils::clampEqDeg(deg);
    if (m_cfcCurveDeg != deg) {
        m_cfcCurveDeg = deg;
        emit cfcCurveDegChanged(m_cfcCurveDeg);
    }
}

void TransmitConfig::setTxFullDuplex(bool fullDuplex) {
    if (m_txFullDuplex != fullDuplex) {
        m_txFullDuplex = fullDuplex;
        emit txFullDuplexChanged(m_txFullDuplex);
    }
}

void TransmitConfig::setRepeaterOffset(double offset) {
    if (m_repeaterOffset != offset) {
        m_repeaterOffset = offset;
        emit repeaterOffsetChanged(m_repeaterOffset);
    }
}

void TransmitConfig::setTxFilterLow(int val) {
    if (m_txFilterLow != val) {
        m_txFilterLow = val;
        emit txFilterLowChanged(m_txFilterLow);
    }
}

void TransmitConfig::setTxFilterHigh(int val) {
    if (m_txFilterHigh != val) {
        m_txFilterHigh = val;
        emit txFilterHighChanged(m_txFilterHigh);
    }
}

void TransmitConfig::setTxUseRxFilter(bool enabled) {
    if (m_txUseRxFilter != enabled) {
        m_txUseRxFilter = enabled;
        emit txUseRxFilterChanged(m_txUseRxFilter);
    }
}

void TransmitConfig::load(const QJsonObject &json) {
    ensureCfcDefaults();
    if (json.contains("micSource")) setMicSource(json["micSource"].toInt());
    if (json.contains("micInputDev")) setMicInputDev(json["micInputDev"].toInt());
    if (json.contains("digitalAudioInputDev")) setDigitalAudioInputDev(json["digitalAudioInputDev"].toInt());
    if (json.contains("micInputSourceName")) setMicInputSourceName(json["micInputSourceName"].toString());
    if (json.contains("digitalInputSourceName")) setDigitalInputSourceName(json["digitalInputSourceName"].toString());
    if (json.contains("micGain")) setMicGain(json["micGain"].toDouble());
    if (json.contains("driveLevel")) setDriveLevel(json["driveLevel"].toInt());
    if (json.contains("tunePower")) setTunePower(json["tunePower"].toInt());
    if (json.contains("paEnabled")) setPaEnabled(json["paEnabled"].toBool());
    if (json.contains("fmPreemphasis")) setFmPreemphasis(json["fmPreemphasis"].toInt());
    if (json.contains("phaseRotator")) setPhaseRotator(json["phaseRotator"].toInt());
    if (json.contains("phaseRotatorAuto")) setPhaseRotatorAuto(json["phaseRotatorAuto"].toBool());
    if (json.contains("phaseRotatorStatus")) setPhaseRotatorStatus(json["phaseRotatorStatus"].toString());
    if (json.contains("amCarrierLevel")) setAmCarrierLevel(json["amCarrierLevel"].toDouble());
    if (json.contains("audioCompression")) setAudioCompression(json["audioCompression"].toInt());
    if (json.contains("fmDeviation")) setFmDeviation(json["fmDeviation"].toDouble());
    if (json.contains("ctcssToneHz")) setCtcssToneHz(json["ctcssToneHz"].toInt());
    if (json.contains("txEqEnabled")) setTxEqEnabled(json["txEqEnabled"].toBool());
    if (json.contains("txEqCurveDeg")) setTxEqCurveDeg(json["txEqCurveDeg"].toInt());
    if (json.contains("cfcEnabled")) setCfcEnabled(json["cfcEnabled"].toBool());
    if (json.contains("cfcPeqEnabled")) setCfcPeqEnabled(json["cfcPeqEnabled"].toBool());
    if (json.contains("cfcPrecomp")) setCfcPrecomp(json["cfcPrecomp"].toDouble());
    if (json.contains("cfcPrePeq")) setCfcPrePeq(json["cfcPrePeq"].toDouble());
    if (json.contains("cfcCurveDeg")) setCfcCurveDeg(json["cfcCurveDeg"].toInt());
    if (json.contains("txFullDuplex")) setTxFullDuplex(json["txFullDuplex"].toBool());
    if (json.contains("repeaterOffset")) setRepeaterOffset(json["repeaterOffset"].toDouble());
    if (json.contains("txFilterLow")) setTxFilterLow(json["txFilterLow"].toInt());
    if (json.contains("txFilterHigh")) setTxFilterHigh(json["txFilterHigh"].toInt());
    if (json.contains("txUseRxFilter")) setTxUseRxFilter(json["txUseRxFilter"].toBool());

    if (json.contains("txEqBands") && json["txEqBands"].isArray())
        setTxEqBands(SettingsUtils::jsonArrayToVector<int>(json, QStringLiteral("txEqBands"), kEqBands));
    if (json.contains("cfcLevels") && json["cfcLevels"].isArray())
        setCfcLevels(SettingsUtils::jsonArrayToVector<double>(json, QStringLiteral("cfcLevels"), kCfcBands));
    if (json.contains("cfcPost") && json["cfcPost"].isArray())
        setCfcPost(SettingsUtils::jsonArrayToVector<double>(json, QStringLiteral("cfcPost"), kCfcBands));
}

void TransmitConfig::save(QJsonObject &json) const {
    json["micSource"] = m_micSource;
    json["micInputDev"] = m_micInputDev;
    json["digitalAudioInputDev"] = m_digitalAudioInputDev;
    json["micInputSourceName"] = m_micInputSourceName;
    json["digitalInputSourceName"] = m_digitalInputSourceName;
    json["micGain"] = m_micGain;
    json["driveLevel"] = m_driveLevel;
    json["tunePower"] = m_tunePower;
    json["paEnabled"] = m_paEnabled;
    json["fmPreemphasis"] = m_fmPreemphasis;
    json["phaseRotator"] = m_phaseRotator;
    json["phaseRotatorAuto"] = m_phaseRotatorAuto;
    json["amCarrierLevel"] = m_amCarrierLevel;
    json["audioCompression"] = m_audioCompression;
    json["fmDeviation"] = m_fmDeviation;
    json["ctcssToneHz"] = m_ctcssToneHz;
    json["txEqEnabled"] = m_txEqEnabled;
    json["txEqCurveDeg"] = m_txEqCurveDeg;
    json["cfcEnabled"] = m_cfcEnabled;
    json["cfcPeqEnabled"] = m_cfcPeqEnabled;
    json["cfcPrecomp"] = m_cfcPrecomp;
    json["cfcPrePeq"] = m_cfcPrePeq;
    json["cfcCurveDeg"] = m_cfcCurveDeg;
    json["txFullDuplex"] = m_txFullDuplex;
    json["repeaterOffset"] = m_repeaterOffset;
    json["txFilterLow"] = m_txFilterLow;
    json["txFilterHigh"] = m_txFilterHigh;
    json["txUseRxFilter"] = m_txUseRxFilter;

    json["txEqBands"] = SettingsUtils::toJsonArray(m_txEqBands);
    json["cfcLevels"] = SettingsUtils::toJsonArray(m_cfcLevels);
    json["cfcPost"] = SettingsUtils::toJsonArray(m_cfcPost);
}

void TransmitConfig::loadIni(QSettings *settings) {
    ensureCfcDefaults();

    QString str = iniValue(settings, "server/mic_source", nullptr, QStringLiteral("penelope")).toString();
    setMicSource(str == QLatin1String("janus") ? 0 : 1);

    setMicInputDev(iniValue(settings, "mic_InputDevice", "audio/mic_input_dev", 0).toInt());
    setDigitalAudioInputDev(iniValue(settings, "digital_audio_InputDevice", "audio/digital_audio_input_dev", 0).toInt());
    setMicInputSourceName(iniValue(settings, "mic_input_source", "audio/mic_input_source_name",
                                   (micInputDev() > 0) ? QStringLiteral("default") : QString()).toString());
    setDigitalInputSourceName(iniValue(settings, "digital_input_source", "audio/digital_input_source_name",
                                       (digitalAudioInputDev() > 0) ? QStringLiteral("default") : QStringLiteral("none")).toString());

    if (micInputSourceName().isEmpty()) {
        setMicInputSourceName(QStringLiteral("default"));
        setMicInputDev(1);
    }
    if (digitalInputSourceName().isEmpty()) {
        setDigitalInputSourceName((digitalAudioInputDev() > 0) ? QStringLiteral("default") : QStringLiteral("none"));
    }

    setMicGain(iniValue(settings, "micGain", "audio/mic_gain", 0).toDouble());
    setDriveLevel(SettingsUtils::clampDriveLevel(iniValue(settings, "driveLevel", "audio/drive_level", 0).toInt()));
    setTunePower(settings->value("audio/tune_power", 10).toInt());
    setPaEnabled(settings->value("audio/pa_enabled", true).toBool());
    setFmPreemphasis(iniValue(settings, "fm_preemphesize", "audio/fm_preemphasis", 1).toInt());
    setPhaseRotator(iniValue(settings, "audio_phase_rotator", "audio/phase_rotator", 1).toInt());
    setPhaseRotatorAuto(iniValue(settings, "audio_phase_rotator_auto", "audio/phase_rotator_auto", false).toBool());
    setAmCarrierLevel(iniValue(settings, "am_carrierlevel", "audio/am_carrier_level", 0.5).toDouble());
    setAudioCompression(iniValue(settings, "audiocompression", "audio/audio_compression", 0).toInt());
    setFmDeviation(iniValue(settings, "fmdeveation", "audio/fm_deviation", 5000.0).toDouble());
    setCtcssToneHz(iniValue(settings, "ctcss_tone_hz", "audio/ctcss_tone_hz", 0).toInt());
    setTxEqEnabled(iniValue(settings, "tx_eq_enabled", "audio/tx_eq_enabled", false).toBool());
    setTxEqCurveDeg(iniValue(settings, "tx_eq_curve_deg", "audio/tx_eq_curve_deg", 0).toInt());
    setCfcEnabled(iniValue(settings, "cfc_enabled", "audio/cfc_enabled", false).toBool());
    setCfcPeqEnabled(iniValue(settings, "cfc_peq_enabled", "audio/cfc_peq_enabled", false).toBool());
    setCfcPrecomp(iniValue(settings, "cfc_precomp", "audio/cfc_precomp", 3.0).toDouble());
    setCfcPrePeq(iniValue(settings, "cfc_prepeq", "audio/cfc_pre_peq", -9.0).toDouble());
    setCfcCurveDeg(iniValue(settings, "cfc_curve_deg", "audio/cfc_curve_deg", 0).toInt());

    setTxFullDuplex(settings->value("radio/txFullDuplex", true).toBool());
    setRepeaterOffset(settings->value("repeater_offset", 0.0).toDouble());
    setTxFilterLow(iniValue(settings, "tx_filter_low", "audio/tx_filter_low", 100).toInt());
    setTxFilterHigh(iniValue(settings, "tx_filter_high", "audio/tx_filter_high", 2900).toInt());
    setTxUseRxFilter(iniValue(settings, "tx_use_rx_filter", "audio/tx_use_rx_filter", false).toBool());

    QVector<int> bands(kEqBands, 0);
    for (int i = 0; i < kEqBands; ++i) {
        const QString legacy = QStringLiteral("tx_eq_band_%1").arg(i);
        const QString alt = QStringLiteral("audio/tx_eq_band_%1").arg(i);
        if (settings->contains(legacy))
            bands[i] = settings->value(legacy).toInt();
        else if (settings->contains(alt))
            bands[i] = settings->value(alt).toInt();
    }
    setTxEqBands(bands);

    for (int i = 0; i < kCfcBands; ++i) {
        const QString kl = QStringLiteral("cfc_lvl_%1").arg(i);
        const QString klAlt = QStringLiteral("audio/cfc_level_%1").arg(i);
        if (settings->contains(kl))
            setCfcLevel(i, settings->value(kl).toDouble());
        else if (settings->contains(klAlt))
            setCfcLevel(i, settings->value(klAlt).toDouble());

        const QString kp = QStringLiteral("cfc_post_%1").arg(i);
        const QString kpAlt = QStringLiteral("audio/cfc_post_%1").arg(i);
        if (settings->contains(kp))
            setCfcPostBand(i, settings->value(kp).toDouble());
        else if (settings->contains(kpAlt))
            setCfcPostBand(i, settings->value(kpAlt).toDouble());
    }
}

void TransmitConfig::saveIni(QSettings *settings) const {
    settings->setValue("server/mic_source", m_micSource == 0 ? QStringLiteral("janus") : QStringLiteral("penelope"));
    settings->setValue("mic_InputDevice", m_micInputDev);
    settings->setValue("mic_input_source", m_micInputSourceName);
    settings->setValue("digital_audio_InputDevice", m_digitalAudioInputDev);
    settings->setValue("digital_input_source", m_digitalInputSourceName);
    settings->setValue("micGain", m_micGain);
    settings->setValue("driveLevel", m_driveLevel);
    settings->setValue("audio/tune_power", m_tunePower);
    settings->setValue("audio/pa_enabled", m_paEnabled);
    settings->setValue("fm_preemphesize", m_fmPreemphasis);
    settings->setValue("audio_phase_rotator", m_phaseRotator);
    settings->setValue("audio_phase_rotator_auto", m_phaseRotatorAuto);
    settings->setValue("am_carrierlevel", m_amCarrierLevel);
    settings->setValue("audiocompression", m_audioCompression);
    settings->setValue("fmdeveation", m_fmDeviation);
    settings->setValue("ctcss_tone_hz", m_ctcssToneHz);
    settings->setValue("tx_eq_enabled", m_txEqEnabled);
    settings->setValue("tx_eq_curve_deg", m_txEqCurveDeg);
    settings->setValue("cfc_enabled", m_cfcEnabled);
    settings->setValue("cfc_peq_enabled", m_cfcPeqEnabled);
    settings->setValue("cfc_precomp", m_cfcPrecomp);
    settings->setValue("cfc_prepeq", m_cfcPrePeq);
    settings->setValue("cfc_curve_deg", m_cfcCurveDeg);
    settings->setValue("radio/txFullDuplex", m_txFullDuplex);
    settings->setValue("repeater_offset", m_repeaterOffset);
    settings->setValue("tx_filter_low", m_txFilterLow);
    settings->setValue("tx_filter_high", m_txFilterHigh);
    settings->setValue("tx_use_rx_filter", m_txUseRxFilter);

    for (int i = 0; i < kEqBands; ++i)
        settings->setValue(QStringLiteral("tx_eq_band_%1").arg(i), m_txEqBands.value(i));
    for (int i = 0; i < kCfcBands; ++i) {
        settings->setValue(QStringLiteral("cfc_lvl_%1").arg(i), m_cfcLevels.value(i));
        settings->setValue(QStringLiteral("cfc_post_%1").arg(i), m_cfcPost.value(i));
    }
}
