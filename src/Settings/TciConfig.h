#ifndef TCICONFIG_H
#define TCICONFIG_H

#include <QObject>
#include <QJsonObject>
#include <atomic>

class QSettings;

class TciConfig : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool serverEnabled READ serverEnabled WRITE setServerEnabled NOTIFY serverEnabledChanged)
    Q_PROPERTY(float rxGain READ rxGain WRITE setRxGain NOTIFY rxGainChanged)
    Q_PROPERTY(float txGain READ txGain WRITE setTxGain NOTIFY txGainChanged)

public:
    explicit TciConfig(QObject *parent = nullptr);

    bool serverEnabled() const { return m_serverEnabled.load(); }
    void setServerEnabled(bool enabled);

    float rxGain() const { return m_rxGain.load(); }
    void setRxGain(float gain);

    float txGain() const { return m_txGain.load(); }
    void setTxGain(float gain);

    void load(const QJsonObject &json);
    void save(QJsonObject &json) const;

    void loadIni(QSettings *settings);
    void saveIni(QSettings *settings) const;

signals:
    void serverEnabledChanged(bool enabled);
    void rxGainChanged(float gain);
    void txGainChanged(float gain);

private:
    std::atomic<bool> m_serverEnabled;
    std::atomic<float> m_rxGain;
    std::atomic<float> m_txGain;
};

#endif // TCICONFIG_H
