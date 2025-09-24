/****************************************************************************
** Meta object code from reading C++ file 'cusdr_settings.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Source/src/cusdr_settings.h"
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cusdr_settings.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN8SettingsE_t {};
} // unnamed namespace

template <> constexpr inline auto Settings::qt_create_metaobjectdata<qt_meta_tag_ZN8SettingsE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Settings",
        "systemMessageEvent",
        "",
        "msg",
        "masterSwitchChanged",
        "sender",
        "power",
        "radioStateChanged",
        "RadioState",
        "state",
        "systemStateChanged",
        "QSDR::_Error",
        "err",
        "QSDR::_HWInterfaceMode",
        "hwmode",
        "QSDR::_ServerMode",
        "mode",
        "QSDR::_DataEngineState",
        "graphicModeChanged",
        "rx",
        "PanGraphicsMode",
        "panMode",
        "WaterfallColorMode",
        "waterfallColorMode",
        "moxStateChanged",
        "tuneStateChanged",
        "cpuLoadChanged",
        "load",
        "txAllowedChanged",
        "value",
        "multiRxViewChanged",
        "view",
        "sMeterValueChanged",
        "spectrumBufferChanged",
        "qVectorFloat",
        "buffer",
        "postSpectrumBufferChanged",
        "const float*",
        "sampleSizeChanged",
        "size",
        "rxListChanged",
        "QList<Receiver*>",
        "rxList",
        "clientConnectedChanged",
        "connect",
        "clientNoConnectedChanged",
        "client",
        "audioRxChanged",
        "receiverChanged",
        "currentReceiverChanged",
        "connectedChanged",
        "clientConnectedEvent",
        "clientDisconnectedEvent",
        "rxConnectedStatusChanged",
        "framesPerSecondChanged",
        "settingsFilenameChanged",
        "filename",
        "settingsLoadedChanged",
        "loaded",
        "newServerNetworkInterface",
        "nicName",
        "ipAddress",
        "newHPSDRDeviceNIC",
        "serverNICChanged",
        "hpsdrDeviceNICChanged",
        "socketBufferSizeChanged",
        "manualSocketBufferChanged",
        "metisCardListChanged",
        "QList<TNetworkDevicecard>",
        "list",
        "hpsdrDevicesChanged",
        "THPSDRDevices",
        "devices",
        "hpsdrNetworkDeviceChanged",
        "TNetworkDevicecard",
        "card",
        "networkDeviceNumberChanged",
        "networkIOComboBoxEntryAdded",
        "str",
        "clearNetworkIOComboBoxEntrySignal",
        "searchMetisSignal",
        "serverAddrChanged",
        "addr",
        "hpsdrDeviceLocalAddrChanged",
        "serverPortChanged",
        "port",
        "listenPortChanged",
        "audioPortChanged",
        "metisPortChanged",
        "showNetworkIO",
        "showWarning",
        "callsignChanged",
        "mouseWheelFreqStepChanged",
        "mainVolumeChanged",
        "volume",
        "hpsdrHardwareChanged",
        "hermesVersionChanged",
        "mercuryPresenceChanged",
        "mercuryVersionChanged",
        "penelopePresenceChanged",
        "penelopeVersionChanged",
        "pennyLanePresenceChanged",
        "pennyLaneVersionChanged",
        "alexPresenceChanged",
        "excaliburPresenceChanged",
        "metisVersionChanged",
        "alexConfigurationChanged",
        "config",
        "alexStatesChanged",
        "QList<int>",
        "states",
        "alexStateChanged",
        "HamBand",
        "band",
        "alexManualStateChanged",
        "checkFirmwareVersionChanged",
        "pennyOCEnabledChanged",
        "rxJ6PinsChanged",
        "txJ6PinsChanged",
        "protocolSyncChanged",
        "adcOverflowChanged",
        "packetLossChanged",
        "sendIQSignalChanged",
        "rcveIQSignalChanged",
        "numberOfRXChanged",
        "sampleRateChanged",
        "mercuryAttenuatorChanged",
        "ditherChanged",
        "randomChanged",
        "src10MhzChanged",
        "source",
        "src122_88MhzChanged",
        "micSourceChanged",
        "micInputChanged",
        "classChanged",
        "timingChanged",
        "controlBytesOutChanged",
        "uchar*",
        "values",
        "ctrFrequencyChanged",
        "frequency",
        "vfoFrequencyChanged",
        "ncoFrequencyChanged",
        "widebandSpectrumBufferChanged",
        "widebandOptionsChanged",
        "TWideband",
        "options",
        "widebandSpectrumBufferReset",
        "widebandStatusChanged",
        "widebandDataChanged",
        "widebanddBmScaleMinChanged",
        "widebanddBmScaleMaxChanged",
        "wideBandScalePositionChanged",
        "position",
        "panAveragingModeChanged",
        "panDetectorModeChanged",
        "fftSizeChanged",
        "fmsqLevelChanged",
        "level",
        "iqPortChanged",
        "hamBandChanged",
        "byButton",
        "dspModeChanged",
        "DSPMode",
        "adcModeChanged",
        "ADCMode",
        "agcModeChanged",
        "AGCMode",
        "hangEnabled",
        "agcHangEnabledChanged",
        "hang",
        "agcGainChanged",
        "agcThresholdChanged_dB",
        "agcFixedGainChanged_dB",
        "agcMaximumGainChanged_dB",
        "agcHangThresholdChanged",
        "agcHangThresholdSliderChanged",
        "agcHangLevelChanged_dB",
        "agcLineLevelsChanged",
        "thresh",
        "agcVariableGainChanged_dB",
        "agcAttackTimeChanged",
        "agcDecayTimeChanged",
        "agcHangTimeChanged",
        "filterFrequenciesChanged",
        "low",
        "high",
        "cudaDevicesChanged",
        "cudaDriverChanged",
        "cudaRuntimeChanged",
        "cudaCurrentDeviceChanged",
        "cudaLastDeviceChanged",
        "freqRulerPositionChanged",
        "audioFormatChanged",
        "QAudioFormat",
        "format",
        "audioPositionChanged",
        "audioBufferChanged",
        "length",
        "displayWidgetHeightChanged",
        "spectrumSizeChanged",
        "panadapterColorChanged",
        "panGridStatusChanged",
        "peakHoldStatusChanged",
        "panLockedStatusChanged",
        "clickVFOStatusChanged",
        "hairCrossStatusChanged",
        "showAGCLinesStatusChanged",
        "spectrumAveragingChanged",
        "spectrumAveragingCntChanged",
        "waterfallTimeChanged",
        "waterfallOffesetLoChanged",
        "waterfallOffesetHiChanged",
        "sMeterHoldTimeChanged",
        "dBmScaleMinChanged",
        "dBmScaleMaxChanged",
        "agcMaximumGainChanged",
        "noiseBlankerChanged",
        "noiseFilterChanged",
        "nr2GainMethodChanged",
        "nr2NpeMethodChanged",
        "nrAgcChanged",
        "nr2AeChanged",
        "snbChanged",
        "anfChanged",
        "micInputLevelChanged",
        "driveLevelChanged",
        "repeaterModeChanged",
        "repeaterOffsetchanged",
        "fmPremphasizechanged",
        "fmdeveationchanged",
        "amCarrierlevelchanged",
        "audioCompressionchanged",
        "micModeChanged",
        "showRadioPopupChanged",
        "receiverDataReady",
        "CwHangTimeChanged",
        "CwHangTime",
        "CwSidetoneFreqChanged",
        "CwSidetoneFreq",
        "CwKeyReversedChanged",
        "CwKeyReversed",
        "CwKeyerModeChanged",
        "CwKeyerMode",
        "InternalCwChanged",
        "InternalCW",
        "CwKeyerSpeedChanged",
        "CwKeyerSpeed",
        "CwPttDelayChanged",
        "CwPttDelay",
        "CwSidetoneVolumeChanged",
        "CwSidetoneVolume",
        "CwKeyerWeightChanged",
        "CwKeyerWeight",
        "CwKeyerSpacingChanged",
        "CwKeyerSpacing",
        "setMainPower",
        "setDefaultSkin",
        "setSettingsFilename",
        "setSystemMessage",
        "time",
        "setSettingsLoaded",
        "setCPULoad",
        "setCallsign",
        "callsign",
        "setPBOPresence",
        "setFBOPresence",
        "setMainVolume",
        "setMainVolumeMute",
        "setSystemState",
        "setGraphicsState",
        "setTxAllowed",
        "setRadioState",
        "getRadioState",
        "setMultiRxView",
        "setSMeterValue",
        "setSpectrumBuffer",
        "QList<float>",
        "setPostSpectrumBuffer",
        "setSampleSize",
        "setRxList",
        "setMetisCardList",
        "searchHpsdrNetworkDevices",
        "clearMetisCardList",
        "setHPSDRDeviceNumber",
        "setCurrentHPSDRDevice",
        "addNetworkIOComboBoxEntry",
        "clearNetworkIOComboBoxEntry",
        "addServerNetworkInterface",
        "addHPSDRDeviceNIC",
        "setNumberOfNetworkInterfaces",
        "setServerNetworkInterface",
        "index",
        "setHPSDRDeviceNIC",
        "setServerWidgetNIC",
        "setHPSDRWidgetNIC",
        "setServerAddr",
        "setHPSDRDeviceLocalAddr",
        "setServerPort",
        "setListenPort",
        "setAudioPort",
        "setMetisPort",
        "setClientConnected",
        "setClientNoConnected",
        "setRxConnectedStatus",
        "setAudioRx",
        "setConnected",
        "setCheckFirmwareVersion",
        "setHPSDRDevices",
        "setHermesVersion",
        "setHPSDRHardware",
        "setMercuryPresence",
        "setMercuryVersion",
        "setPenelopePresence",
        "setPenelopeVersion",
        "setPennyLanePresence",
        "setPennyLaneVersion",
        "setAlexPresence",
        "setExcaliburPresence",
        "setMetisVersion",
        "setAlexConfiguration",
        "conf",
        "setAlexHPFLoFrequencies",
        "filter",
        "setAlexHPFHiFrequencies",
        "setAlexLPFLoFrequencies",
        "setAlexLPFHiFrequencies",
        "setAlexStates",
        "setAlexState",
        "pos",
        "setAlexToManual",
        "checkAlexState",
        "setPennyOCEnabled",
        "setRxJ6Pin",
        "setRxJ6Pins",
        "setTxJ6Pin",
        "setTxJ6Pins",
        "setIQPort",
        "setProtocolSync",
        "setADCOverflow",
        "setPacketLoss",
        "setSendIQ",
        "setRcveIQ",
        "setReceivers",
        "setCurrentReceiver",
        "setSampleRate",
        "setMercuryAttenuator",
        "setDither",
        "setRandom",
        "set10MhzSource",
        "set122_88MhzSource",
        "setMicSource",
        "setMicInputDev",
        "setMicInputLevel",
        "setDriveLevel",
        "setClass",
        "setTiming",
        "setCtrFrequency",
        "getCtrFrequency",
        "setVFOFrequency",
        "setVfoFrequency",
        "getVfoFrequency",
        "setNCOFrequency",
        "clientDisconnected",
        "setFramesPerSecond",
        "setMouseWheelFreqStep",
        "setSocketBufferSize",
        "setManualSocketBufferSize",
        "setReceiverDataReady",
        "setSpectrumSize",
        "setdBmPanScaleMin",
        "setdBmPanScaleMax",
        "setdBmDistScaleMin",
        "setdBmDistScaleMax",
        "setHamBand",
        "setDSPMode",
        "setADCMode",
        "setAGCMode",
        "setAGCGain",
        "setAGCMaximumGain_dB",
        "setAGCFixedGain_dB",
        "setAGCThreshold_dB",
        "setAGCHangThresholdSlider",
        "setAGCHangThreshold",
        "setAGCHangLevel_dB",
        "setAGCLineLevels",
        "setAGCShowLines",
        "setAGCVariableGain_dB",
        "setAGCAttackTime",
        "setAGCDecayTime",
        "setAGCHangTime",
        "setRXFilter",
        "setfftSize",
        "setfmsqLevel",
        "setWidebandBuffers",
        "setWidebandSpectrumBuffer",
        "resetWidebandSpectrumBuffer",
        "setWidebandOptions",
        "setWidebandStatus",
        "setWidebandData",
        "setWidebanddBmScaleMin",
        "setWidebanddBmScaleMax",
        "setWideBandRulerPosition",
        "setFreqRulerPosition",
        "setAudioFormat",
        "setAudioPosition",
        "setAudioBuffer",
        "moveDisplayWidget",
        "setPanadapterColors",
        "TPanadapterColors",
        "type",
        "setPanGrid",
        "setPeakHold",
        "setPanLocked",
        "setClickVFO",
        "setHairCross",
        "setSpectrumAveraging",
        "setSpectrumAveragingCnt",
        "setWaterfallTime",
        "setWaterfallOffesetLo",
        "setWaterfallOffesetHi",
        "setPanAveragingMode",
        "PanAveragingMode",
        "setPanDetectorMode",
        "PanDetectorMode",
        "setNoiseBlankerMode",
        "nb",
        "setNoiseFilterMode",
        "nr",
        "setNR2GainMethod",
        "setNR2NpeMethod",
        "setNRAgc",
        "setNR2Ae",
        "setAnf",
        "setSnb",
        "setRepeaterMode",
        "setRepeaterOffset",
        "offset",
        "setAudioCompression",
        "setAMCarrierLevel",
        "setFMPreEmphasize",
        "setFmDeveation",
        "setCwHangTime",
        "setCwSidetoneFreq",
        "setCwKeyReversed",
        "setCwKeyerMode",
        "setInternalCw",
        "setCwKeyerSpeed",
        "setCwPttDelay",
        "setCwSidetoneVolume",
        "setCwKeyerWeight",
        "setCwKeyerSpacing",
        "setSMeterHoldTime",
        "showNetworkIODialog",
        "showWarningDialog",
        "showRadioPopupWidget",
        "getCtrFrequencies",
        "QList<long>",
        "getVfoFrequencies"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'systemMessageEvent'
        QtMocHelpers::SignalData<void(const QString &, int)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::Int, 2 },
        }}),
        // Signal 'masterSwitchChanged'
        QtMocHelpers::SignalData<void(QObject *, bool)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Bool, 6 },
        }}),
        // Signal 'radioStateChanged'
        QtMocHelpers::SignalData<void(RadioState)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 8, 9 },
        }}),
        // Signal 'systemStateChanged'
        QtMocHelpers::SignalData<void(QObject *, QSDR::_Error, QSDR::_HWInterfaceMode, QSDR::_ServerMode, QSDR::_DataEngineState)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { 0x80000000 | 11, 12 }, { 0x80000000 | 13, 14 }, { 0x80000000 | 15, 16 },
            { 0x80000000 | 17, 9 },
        }}),
        // Signal 'graphicModeChanged'
        QtMocHelpers::SignalData<void(QObject *, int, PanGraphicsMode, WaterfallColorMode)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { 0x80000000 | 20, 21 }, { 0x80000000 | 22, 23 },
        }}),
        // Signal 'moxStateChanged'
        QtMocHelpers::SignalData<void(QObject *, RadioState)>(24, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { 0x80000000 | 8, 2 },
        }}),
        // Signal 'tuneStateChanged'
        QtMocHelpers::SignalData<void(QObject *, RadioState)>(25, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { 0x80000000 | 8, 2 },
        }}),
        // Signal 'cpuLoadChanged'
        QtMocHelpers::SignalData<void(short)>(26, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Short, 27 },
        }}),
        // Signal 'txAllowedChanged'
        QtMocHelpers::SignalData<void(QObject *, bool)>(28, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Bool, 29 },
        }}),
        // Signal 'multiRxViewChanged'
        QtMocHelpers::SignalData<void(int)>(30, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 31 },
        }}),
        // Signal 'sMeterValueChanged'
        QtMocHelpers::SignalData<void(int, double)>(32, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Double, 29 },
        }}),
        // Signal 'spectrumBufferChanged'
        QtMocHelpers::SignalData<void(int, const qVectorFloat &)>(33, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { 0x80000000 | 34, 35 },
        }}),
        // Signal 'postSpectrumBufferChanged'
        QtMocHelpers::SignalData<void(int, const float *)>(36, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { 0x80000000 | 37, 35 },
        }}),
        // Signal 'sampleSizeChanged'
        QtMocHelpers::SignalData<void(int, int)>(38, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Int, 39 },
        }}),
        // Signal 'rxListChanged'
        QtMocHelpers::SignalData<void(QList<Receiver*>)>(40, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 41, 42 },
        }}),
        // Signal 'clientConnectedChanged'
        QtMocHelpers::SignalData<void(QObject *, bool)>(43, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Bool, 44 },
        }}),
        // Signal 'clientNoConnectedChanged'
        QtMocHelpers::SignalData<void(QObject *, int)>(45, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 46 },
        }}),
        // Signal 'audioRxChanged'
        QtMocHelpers::SignalData<void(QObject *, int)>(47, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 },
        }}),
        // Signal 'receiverChanged'
        QtMocHelpers::SignalData<void(int)>(48, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Signal 'currentReceiverChanged'
        QtMocHelpers::SignalData<void(QObject *, int)>(49, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 },
        }}),
        // Signal 'connectedChanged'
        QtMocHelpers::SignalData<void(QObject *, bool)>(50, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Bool, 44 },
        }}),
        // Signal 'clientConnectedEvent'
        QtMocHelpers::SignalData<void(int)>(51, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 46 },
        }}),
        // Signal 'clientDisconnectedEvent'
        QtMocHelpers::SignalData<void(int)>(52, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 46 },
        }}),
        // Signal 'rxConnectedStatusChanged'
        QtMocHelpers::SignalData<void(QObject *, int, bool)>(53, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::Bool, 29 },
        }}),
        // Signal 'framesPerSecondChanged'
        QtMocHelpers::SignalData<void(QObject *, int, int)>(54, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::Int, 29 },
        }}),
        // Signal 'settingsFilenameChanged'
        QtMocHelpers::SignalData<void(QString)>(55, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 56 },
        }}),
        // Signal 'settingsLoadedChanged'
        QtMocHelpers::SignalData<void(bool)>(57, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 58 },
        }}),
        // Signal 'newServerNetworkInterface'
        QtMocHelpers::SignalData<void(QString, QString)>(59, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 60 }, { QMetaType::QString, 61 },
        }}),
        // Signal 'newHPSDRDeviceNIC'
        QtMocHelpers::SignalData<void(QString, QString)>(62, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 60 }, { QMetaType::QString, 61 },
        }}),
        // Signal 'serverNICChanged'
        QtMocHelpers::SignalData<void(int)>(63, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 2 },
        }}),
        // Signal 'hpsdrDeviceNICChanged'
        QtMocHelpers::SignalData<void(int)>(64, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 2 },
        }}),
        // Signal 'socketBufferSizeChanged'
        QtMocHelpers::SignalData<void(QObject *, int)>(65, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 29 },
        }}),
        // Signal 'manualSocketBufferChanged'
        QtMocHelpers::SignalData<void(QObject *, bool)>(66, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Bool, 29 },
        }}),
        // Signal 'metisCardListChanged'
        QtMocHelpers::SignalData<void(const QList<TNetworkDevicecard> &)>(67, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 68, 69 },
        }}),
        // Signal 'hpsdrDevicesChanged'
        QtMocHelpers::SignalData<void(QObject *, THPSDRDevices)>(70, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { 0x80000000 | 71, 72 },
        }}),
        // Signal 'hpsdrNetworkDeviceChanged'
        QtMocHelpers::SignalData<void(TNetworkDevicecard)>(73, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 74, 75 },
        }}),
        // Signal 'networkDeviceNumberChanged'
        QtMocHelpers::SignalData<void(int)>(76, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Signal 'networkIOComboBoxEntryAdded'
        QtMocHelpers::SignalData<void(QString)>(77, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 78 },
        }}),
        // Signal 'clearNetworkIOComboBoxEntrySignal'
        QtMocHelpers::SignalData<void()>(79, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'searchMetisSignal'
        QtMocHelpers::SignalData<void()>(80, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'serverAddrChanged'
        QtMocHelpers::SignalData<void(QObject *, QString)>(81, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::QString, 82 },
        }}),
        // Signal 'hpsdrDeviceLocalAddrChanged'
        QtMocHelpers::SignalData<void(QObject *, QString)>(83, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::QString, 82 },
        }}),
        // Signal 'serverPortChanged'
        QtMocHelpers::SignalData<void(QObject *, quint16)>(84, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::UShort, 85 },
        }}),
        // Signal 'listenPortChanged'
        QtMocHelpers::SignalData<void(QObject *, quint16)>(86, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::UShort, 85 },
        }}),
        // Signal 'audioPortChanged'
        QtMocHelpers::SignalData<void(QObject *, quint16)>(87, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::UShort, 85 },
        }}),
        // Signal 'metisPortChanged'
        QtMocHelpers::SignalData<void(QObject *, quint16)>(88, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::UShort, 85 },
        }}),
        // Signal 'showNetworkIO'
        QtMocHelpers::SignalData<void()>(89, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'showWarning'
        QtMocHelpers::SignalData<void(const QString &)>(90, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'callsignChanged'
        QtMocHelpers::SignalData<void()>(91, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'mouseWheelFreqStepChanged'
        QtMocHelpers::SignalData<void(QObject *, int, qreal)>(92, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::QReal, 29 },
        }}),
        // Signal 'mainVolumeChanged'
        QtMocHelpers::SignalData<void(QObject *, int, float)>(93, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::Float, 94 },
        }}),
        // Signal 'hpsdrHardwareChanged'
        QtMocHelpers::SignalData<void(int)>(95, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Signal 'hermesVersionChanged'
        QtMocHelpers::SignalData<void(int)>(96, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Signal 'mercuryPresenceChanged'
        QtMocHelpers::SignalData<void(bool)>(97, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 29 },
        }}),
        // Signal 'mercuryVersionChanged'
        QtMocHelpers::SignalData<void(int)>(98, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Signal 'penelopePresenceChanged'
        QtMocHelpers::SignalData<void(bool)>(99, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 29 },
        }}),
        // Signal 'penelopeVersionChanged'
        QtMocHelpers::SignalData<void(int)>(100, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Signal 'pennyLanePresenceChanged'
        QtMocHelpers::SignalData<void(bool)>(101, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 29 },
        }}),
        // Signal 'pennyLaneVersionChanged'
        QtMocHelpers::SignalData<void(int)>(102, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Signal 'alexPresenceChanged'
        QtMocHelpers::SignalData<void(bool)>(103, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 29 },
        }}),
        // Signal 'excaliburPresenceChanged'
        QtMocHelpers::SignalData<void(bool)>(104, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 29 },
        }}),
        // Signal 'metisVersionChanged'
        QtMocHelpers::SignalData<void(int)>(105, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Signal 'alexConfigurationChanged'
        QtMocHelpers::SignalData<void(quint16)>(106, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::UShort, 107 },
        }}),
        // Signal 'alexStatesChanged'
        QtMocHelpers::SignalData<void(const QList<int> &)>(108, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 109, 110 },
        }}),
        // Signal 'alexStateChanged'
        QtMocHelpers::SignalData<void(HamBand, const QList<int> &)>(111, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 112, 113 }, { 0x80000000 | 109, 110 },
        }}),
        // Signal 'alexManualStateChanged'
        QtMocHelpers::SignalData<void(QObject *, bool)>(114, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Bool, 29 },
        }}),
        // Signal 'checkFirmwareVersionChanged'
        QtMocHelpers::SignalData<void(QObject *, bool)>(115, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Bool, 29 },
        }}),
        // Signal 'pennyOCEnabledChanged'
        QtMocHelpers::SignalData<void(bool)>(116, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 29 },
        }}),
        // Signal 'rxJ6PinsChanged'
        QtMocHelpers::SignalData<void(const QList<int> &)>(117, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 109, 110 },
        }}),
        // Signal 'txJ6PinsChanged'
        QtMocHelpers::SignalData<void(const QList<int> &)>(118, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 109, 110 },
        }}),
        // Signal 'protocolSyncChanged'
        QtMocHelpers::SignalData<void(int)>(119, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Signal 'adcOverflowChanged'
        QtMocHelpers::SignalData<void(int)>(120, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Signal 'packetLossChanged'
        QtMocHelpers::SignalData<void(int)>(121, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Signal 'sendIQSignalChanged'
        QtMocHelpers::SignalData<void(int)>(122, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Signal 'rcveIQSignalChanged'
        QtMocHelpers::SignalData<void(int)>(123, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Signal 'numberOfRXChanged'
        QtMocHelpers::SignalData<void(QObject *, int)>(124, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 29 },
        }}),
        // Signal 'sampleRateChanged'
        QtMocHelpers::SignalData<void(QObject *, int)>(125, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 29 },
        }}),
        // Signal 'mercuryAttenuatorChanged'
        QtMocHelpers::SignalData<void(QObject *, HamBand, int)>(126, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { 0x80000000 | 112, 113 }, { QMetaType::Int, 29 },
        }}),
        // Signal 'ditherChanged'
        QtMocHelpers::SignalData<void(QObject *, int)>(127, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 29 },
        }}),
        // Signal 'randomChanged'
        QtMocHelpers::SignalData<void(QObject *, int)>(128, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 29 },
        }}),
        // Signal 'src10MhzChanged'
        QtMocHelpers::SignalData<void(QObject *, int)>(129, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 130 },
        }}),
        // Signal 'src122_88MhzChanged'
        QtMocHelpers::SignalData<void(QObject *, int)>(131, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 130 },
        }}),
        // Signal 'micSourceChanged'
        QtMocHelpers::SignalData<void(int)>(132, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 130 },
        }}),
        // Signal 'micInputChanged'
        QtMocHelpers::SignalData<void(int)>(133, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 130 },
        }}),
        // Signal 'classChanged'
        QtMocHelpers::SignalData<void(QObject *, int)>(134, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 29 },
        }}),
        // Signal 'timingChanged'
        QtMocHelpers::SignalData<void(QObject *, int)>(135, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 29 },
        }}),
        // Signal 'controlBytesOutChanged'
        QtMocHelpers::SignalData<void(QObject *, unsigned char *)>(136, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { 0x80000000 | 137, 138 },
        }}),
        // Signal 'ctrFrequencyChanged'
        QtMocHelpers::SignalData<void(QObject *, int, int, long)>(139, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 16 }, { QMetaType::Int, 19 }, { QMetaType::Long, 140 },
        }}),
        // Signal 'vfoFrequencyChanged'
        QtMocHelpers::SignalData<void(QObject *, int, int, long)>(141, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 16 }, { QMetaType::Int, 19 }, { QMetaType::Long, 140 },
        }}),
        // Signal 'ncoFrequencyChanged'
        QtMocHelpers::SignalData<void(int, long)>(142, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Long, 140 },
        }}),
        // Signal 'widebandSpectrumBufferChanged'
        QtMocHelpers::SignalData<void(const qVectorFloat &)>(143, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 34, 35 },
        }}),
        // Signal 'widebandOptionsChanged'
        QtMocHelpers::SignalData<void(QObject *, TWideband)>(144, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { 0x80000000 | 145, 146 },
        }}),
        // Signal 'widebandSpectrumBufferReset'
        QtMocHelpers::SignalData<void()>(147, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'widebandStatusChanged'
        QtMocHelpers::SignalData<void(QObject *, bool)>(148, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Bool, 29 },
        }}),
        // Signal 'widebandDataChanged'
        QtMocHelpers::SignalData<void(QObject *, bool)>(149, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Bool, 29 },
        }}),
        // Signal 'widebanddBmScaleMinChanged'
        QtMocHelpers::SignalData<void(QObject *, qreal)>(150, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::QReal, 29 },
        }}),
        // Signal 'widebanddBmScaleMaxChanged'
        QtMocHelpers::SignalData<void(QObject *, qreal)>(151, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::QReal, 29 },
        }}),
        // Signal 'wideBandScalePositionChanged'
        QtMocHelpers::SignalData<void(QObject *, float)>(152, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Float, 153 },
        }}),
        // Signal 'panAveragingModeChanged'
        QtMocHelpers::SignalData<void(int, int)>(154, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Int, 16 },
        }}),
        // Signal 'panDetectorModeChanged'
        QtMocHelpers::SignalData<void(int, int)>(155, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Int, 16 },
        }}),
        // Signal 'fftSizeChanged'
        QtMocHelpers::SignalData<void(int, int)>(156, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Int, 39 },
        }}),
        // Signal 'fmsqLevelChanged'
        QtMocHelpers::SignalData<void(int, int)>(157, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Int, 158 },
        }}),
        // Signal 'iqPortChanged'
        QtMocHelpers::SignalData<void(QObject *, int, int)>(159, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::Int, 85 },
        }}),
        // Signal 'hamBandChanged'
        QtMocHelpers::SignalData<void(QObject *, int, bool, HamBand)>(160, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::Bool, 161 }, { 0x80000000 | 112, 113 },
        }}),
        // Signal 'dspModeChanged'
        QtMocHelpers::SignalData<void(QObject *, int, DSPMode)>(162, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { 0x80000000 | 163, 16 },
        }}),
        // Signal 'adcModeChanged'
        QtMocHelpers::SignalData<void(QObject *, int, ADCMode)>(164, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { 0x80000000 | 165, 16 },
        }}),
        // Signal 'agcModeChanged'
        QtMocHelpers::SignalData<void(QObject *, int, AGCMode, bool)>(166, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { 0x80000000 | 167, 16 }, { QMetaType::Bool, 168 },
        }}),
        // Signal 'agcHangEnabledChanged'
        QtMocHelpers::SignalData<void(QObject *, int, bool)>(169, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::Bool, 170 },
        }}),
        // Signal 'agcGainChanged'
        QtMocHelpers::SignalData<void(QObject *, int, int)>(171, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::Int, 29 },
        }}),
        // Signal 'agcThresholdChanged_dB'
        QtMocHelpers::SignalData<void(QObject *, int, qreal)>(172, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::QReal, 29 },
        }}),
        // Signal 'agcFixedGainChanged_dB'
        QtMocHelpers::SignalData<void(QObject *, int, qreal)>(173, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::QReal, 29 },
        }}),
        // Signal 'agcMaximumGainChanged_dB'
        QtMocHelpers::SignalData<void(QObject *, int, qreal)>(174, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::QReal, 29 },
        }}),
        // Signal 'agcHangThresholdChanged'
        QtMocHelpers::SignalData<void(QObject *, int, int)>(175, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::Int, 29 },
        }}),
        // Signal 'agcHangThresholdSliderChanged'
        QtMocHelpers::SignalData<void(QObject *, int, qreal)>(176, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::QReal, 29 },
        }}),
        // Signal 'agcHangLevelChanged_dB'
        QtMocHelpers::SignalData<void(QObject *, int, qreal)>(177, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::QReal, 29 },
        }}),
        // Signal 'agcLineLevelsChanged'
        QtMocHelpers::SignalData<void(QObject *, int, qreal, qreal)>(178, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::QReal, 179 }, { QMetaType::QReal, 170 },
        }}),
        // Signal 'agcVariableGainChanged_dB'
        QtMocHelpers::SignalData<void(QObject *, int, qreal)>(180, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::QReal, 29 },
        }}),
        // Signal 'agcAttackTimeChanged'
        QtMocHelpers::SignalData<void(QObject *, int, qreal)>(181, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::QReal, 29 },
        }}),
        // Signal 'agcDecayTimeChanged'
        QtMocHelpers::SignalData<void(QObject *, int, qreal)>(182, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::QReal, 29 },
        }}),
        // Signal 'agcHangTimeChanged'
        QtMocHelpers::SignalData<void(QObject *, int, qreal)>(183, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::QReal, 29 },
        }}),
        // Signal 'filterFrequenciesChanged'
        QtMocHelpers::SignalData<void(QObject *, int, qreal, qreal)>(184, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::QReal, 185 }, { QMetaType::QReal, 186 },
        }}),
        // Signal 'cudaDevicesChanged'
        QtMocHelpers::SignalData<void(QObject *, int)>(187, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 29 },
        }}),
        // Signal 'cudaDriverChanged'
        QtMocHelpers::SignalData<void(QObject *, int)>(188, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 29 },
        }}),
        // Signal 'cudaRuntimeChanged'
        QtMocHelpers::SignalData<void(QObject *, int)>(189, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 29 },
        }}),
        // Signal 'cudaCurrentDeviceChanged'
        QtMocHelpers::SignalData<void(QObject *, int)>(190, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 29 },
        }}),
        // Signal 'cudaLastDeviceChanged'
        QtMocHelpers::SignalData<void(QObject *, int)>(191, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 29 },
        }}),
        // Signal 'freqRulerPositionChanged'
        QtMocHelpers::SignalData<void(QObject *, int, float)>(192, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::Float, 153 },
        }}),
        // Signal 'audioFormatChanged'
        QtMocHelpers::SignalData<void(QObject *, const QAudioFormat &)>(193, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { 0x80000000 | 194, 195 },
        }}),
        // Signal 'audioPositionChanged'
        QtMocHelpers::SignalData<void(QObject *, qint64)>(196, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::LongLong, 153 },
        }}),
        // Signal 'audioBufferChanged'
        QtMocHelpers::SignalData<void(QObject *, qint64, qint64, const QByteArray &)>(197, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::LongLong, 153 }, { QMetaType::LongLong, 198 }, { QMetaType::QByteArray, 35 },
        }}),
        // Signal 'displayWidgetHeightChanged'
        QtMocHelpers::SignalData<void(int)>(199, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Signal 'spectrumSizeChanged'
        QtMocHelpers::SignalData<void(QObject *, int)>(200, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 29 },
        }}),
        // Signal 'panadapterColorChanged'
        QtMocHelpers::SignalData<void()>(201, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'panGridStatusChanged'
        QtMocHelpers::SignalData<void(bool, int)>(202, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 29 }, { QMetaType::Int, 19 },
        }}),
        // Signal 'peakHoldStatusChanged'
        QtMocHelpers::SignalData<void(bool, int)>(203, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 29 }, { QMetaType::Int, 19 },
        }}),
        // Signal 'panLockedStatusChanged'
        QtMocHelpers::SignalData<void(bool, int)>(204, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 29 }, { QMetaType::Int, 19 },
        }}),
        // Signal 'clickVFOStatusChanged'
        QtMocHelpers::SignalData<void(bool, int)>(205, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 29 }, { QMetaType::Int, 19 },
        }}),
        // Signal 'hairCrossStatusChanged'
        QtMocHelpers::SignalData<void(bool, int)>(206, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 29 }, { QMetaType::Int, 19 },
        }}),
        // Signal 'showAGCLinesStatusChanged'
        QtMocHelpers::SignalData<void(QObject *, bool, int)>(207, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Bool, 29 }, { QMetaType::Int, 19 },
        }}),
        // Signal 'spectrumAveragingChanged'
        QtMocHelpers::SignalData<void(QObject *, int, bool)>(208, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::Bool, 29 },
        }}),
        // Signal 'spectrumAveragingCntChanged'
        QtMocHelpers::SignalData<void(QObject *, int, int)>(209, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::Int, 29 },
        }}),
        // Signal 'waterfallTimeChanged'
        QtMocHelpers::SignalData<void(int, int)>(210, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Int, 29 },
        }}),
        // Signal 'waterfallOffesetLoChanged'
        QtMocHelpers::SignalData<void(int, int)>(211, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Int, 29 },
        }}),
        // Signal 'waterfallOffesetHiChanged'
        QtMocHelpers::SignalData<void(int, int)>(212, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Int, 29 },
        }}),
        // Signal 'sMeterHoldTimeChanged'
        QtMocHelpers::SignalData<void(int)>(213, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Signal 'dBmScaleMinChanged'
        QtMocHelpers::SignalData<void(int, qreal)>(214, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::QReal, 29 },
        }}),
        // Signal 'dBmScaleMaxChanged'
        QtMocHelpers::SignalData<void(int, qreal)>(215, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::QReal, 29 },
        }}),
        // Signal 'agcMaximumGainChanged'
        QtMocHelpers::SignalData<void(QObject *, int, qreal)>(216, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 2 }, { QMetaType::QReal, 29 },
        }}),
        // Signal 'noiseBlankerChanged'
        QtMocHelpers::SignalData<void(int, int)>(217, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Int, 16 },
        }}),
        // Signal 'noiseFilterChanged'
        QtMocHelpers::SignalData<void(int, int)>(218, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Int, 16 },
        }}),
        // Signal 'nr2GainMethodChanged'
        QtMocHelpers::SignalData<void(int, int)>(219, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Int, 29 },
        }}),
        // Signal 'nr2NpeMethodChanged'
        QtMocHelpers::SignalData<void(int, int)>(220, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Int, 29 },
        }}),
        // Signal 'nrAgcChanged'
        QtMocHelpers::SignalData<void(int, int)>(221, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Int, 29 },
        }}),
        // Signal 'nr2AeChanged'
        QtMocHelpers::SignalData<void(int, bool)>(222, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Bool, 29 },
        }}),
        // Signal 'snbChanged'
        QtMocHelpers::SignalData<void(int, bool)>(223, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Bool, 29 },
        }}),
        // Signal 'anfChanged'
        QtMocHelpers::SignalData<void(int, bool)>(224, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Bool, 29 },
        }}),
        // Signal 'micInputLevelChanged'
        QtMocHelpers::SignalData<void(QObject *, int)>(225, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 158 },
        }}),
        // Signal 'driveLevelChanged'
        QtMocHelpers::SignalData<void(QObject *, int)>(226, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 158 },
        }}),
        // Signal 'repeaterModeChanged'
        QtMocHelpers::SignalData<void(bool)>(227, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 16 },
        }}),
        // Signal 'repeaterOffsetchanged'
        QtMocHelpers::SignalData<void(double)>(228, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 29 },
        }}),
        // Signal 'fmPremphasizechanged'
        QtMocHelpers::SignalData<void(double)>(229, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 29 },
        }}),
        // Signal 'fmdeveationchanged'
        QtMocHelpers::SignalData<void(double)>(230, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 29 },
        }}),
        // Signal 'amCarrierlevelchanged'
        QtMocHelpers::SignalData<void(double)>(231, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 158 },
        }}),
        // Signal 'audioCompressionchanged'
        QtMocHelpers::SignalData<void(int)>(232, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 158 },
        }}),
        // Signal 'micModeChanged'
        QtMocHelpers::SignalData<void(bool)>(233, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 16 },
        }}),
        // Signal 'showRadioPopupChanged'
        QtMocHelpers::SignalData<void(bool)>(234, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 29 },
        }}),
        // Signal 'receiverDataReady'
        QtMocHelpers::SignalData<void()>(235, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'CwHangTimeChanged'
        QtMocHelpers::SignalData<void(int)>(236, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 237 },
        }}),
        // Signal 'CwSidetoneFreqChanged'
        QtMocHelpers::SignalData<void(int)>(238, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 239 },
        }}),
        // Signal 'CwKeyReversedChanged'
        QtMocHelpers::SignalData<void(int)>(240, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 241 },
        }}),
        // Signal 'CwKeyerModeChanged'
        QtMocHelpers::SignalData<void(int)>(242, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 243 },
        }}),
        // Signal 'InternalCwChanged'
        QtMocHelpers::SignalData<void(int)>(244, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 245 },
        }}),
        // Signal 'CwKeyerSpeedChanged'
        QtMocHelpers::SignalData<void(int)>(246, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 247 },
        }}),
        // Signal 'CwPttDelayChanged'
        QtMocHelpers::SignalData<void(int)>(248, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 249 },
        }}),
        // Signal 'CwSidetoneVolumeChanged'
        QtMocHelpers::SignalData<void(int)>(250, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 251 },
        }}),
        // Signal 'CwKeyerWeightChanged'
        QtMocHelpers::SignalData<void(int)>(252, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 253 },
        }}),
        // Signal 'CwKeyerSpacingChanged'
        QtMocHelpers::SignalData<void(int)>(254, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 255 },
        }}),
        // Slot 'setMainPower'
        QtMocHelpers::SlotData<void(QObject *, bool)>(256, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Bool, 6 },
        }}),
        // Slot 'setDefaultSkin'
        QtMocHelpers::SlotData<void(QObject *, bool)>(257, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Bool, 29 },
        }}),
        // Slot 'setSettingsFilename'
        QtMocHelpers::SlotData<void(QString)>(258, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 56 },
        }}),
        // Slot 'setSystemMessage'
        QtMocHelpers::SlotData<void(const QString &, int)>(259, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::Int, 260 },
        }}),
        // Slot 'setSettingsLoaded'
        QtMocHelpers::SlotData<void(bool)>(261, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 58 },
        }}),
        // Slot 'setCPULoad'
        QtMocHelpers::SlotData<void(short)>(262, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Short, 27 },
        }}),
        // Slot 'setCallsign'
        QtMocHelpers::SlotData<void(const QString &)>(263, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 264 },
        }}),
        // Slot 'setPBOPresence'
        QtMocHelpers::SlotData<void(bool)>(265, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 29 },
        }}),
        // Slot 'setFBOPresence'
        QtMocHelpers::SlotData<void(bool)>(266, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 29 },
        }}),
        // Slot 'setMainVolume'
        QtMocHelpers::SlotData<void(QObject *, int, float)>(267, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::Float, 94 },
        }}),
        // Slot 'setMainVolumeMute'
        QtMocHelpers::SlotData<void(QObject *, int, bool)>(268, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::Bool, 29 },
        }}),
        // Slot 'setSystemState'
        QtMocHelpers::SlotData<void(QObject *, QSDR::_Error, QSDR::_HWInterfaceMode, QSDR::_ServerMode, QSDR::_DataEngineState)>(269, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { 0x80000000 | 11, 12 }, { 0x80000000 | 13, 14 }, { 0x80000000 | 15, 16 },
            { 0x80000000 | 17, 9 },
        }}),
        // Slot 'setGraphicsState'
        QtMocHelpers::SlotData<void(QObject *, int, PanGraphicsMode, WaterfallColorMode)>(270, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { 0x80000000 | 20, 21 }, { 0x80000000 | 22, 23 },
        }}),
        // Slot 'setTxAllowed'
        QtMocHelpers::SlotData<void(QObject *, bool)>(271, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Bool, 29 },
        }}),
        // Slot 'setRadioState'
        QtMocHelpers::SlotData<RadioState(RadioState)>(272, 2, QMC::AccessPublic, 0x80000000 | 8, {{
            { 0x80000000 | 8, 16 },
        }}),
        // Slot 'getRadioState'
        QtMocHelpers::SlotData<RadioState()>(273, 2, QMC::AccessPublic, 0x80000000 | 8),
        // Slot 'setMultiRxView'
        QtMocHelpers::SlotData<void(int)>(274, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 31 },
        }}),
        // Slot 'setSMeterValue'
        QtMocHelpers::SlotData<void(int, double)>(275, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Double, 29 },
        }}),
        // Slot 'setSpectrumBuffer'
        QtMocHelpers::SlotData<void(int, const QList<float> &)>(276, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { 0x80000000 | 277, 35 },
        }}),
        // Slot 'setPostSpectrumBuffer'
        QtMocHelpers::SlotData<void(int, const float *)>(278, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { 0x80000000 | 37, 2 },
        }}),
        // Slot 'setSampleSize'
        QtMocHelpers::SlotData<void(QObject *, int, int)>(279, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::Int, 39 },
        }}),
        // Slot 'setRxList'
        QtMocHelpers::SlotData<void(QList<Receiver*>)>(280, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 41, 69 },
        }}),
        // Slot 'setMetisCardList'
        QtMocHelpers::SlotData<void(QList<TNetworkDevicecard>)>(281, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 68, 69 },
        }}),
        // Slot 'searchHpsdrNetworkDevices'
        QtMocHelpers::SlotData<void()>(282, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'clearMetisCardList'
        QtMocHelpers::SlotData<void()>(283, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'setHPSDRDeviceNumber'
        QtMocHelpers::SlotData<void(int)>(284, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Slot 'setCurrentHPSDRDevice'
        QtMocHelpers::SlotData<void(TNetworkDevicecard)>(285, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 74, 75 },
        }}),
        // Slot 'addNetworkIOComboBoxEntry'
        QtMocHelpers::SlotData<void(QString)>(286, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 78 },
        }}),
        // Slot 'clearNetworkIOComboBoxEntry'
        QtMocHelpers::SlotData<void()>(287, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'addServerNetworkInterface'
        QtMocHelpers::SlotData<void(QString, QString)>(288, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 60 }, { QMetaType::QString, 61 },
        }}),
        // Slot 'addHPSDRDeviceNIC'
        QtMocHelpers::SlotData<void(QString, QString)>(289, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 60 }, { QMetaType::QString, 61 },
        }}),
        // Slot 'setNumberOfNetworkInterfaces'
        QtMocHelpers::SlotData<void(int)>(290, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Slot 'setServerNetworkInterface'
        QtMocHelpers::SlotData<void(int)>(291, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 292 },
        }}),
        // Slot 'setHPSDRDeviceNIC'
        QtMocHelpers::SlotData<void(int)>(293, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 292 },
        }}),
        // Slot 'setServerWidgetNIC'
        QtMocHelpers::SlotData<void(int)>(294, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 292 },
        }}),
        // Slot 'setHPSDRWidgetNIC'
        QtMocHelpers::SlotData<void(int)>(295, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 292 },
        }}),
        // Slot 'setServerAddr'
        QtMocHelpers::SlotData<void(QObject *, QString)>(296, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::QString, 82 },
        }}),
        // Slot 'setHPSDRDeviceLocalAddr'
        QtMocHelpers::SlotData<void(QObject *, QString)>(297, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::QString, 82 },
        }}),
        // Slot 'setServerPort'
        QtMocHelpers::SlotData<void(QObject *, quint16)>(298, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::UShort, 85 },
        }}),
        // Slot 'setListenPort'
        QtMocHelpers::SlotData<void(QObject *, quint16)>(299, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::UShort, 85 },
        }}),
        // Slot 'setAudioPort'
        QtMocHelpers::SlotData<void(QObject *, quint16)>(300, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::UShort, 85 },
        }}),
        // Slot 'setMetisPort'
        QtMocHelpers::SlotData<void(QObject *, quint16)>(301, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::UShort, 85 },
        }}),
        // Slot 'setClientConnected'
        QtMocHelpers::SlotData<void(QObject *, bool)>(302, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Bool, 29 },
        }}),
        // Slot 'setClientNoConnected'
        QtMocHelpers::SlotData<void(QObject *, int)>(303, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 46 },
        }}),
        // Slot 'setRxConnectedStatus'
        QtMocHelpers::SlotData<void(QObject *, int, bool)>(304, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::Bool, 29 },
        }}),
        // Slot 'setAudioRx'
        QtMocHelpers::SlotData<void(QObject *, int)>(305, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 },
        }}),
        // Slot 'setConnected'
        QtMocHelpers::SlotData<void(QObject *, bool)>(306, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Bool, 29 },
        }}),
        // Slot 'setCheckFirmwareVersion'
        QtMocHelpers::SlotData<void(QObject *, bool)>(307, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Bool, 29 },
        }}),
        // Slot 'setHPSDRDevices'
        QtMocHelpers::SlotData<void(QObject *, THPSDRDevices)>(308, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { 0x80000000 | 71, 72 },
        }}),
        // Slot 'setHermesVersion'
        QtMocHelpers::SlotData<void(int)>(309, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Slot 'setHPSDRHardware'
        QtMocHelpers::SlotData<void(int)>(310, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Slot 'setMercuryPresence'
        QtMocHelpers::SlotData<void(bool)>(311, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 29 },
        }}),
        // Slot 'setMercuryVersion'
        QtMocHelpers::SlotData<void(int)>(312, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Slot 'setPenelopePresence'
        QtMocHelpers::SlotData<void(bool)>(313, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 29 },
        }}),
        // Slot 'setPenelopeVersion'
        QtMocHelpers::SlotData<void(int)>(314, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Slot 'setPennyLanePresence'
        QtMocHelpers::SlotData<void(bool)>(315, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 29 },
        }}),
        // Slot 'setPennyLaneVersion'
        QtMocHelpers::SlotData<void(int)>(316, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Slot 'setAlexPresence'
        QtMocHelpers::SlotData<void(bool)>(317, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 29 },
        }}),
        // Slot 'setExcaliburPresence'
        QtMocHelpers::SlotData<void(bool)>(318, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 29 },
        }}),
        // Slot 'setMetisVersion'
        QtMocHelpers::SlotData<void(int)>(319, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Slot 'setAlexConfiguration'
        QtMocHelpers::SlotData<void(QObject *, quint16)>(320, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::UShort, 321 },
        }}),
        // Slot 'setAlexHPFLoFrequencies'
        QtMocHelpers::SlotData<void(int, long)>(322, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 323 }, { QMetaType::Long, 29 },
        }}),
        // Slot 'setAlexHPFHiFrequencies'
        QtMocHelpers::SlotData<void(int, long)>(324, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 323 }, { QMetaType::Long, 29 },
        }}),
        // Slot 'setAlexLPFLoFrequencies'
        QtMocHelpers::SlotData<void(int, long)>(325, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 323 }, { QMetaType::Long, 29 },
        }}),
        // Slot 'setAlexLPFHiFrequencies'
        QtMocHelpers::SlotData<void(int, long)>(326, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 323 }, { QMetaType::Long, 29 },
        }}),
        // Slot 'setAlexStates'
        QtMocHelpers::SlotData<void(QObject *, const QList<int> &)>(327, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { 0x80000000 | 109, 110 },
        }}),
        // Slot 'setAlexState'
        QtMocHelpers::SlotData<void(QObject *, int, int)>(328, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 329 }, { QMetaType::Int, 29 },
        }}),
        // Slot 'setAlexState'
        QtMocHelpers::SlotData<void(QObject *, int)>(328, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 29 },
        }}),
        // Slot 'setAlexToManual'
        QtMocHelpers::SlotData<void(QObject *, bool)>(330, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Bool, 29 },
        }}),
        // Slot 'checkAlexState'
        QtMocHelpers::SlotData<int(int)>(331, 2, QMC::AccessPublic, QMetaType::Int, {{
            { QMetaType::Int, 9 },
        }}),
        // Slot 'setPennyOCEnabled'
        QtMocHelpers::SlotData<void(QObject *, bool)>(332, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Bool, 29 },
        }}),
        // Slot 'setRxJ6Pin'
        QtMocHelpers::SlotData<void(QObject *, HamBand, int)>(333, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { 0x80000000 | 112, 113 }, { QMetaType::Int, 29 },
        }}),
        // Slot 'setRxJ6Pins'
        QtMocHelpers::SlotData<void(QObject *, const QList<int> &)>(334, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { 0x80000000 | 109, 110 },
        }}),
        // Slot 'setTxJ6Pin'
        QtMocHelpers::SlotData<void(QObject *, HamBand, int)>(335, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { 0x80000000 | 112, 113 }, { QMetaType::Int, 29 },
        }}),
        // Slot 'setTxJ6Pins'
        QtMocHelpers::SlotData<void(QObject *, const QList<int> &)>(336, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { 0x80000000 | 109, 110 },
        }}),
        // Slot 'setIQPort'
        QtMocHelpers::SlotData<void(QObject *, int, int)>(337, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::Int, 85 },
        }}),
        // Slot 'setProtocolSync'
        QtMocHelpers::SlotData<void(int)>(338, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Slot 'setADCOverflow'
        QtMocHelpers::SlotData<void(int)>(339, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Slot 'setPacketLoss'
        QtMocHelpers::SlotData<void(int)>(340, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Slot 'setSendIQ'
        QtMocHelpers::SlotData<void(int)>(341, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Slot 'setRcveIQ'
        QtMocHelpers::SlotData<void(int)>(342, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Slot 'setReceivers'
        QtMocHelpers::SlotData<void(QObject *, int)>(343, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 29 },
        }}),
        // Slot 'setCurrentReceiver'
        QtMocHelpers::SlotData<void(QObject *, int)>(344, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 29 },
        }}),
        // Slot 'setSampleRate'
        QtMocHelpers::SlotData<void(QObject *, int)>(345, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 29 },
        }}),
        // Slot 'setMercuryAttenuator'
        QtMocHelpers::SlotData<void(QObject *, int)>(346, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 29 },
        }}),
        // Slot 'setDither'
        QtMocHelpers::SlotData<void(QObject *, int)>(347, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 29 },
        }}),
        // Slot 'setRandom'
        QtMocHelpers::SlotData<void(QObject *, int)>(348, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 29 },
        }}),
        // Slot 'set10MhzSource'
        QtMocHelpers::SlotData<void(QObject *, int)>(349, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 130 },
        }}),
        // Slot 'set122_88MhzSource'
        QtMocHelpers::SlotData<void(QObject *, int)>(350, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 130 },
        }}),
        // Slot 'setMicSource'
        QtMocHelpers::SlotData<void(int)>(351, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 130 },
        }}),
        // Slot 'setMicInputDev'
        QtMocHelpers::SlotData<void(int)>(352, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 292 },
        }}),
        // Slot 'setMicInputLevel'
        QtMocHelpers::SlotData<void(QObject *, int)>(353, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 158 },
        }}),
        // Slot 'setDriveLevel'
        QtMocHelpers::SlotData<void(QObject *, int)>(354, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 158 },
        }}),
        // Slot 'setClass'
        QtMocHelpers::SlotData<void(QObject *, int)>(355, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 29 },
        }}),
        // Slot 'setTiming'
        QtMocHelpers::SlotData<void(QObject *, int)>(356, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 29 },
        }}),
        // Slot 'setCtrFrequency'
        QtMocHelpers::SlotData<void(QObject *, int, int, long)>(357, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 16 }, { QMetaType::Int, 19 }, { QMetaType::Long, 140 },
        }}),
        // Slot 'setCtrFrequency'
        QtMocHelpers::SlotData<void(int, long)>(357, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Long, 140 },
        }}),
        // Slot 'getCtrFrequency'
        QtMocHelpers::SlotData<long(int)>(358, 2, QMC::AccessPublic, QMetaType::Long, {{
            { QMetaType::Int, 19 },
        }}),
        // Slot 'setVFOFrequency'
        QtMocHelpers::SlotData<void(QObject *, int, int, long)>(359, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 16 }, { QMetaType::Int, 19 }, { QMetaType::Long, 140 },
        }}),
        // Slot 'setVfoFrequency'
        QtMocHelpers::SlotData<void(int, long)>(360, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Long, 140 },
        }}),
        // Slot 'getVfoFrequency'
        QtMocHelpers::SlotData<long(int)>(361, 2, QMC::AccessPublic, QMetaType::Long, {{
            { QMetaType::Int, 19 },
        }}),
        // Slot 'setNCOFrequency'
        QtMocHelpers::SlotData<void(QObject *, bool, int, long)>(362, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Bool, 29 }, { QMetaType::Int, 19 }, { QMetaType::Long, 140 },
        }}),
        // Slot 'clientDisconnected'
        QtMocHelpers::SlotData<void(int)>(363, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 46 },
        }}),
        // Slot 'setFramesPerSecond'
        QtMocHelpers::SlotData<void(QObject *, int, int)>(364, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::Int, 29 },
        }}),
        // Slot 'setMouseWheelFreqStep'
        QtMocHelpers::SlotData<void(QObject *, int, qreal)>(365, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::QReal, 29 },
        }}),
        // Slot 'setSocketBufferSize'
        QtMocHelpers::SlotData<void(QObject *, int)>(366, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 29 },
        }}),
        // Slot 'setManualSocketBufferSize'
        QtMocHelpers::SlotData<void(QObject *, bool)>(367, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Bool, 29 },
        }}),
        // Slot 'setReceiverDataReady'
        QtMocHelpers::SlotData<void()>(368, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'setSpectrumSize'
        QtMocHelpers::SlotData<void(QObject *, int)>(369, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 29 },
        }}),
        // Slot 'setdBmPanScaleMin'
        QtMocHelpers::SlotData<void(int, qreal)>(370, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::QReal, 29 },
        }}),
        // Slot 'setdBmPanScaleMax'
        QtMocHelpers::SlotData<void(int, qreal)>(371, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::QReal, 29 },
        }}),
        // Slot 'setdBmDistScaleMin'
        QtMocHelpers::SlotData<void(qreal)>(372, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QReal, 29 },
        }}),
        // Slot 'setdBmDistScaleMax'
        QtMocHelpers::SlotData<void(qreal)>(373, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QReal, 29 },
        }}),
        // Slot 'setHamBand'
        QtMocHelpers::SlotData<void(QObject *, int, bool, HamBand)>(374, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::Bool, 161 }, { 0x80000000 | 112, 113 },
        }}),
        // Slot 'setDSPMode'
        QtMocHelpers::SlotData<void(QObject *, int, DSPMode)>(375, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { 0x80000000 | 163, 16 },
        }}),
        // Slot 'setADCMode'
        QtMocHelpers::SlotData<void(QObject *, int, ADCMode)>(376, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { 0x80000000 | 165, 16 },
        }}),
        // Slot 'setAGCMode'
        QtMocHelpers::SlotData<void(QObject *, int, AGCMode)>(377, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { 0x80000000 | 167, 16 },
        }}),
        // Slot 'setAGCGain'
        QtMocHelpers::SlotData<void(QObject *, int, int)>(378, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::Int, 29 },
        }}),
        // Slot 'setAGCMaximumGain_dB'
        QtMocHelpers::SlotData<void(QObject *, int, qreal)>(379, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::QReal, 29 },
        }}),
        // Slot 'setAGCFixedGain_dB'
        QtMocHelpers::SlotData<void(QObject *, int, qreal)>(380, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::QReal, 29 },
        }}),
        // Slot 'setAGCThreshold_dB'
        QtMocHelpers::SlotData<void(QObject *, int, qreal)>(381, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::QReal, 29 },
        }}),
        // Slot 'setAGCHangThresholdSlider'
        QtMocHelpers::SlotData<void(QObject *, int, qreal)>(382, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::QReal, 29 },
        }}),
        // Slot 'setAGCHangThreshold'
        QtMocHelpers::SlotData<void(QObject *, int, int)>(383, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::Int, 29 },
        }}),
        // Slot 'setAGCHangLevel_dB'
        QtMocHelpers::SlotData<void(QObject *, int, qreal)>(384, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::QReal, 29 },
        }}),
        // Slot 'setAGCLineLevels'
        QtMocHelpers::SlotData<void(QObject *, int, qreal, qreal)>(385, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::QReal, 179 }, { QMetaType::QReal, 170 },
        }}),
        // Slot 'setAGCShowLines'
        QtMocHelpers::SlotData<void(QObject *, int, bool)>(386, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::Bool, 29 },
        }}),
        // Slot 'setAGCVariableGain_dB'
        QtMocHelpers::SlotData<void(QObject *, int, qreal)>(387, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::QReal, 29 },
        }}),
        // Slot 'setAGCAttackTime'
        QtMocHelpers::SlotData<void(QObject *, int, qreal)>(388, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::QReal, 29 },
        }}),
        // Slot 'setAGCDecayTime'
        QtMocHelpers::SlotData<void(QObject *, int, qreal)>(389, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::QReal, 29 },
        }}),
        // Slot 'setAGCHangTime'
        QtMocHelpers::SlotData<void(QObject *, int, qreal)>(390, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::QReal, 29 },
        }}),
        // Slot 'setRXFilter'
        QtMocHelpers::SlotData<void(QObject *, int, qreal, qreal)>(391, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::QReal, 185 }, { QMetaType::QReal, 186 },
        }}),
        // Slot 'setfftSize'
        QtMocHelpers::SlotData<void(int, int)>(392, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Int, 39 },
        }}),
        // Slot 'setfmsqLevel'
        QtMocHelpers::SlotData<void(int, int)>(393, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Int, 158 },
        }}),
        // Slot 'setWidebandBuffers'
        QtMocHelpers::SlotData<void(QObject *, int)>(394, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 29 },
        }}),
        // Slot 'setWidebandSpectrumBuffer'
        QtMocHelpers::SlotData<void(const qVectorFloat &)>(395, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 34, 35 },
        }}),
        // Slot 'resetWidebandSpectrumBuffer'
        QtMocHelpers::SlotData<void()>(396, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'setWidebandOptions'
        QtMocHelpers::SlotData<void(QObject *, TWideband)>(397, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { 0x80000000 | 145, 146 },
        }}),
        // Slot 'setWidebandStatus'
        QtMocHelpers::SlotData<void(QObject *, bool)>(398, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Bool, 29 },
        }}),
        // Slot 'setWidebandData'
        QtMocHelpers::SlotData<void(QObject *, bool)>(399, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Bool, 29 },
        }}),
        // Slot 'setWidebanddBmScaleMin'
        QtMocHelpers::SlotData<void(QObject *, qreal)>(400, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::QReal, 29 },
        }}),
        // Slot 'setWidebanddBmScaleMax'
        QtMocHelpers::SlotData<void(QObject *, qreal)>(401, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::QReal, 29 },
        }}),
        // Slot 'setWideBandRulerPosition'
        QtMocHelpers::SlotData<void(QObject *, float)>(402, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Float, 329 },
        }}),
        // Slot 'setFreqRulerPosition'
        QtMocHelpers::SlotData<void(QObject *, int, float)>(403, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::Float, 329 },
        }}),
        // Slot 'setAudioFormat'
        QtMocHelpers::SlotData<void(QObject *, const QAudioFormat &)>(404, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { 0x80000000 | 194, 195 },
        }}),
        // Slot 'setAudioPosition'
        QtMocHelpers::SlotData<void(QObject *, qint64)>(405, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::LongLong, 153 },
        }}),
        // Slot 'setAudioBuffer'
        QtMocHelpers::SlotData<void(QObject *, qint64, qint64, const QByteArray &)>(406, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::LongLong, 153 }, { QMetaType::LongLong, 198 }, { QMetaType::QByteArray, 35 },
        }}),
        // Slot 'moveDisplayWidget'
        QtMocHelpers::SlotData<void(int)>(407, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Slot 'setPanadapterColors'
        QtMocHelpers::SlotData<void(TPanadapterColors)>(408, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 409, 410 },
        }}),
        // Slot 'setPanGrid'
        QtMocHelpers::SlotData<void(bool, int)>(411, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 29 }, { QMetaType::Int, 19 },
        }}),
        // Slot 'setPeakHold'
        QtMocHelpers::SlotData<void(bool, int)>(412, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 29 }, { QMetaType::Int, 19 },
        }}),
        // Slot 'setPanLocked'
        QtMocHelpers::SlotData<void(bool, int)>(413, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 29 }, { QMetaType::Int, 19 },
        }}),
        // Slot 'setClickVFO'
        QtMocHelpers::SlotData<void(bool, int)>(414, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 29 }, { QMetaType::Int, 19 },
        }}),
        // Slot 'setHairCross'
        QtMocHelpers::SlotData<void(bool, int)>(415, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 29 }, { QMetaType::Int, 19 },
        }}),
        // Slot 'setSpectrumAveraging'
        QtMocHelpers::SlotData<void(QObject *, int, bool)>(416, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::Bool, 29 },
        }}),
        // Slot 'setSpectrumAveragingCnt'
        QtMocHelpers::SlotData<void(QObject *, int, int)>(417, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 19 }, { QMetaType::Int, 29 },
        }}),
        // Slot 'setWaterfallTime'
        QtMocHelpers::SlotData<void(int, int)>(418, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Int, 29 },
        }}),
        // Slot 'setWaterfallOffesetLo'
        QtMocHelpers::SlotData<void(int, int)>(419, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Int, 29 },
        }}),
        // Slot 'setWaterfallOffesetHi'
        QtMocHelpers::SlotData<void(int, int)>(420, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Int, 29 },
        }}),
        // Slot 'setPanAveragingMode'
        QtMocHelpers::SlotData<void(int, PanAveragingMode)>(421, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { 0x80000000 | 422, 16 },
        }}),
        // Slot 'setPanDetectorMode'
        QtMocHelpers::SlotData<void(int, PanDetectorMode)>(423, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { 0x80000000 | 424, 16 },
        }}),
        // Slot 'setNoiseBlankerMode'
        QtMocHelpers::SlotData<void(int, int)>(425, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Int, 426 },
        }}),
        // Slot 'setNoiseFilterMode'
        QtMocHelpers::SlotData<void(int, int)>(427, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Int, 428 },
        }}),
        // Slot 'setNR2GainMethod'
        QtMocHelpers::SlotData<void(int, int)>(429, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Int, 29 },
        }}),
        // Slot 'setNR2NpeMethod'
        QtMocHelpers::SlotData<void(int, int)>(430, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Int, 29 },
        }}),
        // Slot 'setNRAgc'
        QtMocHelpers::SlotData<void(int, int)>(431, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Int, 29 },
        }}),
        // Slot 'setNR2Ae'
        QtMocHelpers::SlotData<void(int, bool)>(432, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Bool, 29 },
        }}),
        // Slot 'setAnf'
        QtMocHelpers::SlotData<void(int, bool)>(433, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Bool, 29 },
        }}),
        // Slot 'setSnb'
        QtMocHelpers::SlotData<void(int, bool)>(434, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Bool, 29 },
        }}),
        // Slot 'setRepeaterMode'
        QtMocHelpers::SlotData<void(bool)>(435, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 16 },
        }}),
        // Slot 'setRepeaterOffset'
        QtMocHelpers::SlotData<void(int)>(436, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 437 },
        }}),
        // Slot 'setAudioCompression'
        QtMocHelpers::SlotData<void(int)>(438, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 158 },
        }}),
        // Slot 'setAMCarrierLevel'
        QtMocHelpers::SlotData<void(int)>(439, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 158 },
        }}),
        // Slot 'setFMPreEmphasize'
        QtMocHelpers::SlotData<void(int)>(440, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 158 },
        }}),
        // Slot 'setFmDeveation'
        QtMocHelpers::SlotData<void(int)>(441, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 158 },
        }}),
        // Slot 'setCwHangTime'
        QtMocHelpers::SlotData<void(int)>(442, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 237 },
        }}),
        // Slot 'setCwSidetoneFreq'
        QtMocHelpers::SlotData<void(int)>(443, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 239 },
        }}),
        // Slot 'setCwKeyReversed'
        QtMocHelpers::SlotData<void(int)>(444, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 241 },
        }}),
        // Slot 'setCwKeyerMode'
        QtMocHelpers::SlotData<void(int)>(445, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 243 },
        }}),
        // Slot 'setInternalCw'
        QtMocHelpers::SlotData<void(int)>(446, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 245 },
        }}),
        // Slot 'setCwKeyerSpeed'
        QtMocHelpers::SlotData<void(int)>(447, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 247 },
        }}),
        // Slot 'setCwPttDelay'
        QtMocHelpers::SlotData<void(int)>(448, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 249 },
        }}),
        // Slot 'setCwSidetoneVolume'
        QtMocHelpers::SlotData<void(int)>(449, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 251 },
        }}),
        // Slot 'setCwKeyerWeight'
        QtMocHelpers::SlotData<void(int)>(450, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 253 },
        }}),
        // Slot 'setCwKeyerSpacing'
        QtMocHelpers::SlotData<void(int)>(451, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 255 },
        }}),
        // Slot 'setSMeterHoldTime'
        QtMocHelpers::SlotData<void(int)>(452, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 29 },
        }}),
        // Slot 'showNetworkIODialog'
        QtMocHelpers::SlotData<void()>(453, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'showWarningDialog'
        QtMocHelpers::SlotData<void(const QString &)>(454, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Slot 'showRadioPopupWidget'
        QtMocHelpers::SlotData<void()>(455, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'getCtrFrequencies'
        QtMocHelpers::SlotData<QList<long>()>(456, 2, QMC::AccessPublic, 0x80000000 | 457),
        // Slot 'getVfoFrequencies'
        QtMocHelpers::SlotData<QList<long>()>(458, 2, QMC::AccessPublic, 0x80000000 | 457),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<Settings, qt_meta_tag_ZN8SettingsE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Settings::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8SettingsE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8SettingsE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN8SettingsE_t>.metaTypes,
    nullptr
} };

void Settings::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<Settings *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->systemMessageEvent((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 1: _t->masterSwitchChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 2: _t->radioStateChanged((*reinterpret_cast< std::add_pointer_t<RadioState>>(_a[1]))); break;
        case 3: _t->systemStateChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QSDR::_Error>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QSDR::_HWInterfaceMode>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QSDR::_ServerMode>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<QSDR::_DataEngineState>>(_a[5]))); break;
        case 4: _t->graphicModeChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<PanGraphicsMode>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<WaterfallColorMode>>(_a[4]))); break;
        case 5: _t->moxStateChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<RadioState>>(_a[2]))); break;
        case 6: _t->tuneStateChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<RadioState>>(_a[2]))); break;
        case 7: _t->cpuLoadChanged((*reinterpret_cast< std::add_pointer_t<short>>(_a[1]))); break;
        case 8: _t->txAllowedChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 9: _t->multiRxViewChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 10: _t->sMeterValueChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 11: _t->spectrumBufferChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qVectorFloat>>(_a[2]))); break;
        case 12: _t->postSpectrumBufferChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<const float*>>(_a[2]))); break;
        case 13: _t->sampleSizeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 14: _t->rxListChanged((*reinterpret_cast< std::add_pointer_t<QList<Receiver*>>>(_a[1]))); break;
        case 15: _t->clientConnectedChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 16: _t->clientNoConnectedChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 17: _t->audioRxChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 18: _t->receiverChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 19: _t->currentReceiverChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 20: _t->connectedChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 21: _t->clientConnectedEvent((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 22: _t->clientDisconnectedEvent((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 23: _t->rxConnectedStatusChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3]))); break;
        case 24: _t->framesPerSecondChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 25: _t->settingsFilenameChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 26: _t->settingsLoadedChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 27: _t->newServerNetworkInterface((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 28: _t->newHPSDRDeviceNIC((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 29: _t->serverNICChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 30: _t->hpsdrDeviceNICChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 31: _t->socketBufferSizeChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 32: _t->manualSocketBufferChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 33: _t->metisCardListChanged((*reinterpret_cast< std::add_pointer_t<QList<TNetworkDevicecard>>>(_a[1]))); break;
        case 34: _t->hpsdrDevicesChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<THPSDRDevices>>(_a[2]))); break;
        case 35: _t->hpsdrNetworkDeviceChanged((*reinterpret_cast< std::add_pointer_t<TNetworkDevicecard>>(_a[1]))); break;
        case 36: _t->networkDeviceNumberChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 37: _t->networkIOComboBoxEntryAdded((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 38: _t->clearNetworkIOComboBoxEntrySignal(); break;
        case 39: _t->searchMetisSignal(); break;
        case 40: _t->serverAddrChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 41: _t->hpsdrDeviceLocalAddrChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 42: _t->serverPortChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<quint16>>(_a[2]))); break;
        case 43: _t->listenPortChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<quint16>>(_a[2]))); break;
        case 44: _t->audioPortChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<quint16>>(_a[2]))); break;
        case 45: _t->metisPortChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<quint16>>(_a[2]))); break;
        case 46: _t->showNetworkIO(); break;
        case 47: _t->showWarning((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 48: _t->callsignChanged(); break;
        case 49: _t->mouseWheelFreqStepChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 50: _t->mainVolumeChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<float>>(_a[3]))); break;
        case 51: _t->hpsdrHardwareChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 52: _t->hermesVersionChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 53: _t->mercuryPresenceChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 54: _t->mercuryVersionChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 55: _t->penelopePresenceChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 56: _t->penelopeVersionChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 57: _t->pennyLanePresenceChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 58: _t->pennyLaneVersionChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 59: _t->alexPresenceChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 60: _t->excaliburPresenceChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 61: _t->metisVersionChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 62: _t->alexConfigurationChanged((*reinterpret_cast< std::add_pointer_t<quint16>>(_a[1]))); break;
        case 63: _t->alexStatesChanged((*reinterpret_cast< std::add_pointer_t<QList<int>>>(_a[1]))); break;
        case 64: _t->alexStateChanged((*reinterpret_cast< std::add_pointer_t<HamBand>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QList<int>>>(_a[2]))); break;
        case 65: _t->alexManualStateChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 66: _t->checkFirmwareVersionChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 67: _t->pennyOCEnabledChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 68: _t->rxJ6PinsChanged((*reinterpret_cast< std::add_pointer_t<QList<int>>>(_a[1]))); break;
        case 69: _t->txJ6PinsChanged((*reinterpret_cast< std::add_pointer_t<QList<int>>>(_a[1]))); break;
        case 70: _t->protocolSyncChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 71: _t->adcOverflowChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 72: _t->packetLossChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 73: _t->sendIQSignalChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 74: _t->rcveIQSignalChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 75: _t->numberOfRXChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 76: _t->sampleRateChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 77: _t->mercuryAttenuatorChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<HamBand>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 78: _t->ditherChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 79: _t->randomChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 80: _t->src10MhzChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 81: _t->src122_88MhzChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 82: _t->micSourceChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 83: _t->micInputChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 84: _t->classChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 85: _t->timingChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 86: _t->controlBytesOutChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<uchar*>>(_a[2]))); break;
        case 87: _t->ctrFrequencyChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<long>>(_a[4]))); break;
        case 88: _t->vfoFrequencyChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<long>>(_a[4]))); break;
        case 89: _t->ncoFrequencyChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<long>>(_a[2]))); break;
        case 90: _t->widebandSpectrumBufferChanged((*reinterpret_cast< std::add_pointer_t<qVectorFloat>>(_a[1]))); break;
        case 91: _t->widebandOptionsChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<TWideband>>(_a[2]))); break;
        case 92: _t->widebandSpectrumBufferReset(); break;
        case 93: _t->widebandStatusChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 94: _t->widebandDataChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 95: _t->widebanddBmScaleMinChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[2]))); break;
        case 96: _t->widebanddBmScaleMaxChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[2]))); break;
        case 97: _t->wideBandScalePositionChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<float>>(_a[2]))); break;
        case 98: _t->panAveragingModeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 99: _t->panDetectorModeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 100: _t->fftSizeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 101: _t->fmsqLevelChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 102: _t->iqPortChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 103: _t->hamBandChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<HamBand>>(_a[4]))); break;
        case 104: _t->dspModeChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<DSPMode>>(_a[3]))); break;
        case 105: _t->adcModeChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<ADCMode>>(_a[3]))); break;
        case 106: _t->agcModeChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<AGCMode>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[4]))); break;
        case 107: _t->agcHangEnabledChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3]))); break;
        case 108: _t->agcGainChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 109: _t->agcThresholdChanged_dB((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 110: _t->agcFixedGainChanged_dB((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 111: _t->agcMaximumGainChanged_dB((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 112: _t->agcHangThresholdChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 113: _t->agcHangThresholdSliderChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 114: _t->agcHangLevelChanged_dB((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 115: _t->agcLineLevelsChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[4]))); break;
        case 116: _t->agcVariableGainChanged_dB((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 117: _t->agcAttackTimeChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 118: _t->agcDecayTimeChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 119: _t->agcHangTimeChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 120: _t->filterFrequenciesChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[4]))); break;
        case 121: _t->cudaDevicesChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 122: _t->cudaDriverChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 123: _t->cudaRuntimeChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 124: _t->cudaCurrentDeviceChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 125: _t->cudaLastDeviceChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 126: _t->freqRulerPositionChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<float>>(_a[3]))); break;
        case 127: _t->audioFormatChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QAudioFormat>>(_a[2]))); break;
        case 128: _t->audioPositionChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qint64>>(_a[2]))); break;
        case 129: _t->audioBufferChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qint64>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qint64>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[4]))); break;
        case 130: _t->displayWidgetHeightChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 131: _t->spectrumSizeChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 132: _t->panadapterColorChanged(); break;
        case 133: _t->panGridStatusChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 134: _t->peakHoldStatusChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 135: _t->panLockedStatusChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 136: _t->clickVFOStatusChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 137: _t->hairCrossStatusChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 138: _t->showAGCLinesStatusChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 139: _t->spectrumAveragingChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3]))); break;
        case 140: _t->spectrumAveragingCntChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 141: _t->waterfallTimeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 142: _t->waterfallOffesetLoChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 143: _t->waterfallOffesetHiChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 144: _t->sMeterHoldTimeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 145: _t->dBmScaleMinChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[2]))); break;
        case 146: _t->dBmScaleMaxChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[2]))); break;
        case 147: _t->agcMaximumGainChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 148: _t->noiseBlankerChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 149: _t->noiseFilterChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 150: _t->nr2GainMethodChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 151: _t->nr2NpeMethodChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 152: _t->nrAgcChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 153: _t->nr2AeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 154: _t->snbChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 155: _t->anfChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 156: _t->micInputLevelChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 157: _t->driveLevelChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 158: _t->repeaterModeChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 159: _t->repeaterOffsetchanged((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 160: _t->fmPremphasizechanged((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 161: _t->fmdeveationchanged((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 162: _t->amCarrierlevelchanged((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 163: _t->audioCompressionchanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 164: _t->micModeChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 165: _t->showRadioPopupChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 166: _t->receiverDataReady(); break;
        case 167: _t->CwHangTimeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 168: _t->CwSidetoneFreqChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 169: _t->CwKeyReversedChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 170: _t->CwKeyerModeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 171: _t->InternalCwChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 172: _t->CwKeyerSpeedChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 173: _t->CwPttDelayChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 174: _t->CwSidetoneVolumeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 175: _t->CwKeyerWeightChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 176: _t->CwKeyerSpacingChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 177: _t->setMainPower((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 178: _t->setDefaultSkin((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 179: _t->setSettingsFilename((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 180: _t->setSystemMessage((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 181: _t->setSettingsLoaded((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 182: _t->setCPULoad((*reinterpret_cast< std::add_pointer_t<short>>(_a[1]))); break;
        case 183: _t->setCallsign((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 184: _t->setPBOPresence((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 185: _t->setFBOPresence((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 186: _t->setMainVolume((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<float>>(_a[3]))); break;
        case 187: _t->setMainVolumeMute((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3]))); break;
        case 188: _t->setSystemState((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QSDR::_Error>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QSDR::_HWInterfaceMode>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QSDR::_ServerMode>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<QSDR::_DataEngineState>>(_a[5]))); break;
        case 189: _t->setGraphicsState((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<PanGraphicsMode>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<WaterfallColorMode>>(_a[4]))); break;
        case 190: _t->setTxAllowed((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 191: { RadioState _r = _t->setRadioState((*reinterpret_cast< std::add_pointer_t<RadioState>>(_a[1])));
            if (_a[0]) *reinterpret_cast< RadioState*>(_a[0]) = std::move(_r); }  break;
        case 192: { RadioState _r = _t->getRadioState();
            if (_a[0]) *reinterpret_cast< RadioState*>(_a[0]) = std::move(_r); }  break;
        case 193: _t->setMultiRxView((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 194: _t->setSMeterValue((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 195: _t->setSpectrumBuffer((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QList<float>>>(_a[2]))); break;
        case 196: _t->setPostSpectrumBuffer((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<const float*>>(_a[2]))); break;
        case 197: _t->setSampleSize((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 198: _t->setRxList((*reinterpret_cast< std::add_pointer_t<QList<Receiver*>>>(_a[1]))); break;
        case 199: _t->setMetisCardList((*reinterpret_cast< std::add_pointer_t<QList<TNetworkDevicecard>>>(_a[1]))); break;
        case 200: _t->searchHpsdrNetworkDevices(); break;
        case 201: _t->clearMetisCardList(); break;
        case 202: _t->setHPSDRDeviceNumber((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 203: _t->setCurrentHPSDRDevice((*reinterpret_cast< std::add_pointer_t<TNetworkDevicecard>>(_a[1]))); break;
        case 204: _t->addNetworkIOComboBoxEntry((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 205: _t->clearNetworkIOComboBoxEntry(); break;
        case 206: _t->addServerNetworkInterface((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 207: _t->addHPSDRDeviceNIC((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 208: _t->setNumberOfNetworkInterfaces((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 209: _t->setServerNetworkInterface((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 210: _t->setHPSDRDeviceNIC((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 211: _t->setServerWidgetNIC((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 212: _t->setHPSDRWidgetNIC((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 213: _t->setServerAddr((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 214: _t->setHPSDRDeviceLocalAddr((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 215: _t->setServerPort((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<quint16>>(_a[2]))); break;
        case 216: _t->setListenPort((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<quint16>>(_a[2]))); break;
        case 217: _t->setAudioPort((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<quint16>>(_a[2]))); break;
        case 218: _t->setMetisPort((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<quint16>>(_a[2]))); break;
        case 219: _t->setClientConnected((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 220: _t->setClientNoConnected((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 221: _t->setRxConnectedStatus((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3]))); break;
        case 222: _t->setAudioRx((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 223: _t->setConnected((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 224: _t->setCheckFirmwareVersion((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 225: _t->setHPSDRDevices((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<THPSDRDevices>>(_a[2]))); break;
        case 226: _t->setHermesVersion((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 227: _t->setHPSDRHardware((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 228: _t->setMercuryPresence((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 229: _t->setMercuryVersion((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 230: _t->setPenelopePresence((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 231: _t->setPenelopeVersion((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 232: _t->setPennyLanePresence((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 233: _t->setPennyLaneVersion((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 234: _t->setAlexPresence((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 235: _t->setExcaliburPresence((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 236: _t->setMetisVersion((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 237: _t->setAlexConfiguration((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<quint16>>(_a[2]))); break;
        case 238: _t->setAlexHPFLoFrequencies((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<long>>(_a[2]))); break;
        case 239: _t->setAlexHPFHiFrequencies((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<long>>(_a[2]))); break;
        case 240: _t->setAlexLPFLoFrequencies((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<long>>(_a[2]))); break;
        case 241: _t->setAlexLPFHiFrequencies((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<long>>(_a[2]))); break;
        case 242: _t->setAlexStates((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QList<int>>>(_a[2]))); break;
        case 243: _t->setAlexState((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 244: _t->setAlexState((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 245: _t->setAlexToManual((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 246: { int _r = _t->checkAlexState((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = std::move(_r); }  break;
        case 247: _t->setPennyOCEnabled((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 248: _t->setRxJ6Pin((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<HamBand>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 249: _t->setRxJ6Pins((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QList<int>>>(_a[2]))); break;
        case 250: _t->setTxJ6Pin((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<HamBand>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 251: _t->setTxJ6Pins((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QList<int>>>(_a[2]))); break;
        case 252: _t->setIQPort((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 253: _t->setProtocolSync((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 254: _t->setADCOverflow((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 255: _t->setPacketLoss((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 256: _t->setSendIQ((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 257: _t->setRcveIQ((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 258: _t->setReceivers((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 259: _t->setCurrentReceiver((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 260: _t->setSampleRate((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 261: _t->setMercuryAttenuator((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 262: _t->setDither((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 263: _t->setRandom((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 264: _t->set10MhzSource((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 265: _t->set122_88MhzSource((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 266: _t->setMicSource((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 267: _t->setMicInputDev((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 268: _t->setMicInputLevel((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 269: _t->setDriveLevel((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 270: _t->setClass((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 271: _t->setTiming((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 272: _t->setCtrFrequency((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<long>>(_a[4]))); break;
        case 273: _t->setCtrFrequency((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<long>>(_a[2]))); break;
        case 274: { long _r = _t->getCtrFrequency((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< long*>(_a[0]) = std::move(_r); }  break;
        case 275: _t->setVFOFrequency((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<long>>(_a[4]))); break;
        case 276: _t->setVfoFrequency((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<long>>(_a[2]))); break;
        case 277: { long _r = _t->getVfoFrequency((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< long*>(_a[0]) = std::move(_r); }  break;
        case 278: _t->setNCOFrequency((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<long>>(_a[4]))); break;
        case 279: _t->clientDisconnected((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 280: _t->setFramesPerSecond((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 281: _t->setMouseWheelFreqStep((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 282: _t->setSocketBufferSize((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 283: _t->setManualSocketBufferSize((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 284: _t->setReceiverDataReady(); break;
        case 285: _t->setSpectrumSize((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 286: _t->setdBmPanScaleMin((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[2]))); break;
        case 287: _t->setdBmPanScaleMax((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[2]))); break;
        case 288: _t->setdBmDistScaleMin((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1]))); break;
        case 289: _t->setdBmDistScaleMax((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1]))); break;
        case 290: _t->setHamBand((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<HamBand>>(_a[4]))); break;
        case 291: _t->setDSPMode((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<DSPMode>>(_a[3]))); break;
        case 292: _t->setADCMode((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<ADCMode>>(_a[3]))); break;
        case 293: _t->setAGCMode((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<AGCMode>>(_a[3]))); break;
        case 294: _t->setAGCGain((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 295: _t->setAGCMaximumGain_dB((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 296: _t->setAGCFixedGain_dB((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 297: _t->setAGCThreshold_dB((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 298: _t->setAGCHangThresholdSlider((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 299: _t->setAGCHangThreshold((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 300: _t->setAGCHangLevel_dB((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 301: _t->setAGCLineLevels((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[4]))); break;
        case 302: _t->setAGCShowLines((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3]))); break;
        case 303: _t->setAGCVariableGain_dB((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 304: _t->setAGCAttackTime((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 305: _t->setAGCDecayTime((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 306: _t->setAGCHangTime((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 307: _t->setRXFilter((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[4]))); break;
        case 308: _t->setfftSize((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 309: _t->setfmsqLevel((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 310: _t->setWidebandBuffers((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 311: _t->setWidebandSpectrumBuffer((*reinterpret_cast< std::add_pointer_t<qVectorFloat>>(_a[1]))); break;
        case 312: _t->resetWidebandSpectrumBuffer(); break;
        case 313: _t->setWidebandOptions((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<TWideband>>(_a[2]))); break;
        case 314: _t->setWidebandStatus((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 315: _t->setWidebandData((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 316: _t->setWidebanddBmScaleMin((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[2]))); break;
        case 317: _t->setWidebanddBmScaleMax((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[2]))); break;
        case 318: _t->setWideBandRulerPosition((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<float>>(_a[2]))); break;
        case 319: _t->setFreqRulerPosition((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<float>>(_a[3]))); break;
        case 320: _t->setAudioFormat((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QAudioFormat>>(_a[2]))); break;
        case 321: _t->setAudioPosition((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qint64>>(_a[2]))); break;
        case 322: _t->setAudioBuffer((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qint64>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qint64>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[4]))); break;
        case 323: _t->moveDisplayWidget((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 324: _t->setPanadapterColors((*reinterpret_cast< std::add_pointer_t<TPanadapterColors>>(_a[1]))); break;
        case 325: _t->setPanGrid((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 326: _t->setPeakHold((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 327: _t->setPanLocked((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 328: _t->setClickVFO((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 329: _t->setHairCross((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 330: _t->setSpectrumAveraging((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3]))); break;
        case 331: _t->setSpectrumAveragingCnt((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 332: _t->setWaterfallTime((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 333: _t->setWaterfallOffesetLo((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 334: _t->setWaterfallOffesetHi((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 335: _t->setPanAveragingMode((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<PanAveragingMode>>(_a[2]))); break;
        case 336: _t->setPanDetectorMode((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<PanDetectorMode>>(_a[2]))); break;
        case 337: _t->setNoiseBlankerMode((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 338: _t->setNoiseFilterMode((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 339: _t->setNR2GainMethod((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 340: _t->setNR2NpeMethod((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 341: _t->setNRAgc((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 342: _t->setNR2Ae((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 343: _t->setAnf((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 344: _t->setSnb((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 345: _t->setRepeaterMode((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 346: _t->setRepeaterOffset((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 347: _t->setAudioCompression((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 348: _t->setAMCarrierLevel((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 349: _t->setFMPreEmphasize((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 350: _t->setFmDeveation((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 351: _t->setCwHangTime((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 352: _t->setCwSidetoneFreq((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 353: _t->setCwKeyReversed((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 354: _t->setCwKeyerMode((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 355: _t->setInternalCw((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 356: _t->setCwKeyerSpeed((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 357: _t->setCwPttDelay((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 358: _t->setCwSidetoneVolume((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 359: _t->setCwKeyerWeight((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 360: _t->setCwKeyerSpacing((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 361: _t->setSMeterHoldTime((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 362: _t->showNetworkIODialog(); break;
        case 363: _t->showWarningDialog((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 364: _t->showRadioPopupWidget(); break;
        case 365: { QList<long> _r = _t->getCtrFrequencies();
            if (_a[0]) *reinterpret_cast< QList<long>*>(_a[0]) = std::move(_r); }  break;
        case 366: { QList<long> _r = _t->getVfoFrequencies();
            if (_a[0]) *reinterpret_cast< QList<long>*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 4:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QSDR::_DataEngineState >(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QSDR::_Error >(); break;
            case 2:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QSDR::_HWInterfaceMode >(); break;
            case 3:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QSDR::_ServerMode >(); break;
            }
            break;
        case 33:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<TNetworkDevicecard> >(); break;
            }
            break;
        case 35:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< TNetworkDevicecard >(); break;
            }
            break;
        case 63:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<int> >(); break;
            }
            break;
        case 64:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< HamBand >(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<int> >(); break;
            }
            break;
        case 68:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<int> >(); break;
            }
            break;
        case 69:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<int> >(); break;
            }
            break;
        case 77:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< HamBand >(); break;
            }
            break;
        case 103:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 3:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< HamBand >(); break;
            }
            break;
        case 104:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 2:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< DSPMode >(); break;
            }
            break;
        case 105:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 2:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< ADCMode >(); break;
            }
            break;
        case 106:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 2:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< AGCMode >(); break;
            }
            break;
        case 127:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QAudioFormat >(); break;
            }
            break;
        case 188:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 4:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QSDR::_DataEngineState >(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QSDR::_Error >(); break;
            case 2:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QSDR::_HWInterfaceMode >(); break;
            case 3:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QSDR::_ServerMode >(); break;
            }
            break;
        case 195:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<float> >(); break;
            }
            break;
        case 199:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<TNetworkDevicecard> >(); break;
            }
            break;
        case 203:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< TNetworkDevicecard >(); break;
            }
            break;
        case 242:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<int> >(); break;
            }
            break;
        case 248:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< HamBand >(); break;
            }
            break;
        case 249:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<int> >(); break;
            }
            break;
        case 250:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< HamBand >(); break;
            }
            break;
        case 251:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<int> >(); break;
            }
            break;
        case 290:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 3:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< HamBand >(); break;
            }
            break;
        case 291:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 2:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< DSPMode >(); break;
            }
            break;
        case 292:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 2:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< ADCMode >(); break;
            }
            break;
        case 293:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 2:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< AGCMode >(); break;
            }
            break;
        case 320:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QAudioFormat >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(const QString & , int )>(_a, &Settings::systemMessageEvent, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , bool )>(_a, &Settings::masterSwitchChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(RadioState )>(_a, &Settings::radioStateChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , QSDR::_Error , QSDR::_HWInterfaceMode , QSDR::_ServerMode , QSDR::_DataEngineState )>(_a, &Settings::systemStateChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , PanGraphicsMode , WaterfallColorMode )>(_a, &Settings::graphicModeChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , RadioState )>(_a, &Settings::moxStateChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , RadioState )>(_a, &Settings::tuneStateChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(short )>(_a, &Settings::cpuLoadChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , bool )>(_a, &Settings::txAllowedChanged, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::multiRxViewChanged, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int , double )>(_a, &Settings::sMeterValueChanged, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int , const qVectorFloat & )>(_a, &Settings::spectrumBufferChanged, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int , const float * )>(_a, &Settings::postSpectrumBufferChanged, 12))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int , int )>(_a, &Settings::sampleSizeChanged, 13))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QList<Receiver*> )>(_a, &Settings::rxListChanged, 14))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , bool )>(_a, &Settings::clientConnectedChanged, 15))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int )>(_a, &Settings::clientNoConnectedChanged, 16))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int )>(_a, &Settings::audioRxChanged, 17))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::receiverChanged, 18))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int )>(_a, &Settings::currentReceiverChanged, 19))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , bool )>(_a, &Settings::connectedChanged, 20))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::clientConnectedEvent, 21))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::clientDisconnectedEvent, 22))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , bool )>(_a, &Settings::rxConnectedStatusChanged, 23))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , int )>(_a, &Settings::framesPerSecondChanged, 24))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QString )>(_a, &Settings::settingsFilenameChanged, 25))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(bool )>(_a, &Settings::settingsLoadedChanged, 26))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QString , QString )>(_a, &Settings::newServerNetworkInterface, 27))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QString , QString )>(_a, &Settings::newHPSDRDeviceNIC, 28))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::serverNICChanged, 29))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::hpsdrDeviceNICChanged, 30))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int )>(_a, &Settings::socketBufferSizeChanged, 31))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , bool )>(_a, &Settings::manualSocketBufferChanged, 32))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(const QList<TNetworkDevicecard> & )>(_a, &Settings::metisCardListChanged, 33))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , THPSDRDevices )>(_a, &Settings::hpsdrDevicesChanged, 34))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(TNetworkDevicecard )>(_a, &Settings::hpsdrNetworkDeviceChanged, 35))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::networkDeviceNumberChanged, 36))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QString )>(_a, &Settings::networkIOComboBoxEntryAdded, 37))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)()>(_a, &Settings::clearNetworkIOComboBoxEntrySignal, 38))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)()>(_a, &Settings::searchMetisSignal, 39))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , QString )>(_a, &Settings::serverAddrChanged, 40))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , QString )>(_a, &Settings::hpsdrDeviceLocalAddrChanged, 41))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , quint16 )>(_a, &Settings::serverPortChanged, 42))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , quint16 )>(_a, &Settings::listenPortChanged, 43))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , quint16 )>(_a, &Settings::audioPortChanged, 44))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , quint16 )>(_a, &Settings::metisPortChanged, 45))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)()>(_a, &Settings::showNetworkIO, 46))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(const QString & )>(_a, &Settings::showWarning, 47))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)()>(_a, &Settings::callsignChanged, 48))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , qreal )>(_a, &Settings::mouseWheelFreqStepChanged, 49))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , float )>(_a, &Settings::mainVolumeChanged, 50))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::hpsdrHardwareChanged, 51))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::hermesVersionChanged, 52))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(bool )>(_a, &Settings::mercuryPresenceChanged, 53))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::mercuryVersionChanged, 54))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(bool )>(_a, &Settings::penelopePresenceChanged, 55))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::penelopeVersionChanged, 56))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(bool )>(_a, &Settings::pennyLanePresenceChanged, 57))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::pennyLaneVersionChanged, 58))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(bool )>(_a, &Settings::alexPresenceChanged, 59))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(bool )>(_a, &Settings::excaliburPresenceChanged, 60))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::metisVersionChanged, 61))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(quint16 )>(_a, &Settings::alexConfigurationChanged, 62))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(const QList<int> & )>(_a, &Settings::alexStatesChanged, 63))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(HamBand , const QList<int> & )>(_a, &Settings::alexStateChanged, 64))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , bool )>(_a, &Settings::alexManualStateChanged, 65))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , bool )>(_a, &Settings::checkFirmwareVersionChanged, 66))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(bool )>(_a, &Settings::pennyOCEnabledChanged, 67))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(const QList<int> & )>(_a, &Settings::rxJ6PinsChanged, 68))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(const QList<int> & )>(_a, &Settings::txJ6PinsChanged, 69))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::protocolSyncChanged, 70))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::adcOverflowChanged, 71))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::packetLossChanged, 72))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::sendIQSignalChanged, 73))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::rcveIQSignalChanged, 74))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int )>(_a, &Settings::numberOfRXChanged, 75))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int )>(_a, &Settings::sampleRateChanged, 76))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , HamBand , int )>(_a, &Settings::mercuryAttenuatorChanged, 77))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int )>(_a, &Settings::ditherChanged, 78))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int )>(_a, &Settings::randomChanged, 79))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int )>(_a, &Settings::src10MhzChanged, 80))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int )>(_a, &Settings::src122_88MhzChanged, 81))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::micSourceChanged, 82))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::micInputChanged, 83))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int )>(_a, &Settings::classChanged, 84))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int )>(_a, &Settings::timingChanged, 85))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , unsigned char * )>(_a, &Settings::controlBytesOutChanged, 86))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , int , long )>(_a, &Settings::ctrFrequencyChanged, 87))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , int , long )>(_a, &Settings::vfoFrequencyChanged, 88))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int , long )>(_a, &Settings::ncoFrequencyChanged, 89))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(const qVectorFloat & )>(_a, &Settings::widebandSpectrumBufferChanged, 90))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , TWideband )>(_a, &Settings::widebandOptionsChanged, 91))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)()>(_a, &Settings::widebandSpectrumBufferReset, 92))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , bool )>(_a, &Settings::widebandStatusChanged, 93))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , bool )>(_a, &Settings::widebandDataChanged, 94))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , qreal )>(_a, &Settings::widebanddBmScaleMinChanged, 95))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , qreal )>(_a, &Settings::widebanddBmScaleMaxChanged, 96))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , float )>(_a, &Settings::wideBandScalePositionChanged, 97))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int , int )>(_a, &Settings::panAveragingModeChanged, 98))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int , int )>(_a, &Settings::panDetectorModeChanged, 99))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int , int )>(_a, &Settings::fftSizeChanged, 100))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int , int )>(_a, &Settings::fmsqLevelChanged, 101))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , int )>(_a, &Settings::iqPortChanged, 102))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , bool , HamBand )>(_a, &Settings::hamBandChanged, 103))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , DSPMode )>(_a, &Settings::dspModeChanged, 104))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , ADCMode )>(_a, &Settings::adcModeChanged, 105))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , AGCMode , bool )>(_a, &Settings::agcModeChanged, 106))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , bool )>(_a, &Settings::agcHangEnabledChanged, 107))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , int )>(_a, &Settings::agcGainChanged, 108))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , qreal )>(_a, &Settings::agcThresholdChanged_dB, 109))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , qreal )>(_a, &Settings::agcFixedGainChanged_dB, 110))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , qreal )>(_a, &Settings::agcMaximumGainChanged_dB, 111))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , int )>(_a, &Settings::agcHangThresholdChanged, 112))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , qreal )>(_a, &Settings::agcHangThresholdSliderChanged, 113))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , qreal )>(_a, &Settings::agcHangLevelChanged_dB, 114))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , qreal , qreal )>(_a, &Settings::agcLineLevelsChanged, 115))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , qreal )>(_a, &Settings::agcVariableGainChanged_dB, 116))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , qreal )>(_a, &Settings::agcAttackTimeChanged, 117))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , qreal )>(_a, &Settings::agcDecayTimeChanged, 118))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , qreal )>(_a, &Settings::agcHangTimeChanged, 119))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , qreal , qreal )>(_a, &Settings::filterFrequenciesChanged, 120))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int )>(_a, &Settings::cudaDevicesChanged, 121))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int )>(_a, &Settings::cudaDriverChanged, 122))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int )>(_a, &Settings::cudaRuntimeChanged, 123))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int )>(_a, &Settings::cudaCurrentDeviceChanged, 124))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int )>(_a, &Settings::cudaLastDeviceChanged, 125))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , float )>(_a, &Settings::freqRulerPositionChanged, 126))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , const QAudioFormat & )>(_a, &Settings::audioFormatChanged, 127))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , qint64 )>(_a, &Settings::audioPositionChanged, 128))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , qint64 , qint64 , const QByteArray & )>(_a, &Settings::audioBufferChanged, 129))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::displayWidgetHeightChanged, 130))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int )>(_a, &Settings::spectrumSizeChanged, 131))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)()>(_a, &Settings::panadapterColorChanged, 132))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(bool , int )>(_a, &Settings::panGridStatusChanged, 133))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(bool , int )>(_a, &Settings::peakHoldStatusChanged, 134))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(bool , int )>(_a, &Settings::panLockedStatusChanged, 135))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(bool , int )>(_a, &Settings::clickVFOStatusChanged, 136))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(bool , int )>(_a, &Settings::hairCrossStatusChanged, 137))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , bool , int )>(_a, &Settings::showAGCLinesStatusChanged, 138))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , bool )>(_a, &Settings::spectrumAveragingChanged, 139))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , int )>(_a, &Settings::spectrumAveragingCntChanged, 140))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int , int )>(_a, &Settings::waterfallTimeChanged, 141))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int , int )>(_a, &Settings::waterfallOffesetLoChanged, 142))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int , int )>(_a, &Settings::waterfallOffesetHiChanged, 143))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::sMeterHoldTimeChanged, 144))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int , qreal )>(_a, &Settings::dBmScaleMinChanged, 145))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int , qreal )>(_a, &Settings::dBmScaleMaxChanged, 146))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int , qreal )>(_a, &Settings::agcMaximumGainChanged, 147))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int , int )>(_a, &Settings::noiseBlankerChanged, 148))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int , int )>(_a, &Settings::noiseFilterChanged, 149))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int , int )>(_a, &Settings::nr2GainMethodChanged, 150))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int , int )>(_a, &Settings::nr2NpeMethodChanged, 151))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int , int )>(_a, &Settings::nrAgcChanged, 152))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int , bool )>(_a, &Settings::nr2AeChanged, 153))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int , bool )>(_a, &Settings::snbChanged, 154))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int , bool )>(_a, &Settings::anfChanged, 155))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int )>(_a, &Settings::micInputLevelChanged, 156))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(QObject * , int )>(_a, &Settings::driveLevelChanged, 157))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(bool )>(_a, &Settings::repeaterModeChanged, 158))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(double )>(_a, &Settings::repeaterOffsetchanged, 159))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(double )>(_a, &Settings::fmPremphasizechanged, 160))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(double )>(_a, &Settings::fmdeveationchanged, 161))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(double )>(_a, &Settings::amCarrierlevelchanged, 162))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::audioCompressionchanged, 163))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(bool )>(_a, &Settings::micModeChanged, 164))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(bool )>(_a, &Settings::showRadioPopupChanged, 165))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)()>(_a, &Settings::receiverDataReady, 166))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::CwHangTimeChanged, 167))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::CwSidetoneFreqChanged, 168))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::CwKeyReversedChanged, 169))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::CwKeyerModeChanged, 170))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::InternalCwChanged, 171))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::CwKeyerSpeedChanged, 172))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::CwPttDelayChanged, 173))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::CwSidetoneVolumeChanged, 174))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::CwKeyerWeightChanged, 175))
            return;
        if (QtMocHelpers::indexOfMethod<void (Settings::*)(int )>(_a, &Settings::CwKeyerSpacingChanged, 176))
            return;
    }
}

const QMetaObject *Settings::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Settings::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8SettingsE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Settings::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 367)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 367;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 367)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 367;
    }
    return _id;
}

// SIGNAL 0
void Settings::systemMessageEvent(const QString & _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void Settings::masterSwitchChanged(QObject * _t1, bool _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2);
}

// SIGNAL 2
void Settings::radioStateChanged(RadioState _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void Settings::systemStateChanged(QObject * _t1, QSDR::_Error _t2, QSDR::_HWInterfaceMode _t3, QSDR::_ServerMode _t4, QSDR::_DataEngineState _t5)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2, _t3, _t4, _t5);
}

// SIGNAL 4
void Settings::graphicModeChanged(QObject * _t1, int _t2, PanGraphicsMode _t3, WaterfallColorMode _t4)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2, _t3, _t4);
}

// SIGNAL 5
void Settings::moxStateChanged(QObject * _t1, RadioState _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1, _t2);
}

// SIGNAL 6
void Settings::tuneStateChanged(QObject * _t1, RadioState _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1, _t2);
}

// SIGNAL 7
void Settings::cpuLoadChanged(short _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1);
}

// SIGNAL 8
void Settings::txAllowedChanged(QObject * _t1, bool _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1, _t2);
}

// SIGNAL 9
void Settings::multiRxViewChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 9, nullptr, _t1);
}

// SIGNAL 10
void Settings::sMeterValueChanged(int _t1, double _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 10, nullptr, _t1, _t2);
}

// SIGNAL 11
void Settings::spectrumBufferChanged(int _t1, const qVectorFloat & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 11, nullptr, _t1, _t2);
}

// SIGNAL 12
void Settings::postSpectrumBufferChanged(int _t1, const float * _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 12, nullptr, _t1, _t2);
}

// SIGNAL 13
void Settings::sampleSizeChanged(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 13, nullptr, _t1, _t2);
}

// SIGNAL 14
void Settings::rxListChanged(QList<Receiver*> _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 14, nullptr, _t1);
}

// SIGNAL 15
void Settings::clientConnectedChanged(QObject * _t1, bool _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 15, nullptr, _t1, _t2);
}

// SIGNAL 16
void Settings::clientNoConnectedChanged(QObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 16, nullptr, _t1, _t2);
}

// SIGNAL 17
void Settings::audioRxChanged(QObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 17, nullptr, _t1, _t2);
}

// SIGNAL 18
void Settings::receiverChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 18, nullptr, _t1);
}

// SIGNAL 19
void Settings::currentReceiverChanged(QObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 19, nullptr, _t1, _t2);
}

// SIGNAL 20
void Settings::connectedChanged(QObject * _t1, bool _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 20, nullptr, _t1, _t2);
}

// SIGNAL 21
void Settings::clientConnectedEvent(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 21, nullptr, _t1);
}

// SIGNAL 22
void Settings::clientDisconnectedEvent(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 22, nullptr, _t1);
}

// SIGNAL 23
void Settings::rxConnectedStatusChanged(QObject * _t1, int _t2, bool _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 23, nullptr, _t1, _t2, _t3);
}

// SIGNAL 24
void Settings::framesPerSecondChanged(QObject * _t1, int _t2, int _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 24, nullptr, _t1, _t2, _t3);
}

// SIGNAL 25
void Settings::settingsFilenameChanged(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 25, nullptr, _t1);
}

// SIGNAL 26
void Settings::settingsLoadedChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 26, nullptr, _t1);
}

// SIGNAL 27
void Settings::newServerNetworkInterface(QString _t1, QString _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 27, nullptr, _t1, _t2);
}

// SIGNAL 28
void Settings::newHPSDRDeviceNIC(QString _t1, QString _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 28, nullptr, _t1, _t2);
}

// SIGNAL 29
void Settings::serverNICChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 29, nullptr, _t1);
}

// SIGNAL 30
void Settings::hpsdrDeviceNICChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 30, nullptr, _t1);
}

// SIGNAL 31
void Settings::socketBufferSizeChanged(QObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 31, nullptr, _t1, _t2);
}

// SIGNAL 32
void Settings::manualSocketBufferChanged(QObject * _t1, bool _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 32, nullptr, _t1, _t2);
}

// SIGNAL 33
void Settings::metisCardListChanged(const QList<TNetworkDevicecard> & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 33, nullptr, _t1);
}

// SIGNAL 34
void Settings::hpsdrDevicesChanged(QObject * _t1, THPSDRDevices _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 34, nullptr, _t1, _t2);
}

// SIGNAL 35
void Settings::hpsdrNetworkDeviceChanged(TNetworkDevicecard _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 35, nullptr, _t1);
}

// SIGNAL 36
void Settings::networkDeviceNumberChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 36, nullptr, _t1);
}

// SIGNAL 37
void Settings::networkIOComboBoxEntryAdded(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 37, nullptr, _t1);
}

// SIGNAL 38
void Settings::clearNetworkIOComboBoxEntrySignal()
{
    QMetaObject::activate(this, &staticMetaObject, 38, nullptr);
}

// SIGNAL 39
void Settings::searchMetisSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 39, nullptr);
}

// SIGNAL 40
void Settings::serverAddrChanged(QObject * _t1, QString _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 40, nullptr, _t1, _t2);
}

// SIGNAL 41
void Settings::hpsdrDeviceLocalAddrChanged(QObject * _t1, QString _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 41, nullptr, _t1, _t2);
}

// SIGNAL 42
void Settings::serverPortChanged(QObject * _t1, quint16 _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 42, nullptr, _t1, _t2);
}

// SIGNAL 43
void Settings::listenPortChanged(QObject * _t1, quint16 _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 43, nullptr, _t1, _t2);
}

// SIGNAL 44
void Settings::audioPortChanged(QObject * _t1, quint16 _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 44, nullptr, _t1, _t2);
}

// SIGNAL 45
void Settings::metisPortChanged(QObject * _t1, quint16 _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 45, nullptr, _t1, _t2);
}

// SIGNAL 46
void Settings::showNetworkIO()
{
    QMetaObject::activate(this, &staticMetaObject, 46, nullptr);
}

// SIGNAL 47
void Settings::showWarning(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 47, nullptr, _t1);
}

// SIGNAL 48
void Settings::callsignChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 48, nullptr);
}

// SIGNAL 49
void Settings::mouseWheelFreqStepChanged(QObject * _t1, int _t2, qreal _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 49, nullptr, _t1, _t2, _t3);
}

// SIGNAL 50
void Settings::mainVolumeChanged(QObject * _t1, int _t2, float _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 50, nullptr, _t1, _t2, _t3);
}

// SIGNAL 51
void Settings::hpsdrHardwareChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 51, nullptr, _t1);
}

// SIGNAL 52
void Settings::hermesVersionChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 52, nullptr, _t1);
}

// SIGNAL 53
void Settings::mercuryPresenceChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 53, nullptr, _t1);
}

// SIGNAL 54
void Settings::mercuryVersionChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 54, nullptr, _t1);
}

// SIGNAL 55
void Settings::penelopePresenceChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 55, nullptr, _t1);
}

// SIGNAL 56
void Settings::penelopeVersionChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 56, nullptr, _t1);
}

// SIGNAL 57
void Settings::pennyLanePresenceChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 57, nullptr, _t1);
}

// SIGNAL 58
void Settings::pennyLaneVersionChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 58, nullptr, _t1);
}

// SIGNAL 59
void Settings::alexPresenceChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 59, nullptr, _t1);
}

// SIGNAL 60
void Settings::excaliburPresenceChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 60, nullptr, _t1);
}

// SIGNAL 61
void Settings::metisVersionChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 61, nullptr, _t1);
}

// SIGNAL 62
void Settings::alexConfigurationChanged(quint16 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 62, nullptr, _t1);
}

// SIGNAL 63
void Settings::alexStatesChanged(const QList<int> & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 63, nullptr, _t1);
}

// SIGNAL 64
void Settings::alexStateChanged(HamBand _t1, const QList<int> & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 64, nullptr, _t1, _t2);
}

// SIGNAL 65
void Settings::alexManualStateChanged(QObject * _t1, bool _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 65, nullptr, _t1, _t2);
}

// SIGNAL 66
void Settings::checkFirmwareVersionChanged(QObject * _t1, bool _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 66, nullptr, _t1, _t2);
}

// SIGNAL 67
void Settings::pennyOCEnabledChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 67, nullptr, _t1);
}

// SIGNAL 68
void Settings::rxJ6PinsChanged(const QList<int> & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 68, nullptr, _t1);
}

// SIGNAL 69
void Settings::txJ6PinsChanged(const QList<int> & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 69, nullptr, _t1);
}

// SIGNAL 70
void Settings::protocolSyncChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 70, nullptr, _t1);
}

// SIGNAL 71
void Settings::adcOverflowChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 71, nullptr, _t1);
}

// SIGNAL 72
void Settings::packetLossChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 72, nullptr, _t1);
}

// SIGNAL 73
void Settings::sendIQSignalChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 73, nullptr, _t1);
}

// SIGNAL 74
void Settings::rcveIQSignalChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 74, nullptr, _t1);
}

// SIGNAL 75
void Settings::numberOfRXChanged(QObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 75, nullptr, _t1, _t2);
}

// SIGNAL 76
void Settings::sampleRateChanged(QObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 76, nullptr, _t1, _t2);
}

// SIGNAL 77
void Settings::mercuryAttenuatorChanged(QObject * _t1, HamBand _t2, int _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 77, nullptr, _t1, _t2, _t3);
}

// SIGNAL 78
void Settings::ditherChanged(QObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 78, nullptr, _t1, _t2);
}

// SIGNAL 79
void Settings::randomChanged(QObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 79, nullptr, _t1, _t2);
}

// SIGNAL 80
void Settings::src10MhzChanged(QObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 80, nullptr, _t1, _t2);
}

// SIGNAL 81
void Settings::src122_88MhzChanged(QObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 81, nullptr, _t1, _t2);
}

// SIGNAL 82
void Settings::micSourceChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 82, nullptr, _t1);
}

// SIGNAL 83
void Settings::micInputChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 83, nullptr, _t1);
}

// SIGNAL 84
void Settings::classChanged(QObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 84, nullptr, _t1, _t2);
}

// SIGNAL 85
void Settings::timingChanged(QObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 85, nullptr, _t1, _t2);
}

// SIGNAL 86
void Settings::controlBytesOutChanged(QObject * _t1, unsigned char * _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 86, nullptr, _t1, _t2);
}

// SIGNAL 87
void Settings::ctrFrequencyChanged(QObject * _t1, int _t2, int _t3, long _t4)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 87, nullptr, _t1, _t2, _t3, _t4);
}

// SIGNAL 88
void Settings::vfoFrequencyChanged(QObject * _t1, int _t2, int _t3, long _t4)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 88, nullptr, _t1, _t2, _t3, _t4);
}

// SIGNAL 89
void Settings::ncoFrequencyChanged(int _t1, long _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 89, nullptr, _t1, _t2);
}

// SIGNAL 90
void Settings::widebandSpectrumBufferChanged(const qVectorFloat & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 90, nullptr, _t1);
}

// SIGNAL 91
void Settings::widebandOptionsChanged(QObject * _t1, TWideband _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 91, nullptr, _t1, _t2);
}

// SIGNAL 92
void Settings::widebandSpectrumBufferReset()
{
    QMetaObject::activate(this, &staticMetaObject, 92, nullptr);
}

// SIGNAL 93
void Settings::widebandStatusChanged(QObject * _t1, bool _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 93, nullptr, _t1, _t2);
}

// SIGNAL 94
void Settings::widebandDataChanged(QObject * _t1, bool _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 94, nullptr, _t1, _t2);
}

// SIGNAL 95
void Settings::widebanddBmScaleMinChanged(QObject * _t1, qreal _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 95, nullptr, _t1, _t2);
}

// SIGNAL 96
void Settings::widebanddBmScaleMaxChanged(QObject * _t1, qreal _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 96, nullptr, _t1, _t2);
}

// SIGNAL 97
void Settings::wideBandScalePositionChanged(QObject * _t1, float _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 97, nullptr, _t1, _t2);
}

// SIGNAL 98
void Settings::panAveragingModeChanged(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 98, nullptr, _t1, _t2);
}

// SIGNAL 99
void Settings::panDetectorModeChanged(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 99, nullptr, _t1, _t2);
}

// SIGNAL 100
void Settings::fftSizeChanged(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 100, nullptr, _t1, _t2);
}

// SIGNAL 101
void Settings::fmsqLevelChanged(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 101, nullptr, _t1, _t2);
}

// SIGNAL 102
void Settings::iqPortChanged(QObject * _t1, int _t2, int _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 102, nullptr, _t1, _t2, _t3);
}

// SIGNAL 103
void Settings::hamBandChanged(QObject * _t1, int _t2, bool _t3, HamBand _t4)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 103, nullptr, _t1, _t2, _t3, _t4);
}

// SIGNAL 104
void Settings::dspModeChanged(QObject * _t1, int _t2, DSPMode _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 104, nullptr, _t1, _t2, _t3);
}

// SIGNAL 105
void Settings::adcModeChanged(QObject * _t1, int _t2, ADCMode _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 105, nullptr, _t1, _t2, _t3);
}

// SIGNAL 106
void Settings::agcModeChanged(QObject * _t1, int _t2, AGCMode _t3, bool _t4)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 106, nullptr, _t1, _t2, _t3, _t4);
}

// SIGNAL 107
void Settings::agcHangEnabledChanged(QObject * _t1, int _t2, bool _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 107, nullptr, _t1, _t2, _t3);
}

// SIGNAL 108
void Settings::agcGainChanged(QObject * _t1, int _t2, int _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 108, nullptr, _t1, _t2, _t3);
}

// SIGNAL 109
void Settings::agcThresholdChanged_dB(QObject * _t1, int _t2, qreal _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 109, nullptr, _t1, _t2, _t3);
}

// SIGNAL 110
void Settings::agcFixedGainChanged_dB(QObject * _t1, int _t2, qreal _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 110, nullptr, _t1, _t2, _t3);
}

// SIGNAL 111
void Settings::agcMaximumGainChanged_dB(QObject * _t1, int _t2, qreal _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 111, nullptr, _t1, _t2, _t3);
}

// SIGNAL 112
void Settings::agcHangThresholdChanged(QObject * _t1, int _t2, int _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 112, nullptr, _t1, _t2, _t3);
}

// SIGNAL 113
void Settings::agcHangThresholdSliderChanged(QObject * _t1, int _t2, qreal _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 113, nullptr, _t1, _t2, _t3);
}

// SIGNAL 114
void Settings::agcHangLevelChanged_dB(QObject * _t1, int _t2, qreal _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 114, nullptr, _t1, _t2, _t3);
}

// SIGNAL 115
void Settings::agcLineLevelsChanged(QObject * _t1, int _t2, qreal _t3, qreal _t4)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 115, nullptr, _t1, _t2, _t3, _t4);
}

// SIGNAL 116
void Settings::agcVariableGainChanged_dB(QObject * _t1, int _t2, qreal _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 116, nullptr, _t1, _t2, _t3);
}

// SIGNAL 117
void Settings::agcAttackTimeChanged(QObject * _t1, int _t2, qreal _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 117, nullptr, _t1, _t2, _t3);
}

// SIGNAL 118
void Settings::agcDecayTimeChanged(QObject * _t1, int _t2, qreal _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 118, nullptr, _t1, _t2, _t3);
}

// SIGNAL 119
void Settings::agcHangTimeChanged(QObject * _t1, int _t2, qreal _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 119, nullptr, _t1, _t2, _t3);
}

// SIGNAL 120
void Settings::filterFrequenciesChanged(QObject * _t1, int _t2, qreal _t3, qreal _t4)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 120, nullptr, _t1, _t2, _t3, _t4);
}

// SIGNAL 121
void Settings::cudaDevicesChanged(QObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 121, nullptr, _t1, _t2);
}

// SIGNAL 122
void Settings::cudaDriverChanged(QObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 122, nullptr, _t1, _t2);
}

// SIGNAL 123
void Settings::cudaRuntimeChanged(QObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 123, nullptr, _t1, _t2);
}

// SIGNAL 124
void Settings::cudaCurrentDeviceChanged(QObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 124, nullptr, _t1, _t2);
}

// SIGNAL 125
void Settings::cudaLastDeviceChanged(QObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 125, nullptr, _t1, _t2);
}

// SIGNAL 126
void Settings::freqRulerPositionChanged(QObject * _t1, int _t2, float _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 126, nullptr, _t1, _t2, _t3);
}

// SIGNAL 127
void Settings::audioFormatChanged(QObject * _t1, const QAudioFormat & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 127, nullptr, _t1, _t2);
}

// SIGNAL 128
void Settings::audioPositionChanged(QObject * _t1, qint64 _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 128, nullptr, _t1, _t2);
}

// SIGNAL 129
void Settings::audioBufferChanged(QObject * _t1, qint64 _t2, qint64 _t3, const QByteArray & _t4)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 129, nullptr, _t1, _t2, _t3, _t4);
}

// SIGNAL 130
void Settings::displayWidgetHeightChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 130, nullptr, _t1);
}

// SIGNAL 131
void Settings::spectrumSizeChanged(QObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 131, nullptr, _t1, _t2);
}

// SIGNAL 132
void Settings::panadapterColorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 132, nullptr);
}

// SIGNAL 133
void Settings::panGridStatusChanged(bool _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 133, nullptr, _t1, _t2);
}

// SIGNAL 134
void Settings::peakHoldStatusChanged(bool _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 134, nullptr, _t1, _t2);
}

// SIGNAL 135
void Settings::panLockedStatusChanged(bool _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 135, nullptr, _t1, _t2);
}

// SIGNAL 136
void Settings::clickVFOStatusChanged(bool _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 136, nullptr, _t1, _t2);
}

// SIGNAL 137
void Settings::hairCrossStatusChanged(bool _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 137, nullptr, _t1, _t2);
}

// SIGNAL 138
void Settings::showAGCLinesStatusChanged(QObject * _t1, bool _t2, int _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 138, nullptr, _t1, _t2, _t3);
}

// SIGNAL 139
void Settings::spectrumAveragingChanged(QObject * _t1, int _t2, bool _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 139, nullptr, _t1, _t2, _t3);
}

// SIGNAL 140
void Settings::spectrumAveragingCntChanged(QObject * _t1, int _t2, int _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 140, nullptr, _t1, _t2, _t3);
}

// SIGNAL 141
void Settings::waterfallTimeChanged(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 141, nullptr, _t1, _t2);
}

// SIGNAL 142
void Settings::waterfallOffesetLoChanged(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 142, nullptr, _t1, _t2);
}

// SIGNAL 143
void Settings::waterfallOffesetHiChanged(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 143, nullptr, _t1, _t2);
}

// SIGNAL 144
void Settings::sMeterHoldTimeChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 144, nullptr, _t1);
}

// SIGNAL 145
void Settings::dBmScaleMinChanged(int _t1, qreal _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 145, nullptr, _t1, _t2);
}

// SIGNAL 146
void Settings::dBmScaleMaxChanged(int _t1, qreal _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 146, nullptr, _t1, _t2);
}

// SIGNAL 147
void Settings::agcMaximumGainChanged(QObject * _t1, int _t2, qreal _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 147, nullptr, _t1, _t2, _t3);
}

// SIGNAL 148
void Settings::noiseBlankerChanged(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 148, nullptr, _t1, _t2);
}

// SIGNAL 149
void Settings::noiseFilterChanged(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 149, nullptr, _t1, _t2);
}

// SIGNAL 150
void Settings::nr2GainMethodChanged(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 150, nullptr, _t1, _t2);
}

// SIGNAL 151
void Settings::nr2NpeMethodChanged(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 151, nullptr, _t1, _t2);
}

// SIGNAL 152
void Settings::nrAgcChanged(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 152, nullptr, _t1, _t2);
}

// SIGNAL 153
void Settings::nr2AeChanged(int _t1, bool _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 153, nullptr, _t1, _t2);
}

// SIGNAL 154
void Settings::snbChanged(int _t1, bool _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 154, nullptr, _t1, _t2);
}

// SIGNAL 155
void Settings::anfChanged(int _t1, bool _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 155, nullptr, _t1, _t2);
}

// SIGNAL 156
void Settings::micInputLevelChanged(QObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 156, nullptr, _t1, _t2);
}

// SIGNAL 157
void Settings::driveLevelChanged(QObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 157, nullptr, _t1, _t2);
}

// SIGNAL 158
void Settings::repeaterModeChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 158, nullptr, _t1);
}

// SIGNAL 159
void Settings::repeaterOffsetchanged(double _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 159, nullptr, _t1);
}

// SIGNAL 160
void Settings::fmPremphasizechanged(double _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 160, nullptr, _t1);
}

// SIGNAL 161
void Settings::fmdeveationchanged(double _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 161, nullptr, _t1);
}

// SIGNAL 162
void Settings::amCarrierlevelchanged(double _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 162, nullptr, _t1);
}

// SIGNAL 163
void Settings::audioCompressionchanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 163, nullptr, _t1);
}

// SIGNAL 164
void Settings::micModeChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 164, nullptr, _t1);
}

// SIGNAL 165
void Settings::showRadioPopupChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 165, nullptr, _t1);
}

// SIGNAL 166
void Settings::receiverDataReady()
{
    QMetaObject::activate(this, &staticMetaObject, 166, nullptr);
}

// SIGNAL 167
void Settings::CwHangTimeChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 167, nullptr, _t1);
}

// SIGNAL 168
void Settings::CwSidetoneFreqChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 168, nullptr, _t1);
}

// SIGNAL 169
void Settings::CwKeyReversedChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 169, nullptr, _t1);
}

// SIGNAL 170
void Settings::CwKeyerModeChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 170, nullptr, _t1);
}

// SIGNAL 171
void Settings::InternalCwChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 171, nullptr, _t1);
}

// SIGNAL 172
void Settings::CwKeyerSpeedChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 172, nullptr, _t1);
}

// SIGNAL 173
void Settings::CwPttDelayChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 173, nullptr, _t1);
}

// SIGNAL 174
void Settings::CwSidetoneVolumeChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 174, nullptr, _t1);
}

// SIGNAL 175
void Settings::CwKeyerWeightChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 175, nullptr, _t1);
}

// SIGNAL 176
void Settings::CwKeyerSpacingChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 176, nullptr, _t1);
}
QT_WARNING_POP
