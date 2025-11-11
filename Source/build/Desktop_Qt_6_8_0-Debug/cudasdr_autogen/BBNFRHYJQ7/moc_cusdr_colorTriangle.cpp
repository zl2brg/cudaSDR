/****************************************************************************
** Meta object code from reading C++ file 'cusdr_colorTriangle.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/Util/cusdr_colorTriangle.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cusdr_colorTriangle.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN15QtColorTriangleE_t {};
} // unnamed namespace

template <> constexpr inline auto QtColorTriangle::qt_create_metaobjectdata<qt_meta_tag_ZN15QtColorTriangleE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "QtColorTriangle",
        "colorChanged",
        "",
        "col",
        "setColor",
        "getColor"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'colorChanged'
        QtMocHelpers::SignalData<void(const QColor &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QColor, 3 },
        }}),
        // Slot 'setColor'
        QtMocHelpers::SlotData<void(const QColor &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QColor, 3 },
        }}),
        // Slot 'getColor'
        QtMocHelpers::SlotData<QColor()>(5, 2, QMC::AccessPublic, QMetaType::QColor),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<QtColorTriangle, qt_meta_tag_ZN15QtColorTriangleE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject QtColorTriangle::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15QtColorTriangleE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15QtColorTriangleE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15QtColorTriangleE_t>.metaTypes,
    nullptr
} };

void QtColorTriangle::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<QtColorTriangle *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->colorChanged((*reinterpret_cast< std::add_pointer_t<QColor>>(_a[1]))); break;
        case 1: _t->setColor((*reinterpret_cast< std::add_pointer_t<QColor>>(_a[1]))); break;
        case 2: { QColor _r = _t->getColor();
            if (_a[0]) *reinterpret_cast< QColor*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (QtColorTriangle::*)(const QColor & )>(_a, &QtColorTriangle::colorChanged, 0))
            return;
    }
}

const QMetaObject *QtColorTriangle::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtColorTriangle::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15QtColorTriangleE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int QtColorTriangle::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void QtColorTriangle::colorChanged(const QColor & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}
QT_WARNING_POP
