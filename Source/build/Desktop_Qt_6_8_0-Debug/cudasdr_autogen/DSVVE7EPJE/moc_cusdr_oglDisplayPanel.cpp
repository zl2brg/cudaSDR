/****************************************************************************
** Meta object code from reading C++ file 'cusdr_oglDisplayPanel.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/GL/cusdr_oglDisplayPanel.h"
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cusdr_oglDisplayPanel.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN15OGLDisplayPanelE_t {};
} // unnamed namespace

template <> constexpr inline auto OGLDisplayPanel::qt_create_metaobjectdata<qt_meta_tag_ZN15OGLDisplayPanelE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "OGLDisplayPanel",
        "showEvent",
        "",
        "sender",
        "closeEvent",
        "messageEvent",
        "msg",
        "setSampleRate",
        "value",
        "setFrequency",
        "mode",
        "rx",
        "freq",
        "systemStateChanged",
        "QSDR::_Error",
        "err",
        "QSDR::_HWInterfaceMode",
        "hwmode",
        "QSDR::_ServerMode",
        "QSDR::_DataEngineState",
        "state",
        "setupDisplayRegions",
        "size",
        "setSyncStatus",
        "setADCStatus",
        "setPacketLossStatus",
        "setSendIQStatus",
        "setRecvAudioStatus",
        "setCurrentReceiver",
        "setMercuryAttenuator",
        "HamBand",
        "band",
        "setReceivers",
        "setDither",
        "setRandom",
        "set10mhzSource",
        "set122_88mhzSource",
        "setMercuryPresence",
        "setPenelopePresence",
        "setPennylanePresence",
        "setAlexPresence",
        "setExcaliburPresence",
        "setHermesVersion",
        "setMercuryVersion",
        "setPenelopeVersion",
        "setPennylaneVersion",
        "setMetisVersion",
        "setExcaliburVersion",
        "setAlexVersion",
        "setMouseWheelFreqStep",
        "setSMeterValue",
        "setSMeterHoldTime",
        "updateSyncStatus",
        "updateADCStatus",
        "updatePacketLossStatus"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'showEvent'
        QtMocHelpers::SignalData<void(QObject *)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 },
        }}),
        // Signal 'closeEvent'
        QtMocHelpers::SignalData<void(QObject *)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 },
        }}),
        // Signal 'messageEvent'
        QtMocHelpers::SignalData<void(QString)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 },
        }}),
        // Slot 'setSampleRate'
        QtMocHelpers::SlotData<void(QObject *, int)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 8 },
        }}),
        // Slot 'setFrequency'
        QtMocHelpers::SlotData<void(QObject *, int, int, long)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 10 }, { QMetaType::Int, 11 }, { QMetaType::Long, 12 },
        }}),
        // Slot 'systemStateChanged'
        QtMocHelpers::SlotData<void(QObject *, QSDR::_Error, QSDR::_HWInterfaceMode, QSDR::_ServerMode, QSDR::_DataEngineState)>(13, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { 0x80000000 | 14, 15 }, { 0x80000000 | 16, 17 }, { 0x80000000 | 18, 10 },
            { 0x80000000 | 19, 20 },
        }}),
        // Slot 'setupDisplayRegions'
        QtMocHelpers::SlotData<void(QSize)>(21, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QSize, 22 },
        }}),
        // Slot 'setSyncStatus'
        QtMocHelpers::SlotData<void(int)>(23, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 8 },
        }}),
        // Slot 'setADCStatus'
        QtMocHelpers::SlotData<void(int)>(24, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 8 },
        }}),
        // Slot 'setPacketLossStatus'
        QtMocHelpers::SlotData<void(int)>(25, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 8 },
        }}),
        // Slot 'setSendIQStatus'
        QtMocHelpers::SlotData<void(int)>(26, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 8 },
        }}),
        // Slot 'setRecvAudioStatus'
        QtMocHelpers::SlotData<void(int)>(27, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 8 },
        }}),
        // Slot 'setCurrentReceiver'
        QtMocHelpers::SlotData<void(QObject *, int)>(28, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 8 },
        }}),
        // Slot 'setMercuryAttenuator'
        QtMocHelpers::SlotData<void(QObject *, HamBand, int)>(29, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { 0x80000000 | 30, 31 }, { QMetaType::Int, 8 },
        }}),
        // Slot 'setReceivers'
        QtMocHelpers::SlotData<void(QObject *, int)>(32, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 8 },
        }}),
        // Slot 'setDither'
        QtMocHelpers::SlotData<void(QObject *, int)>(33, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 8 },
        }}),
        // Slot 'setRandom'
        QtMocHelpers::SlotData<void(QObject *, int)>(34, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 8 },
        }}),
        // Slot 'set10mhzSource'
        QtMocHelpers::SlotData<void(QObject *, int)>(35, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 8 },
        }}),
        // Slot 'set122_88mhzSource'
        QtMocHelpers::SlotData<void(QObject *, int)>(36, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 8 },
        }}),
        // Slot 'setMercuryPresence'
        QtMocHelpers::SlotData<void(bool)>(37, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 8 },
        }}),
        // Slot 'setPenelopePresence'
        QtMocHelpers::SlotData<void(bool)>(38, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 8 },
        }}),
        // Slot 'setPennylanePresence'
        QtMocHelpers::SlotData<void(bool)>(39, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 8 },
        }}),
        // Slot 'setAlexPresence'
        QtMocHelpers::SlotData<void(bool)>(40, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 8 },
        }}),
        // Slot 'setExcaliburPresence'
        QtMocHelpers::SlotData<void(bool)>(41, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 8 },
        }}),
        // Slot 'setHermesVersion'
        QtMocHelpers::SlotData<void(int)>(42, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 8 },
        }}),
        // Slot 'setMercuryVersion'
        QtMocHelpers::SlotData<void(int)>(43, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 8 },
        }}),
        // Slot 'setPenelopeVersion'
        QtMocHelpers::SlotData<void(int)>(44, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 8 },
        }}),
        // Slot 'setPennylaneVersion'
        QtMocHelpers::SlotData<void(int)>(45, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 8 },
        }}),
        // Slot 'setMetisVersion'
        QtMocHelpers::SlotData<void(int)>(46, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 8 },
        }}),
        // Slot 'setExcaliburVersion'
        QtMocHelpers::SlotData<void(QObject *, int)>(47, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 8 },
        }}),
        // Slot 'setAlexVersion'
        QtMocHelpers::SlotData<void(QObject *, int)>(48, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 8 },
        }}),
        // Slot 'setMouseWheelFreqStep'
        QtMocHelpers::SlotData<void(QObject *, int, qreal)>(49, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 11 }, { QMetaType::QReal, 8 },
        }}),
        // Slot 'setSMeterValue'
        QtMocHelpers::SlotData<void(int, double)>(50, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 11 }, { QMetaType::Double, 8 },
        }}),
        // Slot 'setSMeterHoldTime'
        QtMocHelpers::SlotData<void(int)>(51, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 8 },
        }}),
        // Slot 'updateSyncStatus'
        QtMocHelpers::SlotData<void()>(52, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'updateADCStatus'
        QtMocHelpers::SlotData<void()>(53, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'updatePacketLossStatus'
        QtMocHelpers::SlotData<void()>(54, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<OGLDisplayPanel, qt_meta_tag_ZN15OGLDisplayPanelE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject OGLDisplayPanel::staticMetaObject = { {
    QMetaObject::SuperData::link<QOpenGLWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15OGLDisplayPanelE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15OGLDisplayPanelE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15OGLDisplayPanelE_t>.metaTypes,
    nullptr
} };

void OGLDisplayPanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<OGLDisplayPanel *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->showEvent((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1]))); break;
        case 1: _t->closeEvent((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1]))); break;
        case 2: _t->messageEvent((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->setSampleRate((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 4: _t->setFrequency((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<long>>(_a[4]))); break;
        case 5: _t->systemStateChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QSDR::_Error>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QSDR::_HWInterfaceMode>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QSDR::_ServerMode>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<QSDR::_DataEngineState>>(_a[5]))); break;
        case 6: _t->setupDisplayRegions((*reinterpret_cast< std::add_pointer_t<QSize>>(_a[1]))); break;
        case 7: _t->setSyncStatus((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 8: _t->setADCStatus((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 9: _t->setPacketLossStatus((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 10: _t->setSendIQStatus((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 11: _t->setRecvAudioStatus((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 12: _t->setCurrentReceiver((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 13: _t->setMercuryAttenuator((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<HamBand>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 14: _t->setReceivers((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 15: _t->setDither((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 16: _t->setRandom((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 17: _t->set10mhzSource((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 18: _t->set122_88mhzSource((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 19: _t->setMercuryPresence((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 20: _t->setPenelopePresence((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 21: _t->setPennylanePresence((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 22: _t->setAlexPresence((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 23: _t->setExcaliburPresence((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 24: _t->setHermesVersion((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 25: _t->setMercuryVersion((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 26: _t->setPenelopeVersion((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 27: _t->setPennylaneVersion((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 28: _t->setMetisVersion((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 29: _t->setExcaliburVersion((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 30: _t->setAlexVersion((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 31: _t->setMouseWheelFreqStep((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 32: _t->setSMeterValue((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 33: _t->setSMeterHoldTime((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 34: _t->updateSyncStatus(); break;
        case 35: _t->updateADCStatus(); break;
        case 36: _t->updatePacketLossStatus(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 5:
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
        case 13:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< HamBand >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (OGLDisplayPanel::*)(QObject * )>(_a, &OGLDisplayPanel::showEvent, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (OGLDisplayPanel::*)(QObject * )>(_a, &OGLDisplayPanel::closeEvent, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (OGLDisplayPanel::*)(QString )>(_a, &OGLDisplayPanel::messageEvent, 2))
            return;
    }
}

const QMetaObject *OGLDisplayPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *OGLDisplayPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15OGLDisplayPanelE_t>.strings))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "QOpenGLFunctions"))
        return static_cast< QOpenGLFunctions*>(this);
    return QOpenGLWidget::qt_metacast(_clname);
}

int OGLDisplayPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QOpenGLWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 37)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 37;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 37)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 37;
    }
    return _id;
}

// SIGNAL 0
void OGLDisplayPanel::showEvent(QObject * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void OGLDisplayPanel::closeEvent(QObject * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void OGLDisplayPanel::messageEvent(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}
QT_WARNING_POP
