#include "AlexConfig.h"
#include <QSettings>
#include "Util/settings_utils.h"

int AlexConfig::normalizedState(int state)
{
    if ((state & 0x3) == 0)
        state |= 1;
    if (((state >> 5) & 0x3) == 0)
        state |= 33;
    return state;
}

AlexConfig::AlexConfig(QObject *parent)
    : QObject(parent)
    , m_manualFilterSelect(false)
    , m_bypassAll(false)
    , m_amp6m(false)
    , m_hpf1_5MHz(false)
    , m_hpf6_5MHz(false)
    , m_hpf9_5MHz(false)
    , m_hpf13MHz(false)
    , m_hpf20MHz(false)
    , m_lpf160m(false)
    , m_lpf80m(false)
    , m_lpf60_40m(false)
    , m_lpf30_20m(false)
    , m_lpf17_15m(false)
    , m_lpf12_10m(false)
    , m_lpf6m(false)
    , m_alexConfig(0)
    , m_attenuation(0)
{
    m_hpfLoFreqs << 1500000L << 7000000L << 10100000L << 14000000L << 21000000L << 50000000L;
    m_hpfHiFreqs << 5500000L << 7300000L << 10150000L << 18168000L << 29700000L << 54000000L;
    m_lpfLoFreqs << 1800000L << 3500000L << 5330000L << 10100000L << 18068000L << 24890000L << 50000000L;
    m_lpfHiFreqs << 2000000L << 4000000L << 7300000L << 14350000L << 21450000L << 29700000L << 54000000L;

    for (int i = 0; i < kAlexStateCount; ++i)
        m_alexStates << 0;
}

void AlexConfig::updateBitmask() {
    quint16 mask = 0;
    if (m_manualFilterSelect) mask |= 0x01;
    if (m_bypassAll)          mask |= 0x02;
    if (m_amp6m)              mask |= 0x04;
    if (m_hpf1_5MHz)          mask |= 0x08;
    if (m_hpf6_5MHz)          mask |= 0x10;
    if (m_hpf9_5MHz)          mask |= 0x20;
    if (m_hpf13MHz)           mask |= 0x40;
    if (m_hpf20MHz)           mask |= 0x80;
    if (m_lpf160m)            mask |= 0x100;
    if (m_lpf80m)             mask |= 0x200;
    if (m_lpf60_40m)          mask |= 0x400;
    if (m_lpf30_20m)          mask |= 0x800;
    if (m_lpf17_15m)          mask |= 0x1000;
    if (m_lpf12_10m)          mask |= 0x2000;
    if (m_lpf6m)              mask |= 0x4000;
    if (m_alexConfig != mask) {
        m_alexConfig = mask;
        emit alexConfigChanged(m_alexConfig);
    }
}

