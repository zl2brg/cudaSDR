#ifndef HARDWARECONFIG_H
#define HARDWARECONFIG_H

#include <QObject>
#include <QJsonObject>

typedef struct _hpsdrDevices {
    bool 	mercuryPresence;
    bool 	penelopePresence;
    bool 	pennylanePresence;
    bool 	excaliburPresence;
    bool 	alexPresence;
    bool	hermesPresence;
    bool	metisPresence;

    unsigned char 	mercuryFWVersion;
    unsigned char 	penelopeFWVersion;
    unsigned char 	pennylaneFWVersion;
    unsigned char 	excaliburFWVersion;
    unsigned char 	alexFWVersion;
    unsigned char	hermesFWVersion;
    unsigned char  	metisFWVersion;
} THPSDRDevices;

class QSettings;

class HardwareConfig : public QObject {
    Q_OBJECT
    Q_PROPERTY(int hpsdrHardware READ hpsdrHardware WRITE setHpsdrHardware NOTIFY hpsdrHardwareChanged)
    Q_PROPERTY(bool checkFirmwareVersions READ checkFirmwareVersions WRITE setCheckFirmwareVersions NOTIFY checkFirmwareVersionsChanged)
    Q_PROPERTY(int source10Mhz READ source10Mhz WRITE setSource10Mhz NOTIFY source10MhzChanged)
    Q_PROPERTY(int source122_88Mhz READ source122_88Mhz WRITE setSource122_88Mhz NOTIFY source122_88MhzChanged)
    Q_PROPERTY(int rxClass READ rxClass WRITE setRxClass NOTIFY rxClassChanged)
    Q_PROPERTY(int rxTiming READ rxTiming WRITE setRxTiming NOTIFY rxTimingChanged)

public:
    explicit HardwareConfig(QObject *parent = nullptr);

    int hpsdrHardware() const { return m_hpsdrHardware; }
    void setHpsdrHardware(int val);

    bool checkFirmwareVersions() const { return m_checkFirmwareVersions; }
    void setCheckFirmwareVersions(bool val);

    int source10Mhz() const { return m_10MhzSource; }
    void setSource10Mhz(int source);

    int source122_88Mhz() const { return m_122_88MhzSource; }
    void setSource122_88Mhz(int source);

    int rxClass() const { return m_rxClass; }
    void setRxClass(int val);

    int rxTiming() const { return m_rxTiming; }
    void setRxTiming(int val);

    int receiverCount() const { return m_receiverCount; }
    void setReceiverCount(int count);

    /** QSDR::_HWInterfaceMode as int (NoInterfaceMode=0, Metis=1, Hermes=2, SoapySDR=3). */
    int hwInterface() const { return m_hwInterface; }
    void setHwInterface(int mode);

    bool dither() const { return m_dither; }
    void setDither(bool enabled);

    bool random() const { return m_random; }
    void setRandom(bool enabled);

    THPSDRDevices devices() const { return m_devices; }
    void setDevices(const THPSDRDevices &devices);

    void load(const QJsonObject &json);
    void save(QJsonObject &json) const;

    void loadIni(QSettings *settings);
    void saveIni(QSettings *settings) const;

signals:
    void hpsdrHardwareChanged(int val);
    void checkFirmwareVersionsChanged(bool val);
    void source10MhzChanged(int source);
    void source122_88MhzChanged(int source);
    void rxClassChanged(int val);
    void rxTimingChanged(int val);
    void receiverCountChanged(int count);
    void hwInterfaceChanged(int mode);
    void ditherChanged(bool enabled);
    void randomChanged(bool enabled);
    void devicesChanged();

private:
    int m_hpsdrHardware;
    bool m_checkFirmwareVersions;
    int m_10MhzSource;
    int m_122_88MhzSource;
    int m_rxClass;
    int m_rxTiming;
    int m_receiverCount;
    int m_hwInterface;
    bool m_dither;
    bool m_random;
    THPSDRDevices m_devices;
};

#endif // HARDWARECONFIG_H
