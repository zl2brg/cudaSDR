/****************************************************************************
** Meta object code from reading C++ file 'cusdr_3DOptionsWidget.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/cusdr_3DOptionsWidget.h"
#include <QtGui/qtextcursor.h>
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#include <QtNetwork/QSslError>
#include <QtGui/qscreen.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cusdr_3DOptionsWidget.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN15Options3DWidgetE_t {};
} // unnamed namespace

template <> constexpr inline auto Options3DWidget::qt_create_metaobjectdata<qt_meta_tag_ZN15Options3DWidgetE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Options3DWidget",
        "show3DPanadapterChanged",
        "",
        "enabled",
        "heightScaleValueChanged",
        "scale",
        "frequencyScaleValueChanged",
        "timeScaleValueChanged",
        "updateIntervalValueChanged",
        "interval",
        "showGridValueChanged",
        "show",
        "showAxesValueChanged",
        "wireframeModeValueChanged",
        "wireframe",
        "waterfallOffsetValueChanged",
        "offset",
        "enable3DChanged",
        "heightScaleChanged",
        "value",
        "frequencyScaleChanged",
        "timeScaleChanged",
        "updateIntervalChanged",
        "showGridChanged",
        "showAxesChanged",
        "wireframeModeChanged",
        "waterfallOffsetChanged"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'show3DPanadapterChanged'
        QtMocHelpers::SignalData<void(bool)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 3 },
        }}),
        // Signal 'heightScaleValueChanged'
        QtMocHelpers::SignalData<void(float)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 5 },
        }}),
        // Signal 'frequencyScaleValueChanged'
        QtMocHelpers::SignalData<void(float)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 5 },
        }}),
        // Signal 'timeScaleValueChanged'
        QtMocHelpers::SignalData<void(float)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 5 },
        }}),
        // Signal 'updateIntervalValueChanged'
        QtMocHelpers::SignalData<void(int)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 9 },
        }}),
        // Signal 'showGridValueChanged'
        QtMocHelpers::SignalData<void(bool)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 11 },
        }}),
        // Signal 'showAxesValueChanged'
        QtMocHelpers::SignalData<void(bool)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 11 },
        }}),
        // Signal 'wireframeModeValueChanged'
        QtMocHelpers::SignalData<void(bool)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 14 },
        }}),
        // Signal 'waterfallOffsetValueChanged'
        QtMocHelpers::SignalData<void(float)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 16 },
        }}),
        // Slot 'enable3DChanged'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'heightScaleChanged'
        QtMocHelpers::SlotData<void(int)>(18, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 19 },
        }}),
        // Slot 'frequencyScaleChanged'
        QtMocHelpers::SlotData<void(int)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 19 },
        }}),
        // Slot 'timeScaleChanged'
        QtMocHelpers::SlotData<void(int)>(21, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 19 },
        }}),
        // Slot 'updateIntervalChanged'
        QtMocHelpers::SlotData<void(int)>(22, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 19 },
        }}),
        // Slot 'showGridChanged'
        QtMocHelpers::SlotData<void()>(23, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'showAxesChanged'
        QtMocHelpers::SlotData<void()>(24, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'wireframeModeChanged'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'waterfallOffsetChanged'
        QtMocHelpers::SlotData<void(int)>(26, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 19 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<Options3DWidget, qt_meta_tag_ZN15Options3DWidgetE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Options3DWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15Options3DWidgetE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15Options3DWidgetE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15Options3DWidgetE_t>.metaTypes,
    nullptr
} };

void Options3DWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<Options3DWidget *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->show3DPanadapterChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 1: _t->heightScaleValueChanged((*reinterpret_cast< std::add_pointer_t<float>>(_a[1]))); break;
        case 2: _t->frequencyScaleValueChanged((*reinterpret_cast< std::add_pointer_t<float>>(_a[1]))); break;
        case 3: _t->timeScaleValueChanged((*reinterpret_cast< std::add_pointer_t<float>>(_a[1]))); break;
        case 4: _t->updateIntervalValueChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->showGridValueChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 6: _t->showAxesValueChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 7: _t->wireframeModeValueChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 8: _t->waterfallOffsetValueChanged((*reinterpret_cast< std::add_pointer_t<float>>(_a[1]))); break;
        case 9: _t->enable3DChanged(); break;
        case 10: _t->heightScaleChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 11: _t->frequencyScaleChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 12: _t->timeScaleChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 13: _t->updateIntervalChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 14: _t->showGridChanged(); break;
        case 15: _t->showAxesChanged(); break;
        case 16: _t->wireframeModeChanged(); break;
        case 17: _t->waterfallOffsetChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (Options3DWidget::*)(bool )>(_a, &Options3DWidget::show3DPanadapterChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (Options3DWidget::*)(float )>(_a, &Options3DWidget::heightScaleValueChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (Options3DWidget::*)(float )>(_a, &Options3DWidget::frequencyScaleValueChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (Options3DWidget::*)(float )>(_a, &Options3DWidget::timeScaleValueChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (Options3DWidget::*)(int )>(_a, &Options3DWidget::updateIntervalValueChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (Options3DWidget::*)(bool )>(_a, &Options3DWidget::showGridValueChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (Options3DWidget::*)(bool )>(_a, &Options3DWidget::showAxesValueChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (Options3DWidget::*)(bool )>(_a, &Options3DWidget::wireframeModeValueChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (Options3DWidget::*)(float )>(_a, &Options3DWidget::waterfallOffsetValueChanged, 8))
            return;
    }
}

const QMetaObject *Options3DWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Options3DWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15Options3DWidgetE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Options3DWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 18)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 18;
    }
    return _id;
}

// SIGNAL 0
void Options3DWidget::show3DPanadapterChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void Options3DWidget::heightScaleValueChanged(float _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void Options3DWidget::frequencyScaleValueChanged(float _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void Options3DWidget::timeScaleValueChanged(float _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void Options3DWidget::updateIntervalValueChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void Options3DWidget::showGridValueChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void Options3DWidget::showAxesValueChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}

// SIGNAL 7
void Options3DWidget::wireframeModeValueChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1);
}

// SIGNAL 8
void Options3DWidget::waterfallOffsetValueChanged(float _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1);
}
QT_WARNING_POP
