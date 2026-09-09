/**
 * @file  SdrDeviceManager.cpp
 * @brief Centralized SDR device discovery and lifecycle management service.
 * @author Simon Eatough ZL2BRG
 */

#include "SdrDeviceManager.h"
#include "DataEngine/Drivers/SimulatedDevice.h"
#include "DataEngine/Drivers/HpsdrDevice.h"
#include "DataEngine/Drivers/SoapyDevice.h"
#include "cusdr_settings.h"
#include "Settings/SoapyConfig.h"
#include "DataEngine/cusdr_dataEngine.h"
#include "DataEngine/cusdr_dataIO.h"
#include <QTimer>

SdrDeviceManager::SdrDeviceManager(QObject* parent)
    : QObject(parent)
{
    // Always include a simulated device in the catalog
    registerDevice(simulatedDeviceInfo());
}

SdrDeviceManager::~SdrDeviceManager() = default;

SdrDeviceManager* SdrDeviceManager::instance()
{
    static SdrDeviceManager s_instance;
    return &s_instance;
}

void SdrDeviceManager::setDataEngine(DataEngine* engine)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_dataEngine = engine;
}

DataEngine* SdrDeviceManager::dataEngine() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_dataEngine;
}

QList<SdrDeviceInfo> SdrDeviceManager::availableDevices() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_devices;
}

QList<SdrDeviceInfo> SdrDeviceManager::devicesByType(DeviceType type) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    QList<SdrDeviceInfo> result;
    for (const auto& dev : m_devices) {
        if (dev.type == type) {
            result.append(dev);
        }
    }
    return result;
}

SdrDeviceInfo SdrDeviceManager::deviceById(const QString& id) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& dev : m_devices) {
        if (dev.id == id) {
            return dev;
        }
    }
    return SdrDeviceInfo();
}

bool SdrDeviceManager::hasDevice(const QString& id) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& dev : m_devices) {
        if (dev.id == id) {
            return true;
        }
    }
    return false;
}

int SdrDeviceManager::deviceCount() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_devices.size();
}

void SdrDeviceManager::selectDevice(const QString& id)
{
    SdrDeviceInfo info = deviceById(id);
    if (!info.isValid()) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_selectedDeviceId = id;
    }

    Settings* set = Settings::instance();
    if (set) {
        if (info.type == DeviceType::HpsdrP1 || info.type == DeviceType::HpsdrP2) {
            TNetworkDevicecard card;
            if (findNetworkCard(id, card)) {
                set->setCurrentHPSDRDevice(card);
            }
        } else if (info.type == DeviceType::SoapySDR) {
            TSoapyDevice dev;
            if (findSoapyDevice(id, dev)) {
                set->setCurrentSoapyDevice(dev);
            }
        }
    }

    emit selectedDeviceChanged(info);
}

SdrDeviceInfo SdrDeviceManager::selectedDevice() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_selectedDeviceId.isEmpty()) {
        if (!m_devices.isEmpty()) {
            return m_devices.first();
        }
        return simulatedDeviceInfo();
    }
    for (const auto& dev : m_devices) {
        if (dev.id == m_selectedDeviceId) {
            return dev;
        }
    }
    return SdrDeviceInfo();
}

QString SdrDeviceManager::selectedDeviceId() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_selectedDeviceId;
}

bool SdrDeviceManager::findNetworkCard(const QString& id, TNetworkDevicecard& outCard) const
{
    Settings* set = Settings::instance();
    if (!set) return false;
    const auto list = set->getMetisCardsList();
    for (const auto& card : list) {
        if (fromNetworkCard(card).id == id) {
            outCard = card;
            return true;
        }
    }
    return false;
}

bool SdrDeviceManager::findSoapyDevice(const QString& id, TSoapyDevice& outDev) const
{
    Settings* set = Settings::instance();
    if (!set) return false;
    const auto list = set->getSoapyDeviceList();
    for (const auto& dev : list) {
        if (fromSoapyDevice(dev).id == id) {
            outDev = dev;
            return true;
        }
    }
    return false;
}

