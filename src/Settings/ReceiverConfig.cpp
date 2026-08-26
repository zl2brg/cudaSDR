#include "ReceiverConfig.h"
#include "cusdr_settings.h"
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
    , m_vfoAFrequency(7050000)
    , m_vfoBFrequency(7050000)
    , m_activeVfo(0)
    , m_filterSlope(1)
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

void ReceiverConfig::setVfoAFrequency(qint64 freq) {
    if (m_vfoAFrequency != freq) {
        m_vfoAFrequency = freq;
        emit vfoAFrequencyChanged(m_vfoAFrequency);
    }
}

void ReceiverConfig::setVfoBFrequency(qint64 freq) {
    if (m_vfoBFrequency != freq) {
        m_vfoBFrequency = freq;
        emit vfoBFrequencyChanged(m_vfoBFrequency);
    }
}

void ReceiverConfig::setActiveVfo(int vfo) {
    const int clamped = (vfo == 1) ? 1 : 0;
    if (m_activeVfo != clamped) {
        m_activeVfo = clamped;
        emit activeVfoChanged(m_activeVfo);
    }
}

void ReceiverConfig::setFilterSlope(int slope) {
    if (slope < 0)
        slope = 0;
    if (m_filterSlope != slope) {
        m_filterSlope = slope;
        emit filterSlopeChanged(m_filterSlope);
    }
}

void ReceiverConfig::applyTo(TReceiver &rx) const {
    rx.dspCore = m_dspCore;
    rx.hamBand = m_hamBand;
    rx.dspMode = m_dspMode;
    rx.adcMode = m_adcMode;
    rx.agcMode = m_agcMode;
    rx.ctrFrequency = m_ctrFrequency;
    rx.vfoFrequency = m_vfoFrequency;
    rx.vfoAFrequency = m_vfoAFrequency;
    rx.vfoBFrequency = m_vfoBFrequency;
    rx.activeVfo = m_activeVfo;
    rx.filterSlope = m_filterSlope;
    if (m_hamBand >= 0 && m_hamBand < rx.dspModeList.size())
        rx.dspModeList[m_hamBand] = m_dspMode;
}

void ReceiverConfig::fromReceiver(const TReceiver &rx) {
    setDspCore(rx.dspCore);
    setHamBand(rx.hamBand);
    setDspMode(rx.dspMode);
    setAdcMode(rx.adcMode);
    setAgcMode(rx.agcMode);
    setCtrFrequency(rx.ctrFrequency);
    setVfoFrequency(rx.vfoFrequency);
    setVfoAFrequency(rx.vfoAFrequency);
    setVfoBFrequency(rx.vfoBFrequency);
    setActiveVfo(rx.activeVfo);
    setFilterSlope(rx.filterSlope);
}

void ReceiverConfig::load(const QJsonObject &json) {
    if (json.contains(QLatin1String("dspCore")))
        setDspCore(static_cast<QSDR::_DSPCore>(json.value(QLatin1String("dspCore")).toInt()));
    if (json.contains(QLatin1String("hamBand")))
        setHamBand(static_cast<HamBand>(json.value(QLatin1String("hamBand")).toInt()));
    if (json.contains(QLatin1String("dspMode")))
        setDspMode(static_cast<DSPMode>(json.value(QLatin1String("dspMode")).toInt()));
    if (json.contains(QLatin1String("adcMode")))
        setAdcMode(static_cast<ADCMode>(json.value(QLatin1String("adcMode")).toInt()));
    if (json.contains(QLatin1String("agcMode")))
        setAgcMode(static_cast<AGCMode>(json.value(QLatin1String("agcMode")).toInt()));
    if (json.contains(QLatin1String("ctrFrequency")))
        setCtrFrequency(static_cast<qint64>(json.value(QLatin1String("ctrFrequency")).toDouble()));
    if (json.contains(QLatin1String("vfoFrequency")))
        setVfoFrequency(static_cast<qint64>(json.value(QLatin1String("vfoFrequency")).toDouble()));
    if (json.contains(QLatin1String("vfoAFrequency")))
        setVfoAFrequency(static_cast<qint64>(json.value(QLatin1String("vfoAFrequency")).toDouble()));
    else
        setVfoAFrequency(vfoFrequency());
    if (json.contains(QLatin1String("vfoBFrequency")))
        setVfoBFrequency(static_cast<qint64>(json.value(QLatin1String("vfoBFrequency")).toDouble()));
    else
        setVfoBFrequency(vfoFrequency());
    if (json.contains(QLatin1String("activeVfo")))
        setActiveVfo(json.value(QLatin1String("activeVfo")).toInt());
    else
        setActiveVfo(0);
    if (json.contains(QLatin1String("filterSlope")))
        setFilterSlope(json.value(QLatin1String("filterSlope")).toInt(1));
}

