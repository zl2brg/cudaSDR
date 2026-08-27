#include "PennyConfig.h"
#include "Util/settings_utils.h"

#include <QSettings>

PennyConfig::PennyConfig(QObject *parent)
    : QObject(parent)
    , m_rxJ6(kPinCount, 0)
    , m_txJ6(kPinCount, 0)
{}

QList<int> PennyConfig::normalizedPins(const QList<int> &pins)
{
    QList<int> out(kPinCount, 0);
    const int n = qMin(pins.size(), kPinCount);
    for (int i = 0; i < n; ++i)
        out[i] = SettingsUtils::clampIntRange(pins.at(i), 0, 255, 0);
    return out;
}

void PennyConfig::setOcEnabled(bool enabled)
{
    if (m_ocEnabled == enabled)
        return;
    m_ocEnabled = enabled;
    emit ocEnabledChanged(m_ocEnabled);
}

void PennyConfig::setRxJ6(const QList<int> &pins)
{
    const QList<int> next = normalizedPins(pins);
    if (m_rxJ6 == next)
        return;
    m_rxJ6 = next;
    emit rxJ6Changed();
}

void PennyConfig::setTxJ6(const QList<int> &pins)
{
    const QList<int> next = normalizedPins(pins);
    if (m_txJ6 == next)
        return;
    m_txJ6 = next;
    emit txJ6Changed();
}

void PennyConfig::load(const QJsonObject &json)
{
    if (json.contains(QLatin1String("ocEnabled")))
        setOcEnabled(json.value(QLatin1String("ocEnabled")).toBool());
    if (json.contains(QLatin1String("rxJ6")) && json.value(QLatin1String("rxJ6")).isArray()) {
        const auto vec = SettingsUtils::jsonArrayToVector<int>(json, QStringLiteral("rxJ6"), kPinCount, 0);
        setRxJ6(QList<int>(vec.begin(), vec.end()));
    }
    if (json.contains(QLatin1String("txJ6")) && json.value(QLatin1String("txJ6")).isArray()) {
        const auto vec = SettingsUtils::jsonArrayToVector<int>(json, QStringLiteral("txJ6"), kPinCount, 0);
        setTxJ6(QList<int>(vec.begin(), vec.end()));
    }
}

void PennyConfig::save(QJsonObject &json) const
{
    json[QLatin1String("ocEnabled")] = m_ocEnabled;
    json[QLatin1String("rxJ6")] = SettingsUtils::toJsonArray(m_rxJ6);
    json[QLatin1String("txJ6")] = SettingsUtils::toJsonArray(m_txJ6);
}

void PennyConfig::loadIni(QSettings *settings, const QStringList &bandKeys)
{
    setOcEnabled(SettingsUtils::iniOn(settings, QStringLiteral("penny/OCenabled")));
    QList<int> rx(kPinCount, 0);
    QList<int> tx(kPinCount, 0);
    const int n = qMin(bandKeys.size(), kPinCount);
    for (int i = 0; i < n; ++i) {
        rx[i] = SettingsUtils::clampIntRange(
            settings->value(QStringLiteral("penny/rxState") + bandKeys.at(i), 0).toInt(), 0, 255, 0);
        tx[i] = SettingsUtils::clampIntRange(
            settings->value(QStringLiteral("penny/txState") + bandKeys.at(i), 0).toInt(), 0, 255, 0);
    }
    setRxJ6(rx);
    setTxJ6(tx);
}

void PennyConfig::saveIni(QSettings *settings, const QStringList &bandKeys) const
{
    SettingsUtils::setIniOn(settings, QStringLiteral("penny/OCenabled"), m_ocEnabled);
    const int n = qMin(bandKeys.size(), kPinCount);
    for (int i = 0; i < n; ++i) {
        settings->setValue(QStringLiteral("penny/rxState") + bandKeys.at(i), m_rxJ6.at(i));
        settings->setValue(QStringLiteral("penny/txState") + bandKeys.at(i), m_txJ6.at(i));
    }
}
