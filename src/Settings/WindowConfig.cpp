#include "WindowConfig.h"
#include <QSettings>
#include "Util/settings_utils.h"

WindowConfig::WindowConfig(QObject *parent)
    : QObject(parent)
    , m_minimumWidgetWidth(300)
    , m_minimumGroupBoxWidth(250)
    , m_multiRxView(0)
{}

void WindowConfig::setMinimumWidgetWidth(int value)
{
    value = SettingsUtils::clampMinimumWidgetWidth(value);
    if (m_minimumWidgetWidth == value)
        return;
    m_minimumWidgetWidth = value;
    emit minimumWidgetWidthChanged(m_minimumWidgetWidth);
}

void WindowConfig::setMinimumGroupBoxWidth(int value)
{
    value = SettingsUtils::clampMinimumGroupBoxWidth(value, m_minimumWidgetWidth);
    if (m_minimumGroupBoxWidth == value)
        return;
    m_minimumGroupBoxWidth = value;
    emit minimumGroupBoxWidthChanged(m_minimumGroupBoxWidth);
}

void WindowConfig::setMultiRxView(int view)
{
    view = SettingsUtils::clampMultiRxView(view);
    if (m_multiRxView == view)
        return;
    m_multiRxView = view;
    emit multiRxViewChanged(m_multiRxView);
}

void WindowConfig::load(const QJsonObject &json)
{
    if (json.contains(QLatin1String("minimumWidgetWidth")))
        setMinimumWidgetWidth(json.value(QLatin1String("minimumWidgetWidth")).toInt());
    if (json.contains(QLatin1String("minimumGroupBoxWidth")))
        setMinimumGroupBoxWidth(json.value(QLatin1String("minimumGroupBoxWidth")).toInt());
    if (json.contains(QLatin1String("multiRxView")))
        setMultiRxView(json.value(QLatin1String("multiRxView")).toInt());
}

void WindowConfig::save(QJsonObject &json) const
{
    json[QLatin1String("minimumWidgetWidth")] = m_minimumWidgetWidth;
    json[QLatin1String("minimumGroupBoxWidth")] = m_minimumGroupBoxWidth;
    json[QLatin1String("multiRxView")] = m_multiRxView;
}

void WindowConfig::loadIni(QSettings *settings)
{
    setMinimumWidgetWidth(settings->value(QStringLiteral("window/minimumWidgetWidth"), 300).toInt());
    setMinimumGroupBoxWidth(settings->value(QStringLiteral("window/minimumGroupBoxWidth"), 250).toInt());
    setMultiRxView(settings->value(QStringLiteral("window/multiRxView"), 0).toInt());
}

void WindowConfig::saveIni(QSettings *settings) const
{
    settings->setValue(QStringLiteral("window/minimumWidgetWidth"), m_minimumWidgetWidth);
    settings->setValue(QStringLiteral("window/minimumGroupBoxWidth"), m_minimumGroupBoxWidth);
    settings->setValue(QStringLiteral("window/multiRxView"), m_multiRxView);
}
