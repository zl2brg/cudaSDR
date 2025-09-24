/****************************************************************************
** Meta object code from reading C++ file 'cusdr_colorsWidget.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/cusdr_colorsWidget.h"
#include <QtGui/qtextcursor.h>
#include <QtGui/qscreen.h>
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cusdr_colorsWidget.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN18ColorOptionsWidgetE_t {};
} // unnamed namespace

template <> constexpr inline auto ColorOptionsWidget::qt_create_metaobjectdata<qt_meta_tag_ZN18ColorOptionsWidgetE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ColorOptionsWidget",
        "colorChanged",
        "",
        "color",
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
        "colorChooserChanged",
        "resetColors",
        "acceptColors",
        "triangleColorChanged",
        "sampleRateChanged",
        "value"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'colorChanged'
        QtMocHelpers::SignalData<void(const QColor &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QColor, 3 },
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
        // Slot 'colorChooserChanged'
        QtMocHelpers::SlotData<void()>(23, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'resetColors'
        QtMocHelpers::SlotData<void()>(24, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'acceptColors'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'triangleColorChanged'
        QtMocHelpers::SlotData<void(QColor)>(26, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QColor, 3 },
        }}),
        // Slot 'sampleRateChanged'
        QtMocHelpers::SlotData<void(QObject *, int)>(27, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 7 }, { QMetaType::Int, 28 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ColorOptionsWidget, qt_meta_tag_ZN18ColorOptionsWidgetE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ColorOptionsWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18ColorOptionsWidgetE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18ColorOptionsWidgetE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN18ColorOptionsWidgetE_t>.metaTypes,
    nullptr
} };

void ColorOptionsWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ColorOptionsWidget *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->colorChanged((*reinterpret_cast< std::add_pointer_t<QColor>>(_a[1]))); break;
        case 1: { QSize _r = _t->sizeHint();
            if (_a[0]) *reinterpret_cast< QSize*>(_a[0]) = std::move(_r); }  break;
        case 2: { QSize _r = _t->minimumSizeHint();
            if (_a[0]) *reinterpret_cast< QSize*>(_a[0]) = std::move(_r); }  break;
        case 3: _t->systemStateChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QSDR::_Error>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QSDR::_HWInterfaceMode>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QSDR::_ServerMode>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<QSDR::_DataEngineState>>(_a[5]))); break;
        case 4: _t->graphicModeChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<PanGraphicsMode>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<WaterfallColorMode>>(_a[4]))); break;
        case 5: _t->setCurrentReceiver((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 6: _t->colorChooserChanged(); break;
        case 7: _t->resetColors(); break;
        case 8: _t->acceptColors(); break;
        case 9: _t->triangleColorChanged((*reinterpret_cast< std::add_pointer_t<QColor>>(_a[1]))); break;
        case 10: _t->sampleRateChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
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
        if (QtMocHelpers::indexOfMethod<void (ColorOptionsWidget::*)(const QColor & )>(_a, &ColorOptionsWidget::colorChanged, 0))
            return;
    }
}

const QMetaObject *ColorOptionsWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ColorOptionsWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18ColorOptionsWidgetE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int ColorOptionsWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void ColorOptionsWidget::colorChanged(const QColor & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}
QT_WARNING_POP