void AlexConfig::applyBitmask(quint16 config, bool emitFlags) {
    const bool manual = (config & 0x01) != 0;
    const bool bypass = (config & 0x02) != 0;
    const bool amp = (config & 0x04) != 0;
    const bool h15 = (config & 0x08) != 0;
    const bool h65 = (config & 0x10) != 0;
    const bool h95 = (config & 0x20) != 0;
    const bool h13 = (config & 0x40) != 0;
    const bool h20 = (config & 0x80) != 0;
    const bool l160 = (config & 0x100) != 0;
    const bool l80 = (config & 0x200) != 0;
    const bool l40 = (config & 0x400) != 0;
    const bool l20 = (config & 0x800) != 0;
    const bool l15 = (config & 0x1000) != 0;
    const bool l10 = (config & 0x2000) != 0;
    const bool l6 = (config & 0x4000) != 0;

    if (emitFlags) {
        if (m_manualFilterSelect != manual) emit manualFilterSelectChanged(manual);
        if (m_bypassAll != bypass) emit bypassAllChanged(bypass);
        if (m_amp6m != amp) emit amp6mChanged(amp);
        if (m_hpf1_5MHz != h15) emit hpf1_5MHzChanged(h15);
        if (m_hpf6_5MHz != h65) emit hpf6_5MHzChanged(h65);
        if (m_hpf9_5MHz != h95) emit hpf9_5MHzChanged(h95);
        if (m_hpf13MHz != h13) emit hpf13MHzChanged(h13);
        if (m_hpf20MHz != h20) emit hpf20MHzChanged(h20);
        if (m_lpf160m != l160) emit lpf160mChanged(l160);
        if (m_lpf80m != l80) emit lpf80mChanged(l80);
        if (m_lpf60_40m != l40) emit lpf60_40mChanged(l40);
        if (m_lpf30_20m != l20) emit lpf30_20mChanged(l20);
        if (m_lpf17_15m != l15) emit lpf17_15mChanged(l15);
        if (m_lpf12_10m != l10) emit lpf12_10mChanged(l10);
        if (m_lpf6m != l6) emit lpf6mChanged(l6);
    }

    m_manualFilterSelect = manual;
    m_bypassAll = bypass;
    m_amp6m = amp;
    m_hpf1_5MHz = h15;
    m_hpf6_5MHz = h65;
    m_hpf9_5MHz = h95;
    m_hpf13MHz = h13;
    m_hpf20MHz = h20;
    m_lpf160m = l160;
    m_lpf80m = l80;
    m_lpf60_40m = l40;
    m_lpf30_20m = l20;
    m_lpf17_15m = l15;
    m_lpf12_10m = l10;
    m_lpf6m = l6;
    m_alexConfig = config;
}

void AlexConfig::setFlag(bool &field, bool enabled, void (AlexConfig::*signal)(bool)) {
    if (field == enabled)
        return;
    field = enabled;
    updateBitmask();
    emit (this->*signal)(field);
}

void AlexConfig::setManualFilterSelect(bool manual) {
    setFlag(m_manualFilterSelect, manual, &AlexConfig::manualFilterSelectChanged);
}

void AlexConfig::setBypassAll(bool bypass) {
    setFlag(m_bypassAll, bypass, &AlexConfig::bypassAllChanged);
}

void AlexConfig::setAmp6m(bool enabled) {
    setFlag(m_amp6m, enabled, &AlexConfig::amp6mChanged);
}

void AlexConfig::setHpf1_5MHz(bool enabled) {
    setFlag(m_hpf1_5MHz, enabled, &AlexConfig::hpf1_5MHzChanged);
}

void AlexConfig::setHpf6_5MHz(bool enabled) {
    setFlag(m_hpf6_5MHz, enabled, &AlexConfig::hpf6_5MHzChanged);
}

void AlexConfig::setHpf9_5MHz(bool enabled) {
    setFlag(m_hpf9_5MHz, enabled, &AlexConfig::hpf9_5MHzChanged);
}

void AlexConfig::setHpf13MHz(bool enabled) {
    setFlag(m_hpf13MHz, enabled, &AlexConfig::hpf13MHzChanged);
}

void AlexConfig::setHpf20MHz(bool enabled) {
    setFlag(m_hpf20MHz, enabled, &AlexConfig::hpf20MHzChanged);
}

void AlexConfig::setLpf160m(bool enabled) {
    setFlag(m_lpf160m, enabled, &AlexConfig::lpf160mChanged);
}

void AlexConfig::setLpf80m(bool enabled) {
    setFlag(m_lpf80m, enabled, &AlexConfig::lpf80mChanged);
}

void AlexConfig::setLpf60_40m(bool enabled) {
    setFlag(m_lpf60_40m, enabled, &AlexConfig::lpf60_40mChanged);
}

void AlexConfig::setLpf30_20m(bool enabled) {
    setFlag(m_lpf30_20m, enabled, &AlexConfig::lpf30_20mChanged);
}

