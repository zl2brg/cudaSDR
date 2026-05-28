#ifndef CUSDR_DEVICE_IDENTITY_H
#define CUSDR_DEVICE_IDENTITY_H

#include <QVariant>
#include <QList>

#include "../cusdr_settings.h"

inline bool sameSoapyDevice(const TSoapyDevice &a, const TSoapyDevice &b) {
    if (!a.serial.isEmpty() || !b.serial.isEmpty())
        return a.driver == b.driver && a.serial == b.serial;

    return a.driver == b.driver
        && a.hardware == b.hardware
        && a.name == b.name
        && a.label == b.label
        && a.args == b.args;
}

inline bool sameHpsdrDeviceByMac(const TNetworkDevicecard &a, const TNetworkDevicecard &b) {
    return QString::fromLatin1(a.mac_address) == QString::fromLatin1(b.mac_address);
}

inline bool matchesLastConnected(const QVariant &dev, const TSDRDevice &lastDevice) {
    if (dev.canConvert<TNetworkDevicecard>() && lastDevice.deviceClass == DeviceClass_HPSDR) {
        const TNetworkDevicecard card = dev.value<TNetworkDevicecard>();
        return lastDevice.serialNumber == QString::fromLatin1(card.mac_address);
    }
#ifdef HAVE_SOAPYSDR
    if (dev.canConvert<TSoapyDevice>() && lastDevice.deviceClass == DeviceClass_SoapySDR) {
        const TSoapyDevice soapy = dev.value<TSoapyDevice>();
        return lastDevice.serialNumber == soapy.serial && lastDevice.deviceType == soapy.driver;
    }
#endif
    return false;
}

inline QVariant findLastConnectedMatch(const QList<QVariant> &devices, const TSDRDevice &lastDevice) {
    for (const QVariant &dev : devices) {
        if (matchesLastConnected(dev, lastDevice))
            return dev;
    }
    return QVariant();
}

#endif // CUSDR_DEVICE_IDENTITY_H