void ReceiverConfig::save(QJsonObject &json) const {
    json[QLatin1String("dspCore")] = static_cast<int>(m_dspCore);
    json[QLatin1String("hamBand")] = static_cast<int>(m_hamBand);
    json[QLatin1String("dspMode")] = static_cast<int>(m_dspMode);
    json[QLatin1String("adcMode")] = static_cast<int>(m_adcMode);
    json[QLatin1String("agcMode")] = static_cast<int>(m_agcMode);
    json[QLatin1String("ctrFrequency")] = static_cast<double>(m_ctrFrequency);
    json[QLatin1String("vfoFrequency")] = static_cast<double>(m_vfoFrequency);
    json[QLatin1String("vfoAFrequency")] = static_cast<double>(m_vfoAFrequency);
    json[QLatin1String("vfoBFrequency")] = static_cast<double>(m_vfoBFrequency);
    json[QLatin1String("activeVfo")] = m_activeVfo;
    json[QLatin1String("filterSlope")] = m_filterSlope;
}

void ReceiverConfig::loadIni(QSettings *settings) {
    // Must match Settings::m_rxStringList ("receiver0", …) used by load/saveSettings.
    const QString prefix = QStringLiteral("receiver%1").arg(m_id);
    // Legacy prefix from the brief period when ReceiverConfig wrote "rxN/…"
    const QString legacyPrefix = QStringLiteral("rx%1").arg(m_id);

    auto keyOrLegacy = [&](const char *suffix) -> QString {
        QString cstr = prefix + QLatin1Char('/') + QLatin1String(suffix);
        if (!settings->contains(cstr))
            cstr = legacyPrefix + QLatin1Char('/') + QLatin1String(suffix);
        return cstr;
    };

    const QString dspCoreStr = settings->value(keyOrLegacy("dspCore"), QStringLiteral("qtdsp")).toString();
    if (dspCoreStr == QLatin1String("qtdsp"))
        setDspCore(QSDR::QtDSP);

    setCtrFrequency(static_cast<qint64>(settings->value(keyOrLegacy("centerFrequency"), 7050000.0).toDouble()));
    setVfoFrequency(static_cast<qint64>(settings->value(keyOrLegacy("vfoFrequency"), 7050000.0).toDouble()));

    {
        const QString cstr = keyOrLegacy("vfoAFrequency");
        if (settings->contains(cstr))
            setVfoAFrequency(static_cast<qint64>(settings->value(cstr).toDouble()));
        else
            setVfoAFrequency(vfoFrequency());
    }
    {
        const QString cstr = keyOrLegacy("vfoBFrequency");
        if (settings->contains(cstr))
            setVfoBFrequency(static_cast<qint64>(settings->value(cstr).toDouble()));
        else
            setVfoBFrequency(vfoFrequency());
    }

    setActiveVfo(settings->value(keyOrLegacy("activeVfo"), 0).toInt());
    setFilterSlope(settings->value(keyOrLegacy("filterSlope"), 1).toInt());
}

void ReceiverConfig::saveIni(QSettings *settings) const {
    const QString prefix = QStringLiteral("receiver%1").arg(m_id);

    if (m_dspCore == QSDR::QtDSP)
        settings->setValue(prefix + QStringLiteral("/dspCore"), QStringLiteral("qtdsp"));

    settings->setValue(prefix + QStringLiteral("/centerFrequency"), m_ctrFrequency);
    settings->setValue(prefix + QStringLiteral("/vfoFrequency"), m_vfoFrequency);
    settings->setValue(prefix + QStringLiteral("/vfoAFrequency"), m_vfoAFrequency);
    settings->setValue(prefix + QStringLiteral("/vfoBFrequency"), m_vfoBFrequency);
    settings->setValue(prefix + QStringLiteral("/activeVfo"), m_activeVfo);
    settings->setValue(prefix + QStringLiteral("/filterSlope"), m_filterSlope);
}
