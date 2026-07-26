#include "ReceiverConfig.h"
#include <QSettings>

ReceiverConfig::ReceiverConfig(int id, QObject *parent)
    : QObject(parent)
    , m_id(id)
    , m_dspCore(QSDR::QtDSP)
    , m_hamBand(gen)
    , m_dspMode(USB)
    , m_adcMode(adc1)
    , m_agcMode(agcMED)
    , m_ctrFrequency(7050000)
    , m_vfoFrequency(7050000)
{}

void ReceiverConfig::setDspCore(QSDR::_DSPCore core) {
    if (m_dspCore != core) {
        m_dspCore = core;
        emit dspCoreChanged(m_dspCore);
    }
}

void ReceiverConfig::setHamBand(HamBand band) {
    if (m_hamBand != band) {
        m_hamBand = band;
        emit hamBandChanged(m_hamBand);
    }
}

void ReceiverConfig::setDspMode(DSPMode mode) {
    if (m_dspMode != mode) {
        m_dspMode = mode;
        emit dspModeChanged(m_dspMode);
    }
}

void ReceiverConfig::setAdcMode(ADCMode mode) {
    if (m_adcMode != mode) {
        m_adcMode = mode;
        emit adcModeChanged(m_adcMode);
    }
}

void ReceiverConfig::setAgcMode(AGCMode mode) {
    if (m_agcMode != mode) {
        m_agcMode = mode;
        emit agcModeChanged(m_agcMode);
    }
}

void ReceiverConfig::setCtrFrequency(qint64 freq) {
    if (m_ctrFrequency != freq) {
        m_ctrFrequency = freq;
        emit ctrFrequencyChanged(m_ctrFrequency);
    }
}

void ReceiverConfig::setVfoFrequency(qint64 freq) {
    if (m_vfoFrequency != freq) {
        m_vfoFrequency = freq;
        emit vfoFrequencyChanged(m_vfoFrequency);
    }
}

void ReceiverConfig::setFilterSlope(int slope) {
    if (m_filterSlope != slope) {
        m_filterSlope = slope;
        emit filterSlopeChanged(m_filterSlope);
    }
}

void ReceiverConfig::load(const QJsonObject &json) {
    if (json.contains("dspCore")) m_dspCore = static_cast<QSDR::_DSPCore>(json["dspCore"].toInt());
    if (json.contains("hamBand")) m_hamBand = static_cast<HamBand>(json["hamBand"].toInt());
    if (json.contains("dspMode")) m_dspMode = static_cast<DSPMode>(json["dspMode"].toInt());
    if (json.contains("adcMode")) m_adcMode = static_cast<ADCMode>(json["adcMode"].toInt());
    if (json.contains("agcMode")) m_agcMode = static_cast<AGCMode>(json["agcMode"].toInt());
    if (json.contains("ctrFrequency")) m_ctrFrequency = static_cast<qint64>(json["ctrFrequency"].toDouble());
    if (json.contains("vfoFrequency")) m_vfoFrequency = static_cast<qint64>(json["vfoFrequency"].toDouble());
    if (json.contains("filterSlope")) m_filterSlope = json["filterSlope"].toInt(1);
}

void ReceiverConfig::save(QJsonObject &json) const {
    json["dspCore"] = static_cast<int>(m_dspCore);
    json["hamBand"] = static_cast<int>(m_hamBand);
    json["dspMode"] = static_cast<int>(m_dspMode);
    json["adcMode"] = static_cast<int>(m_adcMode);
    json["agcMode"] = static_cast<int>(m_agcMode);
    json["ctrFrequency"] = static_cast<double>(m_ctrFrequency);
    json["vfoFrequency"] = static_cast<double>(m_vfoFrequency);
    json["filterSlope"] = m_filterSlope;
}

void ReceiverConfig::loadIni(QSettings *settings) {
    // Must match Settings::m_rxStringList ("receiver0", …) used by load/saveSettings.
    QString prefix = QString("receiver%1").arg(m_id);
    // Legacy prefix from the brief period when ReceiverConfig wrote "rxN/…"
    QString legacyPrefix = QString("rx%1").arg(m_id);

    QString cstr = prefix + "/dspCore";
    if (!settings->contains(cstr))
        cstr = legacyPrefix + "/dspCore";
    QString valStr = settings->value(cstr, "qtdsp").toString();
    if (valStr == "qtdsp") {
        setDspCore(QSDR::QtDSP);
    }

    cstr = prefix + "/centerFrequency";
    if (!settings->contains(cstr))
        cstr = legacyPrefix + "/centerFrequency";
    setCtrFrequency(static_cast<qint64>(settings->value(cstr, 7050000.0).toDouble()));

    cstr = prefix + "/vfoFrequency";
    if (!settings->contains(cstr))
        cstr = legacyPrefix + "/vfoFrequency";
    setVfoFrequency(static_cast<qint64>(settings->value(cstr, 7050000.0).toDouble()));

    cstr = prefix + "/filterSlope";
    if (!settings->contains(cstr))
        cstr = legacyPrefix + "/filterSlope";
    setFilterSlope(settings->value(cstr, 1).toInt());
}

void ReceiverConfig::saveIni(QSettings *settings) const {
    QString prefix = QString("receiver%1").arg(m_id);

    QString cstr = prefix + "/dspCore";
    if (m_dspCore == QSDR::QtDSP) {
        settings->setValue(cstr, "qtdsp");
    }

    cstr = prefix + "/centerFrequency";
    settings->setValue(cstr, m_ctrFrequency);

    cstr = prefix + "/vfoFrequency";
    settings->setValue(cstr, m_vfoFrequency);

    cstr = prefix + "/filterSlope";
    settings->setValue(cstr, m_filterSlope);
}
