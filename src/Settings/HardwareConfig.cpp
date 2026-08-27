#include "HardwareConfig.h"
#include <QSettings>

HardwareConfig::HardwareConfig(QObject *parent)
    : QObject(parent)
    , m_hpsdrHardware(0)
    , m_checkFirmwareVersions(true)
    , m_10MhzSource(2) // mercury
    , m_122_88MhzSource(1) // mercury
    , m_rxClass(0)
    , m_rxTiming(0)
    , m_receiverCount(1)
    , m_hwInterface(0)
    , m_dither(false)
    , m_random(false)
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

void HardwareConfig::setReceiverCount(int count) {
    if (count < 1)
        count = 1;
    if (count > 8)
        count = 8;
    if (m_receiverCount != count) {
        m_receiverCount = count;
        emit receiverCountChanged(m_receiverCount);
    }
}

void HardwareConfig::setHwInterface(int mode) {
    if (mode < 0 || mode > 3)
        mode = 0;
    if (m_hwInterface != mode) {
        m_hwInterface = mode;
        emit hwInterfaceChanged(m_hwInterface);
    }
}

void HardwareConfig::setDither(bool enabled) {
    if (m_dither != enabled) {
        m_dither = enabled;
        emit ditherChanged(m_dither);
    }
}

void HardwareConfig::setRandom(bool enabled) {
    if (m_random != enabled) {
        m_random = enabled;
        emit randomChanged(m_random);
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
    if (json.contains("receiverCount")) setReceiverCount(json["receiverCount"].toInt());
    if (json.contains("interface")) setHwInterface(json["interface"].toInt());
    if (json.contains("dither")) setDither(json["dither"].toBool());
    if (json.contains("random")) setRandom(json["random"].toBool());

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
    json["receiverCount"] = m_receiverCount;
    json["interface"] = m_hwInterface;
    json["dither"] = m_dither;
    json["random"] = m_random;

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

void HardwareConfig::loadIni(QSettings *settings) {
    QString str;
    int value;

    str = settings->value("server/10mhzsource", "mercury").toString();
    if (str == "atlas")
        value = 0;
    else if (str == "penelope")
        value = 1;
    else if (str == "mercury")
        value = 2;
    else if (str == "none")
        value = 3;
    else
        value = 2;
    setSource10Mhz(value);

    str = settings->value("server/122_88mhzsource", "mercury").toString();
    if (str == "penelope")
        value = 0;
    else if (str == "mercury")
        value = 1;
    else
        value = 1;
    setSource122_88Mhz(value);
}

void HardwareConfig::saveIni(QSettings *settings) const {
    if (m_10MhzSource == 0)
        settings->setValue("server/10mhzsource", "atlas");
    else if (m_10MhzSource == 1)
        settings->setValue("server/10mhzsource", "penelope");
    else if (m_10MhzSource == 2)
        settings->setValue("server/10mhzsource", "mercury");
    else if (m_10MhzSource == 3)
        settings->setValue("server/10mhzsource", "none");
    else
        settings->setValue("server/10mhzsource", "mercury");

    if (m_122_88MhzSource == 0)
        settings->setValue("server/122_88mhzsource", "penelope");
    else if (m_122_88MhzSource == 1)
        settings->setValue("server/122_88mhzsource", "mercury");
}
