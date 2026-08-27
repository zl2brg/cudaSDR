#ifndef SETTINGSTYPES_H
#define SETTINGSTYPES_H

#include <QMetaType>
#include <QString>

/** Written by Settings::toJson(); fromJson accepts this and older versions. */
constexpr int kSettingsJsonSchemaVersion = 2;

namespace QSDR {
    enum _DSPCore {
        QtDSP,
        CudaDSP,
        ExternalDSP
    };
}

typedef enum _panGraphicsMode {
    Line,		// 0
    FilledLine, // 1
    Solid		// 2
} PanGraphicsMode;

typedef enum _waterfallColorMode {
    Simple,		// 0
    Enhanced	// 1
} WaterfallColorMode;

typedef enum _panAveragingMode {
    AV_MODE_NONE,
    AV_MODE_RECURSIVE,
    AV_MODE_TIME_WINDOW,
    AV_MODE_LOG_RECURSIVE
} PanAveragingMode;

typedef enum _panDetectorMode {
    DET_MODE_PEAK,
    DET_MODE_ROSENFELL,
    DET_MODE_AVERAGE,
    DET_MODE_SAMPLE
} PanDetectorMode;

typedef enum _radioState {
    RX,
    MOX,
    TUNE,
    DUPLEX
} RadioState;

typedef enum _deviceClass {
    DeviceClass_None = 0,
    DeviceClass_HPSDR = 1,
    DeviceClass_SoapySDR = 2
} DeviceClass;

typedef struct _SDRDevice {
    DeviceClass deviceClass = DeviceClass_None;
    QString     deviceType;    // e.g. "Hermes", "rtlsdr"
    QString     serialNumber;  // MAC for HPSDR, serial for Soapy
    QString     label;         // Display name
} TSDRDevice;

Q_DECLARE_METATYPE(QSDR::_DSPCore)
Q_DECLARE_METATYPE(PanGraphicsMode)
Q_DECLARE_METATYPE(WaterfallColorMode)
Q_DECLARE_METATYPE(PanAveragingMode)
Q_DECLARE_METATYPE(PanDetectorMode)
Q_DECLARE_METATYPE(RadioState)
Q_DECLARE_METATYPE(DeviceClass)
Q_DECLARE_METATYPE(TSDRDevice)

#endif // SETTINGSTYPES_H
