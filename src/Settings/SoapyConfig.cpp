#include "SoapyConfig.h"
#include <QSettings>

SoapyConfig::SoapyConfig(QObject *parent)
    : QObject(parent)
    , m_lnaGain(25)
    , m_tiaGain(12)
    , m_pgaGain(12)
    , m_overallGain(60)
    , m_overallGainMin(0)
    , m_overallGainMax(70)
    , m_autoCalibrate(false)
    , m_iqBalance(true)
{}

QString SoapyConfig::rxAntenna() const
{
    QReadLocker locker(&m_lock);
    return m_rxAntenna;
}

void SoapyConfig::setRxAntenna(const QString &antenna)
{
    {
        QWriteLocker locker(&m_lock);
        if (m_rxAntenna == antenna)
            return;
        m_rxAntenna = antenna;
    }
    emit rxAntennaChanged(antenna);
}

QString SoapyConfig::txAntenna() const
{
    QReadLocker locker(&m_lock);
    return m_txAntenna;
}

void SoapyConfig::setTxAntenna(const QString &antenna)
{
    {
        QWriteLocker locker(&m_lock);
        if (m_txAntenna == antenna)
            return;
        m_txAntenna = antenna;
    }
    emit txAntennaChanged(antenna);
}

int SoapyConfig::lnaGain() const
{
    QReadLocker locker(&m_lock);
    return m_lnaGain;
}

void SoapyConfig::setLnaGain(int gain)
{
    {
        QWriteLocker locker(&m_lock);
        if (m_lnaGain == gain)
            return;
        m_lnaGain = gain;
    }
    emit lnaGainChanged(gain);
}

int SoapyConfig::tiaGain() const
{
    QReadLocker locker(&m_lock);
    return m_tiaGain;
}

void SoapyConfig::setTiaGain(int gain)
{
    {
        QWriteLocker locker(&m_lock);
        if (m_tiaGain == gain)
            return;
        m_tiaGain = gain;
    }
    emit tiaGainChanged(gain);
}

int SoapyConfig::pgaGain() const
{
    QReadLocker locker(&m_lock);
    return m_pgaGain;
}

void SoapyConfig::setPgaGain(int gain)
{
    {
        QWriteLocker locker(&m_lock);
        if (m_pgaGain == gain)
            return;
        m_pgaGain = gain;
    }
    emit pgaGainChanged(gain);
}

int SoapyConfig::overallGain() const
{
    QReadLocker locker(&m_lock);
    return m_overallGain;
}

void SoapyConfig::setOverallGain(int gain)
{
    {
        QWriteLocker locker(&m_lock);
        if (m_overallGain == gain)
            return;
        m_overallGain = gain;
    }
    emit overallGainChanged(gain);
}

int SoapyConfig::overallGainMin() const
{
    QReadLocker locker(&m_lock);
    return m_overallGainMin;
}

int SoapyConfig::overallGainMax() const
{
    QReadLocker locker(&m_lock);
    return m_overallGainMax;
}

void SoapyConfig::setOverallGainRange(int minGain, int maxGain)
{
    if (maxGain < minGain)
        qSwap(minGain, maxGain);
    bool gainChanged = false;
    int clampedGain = 0;
    {
        QWriteLocker locker(&m_lock);
        if (m_overallGainMin == minGain && m_overallGainMax == maxGain)
            return;
        m_overallGainMin = minGain;
        m_overallGainMax = maxGain;
        if (m_overallGain < minGain || m_overallGain > maxGain) {
            m_overallGain = qBound(minGain, m_overallGain, maxGain);
            clampedGain = m_overallGain;
            gainChanged = true;
        }
    }
    if (gainChanged)
        emit overallGainChanged(clampedGain);
    emit overallGainRangeChanged(minGain, maxGain);
}

bool SoapyConfig::autoCalibrate() const
{
    QReadLocker locker(&m_lock);
    return m_autoCalibrate;
}

void SoapyConfig::setAutoCalibrate(bool enabled)
{
    {
        QWriteLocker locker(&m_lock);
        if (m_autoCalibrate == enabled)
            return;
        m_autoCalibrate = enabled;
    }
    emit autoCalibrateChanged(enabled);
}

bool SoapyConfig::iqBalance() const
{
    QReadLocker locker(&m_lock);
    return m_iqBalance;
}

void SoapyConfig::setIqBalance(bool enabled)
{
    {
        QWriteLocker locker(&m_lock);
        if (m_iqBalance == enabled)
            return;
        m_iqBalance = enabled;
    }
    emit iqBalanceChanged(enabled);
}

TSoapyDevice SoapyConfig::currentDevice() const
{
    QReadLocker locker(&m_lock);
    return m_currentDevice;
}

void SoapyConfig::setCurrentDevice(const TSoapyDevice &device)
{
    {
        QWriteLocker locker(&m_lock);
        m_currentDevice = device;
    }
    emit currentDeviceChanged(device);
}

QList<TSoapyDevice> SoapyConfig::deviceList() const
{
    QReadLocker locker(&m_lock);
    return m_devices;
}

void SoapyConfig::setDeviceList(const QList<TSoapyDevice> &list)
{
    {
        QWriteLocker locker(&m_lock);
        m_devices = list;
    }
    emit deviceListChanged(list);
}

QStringList SoapyConfig::antennaList() const
{
    QReadLocker locker(&m_lock);
    return m_antennaList;
}

void SoapyConfig::setAntennaList(const QStringList &list)
{
    {
        QWriteLocker locker(&m_lock);
        m_antennaList = list;
    }
    emit antennaListChanged(list);
}

QStringList SoapyConfig::txAntennaList() const
{
    QReadLocker locker(&m_lock);
    return m_txAntennaList;
}

