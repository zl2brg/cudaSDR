#include "WidebandConfig.h"
#include "Util/settings_utils.h"

#include <QSettings>

WidebandConfig::WidebandConfig(QObject *parent)
    : QObject(parent)
{}

void WidebandConfig::setDataEnabled(bool enabled)
{
    if (m_dataEnabled == enabled)
        return;
    m_dataEnabled = enabled;
    if (!m_dataEnabled && m_displayEnabled) {
        m_displayEnabled = false;
        emit displayEnabledChanged(false);
    }
    emit dataEnabledChanged(m_dataEnabled);
}

void WidebandConfig::setDisplayEnabled(bool enabled)
{
    const bool next = m_dataEnabled && enabled;
    if (m_displayEnabled == next)
        return;
    m_displayEnabled = next;
    emit displayEnabledChanged(m_displayEnabled);
}

void WidebandConfig::setAveraging(bool enabled)
{
    if (m_averaging == enabled)
        return;
    m_averaging = enabled;
    emit averagingChanged(m_averaging);
}

void WidebandConfig::setAveragingCnt(int count)
{
    count = SettingsUtils::clampIntRange(count, 1, 1000, 5);
    if (m_averagingCnt == count)
        return;
    m_averagingCnt = count;
    emit averagingCntChanged(m_averagingCnt);
}

void WidebandConfig::setdBmScaleMin(qreal value)
{
    if (value < -200.0 || value > 0.0)
        value = -140.0;
    if (qFuzzyCompare(m_dBmScaleMin, value))
        return;
    m_dBmScaleMin = value;
    emit dBmScaleMinChanged(m_dBmScaleMin);
}

void WidebandConfig::setdBmScaleMax(qreal value)
{
    if (value < -100.0 || value > 0.0)
        value = -10.0;
    if (qFuzzyCompare(m_dBmScaleMax, value))
        return;
    m_dBmScaleMax = value;
    emit dBmScaleMaxChanged(m_dBmScaleMax);
}

void WidebandConfig::setPanMode(PanGraphicsMode mode)
{
    if (mode < Line || mode > Solid)
        mode = Line;
    if (m_panMode == mode)
        return;
    m_panMode = mode;
    emit panModeChanged(m_panMode);
}

void WidebandConfig::load(const QJsonObject &json)
{
    if (json.contains(QLatin1String("data")))
        setDataEnabled(json.value(QLatin1String("data")).toBool());
    if (json.contains(QLatin1String("display")))
        setDisplayEnabled(json.value(QLatin1String("display")).toBool());
    if (json.contains(QLatin1String("averaging")))
        setAveraging(json.value(QLatin1String("averaging")).toBool());
    if (json.contains(QLatin1String("averagingCnt")))
        setAveragingCnt(json.value(QLatin1String("averagingCnt")).toInt());
    if (json.contains(QLatin1String("dBmScaleMin")))
        setdBmScaleMin(json.value(QLatin1String("dBmScaleMin")).toDouble());
    if (json.contains(QLatin1String("dBmScaleMax")))
        setdBmScaleMax(json.value(QLatin1String("dBmScaleMax")).toDouble());
    if (json.contains(QLatin1String("panMode")))
        setPanMode(static_cast<PanGraphicsMode>(json.value(QLatin1String("panMode")).toInt()));
}

void WidebandConfig::save(QJsonObject &json) const
{
    json[QLatin1String("data")] = m_dataEnabled;
    json[QLatin1String("display")] = m_displayEnabled;
    json[QLatin1String("averaging")] = m_averaging;
    json[QLatin1String("averagingCnt")] = m_averagingCnt;
    json[QLatin1String("dBmScaleMin")] = m_dBmScaleMin;
    json[QLatin1String("dBmScaleMax")] = m_dBmScaleMax;
    json[QLatin1String("panMode")] = static_cast<int>(m_panMode);
}

void WidebandConfig::loadIni(QSettings *settings)
{
    setDataEnabled(SettingsUtils::iniOn(settings, QStringLiteral("wideband/widebandData"), QStringLiteral("on")));
    setDisplayEnabled(SettingsUtils::iniOn(settings, QStringLiteral("wideband/widebandDisplay")));
    setAveraging(SettingsUtils::iniOn(settings, QStringLiteral("wideband/averaging"), QStringLiteral("on")));
    setAveragingCnt(settings->value(QStringLiteral("wideband/averagingCnt"), 5).toInt());
    setdBmScaleMin(settings->value(QStringLiteral("wideband/dBmWideBandScaleMin"), -140).toInt());
    setdBmScaleMax(settings->value(QStringLiteral("wideband/dBmWideBandScaleMax"), -10).toInt());

    const QString pan = settings->value(QStringLiteral("wideband/panMode"), QStringLiteral("LINE")).toString();
    if (pan == QLatin1String("FILLEDLINE"))
        setPanMode(FilledLine);
    else if (pan == QLatin1String("SOLID"))
        setPanMode(Solid);
    else
        setPanMode(Line);
}

void WidebandConfig::saveIni(QSettings *settings) const
{
    SettingsUtils::setIniOn(settings, QStringLiteral("wideband/widebandData"), m_dataEnabled);
    SettingsUtils::setIniOn(settings, QStringLiteral("wideband/widebandDisplay"), m_displayEnabled);
    SettingsUtils::setIniOn(settings, QStringLiteral("wideband/averaging"), m_averaging);
    settings->setValue(QStringLiteral("wideband/averagingCnt"), m_averagingCnt);
    settings->setValue(QStringLiteral("wideband/dBmWideBandScaleMin"), static_cast<int>(m_dBmScaleMin));
    settings->setValue(QStringLiteral("wideband/dBmWideBandScaleMax"), static_cast<int>(m_dBmScaleMax));
    if (m_panMode == FilledLine)
        settings->setValue(QStringLiteral("wideband/panMode"), QStringLiteral("FILLEDLINE"));
    else if (m_panMode == Solid)
        settings->setValue(QStringLiteral("wideband/panMode"), QStringLiteral("SOLID"));
    else
        settings->setValue(QStringLiteral("wideband/panMode"), QStringLiteral("LINE"));
}
