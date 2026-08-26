#ifndef SOAPYCONFIG_H
#define SOAPYCONFIG_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QJsonObject>
#include <QMetaType>
#include <QReadWriteLock>

class QSettings;

typedef struct _TSoapyDevice {
    QString driver;
    QString hardware;
    QString name;
    QString serial;
    QString label;
    QMap<QString, QString> args;
} TSoapyDevice;

Q_DECLARE_METATYPE(TSoapyDevice)
Q_DECLARE_METATYPE(QList<TSoapyDevice>)

class SoapyConfig : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString rxAntenna READ rxAntenna WRITE setRxAntenna NOTIFY rxAntennaChanged)
    Q_PROPERTY(QString txAntenna READ txAntenna WRITE setTxAntenna NOTIFY txAntennaChanged)
    Q_PROPERTY(int lnaGain READ lnaGain WRITE setLnaGain NOTIFY lnaGainChanged)
    Q_PROPERTY(int tiaGain READ tiaGain WRITE setTiaGain NOTIFY tiaGainChanged)
    Q_PROPERTY(int pgaGain READ pgaGain WRITE setPgaGain NOTIFY pgaGainChanged)
    Q_PROPERTY(int overallGain READ overallGain WRITE setOverallGain NOTIFY overallGainChanged)
    Q_PROPERTY(bool autoCalibrate READ autoCalibrate WRITE setAutoCalibrate NOTIFY autoCalibrateChanged)
    Q_PROPERTY(bool iqBalance READ iqBalance WRITE setIqBalance NOTIFY iqBalanceChanged)

public:
    explicit SoapyConfig(QObject *parent = nullptr);

    QString rxAntenna() const;
    void setRxAntenna(const QString &antenna);

    QString txAntenna() const;
    void setTxAntenna(const QString &antenna);

    int lnaGain() const;
    void setLnaGain(int gain);

    int tiaGain() const;
    void setTiaGain(int gain);

    int pgaGain() const;
    void setPgaGain(int gain);

    int overallGain() const;
    void setOverallGain(int gain);

    int overallGainMin() const;
    int overallGainMax() const;
    void setOverallGainRange(int minGain, int maxGain);

    bool autoCalibrate() const;
    void setAutoCalibrate(bool enabled);

    bool iqBalance() const;
    void setIqBalance(bool enabled);

    TSoapyDevice currentDevice() const;
    void setCurrentDevice(const TSoapyDevice &device);

    QList<TSoapyDevice> deviceList() const;
    void setDeviceList(const QList<TSoapyDevice> &list);

    QStringList antennaList() const;
    void setAntennaList(const QStringList &list);

    QStringList txAntennaList() const;
    void setTxAntennaList(const QStringList &list);

    QString hardwareKey() const;
    void setHardwareKey(const QString &key);

    void load(const QJsonObject &json);
    void save(QJsonObject &json) const;

    void loadIni(QSettings *settings);
    void saveIni(QSettings *settings) const;

signals:
    void rxAntennaChanged(const QString &antenna);
    void txAntennaChanged(const QString &antenna);
    void lnaGainChanged(int gain);
    void tiaGainChanged(int gain);
    void pgaGainChanged(int gain);
    void overallGainChanged(int gain);
    void overallGainRangeChanged(int minGain, int maxGain);
    void autoCalibrateChanged(bool enabled);
    void iqBalanceChanged(bool enabled);
    void currentDeviceChanged(const TSoapyDevice &device);
    void deviceListChanged(const QList<TSoapyDevice> &list);
    void antennaListChanged(const QStringList &list);
    void txAntennaListChanged(const QStringList &list);
    void hardwareKeyChanged(const QString &key);

private:
    mutable QReadWriteLock m_lock;
    TSoapyDevice m_currentDevice;
    QList<TSoapyDevice> m_devices;
    QString m_rxAntenna;
    QString m_txAntenna;
    QStringList m_antennaList;
    QStringList m_txAntennaList;
    QString m_hardwareKey;
    int m_lnaGain;
    int m_tiaGain;
    int m_pgaGain;
    int m_overallGain;
    int m_overallGainMin;
    int m_overallGainMax;
    bool m_autoCalibrate;
    bool m_iqBalance;
};

#endif // SOAPYCONFIG_H
