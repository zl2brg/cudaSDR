#include "HardwareConfig.h"

HardwareConfig::HardwareConfig(QObject *parent)
    : QObject(parent)
    , m_hpsdrHardware(0)
    , m_checkFirmwareVersions(true)
    , m_10MhzSource(2) // mercury
    , m_122_88MhzSource(1) // mercury
    , m_rxClass(0)
    , m_rxTiming(0)
{
    m_devices.mercuryPresence = true;
    m_devices.penelopePresence = false;
    m_devices.pennylanePresence = false;
    m_devices.excaliburPresence = false;
    m_devices.alexPresence = false;
    m_devices.hermesPresence = false;
    m_devices.metisPresence = false;
    m_devices.mercuryFWVersion = 0;
    m_devices.penelopeFWVersion = 0;
    m_devices.pennylaneFWVersion = 0;
    m_devices.excaliburFWVersion = 0;
    m_devices.alexFWVersion = 0;
    m_devices.hermesFWVersion = 0;
    m_devices.metisFWVersion = 0;
}

void HardwareConfig::setHpsdrHardware(int val) {
    if (m_hpsdrHardware != val) {
        m_hpsdrHardware = val;
        emit hpsdrHardwareChanged(m_hpsdrHardware);
    }
}

void HardwareConfig::setCheckFirmwareVersions(bool val) {
    if (m_checkFirmwareVersions != val) {
        m_checkFirmwareVersions = val;
        emit checkFirmwareVersionsChanged(m_checkFirmwareVersions);
    }
}

void HardwareConfig::setSource10Mhz(int source) {
    if (m_10MhzSource != source) {
        m_10MhzSource = source;
        emit source10MhzChanged(m_10MhzSource);
    }
}

void HardwareConfig::setSource122_88Mhz(int source) {
    if (m_122_88MhzSource != source) {
        m_122_88MhzSource = source;
        emit source122_88MhzChanged(m_122_88MhzSource);
    }
}

void HardwareConfig::setRxClass(int val) {
    if (m_rxClass != val) {
        m_rxClass = val;
        emit rxClassChanged(m_rxClass);
    }
}

void HardwareConfig::setRxTiming(int val) {
    if (m_rxTiming != val) {
        m_rxTiming = val;
        emit rxTimingChanged(m_rxTiming);
    }
}

void HardwareConfig::setDevices(const THPSDRDevices &devices) {
    m_devices = devices;
    emit devicesChanged();
}

void HardwareConfig::load(const QJsonObject &json) {
    if (json.contains("hpsdrHardware")) m_hpsdrHardware = json["hpsdrHardware"].toInt();
    if (json.contains("checkFirmwareVersions")) m_checkFirmwareVersions = json["checkFirmwareVersions"].toBool();
    if (json.contains("source10Mhz")) m_10MhzSource = json["source10Mhz"].toInt();
    if (json.contains("source122_88Mhz")) m_122_88MhzSource = json["source122_88Mhz"].toInt();
    if (json.contains("rxClass")) m_rxClass = json["rxClass"].toInt();
    if (json.contains("rxTiming")) m_rxTiming = json["rxTiming"].toInt();

    if (json.contains("devices")) {
        QJsonObject d = json["devices"].toObject();
        m_devices.mercuryPresence = d["mercuryPresence"].toBool();
        m_devices.penelopePresence = d["penelopePresence"].toBool();
        m_devices.pennylanePresence = d["pennylanePresence"].toBool();
        m_devices.excaliburPresence = d["excaliburPresence"].toBool();
        m_devices.alexPresence = d["alexPresence"].toBool();
        m_devices.hermesPresence = d["hermesPresence"].toBool();
        m_devices.metisPresence = d["metisPresence"].toBool();
    }
}

void HardwareConfig::save(QJsonObject &json) const {
    json["hpsdrHardware"] = m_hpsdrHardware;
    json["checkFirmwareVersions"] = m_checkFirmwareVersions;
    json["source10Mhz"] = m_10MhzSource;
    json["source122_88Mhz"] = m_122_88MhzSource;
    json["rxClass"] = m_rxClass;
    json["rxTiming"] = m_rxTiming;

    QJsonObject d;
    d["mercuryPresence"] = m_devices.mercuryPresence;
    d["penelopePresence"] = m_devices.penelopePresence;
    d["pennylanePresence"] = m_devices.pennylanePresence;
    d["excaliburPresence"] = m_devices.excaliburPresence;
    d["alexPresence"] = m_devices.alexPresence;
    d["hermesPresence"] = m_devices.hermesPresence;
    d["metisPresence"] = m_devices.metisPresence;
    json["devices"] = d;
}