void AlexConfig::setLpf17_15m(bool enabled) {
    setFlag(m_lpf17_15m, enabled, &AlexConfig::lpf17_15mChanged);
}

void AlexConfig::setLpf12_10m(bool enabled) {
    setFlag(m_lpf12_10m, enabled, &AlexConfig::lpf12_10mChanged);
}

void AlexConfig::setLpf6m(bool enabled) {
    setFlag(m_lpf6m, enabled, &AlexConfig::lpf6mChanged);
}

void AlexConfig::setAlexConfig(quint16 config) {
    if (m_alexConfig != config) {
        applyBitmask(config, true);
        emit alexConfigChanged(m_alexConfig);
    }
}

void AlexConfig::setAttenuation(int attn) {
    const int val = qBound(0, attn, 3);
    if (m_attenuation != val) {
        m_attenuation = val;
        emit attenuationChanged(m_attenuation);
    }
}

void AlexConfig::setAlexStates(const QList<int> &states) {
    QList<int> next = states;
    for (int &s : next)
        s = normalizedState(s);
    while (next.size() < kAlexStateCount)
        next.append(0);
    if (m_alexStates == next)
        return;
    m_alexStates = next;
    emit alexStatesChanged();
}

void AlexConfig::setAlexState(int pos, int value) {
    if (pos < 0)
        return;
    value = normalizedState(value);
    while (m_alexStates.size() <= pos)
        m_alexStates.append(0);
    if (m_alexStates.at(pos) == value)
        return;
    m_alexStates[pos] = value;
    emit alexStatesChanged();
}

void AlexConfig::setHpfLoFrequency(int filter, long freq) {
    if (filter >= 0 && filter < m_hpfLoFreqs.size()) {
        m_hpfLoFreqs[filter] = freq;
        emit hpfFrequenciesChanged();
    }
}

void AlexConfig::setHpfHiFrequency(int filter, long freq) {
    if (filter >= 0 && filter < m_hpfHiFreqs.size()) {
        m_hpfHiFreqs[filter] = freq;
        emit hpfFrequenciesChanged();
    }
}

void AlexConfig::setLpfLoFrequency(int filter, long freq) {
    if (filter >= 0 && filter < m_lpfLoFreqs.size()) {
        m_lpfLoFreqs[filter] = freq;
        emit lpfFrequenciesChanged();
    }
}

void AlexConfig::setLpfHiFrequency(int filter, long freq) {
    if (filter >= 0 && filter < m_lpfHiFreqs.size()) {
        m_lpfHiFreqs[filter] = freq;
        emit lpfFrequenciesChanged();
    }
}

