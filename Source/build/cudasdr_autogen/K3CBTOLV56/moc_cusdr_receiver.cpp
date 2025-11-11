/****************************************************************************
** Meta object code from reading C++ file 'cusdr_receiver.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/DataEngine/cusdr_receiver.h"
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cusdr_receiver.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.2. It"
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
struct qt_meta_tag_ZN8ReceiverE_t {};
} // unnamed namespace

template <> constexpr inline auto Receiver::qt_create_metaobjectdata<qt_meta_tag_ZN8ReceiverE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Receiver",
        "messageEvent",
        "",
        "msg",
        "spectrumBufferChanged",
        "rx",
        "qVectorFloat",
        "buffer",
        "sMeterValueChanged",
        "value",
        "outputBufferSignal",
        "CPX",
        "audioBufferSignal",
        "setReceiverData",
        "TReceiver",
        "data",
        "setAudioMode",
        "sender",
        "mode",
        "setServerMode",
        "QSDR::_ServerMode",
        "setPeerAddress",
        "QHostAddress",
        "addr",
        "setSocketDescriptor",
        "setReceiver",
        "setClient",
        "setIQPort",
        "setBSPort",
        "setConnectedStatus",
        "setSampleRate",
        "setHamBand",
        "byBtn",
        "HamBand",
        "band",
        "setDspMode",
        "DSPMode",
        "setADCMode",
        "ADCMode",
        "setAGCMode",
        "AGCMode",
        "hang",
        "setAGCGain",
        "setAudioVolume",
        "setCtrFrequency",
        "frequency",
        "setVfoFrequency",
        "setFilterFrequencies",
        "low",
        "high",
        "setLastCtrFrequencyList",
        "QList<long>",
        "frequencies",
        "setLastVfoFrequencyList",
        "setdBmPanScaleMin",
        "setdBmPanScaleMax",
        "setMercuryAttenuators",
        "QList<int>",
        "attenuators",
        "dspProcessing",
        "stop",
        "setSystemState",
        "QSDR::_Error",
        "err",
        "QSDR::_HWInterfaceMode",
        "hwmode",
        "QSDR::_DataEngineState",
        "state",
        "setFramesPerSecond",
        "initQtWDSPInterface",
        "setAGCMaximumGain_dB",
        "setAGCFixedGain_dB",
        "setAGCThreshold_dB",
        "setAGCHangLevel_dB",
        "setAGCHangThreshold",
        "setAGCSlope_dB",
        "setAGCAttackTime",
        "setAGCDecayTime",
        "setAGCHangTime"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'messageEvent'
        QtMocHelpers::SignalData<void(QString)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'spectrumBufferChanged'
        QtMocHelpers::SignalData<void(int, const qVectorFloat &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 5 }, { 0x80000000 | 6, 7 },
        }}),
        // Signal 'sMeterValueChanged'
        QtMocHelpers::SignalData<void(int, double)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 5 }, { QMetaType::Double, 9 },
        }}),
        // Signal 'outputBufferSignal'
        QtMocHelpers::SignalData<void(int, const CPX &)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 5 }, { 0x80000000 | 11, 7 },
        }}),
        // Signal 'audioBufferSignal'
        QtMocHelpers::SignalData<void(int, const CPX &, int)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 5 }, { 0x80000000 | 11, 7 }, { QMetaType::Int, 2 },
        }}),
        // Slot 'setReceiverData'
        QtMocHelpers::SlotData<void(TReceiver)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 14, 15 },
        }}),
        // Slot 'setAudioMode'
        QtMocHelpers::SlotData<void(QObject *, int)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 17 }, { QMetaType::Int, 18 },
        }}),
        // Slot 'setServerMode'
        QtMocHelpers::SlotData<void(QSDR::_ServerMode)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 20, 18 },
        }}),
        // Slot 'setPeerAddress'
        QtMocHelpers::SlotData<void(QHostAddress)>(21, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 22, 23 },
        }}),
        // Slot 'setSocketDescriptor'
        QtMocHelpers::SlotData<void(int)>(24, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 9 },
        }}),
        // Slot 'setReceiver'
        QtMocHelpers::SlotData<void(int)>(25, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 9 },
        }}),
        // Slot 'setClient'
        QtMocHelpers::SlotData<void(int)>(26, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 9 },
        }}),
        // Slot 'setIQPort'
        QtMocHelpers::SlotData<void(int)>(27, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 9 },
        }}),
        // Slot 'setBSPort'
        QtMocHelpers::SlotData<void(int)>(28, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 9 },
        }}),
        // Slot 'setConnectedStatus'
        QtMocHelpers::SlotData<void(bool)>(29, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 9 },
        }}),
        // Slot 'setSampleRate'
        QtMocHelpers::SlotData<void(QObject *, int)>(30, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 17 }, { QMetaType::Int, 9 },
        }}),
        // Slot 'setHamBand'
        QtMocHelpers::SlotData<void(QObject *, int, bool, HamBand)>(31, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 17 }, { QMetaType::Int, 5 }, { QMetaType::Bool, 32 }, { 0x80000000 | 33, 34 },
        }}),
        // Slot 'setDspMode'
        QtMocHelpers::SlotData<void(QObject *, int, DSPMode)>(35, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 17 }, { QMetaType::Int, 5 }, { 0x80000000 | 36, 18 },
        }}),
        // Slot 'setADCMode'
        QtMocHelpers::SlotData<void(QObject *, int, ADCMode)>(37, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 17 }, { QMetaType::Int, 5 }, { 0x80000000 | 38, 18 },
        }}),
        // Slot 'setAGCMode'
        QtMocHelpers::SlotData<void(QObject *, int, AGCMode, bool)>(39, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 17 }, { QMetaType::Int, 5 }, { 0x80000000 | 40, 18 }, { QMetaType::Bool, 41 },
        }}),
        // Slot 'setAGCGain'
        QtMocHelpers::SlotData<void(QObject *, int, int)>(42, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 17 }, { QMetaType::Int, 5 }, { QMetaType::Int, 9 },
        }}),
        // Slot 'setAudioVolume'
        QtMocHelpers::SlotData<void(QObject *, int, float)>(43, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 17 }, { QMetaType::Int, 5 }, { QMetaType::Float, 9 },
        }}),
        // Slot 'setCtrFrequency'
        QtMocHelpers::SlotData<void(long)>(44, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Long, 45 },
        }}),
        // Slot 'setVfoFrequency'
        QtMocHelpers::SlotData<void(long)>(46, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Long, 45 },
        }}),
        // Slot 'setFilterFrequencies'
        QtMocHelpers::SlotData<void(QObject *, int, qreal, qreal)>(47, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 17 }, { QMetaType::Int, 5 }, { QMetaType::QReal, 48 }, { QMetaType::QReal, 49 },
        }}),
        // Slot 'setLastCtrFrequencyList'
        QtMocHelpers::SlotData<void(const QList<long> &)>(50, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 51, 52 },
        }}),
        // Slot 'setLastVfoFrequencyList'
        QtMocHelpers::SlotData<void(const QList<long> &)>(53, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 51, 52 },
        }}),
        // Slot 'setdBmPanScaleMin'
        QtMocHelpers::SlotData<void(qreal)>(54, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QReal, 9 },
        }}),
        // Slot 'setdBmPanScaleMax'
        QtMocHelpers::SlotData<void(qreal)>(55, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QReal, 9 },
        }}),
        // Slot 'setMercuryAttenuators'
        QtMocHelpers::SlotData<void(const QList<int> &)>(56, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 57, 58 },
        }}),
        // Slot 'dspProcessing'
        QtMocHelpers::SlotData<void()>(59, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'stop'
        QtMocHelpers::SlotData<void()>(60, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'setSystemState'
        QtMocHelpers::SlotData<void(QObject *, QSDR::_Error, QSDR::_HWInterfaceMode, QSDR::_ServerMode, QSDR::_DataEngineState)>(61, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 17 }, { 0x80000000 | 62, 63 }, { 0x80000000 | 64, 65 }, { 0x80000000 | 20, 18 },
            { 0x80000000 | 66, 67 },
        }}),
        // Slot 'setFramesPerSecond'
        QtMocHelpers::SlotData<void(QObject *, int, int)>(68, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 17 }, { QMetaType::Int, 5 }, { QMetaType::Int, 9 },
        }}),
        // Slot 'initQtWDSPInterface'
        QtMocHelpers::SlotData<bool()>(69, 2, QMC::AccessPrivate, QMetaType::Bool),
        // Slot 'setAGCMaximumGain_dB'
        QtMocHelpers::SlotData<void(QObject *, int, qreal)>(70, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 17 }, { QMetaType::Int, 5 }, { QMetaType::QReal, 9 },
        }}),
        // Slot 'setAGCFixedGain_dB'
        QtMocHelpers::SlotData<void(QObject *, int, qreal)>(71, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 17 }, { QMetaType::Int, 5 }, { QMetaType::QReal, 9 },
        }}),
        // Slot 'setAGCThreshold_dB'
        QtMocHelpers::SlotData<void(QObject *, int, qreal)>(72, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 17 }, { QMetaType::Int, 5 }, { QMetaType::QReal, 9 },
        }}),
        // Slot 'setAGCHangLevel_dB'
        QtMocHelpers::SlotData<void(QObject *, int, qreal)>(73, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 17 }, { QMetaType::Int, 5 }, { QMetaType::QReal, 9 },
        }}),
        // Slot 'setAGCHangThreshold'
        QtMocHelpers::SlotData<void(QObject *, int, int)>(74, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 17 }, { QMetaType::Int, 5 }, { QMetaType::Int, 9 },
        }}),
        // Slot 'setAGCSlope_dB'
        QtMocHelpers::SlotData<void(QObject *, int, qreal)>(75, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 17 }, { QMetaType::Int, 5 }, { QMetaType::QReal, 9 },
        }}),
        // Slot 'setAGCAttackTime'
        QtMocHelpers::SlotData<void(QObject *, int, qreal)>(76, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 17 }, { QMetaType::Int, 5 }, { QMetaType::QReal, 9 },
        }}),
        // Slot 'setAGCDecayTime'
        QtMocHelpers::SlotData<void(QObject *, int, qreal)>(77, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 17 }, { QMetaType::Int, 5 }, { QMetaType::QReal, 9 },
        }}),
        // Slot 'setAGCHangTime'
        QtMocHelpers::SlotData<void(QObject *, int, qreal)>(78, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 17 }, { QMetaType::Int, 5 }, { QMetaType::QReal, 9 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<Receiver, qt_meta_tag_ZN8ReceiverE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Receiver::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8ReceiverE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8ReceiverE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN8ReceiverE_t>.metaTypes,
    nullptr
} };

void Receiver::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<Receiver *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->messageEvent((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->spectrumBufferChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qVectorFloat>>(_a[2]))); break;
        case 2: _t->sMeterValueChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 3: _t->outputBufferSignal((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<CPX>>(_a[2]))); break;
        case 4: _t->audioBufferSignal((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<CPX>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 5: _t->setReceiverData((*reinterpret_cast< std::add_pointer_t<TReceiver>>(_a[1]))); break;
        case 6: _t->setAudioMode((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 7: _t->setServerMode((*reinterpret_cast< std::add_pointer_t<QSDR::_ServerMode>>(_a[1]))); break;
        case 8: _t->setPeerAddress((*reinterpret_cast< std::add_pointer_t<QHostAddress>>(_a[1]))); break;
        case 9: _t->setSocketDescriptor((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 10: _t->setReceiver((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 11: _t->setClient((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 12: _t->setIQPort((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 13: _t->setBSPort((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 14: _t->setConnectedStatus((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 15: _t->setSampleRate((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 16: _t->setHamBand((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<HamBand>>(_a[4]))); break;
        case 17: _t->setDspMode((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<DSPMode>>(_a[3]))); break;
        case 18: _t->setADCMode((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<ADCMode>>(_a[3]))); break;
        case 19: _t->setAGCMode((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<AGCMode>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[4]))); break;
        case 20: _t->setAGCGain((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 21: _t->setAudioVolume((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<float>>(_a[3]))); break;
        case 22: _t->setCtrFrequency((*reinterpret_cast< std::add_pointer_t<long>>(_a[1]))); break;
        case 23: _t->setVfoFrequency((*reinterpret_cast< std::add_pointer_t<long>>(_a[1]))); break;
        case 24: _t->setFilterFrequencies((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[4]))); break;
        case 25: _t->setLastCtrFrequencyList((*reinterpret_cast< std::add_pointer_t<QList<long>>>(_a[1]))); break;
        case 26: _t->setLastVfoFrequencyList((*reinterpret_cast< std::add_pointer_t<QList<long>>>(_a[1]))); break;
        case 27: _t->setdBmPanScaleMin((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1]))); break;
        case 28: _t->setdBmPanScaleMax((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1]))); break;
        case 29: _t->setMercuryAttenuators((*reinterpret_cast< std::add_pointer_t<QList<int>>>(_a[1]))); break;
        case 30: _t->dspProcessing(); break;
        case 31: _t->stop(); break;
        case 32: _t->setSystemState((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QSDR::_Error>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QSDR::_HWInterfaceMode>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QSDR::_ServerMode>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<QSDR::_DataEngineState>>(_a[5]))); break;
        case 33: _t->setFramesPerSecond((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 34: { bool _r = _t->initQtWDSPInterface();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 35: _t->setAGCMaximumGain_dB((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 36: _t->setAGCFixedGain_dB((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 37: _t->setAGCThreshold_dB((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 38: _t->setAGCHangLevel_dB((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 39: _t->setAGCHangThreshold((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 40: _t->setAGCSlope_dB((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 41: _t->setAGCAttackTime((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 42: _t->setAGCDecayTime((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 43: _t->setAGCHangTime((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< CPX >(); break;
            }
            break;
        case 4:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< CPX >(); break;
            }
            break;
        case 7:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QSDR::_ServerMode >(); break;
            }
            break;
        case 16:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 3:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< HamBand >(); break;
            }
            break;
        case 17:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 2:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< DSPMode >(); break;
            }
            break;
        case 18:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 2:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< ADCMode >(); break;
            }
            break;
        case 19:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 2:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< AGCMode >(); break;
            }
            break;
        case 25:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<long> >(); break;
            }
            break;
        case 26:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<long> >(); break;
            }
            break;
        case 29:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<int> >(); break;
            }
            break;
        case 32:
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
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (Receiver::*)(QString )>(_a, &Receiver::messageEvent, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (Receiver::*)(int , const qVectorFloat & )>(_a, &Receiver::spectrumBufferChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (Receiver::*)(int , double )>(_a, &Receiver::sMeterValueChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (Receiver::*)(int , const CPX & )>(_a, &Receiver::outputBufferSignal, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (Receiver::*)(int , const CPX & , int )>(_a, &Receiver::audioBufferSignal, 4))
            return;
    }
}

const QMetaObject *Receiver::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Receiver::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8ReceiverE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Receiver::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 44)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 44;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 44)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 44;
    }
    return _id;
}

// SIGNAL 0
void Receiver::messageEvent(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void Receiver::spectrumBufferChanged(int _t1, const qVectorFloat & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2);
}

// SIGNAL 2
void Receiver::sMeterValueChanged(int _t1, double _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void Receiver::outputBufferSignal(int _t1, const CPX & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}

// SIGNAL 4
void Receiver::audioBufferSignal(int _t1, const CPX & _t2, int _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2, _t3);
}
QT_WARNING_POP
