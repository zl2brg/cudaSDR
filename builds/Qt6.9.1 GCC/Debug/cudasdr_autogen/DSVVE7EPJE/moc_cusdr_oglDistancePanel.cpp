/****************************************************************************
** Meta object code from reading C++ file 'cusdr_oglDistancePanel.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../Source/src/GL/cusdr_oglDistancePanel.h"
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cusdr_oglDistancePanel.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN16QGLDistancePanelE_t {};
} // unnamed namespace

template <> constexpr inline auto QGLDistancePanel::qt_create_metaobjectdata<qt_meta_tag_ZN16QGLDistancePanelE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "QGLDistancePanel",
        "showEvent",
        "",
        "sender",
        "closeEvent",
        "messageEvent",
        "msg",
        "coordChanged",
        "x",
        "y",
        "sizeHint",
        "setSpectrumBuffer",
        "const float*",
        "buffer",
        "distanceSpectrumBufferChanged",
        "sampleRate",
        "length",
        "setFrequency",
        "value",
        "freq",
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
        "setFilterFrequencies",
        "lo",
        "hi",
        "setupDisplayRegions",
        "size",
        "setDistanceSpectrumBuffer",
        "setSpectrumAveraging",
        "setSpectrumAveragingCnt",
        "setPanGridStatus",
        "setPanadapterColors",
        "getRegion",
        "p",
        "freqRulerPositionChanged",
        "pos",
        "sampleRateChanged",
        "setChirpFFTShow"
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
        // Slot 'sizeHint'
        QtMocHelpers::SlotData<QSize() const>(10, 2, QMC::AccessPublic, QMetaType::QSize),
        // Slot 'setSpectrumBuffer'
        QtMocHelpers::SlotData<void(const float *)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 12, 13 },
        }}),
        // Slot 'distanceSpectrumBufferChanged'
        QtMocHelpers::SlotData<void(int, qint64, const float *)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 15 }, { QMetaType::LongLong, 16 }, { 0x80000000 | 12, 13 },
        }}),
        // Slot 'setFrequency'
        QtMocHelpers::SlotData<void(QObject *, bool, long)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Bool, 18 }, { QMetaType::Long, 19 },
        }}),
        // Slot 'systemStateChanged'
        QtMocHelpers::SlotData<void(QObject *, QSDR::_Error, QSDR::_HWInterfaceMode, QSDR::_ServerMode, QSDR::_DataEngineState)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { 0x80000000 | 21, 22 }, { 0x80000000 | 23, 24 }, { 0x80000000 | 25, 26 },
            { 0x80000000 | 27, 28 },
        }}),
        // Slot 'graphicModeChanged'
        QtMocHelpers::SlotData<void(QObject *, int, PanGraphicsMode, WaterfallColorMode)>(29, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 30 }, { 0x80000000 | 31, 32 }, { 0x80000000 | 33, 34 },
        }}),
        // Slot 'setupConnections'
        QtMocHelpers::SlotData<void()>(35, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'setFilterFrequencies'
        QtMocHelpers::SlotData<void(QObject *, int, qreal, qreal)>(36, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 30 }, { QMetaType::QReal, 37 }, { QMetaType::QReal, 38 },
        }}),
        // Slot 'setupDisplayRegions'
        QtMocHelpers::SlotData<void(QSize)>(39, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QSize, 40 },
        }}),
        // Slot 'setDistanceSpectrumBuffer'
        QtMocHelpers::SlotData<void(int, qint64, const float *)>(41, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 15 }, { QMetaType::LongLong, 16 }, { 0x80000000 | 12, 13 },
        }}),
        // Slot 'setSpectrumAveraging'
        QtMocHelpers::SlotData<void(bool)>(42, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 18 },
        }}),
        // Slot 'setSpectrumAveragingCnt'
        QtMocHelpers::SlotData<void(int)>(43, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 18 },
        }}),
        // Slot 'setPanGridStatus'
        QtMocHelpers::SlotData<void(bool)>(44, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 18 },
        }}),
        // Slot 'setPanadapterColors'
        QtMocHelpers::SlotData<void()>(45, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'getRegion'
        QtMocHelpers::SlotData<void(QPoint)>(46, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QPoint, 47 },
        }}),
        // Slot 'freqRulerPositionChanged'
        QtMocHelpers::SlotData<void(float, int)>(48, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Float, 49 }, { QMetaType::Int, 30 },
        }}),
        // Slot 'sampleRateChanged'
        QtMocHelpers::SlotData<void(QObject *, int)>(50, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 18 },
        }}),
        // Slot 'setChirpFFTShow'
        QtMocHelpers::SlotData<void(bool)>(51, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 18 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<QGLDistancePanel, qt_meta_tag_ZN16QGLDistancePanelE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject QGLDistancePanel::staticMetaObject = { {
    QMetaObject::SuperData::link<QOpenGLWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16QGLDistancePanelE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16QGLDistancePanelE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN16QGLDistancePanelE_t>.metaTypes,
    nullptr
} };

void QGLDistancePanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<QGLDistancePanel *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->showEvent((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1]))); break;
        case 1: _t->closeEvent((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1]))); break;
        case 2: _t->messageEvent((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->coordChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 4: { QSize _r = _t->sizeHint();
            if (_a[0]) *reinterpret_cast< QSize*>(_a[0]) = std::move(_r); }  break;
        case 5: _t->setSpectrumBuffer((*reinterpret_cast< std::add_pointer_t<const float*>>(_a[1]))); break;
        case 6: _t->distanceSpectrumBufferChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qint64>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<const float*>>(_a[3]))); break;
        case 7: _t->setFrequency((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<long>>(_a[3]))); break;
        case 8: _t->systemStateChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QSDR::_Error>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QSDR::_HWInterfaceMode>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QSDR::_ServerMode>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<QSDR::_DataEngineState>>(_a[5]))); break;
        case 9: _t->graphicModeChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<PanGraphicsMode>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<WaterfallColorMode>>(_a[4]))); break;
        case 10: _t->setupConnections(); break;
        case 11: _t->setFilterFrequencies((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[4]))); break;
        case 12: _t->setupDisplayRegions((*reinterpret_cast< std::add_pointer_t<QSize>>(_a[1]))); break;
        case 13: _t->setDistanceSpectrumBuffer((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qint64>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<const float*>>(_a[3]))); break;
        case 14: _t->setSpectrumAveraging((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 15: _t->setSpectrumAveragingCnt((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 16: _t->setPanGridStatus((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 17: _t->setPanadapterColors(); break;
        case 18: _t->getRegion((*reinterpret_cast< std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 19: _t->freqRulerPositionChanged((*reinterpret_cast< std::add_pointer_t<float>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 20: _t->sampleRateChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 21: _t->setChirpFFTShow((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 8:
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
        if (QtMocHelpers::indexOfMethod<void (QGLDistancePanel::*)(QObject * )>(_a, &QGLDistancePanel::showEvent, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (QGLDistancePanel::*)(QObject * )>(_a, &QGLDistancePanel::closeEvent, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (QGLDistancePanel::*)(QString )>(_a, &QGLDistancePanel::messageEvent, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (QGLDistancePanel::*)(int , int )>(_a, &QGLDistancePanel::coordChanged, 3))
            return;
    }
}

const QMetaObject *QGLDistancePanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QGLDistancePanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16QGLDistancePanelE_t>.strings))
        return static_cast<void*>(this);
    return QOpenGLWidget::qt_metacast(_clname);
}

int QGLDistancePanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QOpenGLWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 22)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 22;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 22)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 22;
    }
    return _id;
}

// SIGNAL 0
void QGLDistancePanel::showEvent(QObject * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void QGLDistancePanel::closeEvent(QObject * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void QGLDistancePanel::messageEvent(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void QGLDistancePanel::coordChanged(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}
QT_WARNING_POP
