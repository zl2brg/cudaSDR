/****************************************************************************
** Meta object code from reading C++ file 'cusdr_audioReceiver.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/DataEngine/cusdr_audioReceiver.h"
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cusdr_audioReceiver.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN13AudioReceiverE_t {};
} // unnamed namespace

template <> constexpr inline auto AudioReceiver::qt_create_metaobjectdata<qt_meta_tag_ZN13AudioReceiverE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "AudioReceiver",
        "messageEvent",
        "",
        "message",
        "rcveIQEvent",
        "sender",
        "value",
        "outputBufferEvent",
        "uchar*",
        "outbuffer",
        "clientConnectedEvent",
        "newData",
        "newAudioData",
        "initClient",
        "displayAudioRcvrSocketError",
        "QAbstractSocket::SocketError",
        "error",
        "readPendingAudioRcvrData"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'messageEvent'
        QtMocHelpers::SignalData<void(QString)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'rcveIQEvent'
        QtMocHelpers::SignalData<void(QObject *, int)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 5 }, { QMetaType::Int, 6 },
        }}),
        // Signal 'outputBufferEvent'
        QtMocHelpers::SignalData<void(unsigned char *)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 8, 9 },
        }}),
        // Signal 'clientConnectedEvent'
        QtMocHelpers::SignalData<void(bool)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 6 },
        }}),
        // Signal 'newData'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'newAudioData'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'initClient'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'displayAudioRcvrSocketError'
        QtMocHelpers::SlotData<void(QAbstractSocket::SocketError)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 15, 16 },
        }}),
        // Slot 'readPendingAudioRcvrData'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AudioReceiver, qt_meta_tag_ZN13AudioReceiverE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject AudioReceiver::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13AudioReceiverE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13AudioReceiverE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN13AudioReceiverE_t>.metaTypes,
    nullptr
} };

void AudioReceiver::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AudioReceiver *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->messageEvent((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->rcveIQEvent((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 2: _t->outputBufferEvent((*reinterpret_cast< std::add_pointer_t<uchar*>>(_a[1]))); break;
        case 3: _t->clientConnectedEvent((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 4: _t->newData(); break;
        case 5: _t->newAudioData(); break;
        case 6: _t->initClient(); break;
        case 7: _t->displayAudioRcvrSocketError((*reinterpret_cast< std::add_pointer_t<QAbstractSocket::SocketError>>(_a[1]))); break;
        case 8: _t->readPendingAudioRcvrData(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 7:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QAbstractSocket::SocketError >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AudioReceiver::*)(QString )>(_a, &AudioReceiver::messageEvent, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (AudioReceiver::*)(QObject * , int )>(_a, &AudioReceiver::rcveIQEvent, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (AudioReceiver::*)(unsigned char * )>(_a, &AudioReceiver::outputBufferEvent, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (AudioReceiver::*)(bool )>(_a, &AudioReceiver::clientConnectedEvent, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (AudioReceiver::*)()>(_a, &AudioReceiver::newData, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (AudioReceiver::*)()>(_a, &AudioReceiver::newAudioData, 5))
            return;
    }
}

const QMetaObject *AudioReceiver::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AudioReceiver::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13AudioReceiverE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int AudioReceiver::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void AudioReceiver::messageEvent(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void AudioReceiver::rcveIQEvent(QObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2);
}

// SIGNAL 2
void AudioReceiver::outputBufferEvent(unsigned char * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void AudioReceiver::clientConnectedEvent(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void AudioReceiver::newData()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void AudioReceiver::newAudioData()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}
QT_WARNING_POP
