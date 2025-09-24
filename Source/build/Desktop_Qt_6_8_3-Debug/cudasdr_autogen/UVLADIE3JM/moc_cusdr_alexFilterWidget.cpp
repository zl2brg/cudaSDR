/****************************************************************************
** Meta object code from reading C++ file 'cusdr_alexFilterWidget.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/cusdr_alexFilterWidget.h"
#include <QtGui/qtextcursor.h>
#include <QtGui/qscreen.h>
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cusdr_alexFilterWidget.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN16AlexFilterWidgetE_t {};
} // unnamed namespace

template <> constexpr inline auto AlexFilterWidget::qt_create_metaobjectdata<qt_meta_tag_ZN16AlexFilterWidgetE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "AlexFilterWidget",
        "showEvent",
        "",
        "sender",
        "closeEvent",
        "messageEvent",
        "alexManualStateChanged",
        "value",
        "hpfLoSpinBoxValueChanged",
        "hpfHiSpinBoxValueChanged",
        "lpfLoSpinBoxValueChanged",
        "lpfHiSpinBoxValueChanged",
        "setFrequency",
        "mode",
        "rx",
        "frequency",
        "setCurrentReceiver",
        "setAlexConfiguration",
        "manualFilterBtnClicked",
        "defaultValuesBtnClicked",
        "bypassAllHPFBtnClicked",
        "lowNoise6mAmpBtnClicked",
        "hpf13MHzBtnClicked",
        "hpf20MHzBtnClicked",
        "hpf9_5MHzBtnClicked",
        "hpf6_5MHzBtnClicked",
        "hpf1_5MHzBtnClicked"
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
            { QMetaType::QString, 2 },
        }}),
        // Slot 'alexManualStateChanged'
        QtMocHelpers::SlotData<void(QObject *, bool)>(6, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Bool, 7 },
        }}),
        // Slot 'hpfLoSpinBoxValueChanged'
        QtMocHelpers::SlotData<void(double)>(8, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 7 },
        }}),
        // Slot 'hpfHiSpinBoxValueChanged'
        QtMocHelpers::SlotData<void(double)>(9, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 7 },
        }}),
        // Slot 'lpfLoSpinBoxValueChanged'
        QtMocHelpers::SlotData<void(double)>(10, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 7 },
        }}),
        // Slot 'lpfHiSpinBoxValueChanged'
        QtMocHelpers::SlotData<void(double)>(11, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 7 },
        }}),
        // Slot 'setFrequency'
        QtMocHelpers::SlotData<void(QObject *, int, int, long)>(12, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 13 }, { QMetaType::Int, 14 }, { QMetaType::Long, 15 },
        }}),
        // Slot 'setCurrentReceiver'
        QtMocHelpers::SlotData<void(QObject *, int)>(16, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 14 },
        }}),
        // Slot 'setAlexConfiguration'
        QtMocHelpers::SlotData<void(double)>(17, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 15 },
        }}),
        // Slot 'manualFilterBtnClicked'
        QtMocHelpers::SlotData<void()>(18, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'defaultValuesBtnClicked'
        QtMocHelpers::SlotData<void()>(19, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'bypassAllHPFBtnClicked'
        QtMocHelpers::SlotData<void()>(20, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'lowNoise6mAmpBtnClicked'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'hpf13MHzBtnClicked'
        QtMocHelpers::SlotData<void()>(22, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'hpf20MHzBtnClicked'
        QtMocHelpers::SlotData<void()>(23, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'hpf9_5MHzBtnClicked'
        QtMocHelpers::SlotData<void()>(24, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'hpf6_5MHzBtnClicked'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'hpf1_5MHzBtnClicked'
        QtMocHelpers::SlotData<void()>(26, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AlexFilterWidget, qt_meta_tag_ZN16AlexFilterWidgetE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject AlexFilterWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16AlexFilterWidgetE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16AlexFilterWidgetE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN16AlexFilterWidgetE_t>.metaTypes,
    nullptr
} };

void AlexFilterWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AlexFilterWidget *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->showEvent((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1]))); break;
        case 1: _t->closeEvent((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1]))); break;
        case 2: _t->messageEvent((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->alexManualStateChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 4: _t->hpfLoSpinBoxValueChanged((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 5: _t->hpfHiSpinBoxValueChanged((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 6: _t->lpfLoSpinBoxValueChanged((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 7: _t->lpfHiSpinBoxValueChanged((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 8: _t->setFrequency((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<long>>(_a[4]))); break;
        case 9: _t->setCurrentReceiver((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 10: _t->setAlexConfiguration((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 11: _t->manualFilterBtnClicked(); break;
        case 12: _t->defaultValuesBtnClicked(); break;
        case 13: _t->bypassAllHPFBtnClicked(); break;
        case 14: _t->lowNoise6mAmpBtnClicked(); break;
        case 15: _t->hpf13MHzBtnClicked(); break;
        case 16: _t->hpf20MHzBtnClicked(); break;
        case 17: _t->hpf9_5MHzBtnClicked(); break;
        case 18: _t->hpf6_5MHzBtnClicked(); break;
        case 19: _t->hpf1_5MHzBtnClicked(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AlexFilterWidget::*)(QObject * )>(_a, &AlexFilterWidget::showEvent, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (AlexFilterWidget::*)(QObject * )>(_a, &AlexFilterWidget::closeEvent, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (AlexFilterWidget::*)(QString )>(_a, &AlexFilterWidget::messageEvent, 2))
            return;
    }
}

const QMetaObject *AlexFilterWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AlexFilterWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16AlexFilterWidgetE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int AlexFilterWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 20)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 20;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 20)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 20;
    }
    return _id;
}

// SIGNAL 0
void AlexFilterWidget::showEvent(QObject * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void AlexFilterWidget::closeEvent(QObject * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void AlexFilterWidget::messageEvent(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}
QT_WARNING_POP