void SoapyConfig::setTxAntennaList(const QStringList &list)
{
    {
        QWriteLocker locker(&m_lock);
        m_txAntennaList = list;
    }
    emit txAntennaListChanged(list);
}

QString SoapyConfig::hardwareKey() const
{
    QReadLocker locker(&m_lock);
    return m_hardwareKey;
}

void SoapyConfig::setHardwareKey(const QString &key)
{
    {
        QWriteLocker locker(&m_lock);
        m_hardwareKey = key;
    }
    emit hardwareKeyChanged(key);
}

void SoapyConfig::load(const QJsonObject &json)
{
    if (json.contains(QLatin1String("rxAntenna")))
        setRxAntenna(json.value(QLatin1String("rxAntenna")).toString());
    if (json.contains(QLatin1String("txAntenna")))
        setTxAntenna(json.value(QLatin1String("txAntenna")).toString());
    if (json.contains(QLatin1String("lnaGain")))
        setLnaGain(json.value(QLatin1String("lnaGain")).toInt());
    if (json.contains(QLatin1String("tiaGain")))
        setTiaGain(json.value(QLatin1String("tiaGain")).toInt());
    if (json.contains(QLatin1String("pgaGain")))
        setPgaGain(json.value(QLatin1String("pgaGain")).toInt());
    if (json.contains(QLatin1String("overallGain")))
        setOverallGain(json.value(QLatin1String("overallGain")).toInt());
    if (json.contains(QLatin1String("autoCalibrate")))
        setAutoCalibrate(json.value(QLatin1String("autoCalibrate")).toBool());
    if (json.contains(QLatin1String("iqBalance")))
        setIqBalance(json.value(QLatin1String("iqBalance")).toBool());
}

void SoapyConfig::save(QJsonObject &json) const
{
    QReadLocker locker(&m_lock);
    json[QLatin1String("rxAntenna")] = m_rxAntenna;
    json[QLatin1String("txAntenna")] = m_txAntenna;
    json[QLatin1String("lnaGain")] = m_lnaGain;
    json[QLatin1String("tiaGain")] = m_tiaGain;
    json[QLatin1String("pgaGain")] = m_pgaGain;
    json[QLatin1String("overallGain")] = m_overallGain;
    json[QLatin1String("autoCalibrate")] = m_autoCalibrate;
    json[QLatin1String("iqBalance")] = m_iqBalance;
}

void SoapyConfig::loadIni(QSettings *settings)
{
    TSoapyDevice device;
    device.label = settings->value(QStringLiteral("SoapySDR/label"), QString()).toString();
    device.driver = settings->value(QStringLiteral("SoapySDR/driver"), QString()).toString();
    device.serial = settings->value(QStringLiteral("SoapySDR/serial"), QString()).toString();
    const QString uri = settings->value(QStringLiteral("SoapySDR/uri"), QString()).toString();
    const QString hostname = settings->value(QStringLiteral("SoapySDR/hostname"), QString()).toString();
    if (!uri.isEmpty())
        device.args.insert(QStringLiteral("uri"), uri);
    if (!hostname.isEmpty())
        device.args.insert(QStringLiteral("hostname"), hostname);
    setCurrentDevice(device);

    setRxAntenna(settings->value(QStringLiteral("SoapySDR/rxAntenna"), QString()).toString());
    setTxAntenna(settings->value(QStringLiteral("SoapySDR/txAntenna"), QString()).toString());
    setLnaGain(settings->value(QStringLiteral("SoapySDR/lnaGain"), 25).toInt());
    setTiaGain(settings->value(QStringLiteral("SoapySDR/tiaGain"), 12).toInt());
    setPgaGain(settings->value(QStringLiteral("SoapySDR/pgaGain"), 12).toInt());
    setOverallGain(settings->value(QStringLiteral("SoapySDR/overallGain"), 60).toInt());
    setAutoCalibrate(settings->value(QStringLiteral("SoapySDR/autoCalibrate"), false).toBool());
    setIqBalance(settings->value(QStringLiteral("SoapySDR/iqBalance"), true).toBool());
    setHardwareKey(QString());
    setAntennaList(QStringList());
    setTxAntennaList(QStringList());
}

void SoapyConfig::saveIni(QSettings *settings) const
{
    QReadLocker locker(&m_lock);
    settings->setValue(QStringLiteral("SoapySDR/label"), m_currentDevice.label);
    settings->setValue(QStringLiteral("SoapySDR/driver"), m_currentDevice.driver);
    settings->setValue(QStringLiteral("SoapySDR/serial"), m_currentDevice.serial);
    settings->setValue(QStringLiteral("SoapySDR/uri"), m_currentDevice.args.value(QStringLiteral("uri")));
    settings->setValue(QStringLiteral("SoapySDR/hostname"), m_currentDevice.args.value(QStringLiteral("hostname")));
    settings->setValue(QStringLiteral("SoapySDR/rxAntenna"), m_rxAntenna);
    settings->setValue(QStringLiteral("SoapySDR/txAntenna"), m_txAntenna);
    settings->setValue(QStringLiteral("SoapySDR/lnaGain"), m_lnaGain);
    settings->setValue(QStringLiteral("SoapySDR/tiaGain"), m_tiaGain);
    settings->setValue(QStringLiteral("SoapySDR/pgaGain"), m_pgaGain);
    settings->setValue(QStringLiteral("SoapySDR/overallGain"), m_overallGain);
    settings->setValue(QStringLiteral("SoapySDR/autoCalibrate"), m_autoCalibrate);
    settings->setValue(QStringLiteral("SoapySDR/iqBalance"), m_iqBalance);
}
