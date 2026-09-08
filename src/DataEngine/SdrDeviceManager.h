/**
 * @file  SdrDeviceManager.h
 * @brief Centralized SDR device discovery and lifecycle management service.
 * @author Simon Eatough ZL2BRG
 */

#ifndef CUDASDR_SDR_DEVICE_MANAGER_H
#define CUDASDR_SDR_DEVICE_MANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <memory>
#include <mutex>
#include "DataEngine/ISdrDevice.h"

class DataEngine;
class DataIO;
struct _networkDeviceCard;
typedef struct _networkDeviceCard TNetworkDevicecard;
struct _TSoapyDevice;
typedef struct _TSoapyDevice TSoapyDevice;

/**
 * @struct SdrDeviceInfo
 * @brief Standardized device metadata record for any SDR hardware.
 */
struct SdrDeviceInfo {
    QString id;                             ///< Unique identifier (MAC address, hardware key, or "simulated")
    QString name;                           ///< Display name (e.g. "Hermes", "LimeSDR", "PlutoSDR")
    DeviceType type = DeviceType::None;    ///< Underlying architecture type
    QString address;                        ///< IP address or URI string
    quint16 port = 0;                       ///< UDP/TCP port
    QString serialNumber;                   ///< Serial number if available
    int protocolVersion = 0;                ///< 1 or 2 for OpenHPSDR, 0 otherwise
    DeviceCapabilities capabilities;        ///< Hardware capabilities descriptor
    QMap<QString, QString> properties;      ///< Extra driver/device-specific kwargs

    bool isValid() const { return !id.isEmpty() && type != DeviceType::None; }
};

/**
 * @class SdrDeviceManager
 * @brief Centralized coordinator for SDR device discovery, cataloging, and factory instantiation.
 */
class SdrDeviceManager : public QObject {
    Q_OBJECT

public:
    explicit SdrDeviceManager(QObject* parent = nullptr);
    ~SdrDeviceManager() override;

    static SdrDeviceManager* instance();

    // Catalog queries
    QList<SdrDeviceInfo> availableDevices() const;
    QList<SdrDeviceInfo> devicesByType(DeviceType type) const;
    SdrDeviceInfo deviceById(const QString& id) const;
    bool hasDevice(const QString& id) const;
    int deviceCount() const;

    // Device registration & conversion
    void registerDevice(const SdrDeviceInfo& info);
    void unregisterDevice(const QString& id);
    void clearDevices();

    // Adapters for legacy discovery data
    void registerNetworkCards(const QList<TNetworkDevicecard>& cards);
    void registerSoapyDevices(const QList<TSoapyDevice>& devices);
    static SdrDeviceInfo fromNetworkCard(const TNetworkDevicecard& card);
    static SdrDeviceInfo fromSoapyDevice(const TSoapyDevice& dev);
    static SdrDeviceInfo simulatedDeviceInfo();

    // Factory methods
    std::unique_ptr<ISdrDevice> createDevice(const SdrDeviceInfo& info, DataEngine* engine = nullptr);
    std::unique_ptr<ISdrDevice> createDevice(DeviceType type, DataEngine* engine = nullptr);
    std::unique_ptr<ISdrDevice> createSimulatedDevice();

    // Discovery control
    void startDiscovery();
    void stopDiscovery();
    bool isDiscovering() const;

signals:
    void deviceDiscovered(const SdrDeviceInfo& device);
    void deviceRemoved(const QString& id);
    void deviceListChanged(const QList<SdrDeviceInfo>& devices);
    void discoveryStarted();
    void discoveryFinished();

private:
    mutable std::mutex m_mutex;
    QList<SdrDeviceInfo> m_devices;
    bool m_discovering = false;
};

#endif // CUDASDR_SDR_DEVICE_MANAGER_H
