/****************************************************************************
** Meta object code from reading C++ file 'cusdr_dataIO.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/DataEngine/cusdr_dataIO.h"
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cusdr_dataIO.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN6DataIOE_t {};
} // unnamed namespace

template <> constexpr inline auto DataIO::qt_create_metaobjectdata<qt_meta_tag_ZN6DataIOE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "DataIO",
        "messageEvent",
        "",
        "message",
        "readydata",
        "stop",
        "initDataReceiverSocket",
        "readData",
        "writeData",
        "sendAudio",
        "u_char*",
        "buf",
        "sendInitFramesToNetworkDevice",
        "rx",
        "networkDeviceStartStop",
        "value",
        "setSampleRate",
        "sender",
        "setManualSocketBufferSize",
        "setSocketBufferSize",
        "displayDataReceiverSocketError",
        "QAbstractSocket::SocketError",
        "error",
        "readDeviceData",
        "new_readDeviceData"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'messageEvent'
        QtMocHelpers::SignalData<void(QString)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'readydata'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'stop'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'initDataReceiverSocket'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'readData'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'writeData'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'sendAudio'
        QtMocHelpers::SlotData<void(u_char *)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 10, 11 },
        }}),
        // Slot 'sendInitFramesToNetworkDevice'
        QtMocHelpers::SlotData<void(int)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 13 },
        }}),
        // Slot 'networkDeviceStartStop'
        QtMocHelpers::SlotData<void(char)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Char, 15 },
        }}),
        // Slot 'setSampleRate'
        QtMocHelpers::SlotData<void(QObject *, int)>(16, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 17 }, { QMetaType::Int, 15 },
        }}),
        // Slot 'setManualSocketBufferSize'
        QtMocHelpers::SlotData<void(QObject *, bool)>(18, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 17 }, { QMetaType::Bool, 15 },
        }}),
        // Slot 'setSocketBufferSize'
        QtMocHelpers::SlotData<void(QObject *, int)>(19, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 17 }, { QMetaType::Int, 15 },
        }}),
        // Slot 'displayDataReceiverSocketError'
        QtMocHelpers::SlotData<void(QAbstractSocket::SocketError)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 21, 22 },
        }}),
        // Slot 'readDeviceData'
        QtMocHelpers::SlotData<void()>(23, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'new_readDeviceData'
        QtMocHelpers::SlotData<void()>(24, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<DataIO, qt_meta_tag_ZN6DataIOE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject DataIO::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6DataIOE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6DataIOE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN6DataIOE_t>.metaTypes,
    nullptr
} };

void DataIO::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DataIO *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->messageEvent((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->readydata(); break;
        case 2: _t->stop(); break;
        case 3: _t->initDataReceiverSocket(); break;
        case 4: _t->readData(); break;
        case 5: _t->writeData(); break;
        case 6: _t->sendAudio((*reinterpret_cast< std::add_pointer_t<u_char*>>(_a[1]))); break;
        case 7: _t->sendInitFramesToNetworkDevice((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 8: _t->networkDeviceStartStop((*reinterpret_cast< std::add_pointer_t<char>>(_a[1]))); break;
        case 9: _t->setSampleRate((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 10: _t->setManualSocketBufferSize((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 11: _t->setSocketBufferSize((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 12: _t->displayDataReceiverSocketError((*reinterpret_cast< std::add_pointer_t<QAbstractSocket::SocketError>>(_a[1]))); break;
        case 13: _t->readDeviceData(); break;
        case 14: _t->new_readDeviceData(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 12:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QAbstractSocket::SocketError >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (DataIO::*)(QString )>(_a, &DataIO::messageEvent, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataIO::*)()>(_a, &DataIO::readydata, 1))
            return;
    }
}

const QMetaObject *DataIO::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DataIO::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN6DataIOE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DataIO::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    }
    return _id;
}

// SIGNAL 0
void DataIO::messageEvent(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void DataIO::readydata()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