void AlexConfig::load(const QJsonObject &json) {
    if (json.contains("alexConfig")) {
        setAlexConfig(static_cast<quint16>(json["alexConfig"].toInt()));
    } else {
        if (json.contains("manualFilterSelect")) setManualFilterSelect(json["manualFilterSelect"].toBool());
        if (json.contains("bypassAll")) setBypassAll(json["bypassAll"].toBool());
        if (json.contains("amp6m")) setAmp6m(json["amp6m"].toBool());
        if (json.contains("hpf1_5MHz")) setHpf1_5MHz(json["hpf1_5MHz"].toBool());
        if (json.contains("hpf6_5MHz")) setHpf6_5MHz(json["hpf6_5MHz"].toBool());
        if (json.contains("hpf9_5MHz")) setHpf9_5MHz(json["hpf9_5MHz"].toBool());
        if (json.contains("hpf13MHz")) setHpf13MHz(json["hpf13MHz"].toBool());
        if (json.contains("hpf20MHz")) setHpf20MHz(json["hpf20MHz"].toBool());
        if (json.contains("lpf160m")) setLpf160m(json["lpf160m"].toBool());
        if (json.contains("lpf80m")) setLpf80m(json["lpf80m"].toBool());
        if (json.contains("lpf60_40m")) setLpf60_40m(json["lpf60_40m"].toBool());
        if (json.contains("lpf30_20m")) setLpf30_20m(json["lpf30_20m"].toBool());
        if (json.contains("lpf17_15m")) setLpf17_15m(json["lpf17_15m"].toBool());
        if (json.contains("lpf12_10m")) setLpf12_10m(json["lpf12_10m"].toBool());
        if (json.contains("lpf6m")) setLpf6m(json["lpf6m"].toBool());
    }
    if (json.contains("attenuation")) setAttenuation(json["attenuation"].toInt());

    SettingsUtils::applyJsonArray<long>(json, QStringLiteral("hpfLoFreqs"), m_hpfLoFreqs.size(),
                                        [this](int i, long f) { setHpfLoFrequency(i, f); });
    SettingsUtils::applyJsonArray<long>(json, QStringLiteral("hpfHiFreqs"), m_hpfHiFreqs.size(),
                                        [this](int i, long f) { setHpfHiFrequency(i, f); });
    SettingsUtils::applyJsonArray<long>(json, QStringLiteral("lpfLoFreqs"), m_lpfLoFreqs.size(),
                                        [this](int i, long f) { setLpfLoFrequency(i, f); });
    SettingsUtils::applyJsonArray<long>(json, QStringLiteral("lpfHiFreqs"), m_lpfHiFreqs.size(),
                                        [this](int i, long f) { setLpfHiFrequency(i, f); });
    if (json.contains("alexStates") && json["alexStates"].isArray()) {
        QList<int> states;
        SettingsUtils::applyJsonArray<int>(json, QStringLiteral("alexStates"), json["alexStates"].toArray().size(),
                                           [&](int, int v) { states.append(v); });
        setAlexStates(states);
    }
}

void AlexConfig::save(QJsonObject &json) const {
    json["manualFilterSelect"] = m_manualFilterSelect;
    json["bypassAll"] = m_bypassAll;
    json["amp6m"] = m_amp6m;
    json["hpf1_5MHz"] = m_hpf1_5MHz;
    json["hpf6_5MHz"] = m_hpf6_5MHz;
    json["hpf9_5MHz"] = m_hpf9_5MHz;
    json["hpf13MHz"] = m_hpf13MHz;
    json["hpf20MHz"] = m_hpf20MHz;
    json["lpf160m"] = m_lpf160m;
    json["lpf80m"] = m_lpf80m;
    json["lpf60_40m"] = m_lpf60_40m;
    json["lpf30_20m"] = m_lpf30_20m;
    json["lpf17_15m"] = m_lpf17_15m;
    json["lpf12_10m"] = m_lpf12_10m;
    json["lpf6m"] = m_lpf6m;
    json["alexConfig"] = static_cast<int>(m_alexConfig);
    json["attenuation"] = m_attenuation;
    json["hpfLoFreqs"] = SettingsUtils::toJsonArray(m_hpfLoFreqs);
    json["hpfHiFreqs"] = SettingsUtils::toJsonArray(m_hpfHiFreqs);
    json["lpfLoFreqs"] = SettingsUtils::toJsonArray(m_lpfLoFreqs);
    json["lpfHiFreqs"] = SettingsUtils::toJsonArray(m_lpfHiFreqs);
    json["alexStates"] = SettingsUtils::toJsonArray(m_alexStates);
}

