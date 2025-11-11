/****************************************************************************
** Meta object code from reading C++ file 'cusdr_oglWidebandPanel.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/GL/cusdr_oglWidebandPanel.h"
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cusdr_oglWidebandPanel.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN16QGLWidebandPanelE_t {};
} // unnamed namespace

template <> constexpr inline auto QGLWidebandPanel::qt_create_metaobjectdata<qt_meta_tag_ZN16QGLWidebandPanelE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "QGLWidebandPanel",
        "showEvent",
        "",
        "sender",
        "closeEvent",
        "messageEvent",
        "msg",
        "coordChanged",
        "x",
        "y",
        "minimumSizeHint",
        "sizeHint",
        "systemStateChanged",
        "QSDR::_Error",
        "err",
        "QSDR::_HWInterfaceMode",
        "hwmode",
        "QSDR::_ServerMode",
        "mode",
        "QSDR::_DataEngineState",
        "state",
        "graphicModeChanged",
        "rx",
        "PanGraphicsMode",
        "panMode",
        "WaterfallColorMode",
        "waterfallColorMode",
        "setupConnections",
        "setCurrentReceiver",
        "value",
        "setFrequency",
        "freq",
        "setupDisplayRegions",
        "size",
        "setWidebandSpectrumBuffer",
        "qVectorFloat",
        "buffer",
        "resetWidebandSpectrumBuffer",
        "setPanadapterColors",
        "setPanGridStatus",
        "setMercuryAttenuator",
        "HamBand",
        "band",
        "getRegion",
        "p",
        "sampleRateChanged"
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
        // Signal 'coordChanged'
        QtMocHelpers::SignalData<void(int, int)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 8 }, { QMetaType::Int, 9 },
        }}),
        // Slot 'minimumSizeHint'
        QtMocHelpers::SlotData<QSize() const>(10, 2, QMC::AccessPublic, QMetaType::QSize),
        // Slot 'sizeHint'
        QtMocHelpers::SlotData<QSize() const>(11, 2, QMC::AccessPublic, QMetaType::QSize),
        // Slot 'systemStateChanged'
        QtMocHelpers::SlotData<void(QObject *, QSDR::_Error, QSDR::_HWInterfaceMode, QSDR::_ServerMode, QSDR::_DataEngineState)>(12, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { 0x80000000 | 13, 14 }, { 0x80000000 | 15, 16 }, { 0x80000000 | 17, 18 },
            { 0x80000000 | 19, 20 },
        }}),
        // Slot 'graphicModeChanged'
        QtMocHelpers::SlotData<void(QObject *, int, PanGraphicsMode, WaterfallColorMode)>(21, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 22 }, { 0x80000000 | 23, 24 }, { 0x80000000 | 25, 26 },
        }}),
        // Slot 'setupConnections'
        QtMocHelpers::SlotData<void()>(27, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'setCurrentReceiver'
        QtMocHelpers::SlotData<void(QObject *, int)>(28, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 29 },
        }}),
        // Slot 'setFrequency'
        QtMocHelpers::SlotData<void(QObject *, int, int, long)>(30, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 18 }, { QMetaType::Int, 22 }, { QMetaType::Long, 31 },
        }}),
        // Slot 'setupDisplayRegions'
        QtMocHelpers::SlotData<void(QSize)>(32, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QSize, 33 },
        }}),
        // Slot 'setWidebandSpectrumBuffer'
        QtMocHelpers::SlotData<void(const qVectorFloat &)>(34, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 35, 36 },
        }}),
        // Slot 'resetWidebandSpectrumBuffer'
        QtMocHelpers::SlotData<void()>(37, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'setPanadapterColors'
        QtMocHelpers::SlotData<void()>(38, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'setPanGridStatus'
        QtMocHelpers::SlotData<void(bool, int)>(39, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 29 }, { QMetaType::Int, 22 },
        }}),
        // Slot 'setMercuryAttenuator'
        QtMocHelpers::SlotData<void(QObject *, HamBand, int)>(40, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { 0x80000000 | 41, 42 }, { QMetaType::Int, 29 },
        }}),
        // Slot 'getRegion'
        QtMocHelpers::SlotData<void(QPoint)>(43, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QPoint, 44 },
        }}),
        // Slot 'sampleRateChanged'
        QtMocHelpers::SlotData<void(QObject *, int)>(45, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 29 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<QGLWidebandPanel, qt_meta_tag_ZN16QGLWidebandPanelE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject QGLWidebandPanel::staticMetaObject = { {
    QMetaObject::SuperData::link<QOpenGLWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16QGLWidebandPanelE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16QGLWidebandPanelE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN16QGLWidebandPanelE_t>.metaTypes,
    nullptr
} };

void QGLWidebandPanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<QGLWidebandPanel *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->showEvent((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1]))); break;
        case 1: _t->closeEvent((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1]))); break;
        case 2: _t->messageEvent((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->coordChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 4: { QSize _r = _t->minimumSizeHint();
            if (_a[0]) *reinterpret_cast< QSize*>(_a[0]) = std::move(_r); }  break;
        case 5: { QSize _r = _t->sizeHint();
            if (_a[0]) *reinterpret_cast< QSize*>(_a[0]) = std::move(_r); }  break;
        case 6: _t->systemStateChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QSDR::_Error>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QSDR::_HWInterfaceMode>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QSDR::_ServerMode>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<QSDR::_DataEngineState>>(_a[5]))); break;
        case 7: _t->graphicModeChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<PanGraphicsMode>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<WaterfallColorMode>>(_a[4]))); break;
        case 8: _t->setupConnections(); break;
        case 9: _t->setCurrentReceiver((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 10: _t->setFrequency((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<long>>(_a[4]))); break;
        case 11: _t->setupDisplayRegions((*reinterpret_cast< std::add_pointer_t<QSize>>(_a[1]))); break;
        case 12: _t->setWidebandSpectrumBuffer((*reinterpret_cast< std::add_pointer_t<qVectorFloat>>(_a[1]))); break;
        case 13: _t->resetWidebandSpectrumBuffer(); break;
        case 14: _t->setPanadapterColors(); break;
        case 15: _t->setPanGridStatus((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 16: _t->setMercuryAttenuator((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<HamBand>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 17: _t->getRegion((*reinterpret_cast< std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 18: _t->sampleRateChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 6:
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
        case 16:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< HamBand >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (QGLWidebandPanel::*)(QObject * )>(_a, &QGLWidebandPanel::showEvent, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (QGLWidebandPanel::*)(QObject * )>(_a, &QGLWidebandPanel::closeEvent, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (QGLWidebandPanel::*)(QString )>(_a, &QGLWidebandPanel::messageEvent, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (QGLWidebandPanel::*)(int , int )>(_a, &QGLWidebandPanel::coordChanged, 3))
            return;
    }
}

const QMetaObject *QGLWidebandPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QGLWidebandPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16QGLWidebandPanelE_t>.strings))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "QOpenGLFunctions"))
        return static_cast< QOpenGLFunctions*>(this);
    return QOpenGLWidget::qt_metacast(_clname);
}

int QGLWidebandPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QOpenGLWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    }
    return _id;
}

// SIGNAL 0
void QGLWidebandPanel::showEvent(QObject * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void QGLWidebandPanel::closeEvent(QObject * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void QGLWidebandPanel::messageEvent(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void QGLWidebandPanel::coordChanged(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}
QT_WARNING_POP