void SdrDeviceManager::registerDevice(const SdrDeviceInfo& info)
{
    if (!info.isValid()) {
        return;
    }

    QList<SdrDeviceInfo> currentList;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        bool found = false;
        for (int i = 0; i < m_devices.size(); ++i) {
            if (m_devices[i].id == info.id) {
                m_devices[i] = info;
                found = true;
                break;
            }
        }
        if (!found) {
            m_devices.append(info);
        }
        currentList = m_devices;
    }

    emit deviceDiscovered(info);
    emit deviceListChanged(currentList);
}

void SdrDeviceManager::unregisterDevice(const QString& id)
{
    QList<SdrDeviceInfo> currentList;
    bool removed = false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (int i = 0; i < m_devices.size(); ++i) {
            if (m_devices[i].id == id) {
                m_devices.removeAt(i);
                removed = true;
                break;
            }
        }
        currentList = m_devices;
    }

    if (removed) {
        emit deviceRemoved(id);
        emit deviceListChanged(currentList);
    }
}

void SdrDeviceManager::clearDevices()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_devices.clear();
    }
    emit deviceListChanged({});
}

SdrDeviceInfo SdrDeviceManager::fromNetworkCard(const TNetworkDevicecard& card)
{
    SdrDeviceInfo info;
    info.id = QString::fromLatin1(card.mac_address);
    if (info.id.isEmpty()) {
        info.id = card.ip_address.toString();
    }
    info.name = card.boardName.isEmpty() ? QStringLiteral("OpenHPSDR Device") : card.boardName;
    info.type = (card.protocol == 2) ? DeviceType::HpsdrP2 : DeviceType::HpsdrP1;
    info.address = card.ip_address.toString();
    info.port = 1024;
    info.protocolVersion = card.protocol;

    DeviceCapabilities caps;
    caps.supportsTx = true;
    caps.supportsFullDuplex = (card.protocol == 2);
    caps.supportsWideband = true;
    caps.maxReceivers = qMax(1, card.max_receivers);
    caps.minFrequencyHz = static_cast<qint64>(card.frequency_min > 0 ? card.frequency_min : 100000.0);
    caps.maxFrequencyHz = static_cast<qint64>(card.frequency_max > 0 ? card.frequency_max : 60000000.0);
    caps.supportedSampleRates = {48000, 96000, 192000, 384000};
    info.capabilities = caps;

    info.properties[QStringLiteral("sw_version")] = QString::number(card.sw_version);
    info.properties[QStringLiteral("board_id")] = QString::number(card.boardID);
    info.properties[QStringLiteral("adcs")] = QString::number(card.adcs);
    info.properties[QStringLiteral("dacs")] = QString::number(card.dacs);
    info.properties[QStringLiteral("status")] = QString::number(card.status);
    return info;
}

SdrDeviceInfo SdrDeviceManager::fromSoapyDevice(const TSoapyDevice& dev)
{
    SdrDeviceInfo info;
    info.id = dev.serial.isEmpty() ? (dev.driver + QStringLiteral(":") + dev.hardware) : dev.serial;
    if (info.id.isEmpty() || info.id == QStringLiteral(":")) {
        info.id = QStringLiteral("soapy:unknown");
    }
    info.name = dev.label.isEmpty() ? (dev.name.isEmpty() ? dev.driver : dev.name) : dev.label;
    info.type = DeviceType::SoapySDR;
    info.serialNumber = dev.serial;

    DeviceCapabilities caps;
    caps.supportsTx = true;
    caps.supportsFullDuplex = false;
    caps.supportsWideband = true;
    caps.maxReceivers = 1;
    caps.minFrequencyHz = 100000;
    caps.maxFrequencyHz = 2000000000;
    caps.supportedSampleRates = {48000, 96000, 192000, 384000, 768000, 1536000, 2048000};
    info.capabilities = caps;

    info.properties[QStringLiteral("driver")] = dev.driver;
    info.properties[QStringLiteral("hardware")] = dev.hardware;
    info.properties[QStringLiteral("name")] = dev.name;
    for (auto it = dev.args.constBegin(); it != dev.args.constEnd(); ++it) {
        info.properties[it.key()] = it.value();
    }
    return info;
}