void AlexConfig::loadIni(QSettings *settings) {
    setManualFilterSelect(SettingsUtils::iniOn(settings, QStringLiteral("alex/manual")));
    setBypassAll(SettingsUtils::iniOn(settings, QStringLiteral("alex/bypassAll")));
    setAmp6m(SettingsUtils::iniOn(settings, QStringLiteral("alex/amp6m")));
    setHpf1_5MHz(SettingsUtils::iniOn(settings, QStringLiteral("alex/hpf1_5MHz")));
    setHpf6_5MHz(SettingsUtils::iniOn(settings, QStringLiteral("alex/hpf6_5MHz")));
    setHpf9_5MHz(SettingsUtils::iniOn(settings, QStringLiteral("alex/hpf9_5MHz")));
    setHpf13MHz(SettingsUtils::iniOn(settings, QStringLiteral("alex/hpf13MHz")));
    setHpf20MHz(SettingsUtils::iniOn(settings, QStringLiteral("alex/hpf20MHz")));
    setLpf160m(SettingsUtils::iniOn(settings, QStringLiteral("alex/lpf160m")));
    setLpf80m(SettingsUtils::iniOn(settings, QStringLiteral("alex/lpf80m")));
    setLpf60_40m(SettingsUtils::iniOn(settings, QStringLiteral("alex/lpf60_40m")));
    setLpf30_20m(SettingsUtils::iniOn(settings, QStringLiteral("alex/lpf30_20m")));
    setLpf17_15m(SettingsUtils::iniOn(settings, QStringLiteral("alex/lpf17_15m")));
    setLpf12_10m(SettingsUtils::iniOn(settings, QStringLiteral("alex/lpf12_10m")));
    setLpf6m(SettingsUtils::iniOn(settings, QStringLiteral("alex/lpf6m")));

    setHpfLoFrequency(5, SettingsUtils::clampFreq(settings->value("alex/amp6mLo", 50000000).toDouble(), 49000000, 52500000, 50000000));
    setHpfHiFrequency(5, SettingsUtils::clampFreq(settings->value("alex/amp6mHi", 54000000).toDouble(), 52500000, 55000000, 54000000));
    setHpfLoFrequency(0, SettingsUtils::clampFreq(settings->value("alex/hpf1_5MHzLo", 1500000).toDouble(), 0, 2000000, 1500000));
    setHpfHiFrequency(0, SettingsUtils::clampFreq(settings->value("alex/hpf1_5MHzHi", 5500000).toDouble(), 1600000, 6000000, 5500000));
    setHpfLoFrequency(1, SettingsUtils::clampFreq(settings->value("alex/hpf6_5MHzLo", 7000000).toDouble(), 6000000, 8000000, 7000000));
    setHpfHiFrequency(1, SettingsUtils::clampFreq(settings->value("alex/hpf6_5MHzHi", 7300000).toDouble(), 7000000, 9500000, 7300000));
    setHpfLoFrequency(2, SettingsUtils::clampFreq(settings->value("alex/hpf9_5MHzLo", 10100000).toDouble(), 9000000, 11000000, 10100000));
    setHpfHiFrequency(2, SettingsUtils::clampFreq(settings->value("alex/hpf9_5MHzHi", 10150000).toDouble(), 10000000, 13000000, 10150000));
    setHpfLoFrequency(3, SettingsUtils::clampFreq(settings->value("alex/hpf13MHzLo", 14000000).toDouble(), 12000000, 15000000, 14000000));
    setHpfHiFrequency(3, SettingsUtils::clampFreq(settings->value("alex/hpf13MHzHi", 18168000).toDouble(), 13700000, 19000000, 18168000));
    setHpfLoFrequency(4, SettingsUtils::clampFreq(settings->value("alex/hpf20MHzLo", 21000000).toDouble(), 18000000, 25000000, 21000000));
    setHpfHiFrequency(4, SettingsUtils::clampFreq(settings->value("alex/hpf20MHzHi", 29700000).toDouble(), 25000000, 32000000, 29700000));

    setLpfLoFrequency(0, SettingsUtils::clampFreq(settings->value("alex/lpf160mLo", 1800000).toDouble(), 0, 1900000, 1800000));
    setLpfHiFrequency(0, SettingsUtils::clampFreq(settings->value("alex/lpf160mHi", 2000000).toDouble(), 1000000, 3000000, 2000000));
    setLpfLoFrequency(1, SettingsUtils::clampFreq(settings->value("alex/lpf80mLo", 3500000).toDouble(), 2000000, 4000000, 3500000));
    setLpfHiFrequency(1, SettingsUtils::clampFreq(settings->value("alex/lpf80mHi", 4000000).toDouble(), 2000000, 5000000, 4000000));
    setLpfLoFrequency(2, SettingsUtils::clampFreq(settings->value("alex/lpf60_40mLo", 5330000).toDouble(), 5000000, 11000000, 5330000));
    setLpfHiFrequency(2, SettingsUtils::clampFreq(settings->value("alex/lpf60_40mHi", 7300000).toDouble(), 5000000, 8000000, 7300000));
    setLpfLoFrequency(3, SettingsUtils::clampFreq(settings->value("alex/lpf30_20mLo", 10100000).toDouble(), 9000000, 15000000, 10100000));
    setLpfHiFrequency(3, SettingsUtils::clampFreq(settings->value("alex/lpf30_20mHi", 14350000).toDouble(), 9000000, 15000000, 14350000));
    setLpfLoFrequency(4, SettingsUtils::clampFreq(settings->value("alex/lpf17_15mLo", 18068000).toDouble(), 17000000, 22000000, 18068000));
    setLpfHiFrequency(4, SettingsUtils::clampFreq(settings->value("alex/lpf17_15mHi", 21450000).toDouble(), 17000000, 22000000, 21450000));
    setLpfLoFrequency(5, SettingsUtils::clampFreq(settings->value("alex/lpf12_10mLo", 24890000).toDouble(), 23000000, 30000000, 24890000));
    setLpfHiFrequency(5, SettingsUtils::clampFreq(settings->value("alex/lpf12_10mHi", 29700000).toDouble(), 23000000, 30000000, 29700000));
    setLpfLoFrequency(6, SettingsUtils::clampFreq(settings->value("alex/lpf6mLo", 50000000).toDouble(), 30000000, 52000000, 50000000));
    setLpfHiFrequency(6, SettingsUtils::clampFreq(settings->value("alex/lpf6mHi", 54000000).toDouble(), 52000000, 66000000, 54000000));

    setAttenuation(settings->value("alex/attenuation", 0).toInt());
}

