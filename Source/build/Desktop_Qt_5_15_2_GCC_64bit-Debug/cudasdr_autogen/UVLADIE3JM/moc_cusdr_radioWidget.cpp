/****************************************************************************
** Meta object code from reading C++ file 'cusdr_radioWidget.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/cusdr_radioWidget.h"
#include <QtGui/qtextcursor.h>
#include <QtGui/qscreen.h>
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cusdr_radioWidget.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN11RadioWidgetE_t {};
} // unnamed namespace

template <> constexpr inline auto RadioWidget::qt_create_metaobjectdata<qt_meta_tag_ZN11RadioWidgetE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "RadioWidget",
        "showEvent",
        "",
        "sender",
        "closeEvent",
        "newMessage",
        "msg",
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
        "createBandBtnGroup",
        "createModeBtnGroup",
        "createFilterBtnGroupA",
        "createFilterBtnGroupB",
        "createFilterBtnGroupC",
        "createLabel",
        "QLabel*",
        "text",
        "setCurrentReceiver",
        "value",
        "ctrFrequencyChanged",
        "rx",
        "frequency",
        "vfoFrequencyChanged",
        "bandChangedByBtn",
        "bandChanged",
        "byButton",
        "HamBand",
        "band",
        "dspModeChangedByBtn",
        "dspModeChanged",
        "DSPMode",
        "filterChangedByBtn",
        "filterChanged",
        "low",
        "high",
        "filterGroupChanged",
        "attenuatorChanged",
        "setMercuryAttenuator",
        "ditherChanged",
        "randomChanged"
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
        // Signal 'newMessage'
        QtMocHelpers::SignalData<void(QString)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 },
        }}),
        // Slot 'sizeHint'
        QtMocHelpers::SlotData<QSize() const>(7, 2, QMC::AccessPublic, QMetaType::QSize),
        // Slot 'minimumSizeHint'
        QtMocHelpers::SlotData<QSize() const>(8, 2, QMC::AccessPublic, QMetaType::QSize),
        // Slot 'systemStateChanged'
        QtMocHelpers::SlotData<void(QObject *, QSDR::_Error, QSDR::_HWInterfaceMode, QSDR::_ServerMode, QSDR::_DataEngineState)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { 0x80000000 | 10, 11 }, { 0x80000000 | 12, 13 }, { 0x80000000 | 14, 15 },
            { 0x80000000 | 16, 17 },
        }}),
        // Slot 'createBandBtnGroup'
        QtMocHelpers::SlotData<void()>(18, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'createModeBtnGroup'
        QtMocHelpers::SlotData<void()>(19, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'createFilterBtnGroupA'
        QtMocHelpers::SlotData<void()>(20, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'createFilterBtnGroupB'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'createFilterBtnGroupC'
        QtMocHelpers::SlotData<void()>(22, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'createLabel'
        QtMocHelpers::SlotData<QLabel *(const QString &)>(23, 2, QMC::AccessPrivate, 0x80000000 | 24, {{
            { QMetaType::QString, 25 },
        }}),
        // Slot 'setCurrentReceiver'
        QtMocHelpers::SlotData<void(QObject *, int)>(26, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 27 },
        }}),
        // Slot 'ctrFrequencyChanged'
        QtMocHelpers::SlotData<void(QObject *, int, int, long)>(28, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 15 }, { QMetaType::Int, 29 }, { QMetaType::Long, 30 },
        }}),
        // Slot 'vfoFrequencyChanged'
        QtMocHelpers::SlotData<void(QObject *, int, int, long)>(31, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 15 }, { QMetaType::Int, 29 }, { QMetaType::Long, 30 },
        }}),
        // Slot 'bandChangedByBtn'
        QtMocHelpers::SlotData<void()>(32, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'bandChanged'
        QtMocHelpers::SlotData<void(QObject *, int, bool, HamBand)>(33, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 29 }, { QMetaType::Bool, 34 }, { 0x80000000 | 35, 36 },
        }}),
        // Slot 'dspModeChangedByBtn'
        QtMocHelpers::SlotData<void()>(37, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'dspModeChanged'
        QtMocHelpers::SlotData<void(QObject *, int, DSPMode)>(38, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 29 }, { 0x80000000 | 39, 15 },
        }}),
        // Slot 'filterChangedByBtn'
        QtMocHelpers::SlotData<void()>(40, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'filterChanged'
        QtMocHelpers::SlotData<void(QObject *, int, qreal, qreal)>(41, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 29 }, { QMetaType::QReal, 42 }, { QMetaType::QReal, 43 },
        }}),
        // Slot 'filterGroupChanged'
        QtMocHelpers::SlotData<void(DSPMode)>(44, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 39, 15 },
        }}),
        // Slot 'attenuatorChanged'
        QtMocHelpers::SlotData<void()>(45, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'setMercuryAttenuator'
        QtMocHelpers::SlotData<void(QObject *, HamBand, int)>(46, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { 0x80000000 | 35, 36 }, { QMetaType::Int, 27 },
        }}),
        // Slot 'ditherChanged'
        QtMocHelpers::SlotData<void()>(47, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'randomChanged'
        QtMocHelpers::SlotData<void()>(48, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<RadioWidget, qt_meta_tag_ZN11RadioWidgetE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject RadioWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11RadioWidgetE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11RadioWidgetE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN11RadioWidgetE_t>.metaTypes,
    nullptr
} };

void RadioWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<RadioWidget *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->showEvent((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1]))); break;
        case 1: _t->closeEvent((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1]))); break;
        case 2: _t->newMessage((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: { QSize _r = _t->sizeHint();
            if (_a[0]) *reinterpret_cast< QSize*>(_a[0]) = std::move(_r); }  break;
        case 4: { QSize _r = _t->minimumSizeHint();
            if (_a[0]) *reinterpret_cast< QSize*>(_a[0]) = std::move(_r); }  break;
        case 5: _t->systemStateChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QSDR::_Error>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QSDR::_HWInterfaceMode>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QSDR::_ServerMode>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<QSDR::_DataEngineState>>(_a[5]))); break;
        case 6: _t->createBandBtnGroup(); break;
        case 7: _t->createModeBtnGroup(); break;
        case 8: _t->createFilterBtnGroupA(); break;
        case 9: _t->createFilterBtnGroupB(); break;
        case 10: _t->createFilterBtnGroupC(); break;
        case 11: { QLabel* _r = _t->createLabel((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast< QLabel**>(_a[0]) = std::move(_r); }  break;
        case 12: _t->setCurrentReceiver((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 13: _t->ctrFrequencyChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<long>>(_a[4]))); break;
        case 14: _t->vfoFrequencyChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<long>>(_a[4]))); break;
        case 15: _t->bandChangedByBtn(); break;
        case 16: _t->bandChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<HamBand>>(_a[4]))); break;
        case 17: _t->dspModeChangedByBtn(); break;
        case 18: _t->dspModeChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<DSPMode>>(_a[3]))); break;
        case 19: _t->filterChangedByBtn(); break;
        case 20: _t->filterChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[4]))); break;
        case 21: _t->filterGroupChanged((*reinterpret_cast< std::add_pointer_t<DSPMode>>(_a[1]))); break;
        case 22: _t->attenuatorChanged(); break;
        case 23: _t->setMercuryAttenuator((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<HamBand>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 24: _t->ditherChanged(); break;
        case 25: _t->randomChanged(); break;
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
        case 16:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 3:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< HamBand >(); break;
            }
            break;
        case 18:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 2:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< DSPMode >(); break;
            }
            break;
        case 21:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< DSPMode >(); break;
            }
            break;
        case 23:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< HamBand >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (RadioWidget::*)(QObject * )>(_a, &RadioWidget::showEvent, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (RadioWidget::*)(QObject * )>(_a, &RadioWidget::closeEvent, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (RadioWidget::*)(QString )>(_a, &RadioWidget::newMessage, 2))
            return;
    }
}

const QMetaObject *RadioWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RadioWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11RadioWidgetE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int RadioWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 26)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 26;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 26)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 26;
    }
    return _id;
}

// SIGNAL 0
void RadioWidget::showEvent(QObject * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void RadioWidget::closeEvent(QObject * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void RadioWidget::newMessage(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}
QT_WARNING_POP
