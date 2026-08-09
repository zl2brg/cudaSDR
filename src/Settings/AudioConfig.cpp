#include "AudioConfig.h"
#include <QSettings>
#include <QJsonArray>
#include "Util/settings_utils.h"

namespace {
int clampEqDeg(int deg)
{
    if (deg < 0) return 0;
    if (deg > 8) return 8;
    return deg;
}

double clampDb(double v, double lo, double hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

QJsonArray doublesToJson(const QVector<double> &v)
{
    QJsonArray arr;
    for (double x : v)
        arr.append(x);
    return arr;
}

QVector<double> doublesFromJson(const QJsonArray &arr, int n, double fill)
{
    QVector<double> out(n, fill);
    for (int i = 0; i < qMin(n, arr.size()); ++i)
        out[i] = arr.at(i).toDouble();
    return out;
}
}

AudioConfig::AudioConfig(QObject *parent)
    : QObject(parent)
    , m_micSource(1) // penelope
    , m_micInputDev(0)
    , m_digitalAudioInputDev(0)
    , m_micInputSourceName("")
    , m_digitalInputSourceName("")
    , m_micGain(10.0)
    , m_driveLevel(100)
    , m_fmPreemphasis(1)
    , m_phaseRotator(1)
    , m_phaseRotatorAuto(false)
    , m_amCarrierLevel(0.5)
    , m_audioCompression(0)
    , m_fmDeviation(5000.0)
    , m_ctcssToneHz(0)
    , m_rxEqEnabled(false)
    , m_rxEqBands(11, 0)
    , m_rxEqCurveDeg(0)
    , m_txEqEnabled(false)
    , m_txEqBands(11, 0)
    , m_txEqCurveDeg(0)
    , m_cfcEnabled(false)
    , m_cfcPeqEnabled(false)
    , m_cfcPrecomp(3.0)
    , m_cfcPrePeq(-9.0)
    , m_cfcCurveDeg(0)
    , m_emnrPost2Enabled(false)
    , m_emnrPost2Factor(15.0)
    , m_emnrPost2Nlevel(15.0)
    , m_emnrPost2Taper(12.0)
    , m_emnrPost2Rate(5.0)
    , m_mainVolume(0.1f)
{
    ensureCfcDefaults();
}

void AudioConfig::ensureCfcDefaults()
{
    // deskHPSDR-style voice CFC profile (10 bands; no unused slot-0).
    static const double kFreq[kCfcBands] = {
        50, 150, 300, 500, 750, 1250, 1750, 2300, 2800, 3100
    };
    static const double kLvl[kCfcBands] = {
        0, 0, 3, 3, 3, 6, 6, 6, 9, 9
    };
    m_cfcFreqs.resize(kCfcBands);
    m_cfcLevels.resize(kCfcBands);
    m_cfcPost.resize(kCfcBands);
    for (int i = 0; i < kCfcBands; ++i) {
        m_cfcFreqs[i] = kFreq[i];
        m_cfcLevels[i] = kLvl[i];
        m_cfcPost[i] = 0.0;
    }
}

void AudioConfig::setMicSource(int source) {
    if (m_micSource != source) {
        m_micSource = source;
        emit micSourceChanged(m_micSource);
    }
}

void AudioConfig::setMicInputDev(int dev) {
    if (m_micInputDev != dev) {
        m_micInputDev = dev;
        emit micInputDevChanged(m_micInputDev);
    }
}

void AudioConfig::setDigitalAudioInputDev(int dev) {
    if (m_digitalAudioInputDev != dev) {
        m_digitalAudioInputDev = dev;
        emit digitalAudioInputDevChanged(m_digitalAudioInputDev);
    }
}

void AudioConfig::setMicInputSourceName(const QString &name) {
    if (m_micInputSourceName != name) {
        m_micInputSourceName = name;
        emit micInputSourceNameChanged(m_micInputSourceName);
    }
}

void AudioConfig::setDigitalInputSourceName(const QString &name) {
    if (m_digitalInputSourceName != name) {
        m_digitalInputSourceName = name;
        emit digitalInputSourceNameChanged(m_digitalInputSourceName);
    }
}

void AudioConfig::setMicGain(double gain) {
    if (m_micGain != gain) {
        m_micGain = gain;
        emit micGainChanged(m_micGain);
    }
}

void AudioConfig::setDriveLevel(int level) {
    if (m_driveLevel != level) {
        m_driveLevel = level;
        emit driveLevelChanged(m_driveLevel);
    }
}

void AudioConfig::setFmPreemphasis(int val) {
    if (m_fmPreemphasis != val) {
        m_fmPreemphasis = val;
        emit fmPreemphasisChanged(m_fmPreemphasis);
    }
}

void AudioConfig::setPhaseRotator(int val) {
    int clamped = (val != 0) ? 1 : 0;
    if (m_phaseRotator != clamped) {
        m_phaseRotator = clamped;
        emit phaseRotatorChanged(m_phaseRotator);
    }
}

void AudioConfig::setPhaseRotatorAuto(bool enabled) {
    if (m_phaseRotatorAuto != enabled) {
        m_phaseRotatorAuto = enabled;
        emit phaseRotatorAutoChanged(m_phaseRotatorAuto);
    }
}

void AudioConfig::setAmCarrierLevel(double level) {
    if (m_amCarrierLevel != level) {
        m_amCarrierLevel = level;
        emit amCarrierLevelChanged(m_amCarrierLevel);
    }
}

void AudioConfig::setAudioCompression(int val) {
    if (m_audioCompression != val) {
        m_audioCompression = val;
        emit audioCompressionChanged(m_audioCompression);
    }
}

void AudioConfig::setFmDeviation(double dev) {
    if (m_fmDeviation != dev) {
        m_fmDeviation = dev;
        emit fmDeviationChanged(m_fmDeviation);
    }
}

void AudioConfig::setCtcssToneHz(int hz) {
    int clamped = hz;
    if (clamped < 0)
        clamped = 0;
    if (clamped > 1000)
        clamped = 1000;
    if (m_ctcssToneHz != clamped) {
        m_ctcssToneHz = clamped;
        emit ctcssToneHzChanged(m_ctcssToneHz);
    }
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
    for (int &g : next) {
        if (g < -12) g = -12;
        if (g > 12) g = 12;
    }
    if (next != m_rxEqBands) {
        m_rxEqBands = next;
        emit rxEqBandsChanged();
    }
}

void AudioConfig::setRxEqBand(int index, int gainDb) {
    if (index < 0 || index >= 11)
        return;
    int g = gainDb;
    if (g < -12) g = -12;
    if (g > 12) g = 12;
    if (m_rxEqBands.size() < 11)
        m_rxEqBands.resize(11);
    if (m_rxEqBands[index] != g) {
        m_rxEqBands[index] = g;
        emit rxEqBandsChanged();
    }
}

void AudioConfig::setRxEqCurveDeg(int deg) {
    deg = clampEqDeg(deg);
    if (m_rxEqCurveDeg != deg) {
        m_rxEqCurveDeg = deg;
        emit rxEqCurveDegChanged(m_rxEqCurveDeg);
    }
}

void AudioConfig::setTxEqEnabled(bool enabled) {
    if (m_txEqEnabled != enabled) {
        m_txEqEnabled = enabled;
        emit txEqEnabledChanged(m_txEqEnabled);
    }
}

void AudioConfig::setTxEqBands(const QVector<int> &bands) {
    QVector<int> next = bands;
    if (next.size() < 11)
        next.resize(11);
    for (int &g : next) {
        if (g < -12) g = -12;
        if (g > 12) g = 12;
    }
    if (next != m_txEqBands) {
        m_txEqBands = next;
        emit txEqBandsChanged();
    }
}

void AudioConfig::setTxEqBand(int index, int gainDb) {
    if (index < 0 || index >= 11)
        return;
    int g = gainDb;
    if (g < -12) g = -12;
    if (g > 12) g = 12;
    if (m_txEqBands.size() < 11)
        m_txEqBands.resize(11);
    if (m_txEqBands.at(index) != g) {
        m_txEqBands[index] = g;
        emit txEqBandsChanged();
    }
}

void AudioConfig::setTxEqCurveDeg(int deg) {
    deg = clampEqDeg(deg);
    if (m_txEqCurveDeg != deg) {
        m_txEqCurveDeg = deg;
        emit txEqCurveDegChanged(m_txEqCurveDeg);
    }
}

void AudioConfig::setCfcEnabled(bool enabled) {
    if (m_cfcEnabled != enabled) {
        m_cfcEnabled = enabled;
        emit cfcChanged();
    }
}

void AudioConfig::setCfcPeqEnabled(bool enabled) {
    if (m_cfcPeqEnabled != enabled) {
        m_cfcPeqEnabled = enabled;
        emit cfcChanged();
    }
}

void AudioConfig::setCfcPrecomp(double db) {
    db = clampDb(db, -20.0, 20.0);
    if (m_cfcPrecomp != db) {
        m_cfcPrecomp = db;
        emit cfcChanged();
    }
}

void AudioConfig::setCfcPrePeq(double db) {
    db = clampDb(db, -20.0, 20.0);
    if (m_cfcPrePeq != db) {
        m_cfcPrePeq = db;
        emit cfcChanged();
    }
}

void AudioConfig::setCfcLevel(int index, double db) {
    if (index < 0 || index >= kCfcBands)
        return;
    db = clampDb(db, -20.0, 20.0);
    if (m_cfcLevels.size() < kCfcBands)
        ensureCfcDefaults();
    if (m_cfcLevels.at(index) != db) {
        m_cfcLevels[index] = db;
        emit cfcChanged();
    }
}

void AudioConfig::setCfcPost(int index, double db) {
    if (index < 0 || index >= kCfcBands)
        return;
    db = clampDb(db, -20.0, 20.0);
    if (m_cfcPost.size() < kCfcBands)
        ensureCfcDefaults();
    if (m_cfcPost.at(index) != db) {
        m_cfcPost[index] = db;
        emit cfcChanged();
    }
}

void AudioConfig::setCfcCurveDeg(int deg) {
    deg = clampEqDeg(deg);
    if (m_cfcCurveDeg != deg) {
        m_cfcCurveDeg = deg;
        emit cfcChanged();
    }
}

void AudioConfig::setEmnrPost2Enabled(bool enabled) {
    if (m_emnrPost2Enabled != enabled) {
        m_emnrPost2Enabled = enabled;
        emit emnrPost2Changed();
    }
}

void AudioConfig::setEmnrPost2Factor(double pct) {
    pct = clampDb(pct, 0.0, 100.0);
    if (m_emnrPost2Factor != pct) {
        m_emnrPost2Factor = pct;
        emit emnrPost2Changed();
    }
}

void AudioConfig::setEmnrPost2Nlevel(double pct) {
    pct = clampDb(pct, 0.0, 100.0);
    if (m_emnrPost2Nlevel != pct) {
        m_emnrPost2Nlevel = pct;
        emit emnrPost2Changed();
    }
}

void AudioConfig::setEmnrPost2Taper(double pct) {
    pct = clampDb(pct, 0.0, 100.0);
    if (m_emnrPost2Taper != pct) {
        m_emnrPost2Taper = pct;
        emit emnrPost2Changed();
    }
}

void AudioConfig::setEmnrPost2Rate(double seconds) {
    if (seconds < 0.2) seconds = 0.2;
    if (seconds > 20.0) seconds = 20.0;
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
    if (json.contains("micSource")) m_micSource = json["micSource"].toInt();
    if (json.contains("micInputDev")) m_micInputDev = json["micInputDev"].toInt();
    if (json.contains("digitalAudioInputDev")) m_digitalAudioInputDev = json["digitalAudioInputDev"].toInt();
    if (json.contains("micInputSourceName")) m_micInputSourceName = json["micInputSourceName"].toString();
    if (json.contains("digitalInputSourceName")) m_digitalInputSourceName = json["digitalInputSourceName"].toString();
    if (json.contains("micGain")) m_micGain = json["micGain"].toDouble();
    if (json.contains("driveLevel")) m_driveLevel = json["driveLevel"].toInt();
    if (json.contains("fmPreemphasis")) m_fmPreemphasis = json["fmPreemphasis"].toInt();
    if (json.contains("phaseRotator")) m_phaseRotator = json["phaseRotator"].toInt();
    if (json.contains("phaseRotatorAuto")) m_phaseRotatorAuto = json["phaseRotatorAuto"].toBool();
    if (json.contains("amCarrierLevel")) m_amCarrierLevel = json["amCarrierLevel"].toDouble();
    if (json.contains("audioCompression")) m_audioCompression = json["audioCompression"].toInt();
    if (json.contains("fmDeviation")) m_fmDeviation = json["fmDeviation"].toDouble();
    if (json.contains("ctcssToneHz")) m_ctcssToneHz = json["ctcssToneHz"].toInt();
    if (json.contains("rxEqEnabled")) m_rxEqEnabled = json["rxEqEnabled"].toBool();
    if (json.contains("rxEqBands") && json["rxEqBands"].isArray()) {
        const QJsonArray arr = json["rxEqBands"].toArray();
        m_rxEqBands = QVector<int>(11, 0);
        for (int i = 0; i < qMin(11, arr.size()); ++i)
            m_rxEqBands[i] = arr.at(i).toInt();
    }
    if (json.contains("rxEqCurveDeg")) m_rxEqCurveDeg = clampEqDeg(json["rxEqCurveDeg"].toInt());
    if (json.contains("txEqEnabled")) m_txEqEnabled = json["txEqEnabled"].toBool();
    if (json.contains("txEqBands") && json["txEqBands"].isArray()) {
        const QJsonArray arr = json["txEqBands"].toArray();
        m_txEqBands = QVector<int>(11, 0);
        for (int i = 0; i < qMin(11, arr.size()); ++i)
            m_txEqBands[i] = arr.at(i).toInt();
    }
    if (json.contains("txEqCurveDeg")) m_txEqCurveDeg = clampEqDeg(json["txEqCurveDeg"].toInt());
    if (json.contains("cfcEnabled")) m_cfcEnabled = json["cfcEnabled"].toBool();
    if (json.contains("cfcPeqEnabled")) m_cfcPeqEnabled = json["cfcPeqEnabled"].toBool();
    if (json.contains("cfcPrecomp")) m_cfcPrecomp = json["cfcPrecomp"].toDouble();
    if (json.contains("cfcPrePeq")) m_cfcPrePeq = json["cfcPrePeq"].toDouble();
    if (json.contains("cfcCurveDeg")) m_cfcCurveDeg = clampEqDeg(json["cfcCurveDeg"].toInt());
    if (json.contains("cfcLevels") && json["cfcLevels"].isArray())
        m_cfcLevels = doublesFromJson(json["cfcLevels"].toArray(), kCfcBands, 0.0);
    if (json.contains("cfcPost") && json["cfcPost"].isArray())
        m_cfcPost = doublesFromJson(json["cfcPost"].toArray(), kCfcBands, 0.0);
    // Freqs are fixed defaults; ignore legacy arrays that disagree in length.
    if (m_cfcFreqs.size() != kCfcBands || m_cfcLevels.size() != kCfcBands || m_cfcPost.size() != kCfcBands)
        ensureCfcDefaults();
    if (json.contains("emnrPost2Enabled")) m_emnrPost2Enabled = json["emnrPost2Enabled"].toBool();
    if (json.contains("emnrPost2Factor")) m_emnrPost2Factor = json["emnrPost2Factor"].toDouble();
    if (json.contains("emnrPost2Nlevel")) m_emnrPost2Nlevel = json["emnrPost2Nlevel"].toDouble();
    if (json.contains("emnrPost2Taper")) m_emnrPost2Taper = json["emnrPost2Taper"].toDouble();
    if (json.contains("emnrPost2Rate")) m_emnrPost2Rate = json["emnrPost2Rate"].toDouble();
    if (json.contains("mainVolume")) m_mainVolume = static_cast<float>(json["mainVolume"].toDouble());
}

void AudioConfig::save(QJsonObject &json) const {
    json["micSource"] = m_micSource;
    json["micInputDev"] = m_micInputDev;
    json["digitalAudioInputDev"] = m_digitalAudioInputDev;
    json["micInputSourceName"] = m_micInputSourceName;
    json["digitalInputSourceName"] = m_digitalInputSourceName;
    json["micGain"] = m_micGain;
    json["driveLevel"] = m_driveLevel;
    json["fmPreemphasis"] = m_fmPreemphasis;
    json["phaseRotator"] = m_phaseRotator;
    json["phaseRotatorAuto"] = m_phaseRotatorAuto;
    json["amCarrierLevel"] = m_amCarrierLevel;
    json["audioCompression"] = m_audioCompression;
    json["fmDeviation"] = m_fmDeviation;
    json["ctcssToneHz"] = m_ctcssToneHz;
    json["rxEqEnabled"] = m_rxEqEnabled;
    {
        QJsonArray arr;
        for (int g : m_rxEqBands)
            arr.append(g);
        json["rxEqBands"] = arr;
    }
    json["rxEqCurveDeg"] = m_rxEqCurveDeg;
    json["txEqEnabled"] = m_txEqEnabled;
    {
        QJsonArray arr;
        for (int g : m_txEqBands)
            arr.append(g);
        json["txEqBands"] = arr;
    }
    json["txEqCurveDeg"] = m_txEqCurveDeg;
    json["cfcEnabled"] = m_cfcEnabled;
    json["cfcPeqEnabled"] = m_cfcPeqEnabled;
    json["cfcPrecomp"] = m_cfcPrecomp;
    json["cfcPrePeq"] = m_cfcPrePeq;
    json["cfcCurveDeg"] = m_cfcCurveDeg;
    json["cfcLevels"] = doublesToJson(m_cfcLevels);
    json["cfcPost"] = doublesToJson(m_cfcPost);
    json["emnrPost2Enabled"] = m_emnrPost2Enabled;
    json["emnrPost2Factor"] = m_emnrPost2Factor;
    json["emnrPost2Nlevel"] = m_emnrPost2Nlevel;
    json["emnrPost2Taper"] = m_emnrPost2Taper;
    json["emnrPost2Rate"] = m_emnrPost2Rate;
    json["mainVolume"] = static_cast<double>(m_mainVolume);
}

void AudioConfig::loadIni(QSettings *settings) {
    QString str = settings->value("server/mic_source", "penelope").toString();
    int val = (str == "janus") ? 0 : 1;
    setMicSource(val);

    setMicInputDev(settings->value("mic_InputDevice", 0).toInt());
    setDigitalAudioInputDev(settings->value("digital_audio_InputDevice", 0).toInt());

    setMicInputSourceName(settings->value("mic_input_source",
                                          (micInputDev() > 0) ? QString("default") : QString()).toString());
    setDigitalInputSourceName(settings->value("digital_input_source",
                                              (digitalAudioInputDev() > 0) ? QString("default") : QString("none")).toString());

    if (micInputSourceName().isEmpty()) {
        setMicInputSourceName("default");
        setMicInputDev(1);
    }
    if (digitalInputSourceName().isEmpty()) {
        setDigitalInputSourceName((digitalAudioInputDev() > 0) ? QString("default") : QString("none"));
    }

    setMicGain(settings->value("micGain", 0).toDouble());
    setDriveLevel(SettingsUtils::clampDriveLevel(settings->value("driveLevel", 0).toInt()));

    setFmPreemphasis(settings->value("fm_preemphesize", 1).toInt());
    setPhaseRotator(settings->value("audio_phase_rotator", 1).toInt());
    setPhaseRotatorAuto(settings->value("audio_phase_rotator_auto", false).toBool());
    setAmCarrierLevel(settings->value("am_carrierlevel", 0.5).toDouble());
    setAudioCompression(settings->value("audiocompression", 0).toInt());
    setFmDeviation(settings->value("fmdeveation", 5000.0).toDouble());
    setCtcssToneHz(settings->value("ctcss_tone_hz", 0).toInt());
    setRxEqEnabled(settings->value("rx_eq_enabled", false).toBool());
    {
        QVector<int> bands(11, 0);
        for (int i = 0; i < 11; ++i)
            bands[i] = settings->value(QStringLiteral("rx_eq_band_%1").arg(i), 0).toInt();
        setRxEqBands(bands);
    }
    setRxEqCurveDeg(settings->value("rx_eq_curve_deg", 0).toInt());
    setTxEqEnabled(settings->value("tx_eq_enabled", false).toBool());
    {
        QVector<int> bands(11, 0);
        for (int i = 0; i < 11; ++i)
            bands[i] = settings->value(QStringLiteral("tx_eq_band_%1").arg(i), 0).toInt();
        setTxEqBands(bands);
    }
    setTxEqCurveDeg(settings->value("tx_eq_curve_deg", 0).toInt());

    setCfcEnabled(settings->value("cfc_enabled", false).toBool());
    setCfcPeqEnabled(settings->value("cfc_peq_enabled", false).toBool());
    setCfcPrecomp(settings->value("cfc_precomp", 3.0).toDouble());
    setCfcPrePeq(settings->value("cfc_prepeq", -9.0).toDouble());
    setCfcCurveDeg(settings->value("cfc_curve_deg", 0).toInt());
    for (int i = 0; i < kCfcBands; ++i) {
        setCfcLevel(i, settings->value(QStringLiteral("cfc_lvl_%1").arg(i), m_cfcLevels.value(i)).toDouble());
        setCfcPost(i, settings->value(QStringLiteral("cfc_post_%1").arg(i), m_cfcPost.value(i)).toDouble());
    }

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
    if (m_micSource == 0)
        settings->setValue("server/mic_source", "janus");
    else
        settings->setValue("server/mic_source", "penelope");

    settings->setValue("mic_InputDevice", m_micInputDev);
    settings->setValue("mic_input_source", m_micInputSourceName);
    settings->setValue("digital_audio_InputDevice", m_digitalAudioInputDev);
    settings->setValue("digital_input_source", m_digitalInputSourceName);
    settings->setValue("micGain", m_micGain);
    settings->setValue("driveLevel", m_driveLevel);
    settings->setValue("fm_preemphesize", m_fmPreemphasis);
    settings->setValue("audio_phase_rotator", m_phaseRotator);
    settings->setValue("audio_phase_rotator_auto", m_phaseRotatorAuto);
    settings->setValue("am_carrierlevel", m_amCarrierLevel);
    settings->setValue("audiocompression", m_audioCompression);
    settings->setValue("fmdeveation", m_fmDeviation);
    settings->setValue("ctcss_tone_hz", m_ctcssToneHz);
    settings->setValue("rx_eq_enabled", m_rxEqEnabled);
    for (int i = 0; i < m_rxEqBands.size() && i < 11; ++i)
        settings->setValue(QStringLiteral("rx_eq_band_%1").arg(i), m_rxEqBands.at(i));
    settings->setValue("rx_eq_curve_deg", m_rxEqCurveDeg);
    settings->setValue("tx_eq_enabled", m_txEqEnabled);
    for (int i = 0; i < m_txEqBands.size() && i < 11; ++i)
        settings->setValue(QStringLiteral("tx_eq_band_%1").arg(i), m_txEqBands.at(i));
    settings->setValue("tx_eq_curve_deg", m_txEqCurveDeg);
    settings->setValue("cfc_enabled", m_cfcEnabled);
    settings->setValue("cfc_peq_enabled", m_cfcPeqEnabled);
    settings->setValue("cfc_precomp", m_cfcPrecomp);
    settings->setValue("cfc_prepeq", m_cfcPrePeq);
    settings->setValue("cfc_curve_deg", m_cfcCurveDeg);
    for (int i = 0; i < kCfcBands; ++i) {
        settings->setValue(QStringLiteral("cfc_lvl_%1").arg(i), m_cfcLevels.value(i));
        settings->setValue(QStringLiteral("cfc_post_%1").arg(i), m_cfcPost.value(i));
    }
    settings->setValue("emnr_post2_enabled", m_emnrPost2Enabled);
    settings->setValue("emnr_post2_factor", m_emnrPost2Factor);
    settings->setValue("emnr_post2_nlevel", m_emnrPost2Nlevel);
    settings->setValue("emnr_post2_taper", m_emnrPost2Taper);
    settings->setValue("emnr_post2_rate", m_emnrPost2Rate);
    settings->setValue("server/mainVolume", static_cast<int>(m_mainVolume * 100));
}