void AlexConfig::saveIni(QSettings *settings) const {
    SettingsUtils::setIniOn(settings, QStringLiteral("alex/manual"), m_manualFilterSelect);
    SettingsUtils::setIniOn(settings, QStringLiteral("alex/bypassAll"), m_bypassAll);
    SettingsUtils::setIniOn(settings, QStringLiteral("alex/amp6m"), m_amp6m);
    SettingsUtils::setIniOn(settings, QStringLiteral("alex/hpf1_5MHz"), m_hpf1_5MHz);
    SettingsUtils::setIniOn(settings, QStringLiteral("alex/hpf6_5MHz"), m_hpf6_5MHz);
    SettingsUtils::setIniOn(settings, QStringLiteral("alex/hpf9_5MHz"), m_hpf9_5MHz);
    SettingsUtils::setIniOn(settings, QStringLiteral("alex/hpf13MHz"), m_hpf13MHz);
    SettingsUtils::setIniOn(settings, QStringLiteral("alex/hpf20MHz"), m_hpf20MHz);
    SettingsUtils::setIniOn(settings, QStringLiteral("alex/lpf160m"), m_lpf160m);
    SettingsUtils::setIniOn(settings, QStringLiteral("alex/lpf80m"), m_lpf80m);
    SettingsUtils::setIniOn(settings, QStringLiteral("alex/lpf60_40m"), m_lpf60_40m);
    SettingsUtils::setIniOn(settings, QStringLiteral("alex/lpf30_20m"), m_lpf30_20m);
    SettingsUtils::setIniOn(settings, QStringLiteral("alex/lpf17_15m"), m_lpf17_15m);
    SettingsUtils::setIniOn(settings, QStringLiteral("alex/lpf12_10m"), m_lpf12_10m);
    SettingsUtils::setIniOn(settings, QStringLiteral("alex/lpf6m"), m_lpf6m);

    settings->setValue("alex/amp6mLo", static_cast<int>(m_hpfLoFreqs.value(5)));
    settings->setValue("alex/amp6mHi", static_cast<int>(m_hpfHiFreqs.value(5)));
    settings->setValue("alex/hpf1_5MHzLo", static_cast<int>(m_hpfLoFreqs.value(0)));
    settings->setValue("alex/hpf1_5MHzHi", static_cast<int>(m_hpfHiFreqs.value(0)));
    settings->setValue("alex/hpf6_5MHzLo", static_cast<int>(m_hpfLoFreqs.value(1)));
    settings->setValue("alex/hpf6_5MHzHi", static_cast<int>(m_hpfHiFreqs.value(1)));
    settings->setValue("alex/hpf9_5MHzLo", static_cast<int>(m_hpfLoFreqs.value(2)));
    settings->setValue("alex/hpf9_5MHzHi", static_cast<int>(m_hpfHiFreqs.value(2)));
    settings->setValue("alex/hpf13MHzLo", static_cast<int>(m_hpfLoFreqs.value(3)));
    settings->setValue("alex/hpf13MHzHi", static_cast<int>(m_hpfHiFreqs.value(3)));
    settings->setValue("alex/hpf20MHzLo", static_cast<int>(m_hpfLoFreqs.value(4)));
    settings->setValue("alex/hpf20MHzHi", static_cast<int>(m_hpfHiFreqs.value(4)));
    settings->setValue("alex/lpf160mLo", static_cast<int>(m_lpfLoFreqs.value(0)));
    settings->setValue("alex/lpf160mHi", static_cast<int>(m_lpfHiFreqs.value(0)));
    settings->setValue("alex/lpf80mLo", static_cast<int>(m_lpfLoFreqs.value(1)));
    settings->setValue("alex/lpf80mHi", static_cast<int>(m_lpfHiFreqs.value(1)));
    settings->setValue("alex/lpf60_40mLo", static_cast<int>(m_lpfLoFreqs.value(2)));
    settings->setValue("alex/lpf60_40mHi", static_cast<int>(m_lpfHiFreqs.value(2)));
    settings->setValue("alex/lpf30_20mLo", static_cast<int>(m_lpfLoFreqs.value(3)));
    settings->setValue("alex/lpf30_20mHi", static_cast<int>(m_lpfHiFreqs.value(3)));
    settings->setValue("alex/lpf17_15mLo", static_cast<int>(m_lpfLoFreqs.value(4)));
    settings->setValue("alex/lpf17_15mHi", static_cast<int>(m_lpfHiFreqs.value(4)));
    settings->setValue("alex/lpf12_10mLo", static_cast<int>(m_lpfLoFreqs.value(5)));
    settings->setValue("alex/lpf12_10mHi", static_cast<int>(m_lpfHiFreqs.value(5)));
    settings->setValue("alex/lpf6mLo", static_cast<int>(m_lpfLoFreqs.value(6)));
    settings->setValue("alex/lpf6mHi", static_cast<int>(m_lpfHiFreqs.value(6)));
    settings->setValue("alex/attenuation", m_attenuation);
}

void AlexConfig::loadStates(QSettings *settings, const QStringList &bandKeys) {
    for (int i = 0; i < bandKeys.size(); ++i) {
        const QString key = QStringLiteral("alex/state") + bandKeys.at(i);
        setAlexState(i, settings->value(key, 33).toInt());
    }
}

void AlexConfig::saveStates(QSettings *settings, const QStringList &bandKeys) const {
    for (int i = 0; i < bandKeys.size(); ++i) {
        const QString key = QStringLiteral("alex/state") + bandKeys.at(i);
        settings->setValue(key, m_alexStates.value(i));
    }
}
