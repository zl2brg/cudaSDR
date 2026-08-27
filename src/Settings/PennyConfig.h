#ifndef PENNYCONFIG_H
#define PENNYCONFIG_H

#include <QObject>
#include <QJsonObject>
#include <QList>
#include <QStringList>

class QSettings;

/**
 * Persistence DTO for Penny/Hermes open-collector (J6) pin maps.
 * Runtime state stays on Settings; JSON load/save copies through this object.
 */
class PennyConfig : public QObject {
    Q_OBJECT

public:
    /** MAX_BANDS - 1 (ham bands excluding gen). */
    static constexpr int kPinCount = 21;

    explicit PennyConfig(QObject *parent = nullptr);

    bool ocEnabled() const { return m_ocEnabled; }
    void setOcEnabled(bool enabled);

    QList<int> rxJ6() const { return m_rxJ6; }
    void setRxJ6(const QList<int> &pins);

    QList<int> txJ6() const { return m_txJ6; }
    void setTxJ6(const QList<int> &pins);

    void load(const QJsonObject &json);
    void save(QJsonObject &json) const;

    void loadIni(QSettings *settings, const QStringList &bandKeys);
    void saveIni(QSettings *settings, const QStringList &bandKeys) const;

signals:
    void ocEnabledChanged(bool enabled);
    void rxJ6Changed();
    void txJ6Changed();

private:
    static QList<int> normalizedPins(const QList<int> &pins);

    bool m_ocEnabled = false;
    QList<int> m_rxJ6;
    QList<int> m_txJ6;
};

#endif // PENNYCONFIG_H
