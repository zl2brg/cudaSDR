/****************************************************************************
** Meta object code from reading C++ file 'cusdr_displayWidget.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/cusdr_displayWidget.h"
#include <QtGui/qtextcursor.h>
#include <QtGui/qscreen.h>
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cusdr_displayWidget.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN20DisplayOptionsWidgetE_t {};
} // unnamed namespace

template <> constexpr inline auto DisplayOptionsWidget::qt_create_metaobjectdata<qt_meta_tag_ZN20DisplayOptionsWidgetE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "DisplayOptionsWidget",
        "averagingModeChanged",
        "",
        "value",
        "sizeHint",
        "minimumSizeHint",
        "systemStateChanged",
        "sender",
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
        "setCurrentReceiver",
        "setFramesPerSecond",
        "panModeChanged",
        "wbPanModeChanged",
        "waterfallColorChanged",
        "sMeterChanged",
        "waterfallTimeChanged",
        "waterfallLoOffsetChanged",
        "waterfallHiOffsetChanged",
        "sMeterHoldTimeChanged",
        "fpsValueChanged",
        "averagingFilterCntChanged",
        "setWidebandAveragingCnt",
        "sampleRateChanged",
        "callSignTextChanged",
        "text",
        "callSignChanged",
        "panAverageModeChanged",
        "panDetectorModeChanged",
        "fftSizeChanged",
        "sqLevelChanged"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'averagingModeChanged'
        QtMocHelpers::SignalData<void(bool)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 3 },
        }}),
        // Slot 'sizeHint'
        QtMocHelpers::SlotData<QSize() const>(4, 2, QMC::AccessPublic, QMetaType::QSize),
        // Slot 'minimumSizeHint'
        QtMocHelpers::SlotData<QSize() const>(5, 2, QMC::AccessPublic, QMetaType::QSize),
        // Slot 'systemStateChanged'
        QtMocHelpers::SlotData<void(QObject *, QSDR::_Error, QSDR::_HWInterfaceMode, QSDR::_ServerMode, QSDR::_DataEngineState)>(6, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 7 }, { 0x80000000 | 8, 9 }, { 0x80000000 | 10, 11 }, { 0x80000000 | 12, 13 },
            { 0x80000000 | 14, 15 },
        }}),
        // Slot 'graphicModeChanged'
        QtMocHelpers::SlotData<void(QObject *, int, PanGraphicsMode, WaterfallColorMode)>(16, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 7 }, { QMetaType::Int, 17 }, { 0x80000000 | 18, 19 }, { 0x80000000 | 20, 21 },
        }}),
        // Slot 'setCurrentReceiver'
        QtMocHelpers::SlotData<void(QObject *, int)>(22, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 7 }, { QMetaType::Int, 17 },
        }}),
        // Slot 'setFramesPerSecond'
        QtMocHelpers::SlotData<void(QObject *, int, int)>(23, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 7 }, { QMetaType::Int, 17 }, { QMetaType::Int, 3 },
        }}),
        // Slot 'panModeChanged'
        QtMocHelpers::SlotData<void()>(24, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'wbPanModeChanged'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'waterfallColorChanged'
        QtMocHelpers::SlotData<void()>(26, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'sMeterChanged'
        QtMocHelpers::SlotData<void()>(27, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'waterfallTimeChanged'
        QtMocHelpers::SlotData<void(int)>(28, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Slot 'waterfallLoOffsetChanged'
        QtMocHelpers::SlotData<void(int)>(29, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Slot 'waterfallHiOffsetChanged'
        QtMocHelpers::SlotData<void(int)>(30, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Slot 'sMeterHoldTimeChanged'
        QtMocHelpers::SlotData<void(int)>(31, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Slot 'fpsValueChanged'
        QtMocHelpers::SlotData<void(int)>(32, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Slot 'averagingFilterCntChanged'
        QtMocHelpers::SlotData<void(int)>(33, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Slot 'setWidebandAveragingCnt'
        QtMocHelpers::SlotData<void(int)>(34, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Slot 'sampleRateChanged'
        QtMocHelpers::SlotData<void(QObject *, int)>(35, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 7 }, { QMetaType::Int, 3 },
        }}),
        // Slot 'callSignTextChanged'
        QtMocHelpers::SlotData<void(const QString &)>(36, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 37 },
        }}),
        // Slot 'callSignChanged'
        QtMocHelpers::SlotData<void()>(38, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'panAverageModeChanged'
        QtMocHelpers::SlotData<void(int)>(39, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Slot 'panDetectorModeChanged'
        QtMocHelpers::SlotData<void(int)>(40, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Slot 'fftSizeChanged'
        QtMocHelpers::SlotData<void(int)>(41, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Slot 'sqLevelChanged'
        QtMocHelpers::SlotData<void(int)>(42, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 2 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<DisplayOptionsWidget, qt_meta_tag_ZN20DisplayOptionsWidgetE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject DisplayOptionsWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN20DisplayOptionsWidgetE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN20DisplayOptionsWidgetE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN20DisplayOptionsWidgetE_t>.metaTypes,
    nullptr
} };

void DisplayOptionsWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DisplayOptionsWidget *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->averagingModeChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 1: { QSize _r = _t->sizeHint();
            if (_a[0]) *reinterpret_cast< QSize*>(_a[0]) = std::move(_r); }  break;
        case 2: { QSize _r = _t->minimumSizeHint();
            if (_a[0]) *reinterpret_cast< QSize*>(_a[0]) = std::move(_r); }  break;
        case 3: _t->systemStateChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QSDR::_Error>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QSDR::_HWInterfaceMode>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QSDR::_ServerMode>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<QSDR::_DataEngineState>>(_a[5]))); break;
        case 4: _t->graphicModeChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<PanGraphicsMode>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<WaterfallColorMode>>(_a[4]))); break;
        case 5: _t->setCurrentReceiver((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 6: _t->setFramesPerSecond((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 7: _t->panModeChanged(); break;
        case 8: _t->wbPanModeChanged(); break;
        case 9: _t->waterfallColorChanged(); break;
        case 10: _t->sMeterChanged(); break;
        case 11: _t->waterfallTimeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 12: _t->waterfallLoOffsetChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 13: _t->waterfallHiOffsetChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 14: _t->sMeterHoldTimeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 15: _t->fpsValueChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 16: _t->averagingFilterCntChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 17: _t->setWidebandAveragingCnt((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 18: _t->sampleRateChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 19: _t->callSignTextChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 20: _t->callSignChanged(); break;
        case 21: _t->panAverageModeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 22: _t->panDetectorModeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 23: _t->fftSizeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 24: _t->sqLevelChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
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
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (DisplayOptionsWidget::*)(bool )>(_a, &DisplayOptionsWidget::averagingModeChanged, 0))
            return;
    }
}

const QMetaObject *DisplayOptionsWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DisplayOptionsWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN20DisplayOptionsWidgetE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int DisplayOptionsWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 25)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 25;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 25)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 25;
    }
    return _id;
}

// SIGNAL 0
void DisplayOptionsWidget::averagingModeChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}
QT_WARNING_POP
