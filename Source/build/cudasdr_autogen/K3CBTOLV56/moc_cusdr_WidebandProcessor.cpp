/****************************************************************************
** Meta object code from reading C++ file 'cusdr_WidebandProcessor.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/DataEngine/cusdr_WidebandProcessor.h"
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cusdr_WidebandProcessor.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN21WideBandDataProcessorE_t {};
} // unnamed namespace

template <> constexpr inline auto WideBandDataProcessor::qt_create_metaobjectdata<qt_meta_tag_ZN21WideBandDataProcessorE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "WideBandDataProcessor",
        "messageEvent",
        "",
        "message",
        "wbSpectrumBufferChanged",
        "qVectorFloat",
        "buffer",
        "stop",
        "processWideBandData",
        "processWideBandInputBuffer",
        "getSpectrumData"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'messageEvent'
        QtMocHelpers::SignalData<void(QString)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'wbSpectrumBufferChanged'
        QtMocHelpers::SignalData<void(const qVectorFloat &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 5, 6 },
        }}),
        // Slot 'stop'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'processWideBandData'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'processWideBandInputBuffer'
        QtMocHelpers::SlotData<void(const QByteArray &)>(9, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QByteArray, 6 },
        }}),
        // Slot 'getSpectrumData'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<WideBandDataProcessor, qt_meta_tag_ZN21WideBandDataProcessorE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject WideBandDataProcessor::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN21WideBandDataProcessorE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN21WideBandDataProcessorE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN21WideBandDataProcessorE_t>.metaTypes,
    nullptr
} };

void WideBandDataProcessor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<WideBandDataProcessor *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->messageEvent((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->wbSpectrumBufferChanged((*reinterpret_cast< std::add_pointer_t<qVectorFloat>>(_a[1]))); break;
        case 2: _t->stop(); break;
        case 3: _t->processWideBandData(); break;
        case 4: _t->processWideBandInputBuffer((*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[1]))); break;
        case 5: _t->getSpectrumData(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (WideBandDataProcessor::*)(QString )>(_a, &WideBandDataProcessor::messageEvent, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (WideBandDataProcessor::*)(const qVectorFloat & )>(_a, &WideBandDataProcessor::wbSpectrumBufferChanged, 1))
            return;
    }
}

const QMetaObject *WideBandDataProcessor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WideBandDataProcessor::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN21WideBandDataProcessorE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int WideBandDataProcessor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void WideBandDataProcessor::messageEvent(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void WideBandDataProcessor::wbSpectrumBufferChanged(const qVectorFloat & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}
QT_WARNING_POP
