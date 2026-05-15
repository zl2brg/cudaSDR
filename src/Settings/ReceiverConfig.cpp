#include "ReceiverConfig.h"

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

void ReceiverConfig::setCtrFrequency(long freq) {
    if (m_ctrFrequency != freq) {
        m_ctrFrequency = freq;
        emit ctrFrequencyChanged(m_ctrFrequency);
    }
}

void ReceiverConfig::setVfoFrequency(long freq) {
    if (m_vfoFrequency != freq) {
        m_vfoFrequency = freq;
        emit vfoFrequencyChanged(m_vfoFrequency);
    }
}

void ReceiverConfig::load(const QJsonObject &json) {
    if (json.contains("dspCore")) m_dspCore = static_cast<QSDR::_DSPCore>(json["dspCore"].toInt());
    if (json.contains("hamBand")) m_hamBand = static_cast<HamBand>(json["hamBand"].toInt());
    if (json.contains("dspMode")) m_dspMode = static_cast<DSPMode>(json["dspMode"].toInt());
    if (json.contains("adcMode")) m_adcMode = static_cast<ADCMode>(json["adcMode"].toInt());
    if (json.contains("agcMode")) m_agcMode = static_cast<AGCMode>(json["agcMode"].toInt());
    if (json.contains("ctrFrequency")) m_ctrFrequency = static_cast<long>(json["ctrFrequency"].toDouble());
    if (json.contains("vfoFrequency")) m_vfoFrequency = static_cast<long>(json["vfoFrequency"].toDouble());
}

void ReceiverConfig::save(QJsonObject &json) const {
    json["dspCore"] = static_cast<int>(m_dspCore);
    json["hamBand"] = static_cast<int>(m_hamBand);
    json["dspMode"] = static_cast<int>(m_dspMode);
    json["adcMode"] = static_cast<int>(m_adcMode);
    json["agcMode"] = static_cast<int>(m_agcMode);
    json["ctrFrequency"] = static_cast<double>(m_ctrFrequency);
    json["vfoFrequency"] = static_cast<double>(m_vfoFrequency);
}
