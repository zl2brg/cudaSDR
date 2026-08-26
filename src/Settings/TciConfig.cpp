#include "TciConfig.h"
#include <QSettings>
#include <QtMath>
#include "Util/settings_utils.h"

TciConfig::TciConfig(QObject *parent)
    : QObject(parent)
    , m_serverEnabled(true)
    , m_rxGain(1.0f)
    , m_txGain(1.0f)
{}

void TciConfig::setServerEnabled(bool enabled)
{
    if (m_serverEnabled.load() == enabled)
        return;
    m_serverEnabled.store(enabled);
    emit serverEnabledChanged(enabled);
}

void TciConfig::setRxGain(float gain)
{
    const float clamped = SettingsUtils::clampTciGain(gain);
    if (qAbs(m_rxGain.load() - clamped) < 1e-6f)
        return;
    m_rxGain.store(clamped);
    emit rxGainChanged(clamped);
}

void TciConfig::setTxGain(float gain)
{
    const float clamped = SettingsUtils::clampTciGain(gain);
    if (qAbs(m_txGain.load() - clamped) < 1e-6f)
        return;
    m_txGain.store(clamped);
    emit txGainChanged(clamped);
}

void TciConfig::load(const QJsonObject &json)
{
    if (json.contains(QLatin1String("enabled")))
        setServerEnabled(json.value(QLatin1String("enabled")).toBool());
    if (json.contains(QLatin1String("rxGain")))
        setRxGain(static_cast<float>(json.value(QLatin1String("rxGain")).toDouble()));
    if (json.contains(QLatin1String("txGain")))
        setTxGain(static_cast<float>(json.value(QLatin1String("txGain")).toDouble()));
}

void TciConfig::save(QJsonObject &json) const
{
    json[QLatin1String("enabled")] = m_serverEnabled.load();
    json[QLatin1String("rxGain")] = static_cast<double>(m_rxGain.load());
    json[QLatin1String("txGain")] = static_cast<double>(m_txGain.load());
}

void TciConfig::loadIni(QSettings *settings)
{
    setServerEnabled(settings->value(QStringLiteral("network/tci_enabled"), true).toBool());
    setRxGain(settings->value(QStringLiteral("network/tci_rx_gain"), 1.0).toFloat());
    setTxGain(settings->value(QStringLiteral("network/tci_tx_gain"), 1.0).toFloat());
}

void TciConfig::saveIni(QSettings *settings) const
{
    settings->setValue(QStringLiteral("network/tci_enabled"), m_serverEnabled.load());
    settings->setValue(QStringLiteral("network/tci_rx_gain"), m_rxGain.load());
    settings->setValue(QStringLiteral("network/tci_tx_gain"), m_txGain.load());
}
