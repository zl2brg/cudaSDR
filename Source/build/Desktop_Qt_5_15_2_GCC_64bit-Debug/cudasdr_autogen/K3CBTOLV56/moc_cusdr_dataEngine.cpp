/****************************************************************************
** Meta object code from reading C++ file 'cusdr_dataEngine.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/DataEngine/cusdr_dataEngine.h"
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cusdr_dataEngine.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN10DataEngineE_t {};
} // unnamed namespace

template <> constexpr inline auto DataEngine::qt_create_metaobjectdata<qt_meta_tag_ZN10DataEngineE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "DataEngine",
        "error",
        "",
        "QUdpSocket::SocketError",
        "masterSwitchEvent",
        "sender",
        "power",
        "penelopeVersionInfoEvent",
        "version",
        "hwIOVersionInfoEvent",
        "sendIQEvent",
        "sendIQ",
        "rcveIQEvent",
        "value",
        "chirpDataReady",
        "samples",
        "audioDataReady",
        "clientConnectedEvent",
        "rx",
        "audioRxEvent",
        "systemMessageEvent",
        "str",
        "time",
        "clearSystemMessageEvent",
        "DataProcessorReadyEvent",
        "audioSenderReadyEvent",
        "initDataEngine",
        "stop",
        "setWbSpectrumAveraging",
        "setRxPeerAddress",
        "QHostAddress",
        "address",
        "setRxClient",
        "client",
        "setRx",
        "setRxSocketState",
        "const char*",
        "prop",
        "setRcveIQSignal",
        "setAudioReceiver",
        "setIQPort",
        "port",
        "setRxConnectedStatus",
        "setClientConnected",
        "setClientDisconnected",
        "setFramesPerSecond",
        "processFileBuffer",
        "QList<qreal>",
        "data",
        "setPenelopeVersion",
        "setHwIOVersion",
        "setNumberOfRx",
        "setSampleRate",
        "setMercuryAttenuator",
        "HamBand",
        "band",
        "setDither",
        "setRandom",
        "setTimeStamp",
        "set10MhzSource",
        "source",
        "set122_88MhzSource",
        "setMicSource",
        "setMercuryClass",
        "setMercuryTiming",
        "setHamBand",
        "byBtn",
        "setFrequency",
        "mode",
        "frequency",
        "set_tx_drivelevel",
        "setRepeaterMode",
        "suspend",
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
        "CwKeyerweight",
        "CwKeyerSpacingChanged",
        "CwKeyerSpacing",
        "systemStateChanged",
        "QSDR::_Error",
        "err",
        "QSDR::_HWInterfaceMode",
        "hwmode",
        "QSDR::_ServerMode",
        "QSDR::_DataEngineState",
        "state",
        "setHPSDRDeviceNumber",
        "rxListChanged",
        "QList<Receiver*>",
        "rxList",
        "searchHpsdrNetworkDevices",
        "setCurrentReceiver",
        "setMercuryAttenuators",
        "QList<int>",
        "attn",
        "setAlexConfiguration",
        "conf",
        "setAlexStates",
        "states",
        "setPennyOCEnabled",
        "setRxJ6Pins",
        "list",
        "setTxJ6Pins",
        "radioStateChange",
        "RadioState",
        "dspModeChanged",
        "DSPMode"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'error'
        QtMocHelpers::SignalData<void(QUdpSocket::SocketError)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 1 },
        }}),
        // Signal 'masterSwitchEvent'
        QtMocHelpers::SignalData<void(QObject *, bool)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Bool, 6 },
        }}),
        // Signal 'penelopeVersionInfoEvent'
        QtMocHelpers::SignalData<void(QObject *, int)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 8 },
        }}),
        // Signal 'hwIOVersionInfoEvent'
        QtMocHelpers::SignalData<void(QObject *, int)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 8 },
        }}),
        // Signal 'sendIQEvent'
        QtMocHelpers::SignalData<void(QObject *, int)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 11 },
        }}),
        // Signal 'rcveIQEvent'
        QtMocHelpers::SignalData<void(QObject *, int)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 13 },
        }}),
        // Signal 'chirpDataReady'
        QtMocHelpers::SignalData<void(int)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 15 },
        }}),
        // Signal 'audioDataReady'
        QtMocHelpers::SignalData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'clientConnectedEvent'
        QtMocHelpers::SignalData<void(int)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 18 },
        }}),
        // Signal 'audioRxEvent'
        QtMocHelpers::SignalData<void(int)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 18 },
        }}),
        // Signal 'systemMessageEvent'
        QtMocHelpers::SignalData<void(const QString &, int)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 21 }, { QMetaType::Int, 22 },
        }}),
        // Signal 'clearSystemMessageEvent'
        QtMocHelpers::SignalData<void()>(23, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'DataProcessorReadyEvent'
        QtMocHelpers::SignalData<void()>(24, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'audioSenderReadyEvent'
        QtMocHelpers::SignalData<void(bool)>(25, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 13 },
        }}),
        // Slot 'initDataEngine'
        QtMocHelpers::SlotData<bool()>(26, 2, QMC::AccessPublic, QMetaType::Bool),
        // Slot 'stop'
        QtMocHelpers::SlotData<void()>(27, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'setWbSpectrumAveraging'
        QtMocHelpers::SlotData<void(QObject *, int, int)>(28, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 2 }, { QMetaType::Int, 18 }, { QMetaType::Int, 13 },
        }}),
        // Slot 'setRxPeerAddress'
        QtMocHelpers::SlotData<void(int, QHostAddress)>(29, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 18 }, { 0x80000000 | 30, 31 },
        }}),
        // Slot 'setRxClient'
        QtMocHelpers::SlotData<void(int, int)>(32, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 18 }, { QMetaType::Int, 33 },
        }}),
        // Slot 'setRx'
        QtMocHelpers::SlotData<void(int)>(34, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 18 },
        }}),
        // Slot 'setRxSocketState'
        QtMocHelpers::SlotData<void(int, const char *, QString)>(35, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 18 }, { 0x80000000 | 36, 37 }, { QMetaType::QString, 2 },
        }}),
        // Slot 'setRcveIQSignal'
        QtMocHelpers::SlotData<void(QObject *, int)>(38, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 13 },
        }}),
        // Slot 'setAudioReceiver'
        QtMocHelpers::SlotData<void(QObject *, int)>(39, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 18 },
        }}),
        // Slot 'setIQPort'
        QtMocHelpers::SlotData<void(int, int)>(40, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 18 }, { QMetaType::Int, 41 },
        }}),
        // Slot 'setRxConnectedStatus'
        QtMocHelpers::SlotData<void(QObject *, int, bool)>(42, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 18 }, { QMetaType::Bool, 13 },
        }}),
        // Slot 'setClientConnected'
        QtMocHelpers::SlotData<void(QObject *, int)>(43, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 18 },
        }}),
        // Slot 'setClientConnected'
        QtMocHelpers::SlotData<void(bool)>(43, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 13 },
        }}),
        // Slot 'setClientDisconnected'
        QtMocHelpers::SlotData<void(int)>(44, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 33 },
        }}),
        // Slot 'setFramesPerSecond'
        QtMocHelpers::SlotData<void(QObject *, int, int)>(45, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 18 }, { QMetaType::Int, 13 },
        }}),
        // Slot 'processFileBuffer'
        QtMocHelpers::SlotData<void(const QList<qreal>)>(46, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 47, 48 },
        }}),
        // Slot 'setPenelopeVersion'
        QtMocHelpers::SlotData<void(QObject *, int)>(49, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 8 },
        }}),
        // Slot 'setHwIOVersion'
        QtMocHelpers::SlotData<void(QObject *, int)>(50, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 8 },
        }}),
        // Slot 'setNumberOfRx'
        QtMocHelpers::SlotData<void(QObject *, int)>(51, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 13 },
        }}),
        // Slot 'setSampleRate'
        QtMocHelpers::SlotData<void(QObject *, int)>(52, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 13 },
        }}),
        // Slot 'setMercuryAttenuator'
        QtMocHelpers::SlotData<void(QObject *, HamBand, int)>(53, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { 0x80000000 | 54, 55 }, { QMetaType::Int, 13 },
        }}),
        // Slot 'setDither'
        QtMocHelpers::SlotData<void(QObject *, int)>(56, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 13 },
        }}),
        // Slot 'setRandom'
        QtMocHelpers::SlotData<void(QObject *, int)>(57, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 13 },
        }}),
        // Slot 'setTimeStamp'
        QtMocHelpers::SlotData<void(QObject *, bool)>(58, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Bool, 13 },
        }}),
        // Slot 'set10MhzSource'
        QtMocHelpers::SlotData<void(QObject *, int)>(59, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 60 },
        }}),
        // Slot 'set122_88MhzSource'
        QtMocHelpers::SlotData<void(QObject *, int)>(61, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 60 },
        }}),
        // Slot 'setMicSource'
        QtMocHelpers::SlotData<void(int)>(62, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 60 },
        }}),
        // Slot 'setMercuryClass'
        QtMocHelpers::SlotData<void(QObject *, int)>(63, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 13 },
        }}),
        // Slot 'setMercuryTiming'
        QtMocHelpers::SlotData<void(QObject *, int)>(64, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 13 },
        }}),
        // Slot 'setHamBand'
        QtMocHelpers::SlotData<void(QObject *, int, bool, HamBand)>(65, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 18 }, { QMetaType::Bool, 66 }, { 0x80000000 | 54, 55 },
        }}),
        // Slot 'setFrequency'
        QtMocHelpers::SlotData<void(QObject *, int, int, long)>(67, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 68 }, { QMetaType::Int, 18 }, { QMetaType::Long, 69 },
        }}),
        // Slot 'set_tx_drivelevel'
        QtMocHelpers::SlotData<void(QObject *, int)>(70, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 13 },
        }}),
        // Slot 'setRepeaterMode'
        QtMocHelpers::SlotData<void(bool)>(71, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 2 },
        }}),
        // Slot 'suspend'
        QtMocHelpers::SlotData<void()>(72, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'CwHangTimeChanged'
        QtMocHelpers::SlotData<void(int)>(73, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 74 },
        }}),
        // Slot 'CwSidetoneFreqChanged'
        QtMocHelpers::SlotData<void(int)>(75, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 76 },
        }}),
        // Slot 'CwKeyReversedChanged'
        QtMocHelpers::SlotData<void(int)>(77, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 78 },
        }}),
        // Slot 'CwKeyerModeChanged'
        QtMocHelpers::SlotData<void(int)>(79, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 80 },
        }}),
        // Slot 'InternalCwChanged'
        QtMocHelpers::SlotData<void(int)>(81, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 82 },
        }}),
        // Slot 'CwKeyerSpeedChanged'
        QtMocHelpers::SlotData<void(int)>(83, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 84 },
        }}),
        // Slot 'CwPttDelayChanged'
        QtMocHelpers::SlotData<void(int)>(85, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 86 },
        }}),
        // Slot 'CwSidetoneVolumeChanged'
        QtMocHelpers::SlotData<void(int)>(87, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 88 },
        }}),
        // Slot 'CwKeyerWeightChanged'
        QtMocHelpers::SlotData<void(int)>(89, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 90 },
        }}),
        // Slot 'CwKeyerSpacingChanged'
        QtMocHelpers::SlotData<void(int)>(91, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 92 },
        }}),
        // Slot 'systemStateChanged'
        QtMocHelpers::SlotData<void(QObject *, QSDR::_Error, QSDR::_HWInterfaceMode, QSDR::_ServerMode, QSDR::_DataEngineState)>(93, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { 0x80000000 | 94, 95 }, { 0x80000000 | 96, 97 }, { 0x80000000 | 98, 68 },
            { 0x80000000 | 99, 100 },
        }}),
        // Slot 'setHPSDRDeviceNumber'
        QtMocHelpers::SlotData<void(int)>(101, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 13 },
        }}),
        // Slot 'rxListChanged'
        QtMocHelpers::SlotData<void(QList<Receiver*>)>(102, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 103, 104 },
        }}),
        // Slot 'searchHpsdrNetworkDevices'
        QtMocHelpers::SlotData<void()>(105, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'setCurrentReceiver'
        QtMocHelpers::SlotData<void(QObject *, int)>(106, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 18 },
        }}),
        // Slot 'setMercuryAttenuators'
        QtMocHelpers::SlotData<void(QObject *, QList<int>)>(107, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { 0x80000000 | 108, 109 },
        }}),
        // Slot 'setAlexConfiguration'
        QtMocHelpers::SlotData<void(quint16)>(110, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::UShort, 111 },
        }}),
        // Slot 'setAlexStates'
        QtMocHelpers::SlotData<void(HamBand, const QList<int> &)>(112, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 54, 55 }, { 0x80000000 | 108, 113 },
        }}),
        // Slot 'setPennyOCEnabled'
        QtMocHelpers::SlotData<void(bool)>(114, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 13 },
        }}),
        // Slot 'setRxJ6Pins'
        QtMocHelpers::SlotData<void(const QList<int> &)>(115, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 108, 116 },
        }}),
        // Slot 'setTxJ6Pins'
        QtMocHelpers::SlotData<void(const QList<int> &)>(117, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 108, 116 },
        }}),
        // Slot 'radioStateChange'
        QtMocHelpers::SlotData<void(RadioState)>(118, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 119, 100 },
        }}),
        // Slot 'dspModeChanged'
        QtMocHelpers::SlotData<void(QObject *, int, DSPMode)>(120, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 2 }, { QMetaType::Int, 2 }, { 0x80000000 | 121, 2 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<DataEngine, qt_meta_tag_ZN10DataEngineE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject DataEngine::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10DataEngineE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10DataEngineE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10DataEngineE_t>.metaTypes,
    nullptr
} };

void DataEngine::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DataEngine *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->error((*reinterpret_cast< std::add_pointer_t<QUdpSocket::SocketError>>(_a[1]))); break;
        case 1: _t->masterSwitchEvent((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 2: _t->penelopeVersionInfoEvent((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 3: _t->hwIOVersionInfoEvent((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 4: _t->sendIQEvent((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 5: _t->rcveIQEvent((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 6: _t->chirpDataReady((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 7: _t->audioDataReady(); break;
        case 8: _t->clientConnectedEvent((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 9: _t->audioRxEvent((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 10: _t->systemMessageEvent((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 11: _t->clearSystemMessageEvent(); break;
        case 12: _t->DataProcessorReadyEvent(); break;
        case 13: _t->audioSenderReadyEvent((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 14: { bool _r = _t->initDataEngine();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 15: _t->stop(); break;
        case 16: _t->setWbSpectrumAveraging((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 17: _t->setRxPeerAddress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QHostAddress>>(_a[2]))); break;
        case 18: _t->setRxClient((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 19: _t->setRx((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 20: _t->setRxSocketState((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<const char*>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        case 21: _t->setRcveIQSignal((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 22: _t->setAudioReceiver((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 23: _t->setIQPort((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 24: _t->setRxConnectedStatus((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3]))); break;
        case 25: _t->setClientConnected((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 26: _t->setClientConnected((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 27: _t->setClientDisconnected((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 28: _t->setFramesPerSecond((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 29: _t->processFileBuffer((*reinterpret_cast< std::add_pointer_t<QList<qreal>>>(_a[1]))); break;
        case 30: _t->setPenelopeVersion((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 31: _t->setHwIOVersion((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 32: _t->setNumberOfRx((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 33: _t->setSampleRate((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 34: _t->setMercuryAttenuator((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<HamBand>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 35: _t->setDither((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 36: _t->setRandom((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 37: _t->setTimeStamp((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 38: _t->set10MhzSource((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 39: _t->set122_88MhzSource((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 40: _t->setMicSource((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 41: _t->setMercuryClass((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 42: _t->setMercuryTiming((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 43: _t->setHamBand((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<HamBand>>(_a[4]))); break;
        case 44: _t->setFrequency((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<long>>(_a[4]))); break;
        case 45: _t->set_tx_drivelevel((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 46: _t->setRepeaterMode((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 47: _t->suspend(); break;
        case 48: _t->CwHangTimeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 49: _t->CwSidetoneFreqChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 50: _t->CwKeyReversedChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 51: _t->CwKeyerModeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 52: _t->InternalCwChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 53: _t->CwKeyerSpeedChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 54: _t->CwPttDelayChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 55: _t->CwSidetoneVolumeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 56: _t->CwKeyerWeightChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 57: _t->CwKeyerSpacingChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 58: _t->systemStateChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QSDR::_Error>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QSDR::_HWInterfaceMode>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QSDR::_ServerMode>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<QSDR::_DataEngineState>>(_a[5]))); break;
        case 59: _t->setHPSDRDeviceNumber((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 60: _t->rxListChanged((*reinterpret_cast< std::add_pointer_t<QList<Receiver*>>>(_a[1]))); break;
        case 61: _t->searchHpsdrNetworkDevices(); break;
        case 62: _t->setCurrentReceiver((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 63: _t->setMercuryAttenuators((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QList<int>>>(_a[2]))); break;
        case 64: _t->setAlexConfiguration((*reinterpret_cast< std::add_pointer_t<quint16>>(_a[1]))); break;
        case 65: _t->setAlexStates((*reinterpret_cast< std::add_pointer_t<HamBand>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QList<int>>>(_a[2]))); break;
        case 66: _t->setPennyOCEnabled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 67: _t->setRxJ6Pins((*reinterpret_cast< std::add_pointer_t<QList<int>>>(_a[1]))); break;
        case 68: _t->setTxJ6Pins((*reinterpret_cast< std::add_pointer_t<QList<int>>>(_a[1]))); break;
        case 69: _t->radioStateChange((*reinterpret_cast< std::add_pointer_t<RadioState>>(_a[1]))); break;
        case 70: _t->dspModeChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<DSPMode>>(_a[3]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 29:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<qreal> >(); break;
            }
            break;
        case 34:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< HamBand >(); break;
            }
            break;
        case 43:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 3:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< HamBand >(); break;
            }
            break;
        case 58:
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
        case 60:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<Receiver*> >(); break;
            }
            break;
        case 63:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<int> >(); break;
            }
            break;
        case 65:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< HamBand >(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<int> >(); break;
            }
            break;
        case 67:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
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
        case 70:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 2:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< DSPMode >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (DataEngine::*)(QUdpSocket::SocketError )>(_a, &DataEngine::error, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataEngine::*)(QObject * , bool )>(_a, &DataEngine::masterSwitchEvent, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataEngine::*)(QObject * , int )>(_a, &DataEngine::penelopeVersionInfoEvent, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataEngine::*)(QObject * , int )>(_a, &DataEngine::hwIOVersionInfoEvent, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataEngine::*)(QObject * , int )>(_a, &DataEngine::sendIQEvent, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataEngine::*)(QObject * , int )>(_a, &DataEngine::rcveIQEvent, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataEngine::*)(int )>(_a, &DataEngine::chirpDataReady, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataEngine::*)()>(_a, &DataEngine::audioDataReady, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataEngine::*)(int )>(_a, &DataEngine::clientConnectedEvent, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataEngine::*)(int )>(_a, &DataEngine::audioRxEvent, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataEngine::*)(const QString & , int )>(_a, &DataEngine::systemMessageEvent, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataEngine::*)()>(_a, &DataEngine::clearSystemMessageEvent, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataEngine::*)()>(_a, &DataEngine::DataProcessorReadyEvent, 12))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataEngine::*)(bool )>(_a, &DataEngine::audioSenderReadyEvent, 13))
            return;
    }
}

const QMetaObject *DataEngine::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DataEngine::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10DataEngineE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DataEngine::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 71)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 71;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 71)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 71;
    }
    return _id;
}

// SIGNAL 0
void DataEngine::error(QUdpSocket::SocketError _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void DataEngine::masterSwitchEvent(QObject * _t1, bool _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2);
}

// SIGNAL 2
void DataEngine::penelopeVersionInfoEvent(QObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void DataEngine::hwIOVersionInfoEvent(QObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}

// SIGNAL 4
void DataEngine::sendIQEvent(QObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2);
}

// SIGNAL 5
void DataEngine::rcveIQEvent(QObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1, _t2);
}

// SIGNAL 6
void DataEngine::chirpDataReady(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}

// SIGNAL 7
void DataEngine::audioDataReady()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void DataEngine::clientConnectedEvent(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1);
}

// SIGNAL 9
void DataEngine::audioRxEvent(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 9, nullptr, _t1);
}

// SIGNAL 10
void DataEngine::systemMessageEvent(const QString & _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 10, nullptr, _t1, _t2);
}

// SIGNAL 11
void DataEngine::clearSystemMessageEvent()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}

// SIGNAL 12
void DataEngine::DataProcessorReadyEvent()
{
    QMetaObject::activate(this, &staticMetaObject, 12, nullptr);
}

// SIGNAL 13
void DataEngine::audioSenderReadyEvent(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 13, nullptr, _t1);
}
namespace {
struct qt_meta_tag_ZN13DataProcessorE_t {};
} // unnamed namespace

template <> constexpr inline auto DataProcessor::qt_create_metaobjectdata<qt_meta_tag_ZN13DataProcessorE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "DataProcessor",
        "messageEvent",
        "",
        "message",
        "connectingEvent",
        "addr",
        "port",
        "connectedEvent",
        "disconnectedEvent",
        "serverVersionEvent",
        "version",
        "keyer_event",
        "val",
        "cw",
        "stop",
        "processReadData",
        "processDeviceData",
        "processMicData",
        "displayDataProcessorSocketError",
        "QAbstractSocket::SocketError",
        "error",
        "initDataProcessorSocket",
        "processInputBuffer",
        "buffer",
        "processOutputBuffer",
        "CPX",
        "decodeCCBytes",
        "encodeCCBytes",
        "setOutputBuffer",
        "rx",
        "setAudioBuffer",
        "buffersize",
        "send_hpsdr_data",
        "setAudioBuffer_old",
        "writeData",
        "buffer_tx_data",
        "key_down",
        "key_down_test"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'messageEvent'
        QtMocHelpers::SignalData<void(QString)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'connectingEvent'
        QtMocHelpers::SignalData<void(QString, quint16)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 5 }, { QMetaType::UShort, 6 },
        }}),
        // Signal 'connectedEvent'
        QtMocHelpers::SignalData<void(QString, quint16)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 5 }, { QMetaType::UShort, 6 },
        }}),
        // Signal 'disconnectedEvent'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'serverVersionEvent'
        QtMocHelpers::SignalData<void(QString)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 10 },
        }}),
        // Signal 'keyer_event'
        QtMocHelpers::SignalData<void(int, int)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 12 }, { QMetaType::Int, 13 },
        }}),
        // Slot 'stop'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'processReadData'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'processDeviceData'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'processMicData'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'displayDataProcessorSocketError'
        QtMocHelpers::SlotData<void(QAbstractSocket::SocketError)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 19, 20 },
        }}),
        // Slot 'initDataProcessorSocket'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'processInputBuffer'
        QtMocHelpers::SlotData<void(const QByteArray &)>(22, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QByteArray, 23 },
        }}),
        // Slot 'processOutputBuffer'
        QtMocHelpers::SlotData<void(const CPX &)>(24, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 25, 23 },
        }}),
        // Slot 'decodeCCBytes'
        QtMocHelpers::SlotData<void(const QByteArray &)>(26, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QByteArray, 23 },
        }}),
        // Slot 'encodeCCBytes'
        QtMocHelpers::SlotData<void()>(27, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'setOutputBuffer'
        QtMocHelpers::SlotData<void(int, const CPX &)>(28, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 29 }, { 0x80000000 | 25, 23 },
        }}),
        // Slot 'setAudioBuffer'
        QtMocHelpers::SlotData<void(int, const CPX &, int)>(30, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 29 }, { 0x80000000 | 25, 23 }, { QMetaType::Int, 31 },
        }}),
        // Slot 'send_hpsdr_data'
        QtMocHelpers::SlotData<void(int, const CPX &, int)>(32, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 29 }, { 0x80000000 | 25, 23 }, { QMetaType::Int, 31 },
        }}),
        // Slot 'setAudioBuffer_old'
        QtMocHelpers::SlotData<void(int, const CPX &, int)>(33, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 29 }, { 0x80000000 | 25, 23 }, { QMetaType::Int, 31 },
        }}),
        // Slot 'writeData'
        QtMocHelpers::SlotData<void()>(34, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'buffer_tx_data'
        QtMocHelpers::SlotData<void()>(35, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'key_down'
        QtMocHelpers::SlotData<void(int)>(36, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 2 },
        }}),
        // Slot 'key_down_test'
        QtMocHelpers::SlotData<void(int, int)>(37, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 2 }, { QMetaType::Int, 2 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<DataProcessor, qt_meta_tag_ZN13DataProcessorE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject DataProcessor::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13DataProcessorE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13DataProcessorE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN13DataProcessorE_t>.metaTypes,
    nullptr
} };

void DataProcessor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DataProcessor *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->messageEvent((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->connectingEvent((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<quint16>>(_a[2]))); break;
        case 2: _t->connectedEvent((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<quint16>>(_a[2]))); break;
        case 3: _t->disconnectedEvent(); break;
        case 4: _t->serverVersionEvent((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->keyer_event((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 6: _t->stop(); break;
        case 7: _t->processReadData(); break;
        case 8: _t->processDeviceData(); break;
        case 9: _t->processMicData(); break;
        case 10: _t->displayDataProcessorSocketError((*reinterpret_cast< std::add_pointer_t<QAbstractSocket::SocketError>>(_a[1]))); break;
        case 11: _t->initDataProcessorSocket(); break;
        case 12: _t->processInputBuffer((*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[1]))); break;
        case 13: _t->processOutputBuffer((*reinterpret_cast< std::add_pointer_t<CPX>>(_a[1]))); break;
        case 14: _t->decodeCCBytes((*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[1]))); break;
        case 15: _t->encodeCCBytes(); break;
        case 16: _t->setOutputBuffer((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<CPX>>(_a[2]))); break;
        case 17: _t->setAudioBuffer((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<CPX>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 18: _t->send_hpsdr_data((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<CPX>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 19: _t->setAudioBuffer_old((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<CPX>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 20: _t->writeData(); break;
        case 21: _t->buffer_tx_data(); break;
        case 22: _t->key_down((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 23: _t->key_down_test((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 10:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QAbstractSocket::SocketError >(); break;
            }
            break;
        case 13:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< CPX >(); break;
            }
            break;
        case 16:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< CPX >(); break;
            }
            break;
        case 17:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< CPX >(); break;
            }
            break;
        case 18:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< CPX >(); break;
            }
            break;
        case 19:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< CPX >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (DataProcessor::*)(QString )>(_a, &DataProcessor::messageEvent, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataProcessor::*)(QString , quint16 )>(_a, &DataProcessor::connectingEvent, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataProcessor::*)(QString , quint16 )>(_a, &DataProcessor::connectedEvent, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataProcessor::*)()>(_a, &DataProcessor::disconnectedEvent, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataProcessor::*)(QString )>(_a, &DataProcessor::serverVersionEvent, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataProcessor::*)(int , int )>(_a, &DataProcessor::keyer_event, 5))
            return;
    }
}

const QMetaObject *DataProcessor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DataProcessor::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13DataProcessorE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DataProcessor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 24)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 24;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 24)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 24;
    }
    return _id;
}

// SIGNAL 0
void DataProcessor::messageEvent(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void DataProcessor::connectingEvent(QString _t1, quint16 _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2);
}

// SIGNAL 2
void DataProcessor::connectedEvent(QString _t1, quint16 _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void DataProcessor::disconnectedEvent()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void DataProcessor::serverVersionEvent(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void DataProcessor::keyer_event(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1, _t2);
}
namespace {
struct qt_meta_tag_ZN17AudioOutProcessorE_t {};
} // unnamed namespace

template <> constexpr inline auto AudioOutProcessor::qt_create_metaobjectdata<qt_meta_tag_ZN17AudioOutProcessorE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "AudioOutProcessor",
        "stop",
        "",
        "processData",
        "processDeviceData"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'stop'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'processData'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'processDeviceData'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AudioOutProcessor, qt_meta_tag_ZN17AudioOutProcessorE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject AudioOutProcessor::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17AudioOutProcessorE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17AudioOutProcessorE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN17AudioOutProcessorE_t>.metaTypes,
    nullptr
} };

void AudioOutProcessor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AudioOutProcessor *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->stop(); break;
        case 1: _t->processData(); break;
        case 2: _t->processDeviceData(); break;
        default: ;
        }
    }
    (void)_a;
}

const QMetaObject *AudioOutProcessor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AudioOutProcessor::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17AudioOutProcessorE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int AudioOutProcessor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}
QT_WARNING_POP