SdrDeviceInfo SdrDeviceManager::simulatedDeviceInfo()
{
    SdrDeviceInfo info;
    info.id = QStringLiteral("simulated:0");
    info.name = QStringLiteral("Simulated SDR Device");
    info.type = DeviceType::Simulated;
    SimulatedDevice sim;
    info.capabilities = sim.capabilities();
    return info;
}

void SdrDeviceManager::registerNetworkCards(const QList<TNetworkDevicecard>& cards)
{
    for (const auto& card : cards) {
        registerDevice(fromNetworkCard(card));
    }
}

void SdrDeviceManager::registerSoapyDevices(const QList<TSoapyDevice>& devices)
{
    for (const auto& dev : devices) {
        registerDevice(fromSoapyDevice(dev));
    }
}

std::unique_ptr<ISdrDevice> SdrDeviceManager::createDevice(const SdrDeviceInfo& info, DataEngine* engine)
{
    if (!engine) {
        std::lock_guard<std::mutex> lock(m_mutex);
        engine = m_dataEngine;
    }

    switch (info.type) {
    case DeviceType::Simulated:
        return createSimulatedDevice();

    case DeviceType::HpsdrP1:
    case DeviceType::HpsdrP2: {
        DataIO* io = engine ? engine->m_dataIO : nullptr;
        IHPSDRProtocol* proto = engine ? engine->m_protocol.get() : nullptr;
        auto dev = std::make_unique<HpsdrDevice>(io, proto, info.type == DeviceType::HpsdrP2);
        dev->setDeviceName(info.name);
        return dev;
    }

    case DeviceType::SoapySDR: {
        SoapySDRDataSource* source = engine ? engine->m_soapySDRSource : nullptr;
        return std::make_unique<SoapyDevice>(engine, source);
    }

    default:
        return nullptr;
    }
}

std::unique_ptr<ISdrDevice> SdrDeviceManager::createDevice(DeviceType type, DataEngine* engine)
{
    const auto devs = devicesByType(type);
    if (!devs.isEmpty()) {
        return createDevice(devs.first(), engine);
    }

    // Fallback default info
    SdrDeviceInfo fallback;
    fallback.type = type;
    fallback.id = QStringLiteral("default");
    fallback.name = (type == DeviceType::Simulated) ? QStringLiteral("Simulated SDR") : QStringLiteral("Default SDR");
    return createDevice(fallback, engine);
}

std::unique_ptr<ISdrDevice> SdrDeviceManager::createSimulatedDevice()
{
    return std::make_unique<SimulatedDevice>();
}

void SdrDeviceManager::startDiscovery(bool async)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_discovering = true;
        m_pendingScans.clear();
        if (async) {
            m_pendingScans.insert(QStringLiteral("HPSDR"));
#ifdef HAVE_SOAPYSDR
            m_pendingScans.insert(QStringLiteral("SoapySDR"));
#endif
        }
    }
    emit discoveryStarted();

    // Ensure simulated device is registered
    registerDevice(simulatedDeviceInfo());

    Settings* set = Settings::instance();
    if (set) {
        if (async) {
            set->searchDevices();
            // Safety timeout in case a driver stalls
            QTimer::singleShot(4000, this, [this]() {
                if (isDiscovering()) {
                    stopDiscovery();
                }
            });
            return;
        }

        // Synchronous import of cached results
        registerNetworkCards(set->getMetisCardsList());
        registerSoapyDevices(set->getSoapyDeviceList());
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_discovering = false;
        m_pendingScans.clear();
    }
    emit discoveryFinished();
}

void SdrDeviceManager::notifyDiscoveryStepFinished(const QString& scanName, int count)
{
    Q_UNUSED(count)
    bool finished = false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_pendingScans.remove(scanName);
        if (m_discovering && m_pendingScans.isEmpty()) {
            m_discovering = false;
            finished = true;
        }
    }
    if (finished) {
        emit discoveryFinished();
    }
}

void SdrDeviceManager::stopDiscovery()
{
    bool wasDiscovering = false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        wasDiscovering = m_discovering;
        m_discovering = false;
        m_pendingScans.clear();
    }
    if (wasDiscovering) {
        emit discoveryFinished();
    }
}

bool SdrDeviceManager::isDiscovering() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_discovering;
}
