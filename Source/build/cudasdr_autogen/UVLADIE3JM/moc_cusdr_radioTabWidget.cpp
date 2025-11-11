/****************************************************************************
** Meta object code from reading C++ file 'cusdr_radioTabWidget.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/cusdr_radioTabWidget.h"
#include <QtGui/qtextcursor.h>
#include <QtGui/qscreen.h>
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cusdr_radioTabWidget.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN14RadioTabWidgetE_t {};
} // unnamed namespace

template <> constexpr inline auto RadioTabWidget::qt_create_metaobjectdata<qt_meta_tag_ZN14RadioTabWidgetE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "RadioTabWidget",
        "showEvent",
        "",
        "sender",
        "closeEvent",
        "messageEvent",
        "message",
        "sizeHint",
        "minimumSizeHint",
        "systemStateChanged",
        "QSDR::_Error",
        "err",
        "QSDR::_HWInterfaceMode",
        "hwmode",
        "QSDR::_ServerMode",
        "mode",
        "QSDR::_DataEngineState",
        "state",
        "setPennyPresence",
        "value"
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
        // Slot 'sizeHint'
        QtMocHelpers::SlotData<QSize() const>(7, 2, QMC::AccessPublic, QMetaType::QSize),
        // Slot 'minimumSizeHint'
        QtMocHelpers::SlotData<QSize() const>(8, 2, QMC::AccessPublic, QMetaType::QSize),
        // Slot 'systemStateChanged'
        QtMocHelpers::SlotData<void(QObject *, QSDR::_Error, QSDR::_HWInterfaceMode, QSDR::_ServerMode, QSDR::_DataEngineState)>(9, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { 0x80000000 | 10, 11 }, { 0x80000000 | 12, 13 }, { 0x80000000 | 14, 15 },
            { 0x80000000 | 16, 17 },
        }}),
        // Slot 'setPennyPresence'
        QtMocHelpers::SlotData<void(bool)>(18, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 19 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<RadioTabWidget, qt_meta_tag_ZN14RadioTabWidgetE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject RadioTabWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QTabWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14RadioTabWidgetE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14RadioTabWidgetE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN14RadioTabWidgetE_t>.metaTypes,
    nullptr
} };

void RadioTabWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<RadioTabWidget *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->showEvent((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1]))); break;
        case 1: _t->closeEvent((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1]))); break;
        case 2: _t->messageEvent((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: { QSize _r = _t->sizeHint();
            if (_a[0]) *reinterpret_cast< QSize*>(_a[0]) = std::move(_r); }  break;
        case 4: { QSize _r = _t->minimumSizeHint();
            if (_a[0]) *reinterpret_cast< QSize*>(_a[0]) = std::move(_r); }  break;
        case 5: _t->systemStateChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QSDR::_Error>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QSDR::_HWInterfaceMode>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QSDR::_ServerMode>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<QSDR::_DataEngineState>>(_a[5]))); break;
        case 6: _t->setPennyPresence((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
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
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (RadioTabWidget::*)(QObject * )>(_a, &RadioTabWidget::showEvent, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (RadioTabWidget::*)(QObject * )>(_a, &RadioTabWidget::closeEvent, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (RadioTabWidget::*)(QString )>(_a, &RadioTabWidget::messageEvent, 2))
            return;
    }
}

const QMetaObject *RadioTabWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RadioTabWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14RadioTabWidgetE_t>.strings))
        return static_cast<void*>(this);
    return QTabWidget::qt_metacast(_clname);
}

int RadioTabWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTabWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void RadioTabWidget::showEvent(QObject * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void RadioTabWidget::closeEvent(QObject * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void RadioTabWidget::messageEvent(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}
QT_WARNING_POP
