/**
 * @file  IDeviceDiscoverer.h
 * @brief Abstract interface for SDR device discovery.
 *
 * Concrete implementations:
 *   - Discoverer        : HPSDR Metis/Hermes UDP broadcast discovery
 *   - Protocol2Discoverer (future): Protocol 2 TCP handshake discovery
 *   - SoapyDiscoverer   (future): SoapySDR::Device::enumerate() wrapper
 */

#ifndef IDEVICEDISCOVERER_H
#define IDEVICEDISCOVERER_H

#include <QObject>

class IDeviceDiscoverer : public QObject {
    Q_OBJECT

public:
    explicit IDeviceDiscoverer(QObject* parent = nullptr) : QObject(parent) {}
    ~IDeviceDiscoverer() override = default;

    IDeviceDiscoverer(const IDeviceDiscoverer&)            = delete;
    IDeviceDiscoverer& operator=(const IDeviceDiscoverer&) = delete;

    /**
     * @brief Perform synchronous device discovery.
     * @return Number of devices found, or -1 on error.
     */
    virtual int  findDevices() = 0;

    /** @brief Clear the internal list of discovered devices. */
    virtual void clear() = 0;

    /** @brief Gracefully shut down / release the physical device. */
    virtual void shutdown() = 0;

public slots:
    /**
     * @brief Run discovery + initialisation.
     *        Connected to the owning QThread::started() signal by DataEngine.
     */
    virtual void initDevice() = 0;
};

#endif // IDEVICEDISCOVERER_H
