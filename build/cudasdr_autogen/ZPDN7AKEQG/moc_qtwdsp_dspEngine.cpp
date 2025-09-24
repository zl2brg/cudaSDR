/****************************************************************************
** Meta object code from reading C++ file 'qtwdsp_dspEngine.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../Source/src/QtWDSP/qtwdsp_dspEngine.h"
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qtwdsp_dspEngine.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN11QWDSPEngineE_t {};
} // unnamed namespace

template <> constexpr inline auto QWDSPEngine::qt_create_metaobjectdata<qt_meta_tag_ZN11QWDSPEngineE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "QWDSPEngine",
        "getQtDSPStatus",
        "",
        "setNCOFrequency",
        "rx",
        "value",
        "setSampleRate",
        "sender",
        "setSampleSize",
        "size",
        "setQtDSPStatus",
        "setVolume",
        "setDSPMode",
        "DSPMode",
        "mode",
        "setAGCMode",
        "AGCMode",
        "setFilter",
        "low",
        "high",
        "setAGCMaximumGain",
        "setAGCHangThreshold",
        "setAGCLineValues",
        "setAGCThreshold",
        "threshold",
        "setAGCHangTime",
        "hang",
        "setAGCHangLevel",
        "level",
        "setAGCAttackTime",
        "setAGCDecayTime",
        "setAGCSlope",
        "setFramesPerSecond",
        "setPanAdaptorAveragingMode",
        "setPanAdaptorDetectorMode",
        "setPanAdaptorAveragingCnt",
        "setfftSize",
        "setfmsqLevel",
        "setFilterMode",
        "setNoiseBlankerMode",
        "nb",
        "setNoiseFilterMode",
        "nr",
        "setNrAGC",
        "setNr2GainMethod",
        "setNr2NpeMethod",
        "setNr2Ae",
        "setanf",
        "setsnb",
        "set_txrx",
        "RadioState",
        "state"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'getQtDSPStatus'
        QtMocHelpers::SlotData<bool()>(1, 2, QMC::AccessPublic, QMetaType::Bool),
        // Slot 'setNCOFrequency'
        QtMocHelpers::SlotData<void(int, long)>(3, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 }, { QMetaType::Long, 5 },
        }}),
        // Slot 'setSampleRate'
        QtMocHelpers::SlotData<void(QObject *, int)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 7 }, { QMetaType::Int, 5 },
        }}),
        // Slot 'setSampleSize'
        QtMocHelpers::SlotData<void(int, int)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 }, { QMetaType::Int, 9 },
        }}),
        // Slot 'setQtDSPStatus'
        QtMocHelpers::SlotData<void(bool)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 5 },
        }}),
        // Slot 'setVolume'
        QtMocHelpers::SlotData<void(float)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 5 },
        }}),
        // Slot 'setDSPMode'
        QtMocHelpers::SlotData<void(DSPMode)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 13, 14 },
        }}),
        // Slot 'setAGCMode'
        QtMocHelpers::SlotData<void(AGCMode)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 16, 14 },
        }}),
        // Slot 'setFilter'
        QtMocHelpers::SlotData<void(double, double)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 18 }, { QMetaType::Double, 19 },
        }}),
        // Slot 'setAGCMaximumGain'
        QtMocHelpers::SlotData<void(qreal)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QReal, 2 },
        }}),
        // Slot 'setAGCHangThreshold'
        QtMocHelpers::SlotData<void(int, double)>(21, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 }, { QMetaType::Double, 2 },
        }}),
        // Slot 'setAGCLineValues'
        QtMocHelpers::SlotData<void(int)>(22, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 },
        }}),
        // Slot 'setAGCThreshold'
        QtMocHelpers::SlotData<void(double)>(23, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 24 },
        }}),
        // Slot 'setAGCHangTime'
        QtMocHelpers::SlotData<void(int)>(25, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 26 },
        }}),
        // Slot 'setAGCHangLevel'
        QtMocHelpers::SlotData<void(double)>(27, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 28 },
        }}),
        // Slot 'setAGCAttackTime'
        QtMocHelpers::SlotData<void(int, int)>(29, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 }, { QMetaType::Int, 5 },
        }}),
        // Slot 'setAGCDecayTime'
        QtMocHelpers::SlotData<void(int, int)>(30, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 }, { QMetaType::Int, 5 },
        }}),
        // Slot 'setAGCSlope'
        QtMocHelpers::SlotData<void(int, int)>(31, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 }, { QMetaType::Int, 5 },
        }}),
        // Slot 'setFramesPerSecond'
        QtMocHelpers::SlotData<void(QObject *, int, int)>(32, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 7 }, { QMetaType::Int, 4 }, { QMetaType::Int, 5 },
        }}),
        // Slot 'setPanAdaptorAveragingMode'
        QtMocHelpers::SlotData<void(int, int)>(33, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 }, { QMetaType::Int, 5 },
        }}),
        // Slot 'setPanAdaptorDetectorMode'
        QtMocHelpers::SlotData<void(int, int)>(34, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 }, { QMetaType::Int, 5 },
        }}),
        // Slot 'setPanAdaptorAveragingCnt'
        QtMocHelpers::SlotData<void(QObject *, int, int)>(35, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 2 }, { QMetaType::Int, 4 }, { QMetaType::Int, 5 },
        }}),
        // Slot 'setfftSize'
        QtMocHelpers::SlotData<void(int, int)>(36, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 }, { QMetaType::Int, 5 },
        }}),
        // Slot 'setfmsqLevel'
        QtMocHelpers::SlotData<void(int, int)>(37, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 }, { QMetaType::Int, 5 },
        }}),
        // Slot 'setFilterMode'
        QtMocHelpers::SlotData<void(int)>(38, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 },
        }}),
        // Slot 'setNoiseBlankerMode'
        QtMocHelpers::SlotData<void(int, int)>(39, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 }, { QMetaType::Int, 40 },
        }}),
        // Slot 'setNoiseFilterMode'
        QtMocHelpers::SlotData<void(int, int)>(41, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 }, { QMetaType::Int, 42 },
        }}),
        // Slot 'setNrAGC'
        QtMocHelpers::SlotData<void(int, int)>(43, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 }, { QMetaType::Int, 5 },
        }}),
        // Slot 'setNr2GainMethod'
        QtMocHelpers::SlotData<void(int, int)>(44, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 }, { QMetaType::Int, 5 },
        }}),
        // Slot 'setNr2NpeMethod'
        QtMocHelpers::SlotData<void(int, int)>(45, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 }, { QMetaType::Int, 5 },
        }}),
        // Slot 'setNr2Ae'
        QtMocHelpers::SlotData<void(int, bool)>(46, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 }, { QMetaType::Bool, 5 },
        }}),
        // Slot 'setanf'
        QtMocHelpers::SlotData<void(int, bool)>(47, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 }, { QMetaType::Bool, 5 },
        }}),
        // Slot 'setsnb'
        QtMocHelpers::SlotData<void(int, bool)>(48, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 }, { QMetaType::Bool, 5 },
        }}),
        // Slot 'set_txrx'
        QtMocHelpers::SlotData<void(RadioState)>(49, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 50, 51 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<QWDSPEngine, qt_meta_tag_ZN11QWDSPEngineE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject QWDSPEngine::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11QWDSPEngineE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11QWDSPEngineE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN11QWDSPEngineE_t>.metaTypes,
    nullptr
} };

void QWDSPEngine::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<QWDSPEngine *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: { bool _r = _t->getQtDSPStatus();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 1: _t->setNCOFrequency((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<long>>(_a[2]))); break;
        case 2: _t->setSampleRate((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 3: _t->setSampleSize((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 4: _t->setQtDSPStatus((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 5: _t->setVolume((*reinterpret_cast< std::add_pointer_t<float>>(_a[1]))); break;
        case 6: _t->setDSPMode((*reinterpret_cast< std::add_pointer_t<DSPMode>>(_a[1]))); break;
        case 7: _t->setAGCMode((*reinterpret_cast< std::add_pointer_t<AGCMode>>(_a[1]))); break;
        case 8: _t->setFilter((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 9: _t->setAGCMaximumGain((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1]))); break;
        case 10: _t->setAGCHangThreshold((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 11: _t->setAGCLineValues((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 12: _t->setAGCThreshold((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 13: _t->setAGCHangTime((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 14: _t->setAGCHangLevel((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 15: _t->setAGCAttackTime((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 16: _t->setAGCDecayTime((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 17: _t->setAGCSlope((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 18: _t->setFramesPerSecond((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 19: _t->setPanAdaptorAveragingMode((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 20: _t->setPanAdaptorDetectorMode((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 21: _t->setPanAdaptorAveragingCnt((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 22: _t->setfftSize((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 23: _t->setfmsqLevel((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 24: _t->setFilterMode((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 25: _t->setNoiseBlankerMode((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 26: _t->setNoiseFilterMode((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 27: _t->setNrAGC((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 28: _t->setNr2GainMethod((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 29: _t->setNr2NpeMethod((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 30: _t->setNr2Ae((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 31: _t->setanf((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 32: _t->setsnb((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 33: _t->set_txrx((*reinterpret_cast< std::add_pointer_t<RadioState>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 6:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< DSPMode >(); break;
            }
            break;
        case 7:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< AGCMode >(); break;
            }
            break;
        }
    }
}

const QMetaObject *QWDSPEngine::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QWDSPEngine::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11QWDSPEngineE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int QWDSPEngine::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 34)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 34;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 34)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 34;
    }
    return _id;
}
QT_WARNING_POP
