/****************************************************************************
** Meta object code from reading C++ file 'cusdr_mainWidget.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../cusdr_mainWidget.h"
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#include <QtNetwork/QSslError>
#include <QtGui/qtextcursor.h>
#include <QtGui/qscreen.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cusdr_mainWidget.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN10MainWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto MainWindow::qt_create_metaobjectdata<qt_meta_tag_ZN10MainWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MainWindow",
        "setAGCSliderValue",
        "",
        "value",
        "update",
        "masterSwitchChanged",
        "sender",
        "power",
        "startButtonClickedEvent",
        "widgetBtnClickedEvent",
        "wideBandBtnClickedEvent",
        "radioStateChange",
        "RadioState",
        "state",
        "alexBtnClickedEvent",
        "muteBtnClickedEvent",
        "moxBtnClickedEvent",
        "tunBtnClickedEvent",
        "showWidgetEvent",
        "closeWidgetEvent",
        "suspendSignal",
        "showWarningDialog",
        "msg",
        "systemStateChanged",
        "QSDR::_Error",
        "err",
        "QSDR::_HWInterfaceMode",
        "hwmode",
        "QSDR::_ServerMode",
        "mode",
        "QSDR::_DataEngineState",
        "ctrlDisplayBtnClickedEvent",
        "closeMainWindow",
        "maximizeMainWindow",
        "setMainWindowGeometry",
        "updateTitle",
        "updateStatusBar",
        "load",
        "setFullScreen",
        "getRegion",
        "cusdr_setup",
        "setServerMode",
        "setTxAllowed",
        "setCurrentReceiver",
        "rx",
        "setNumberOfReceivers",
        "setSDRMode",
        "getNetworkInterfaces",
        "setMainVolume",
        "setMicLevel",
        "setDriveLevel",
        "setADCMode",
        "ADCMode",
        "setAGCMode",
        "AGCMode",
        "hang",
        "setAGCGain",
        "getLastFrequency",
        "addReceiver",
        "alexPresenceChanged",
        "alexConfigurationChanged",
        "conf",
        "alexStateChanged",
        "HamBand",
        "band",
        "QList<int>",
        "states",
        "setAttenuator",
        "mercuryAttenuatorChanged",
        "showStatusBarMessage",
        "time",
        "clearStatusBarMessage",
        "showNetworkIODialog",
        "addNetworkIOComboBoxEntry",
        "str",
        "clearNetworkIOComboBoxEntry",
        "widebandVisibilityChanged",
        "showRadioPopup"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'setAGCSliderValue'
        QtMocHelpers::SignalData<void(int)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Slot 'update'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'masterSwitchChanged'
        QtMocHelpers::SlotData<void(QObject *, bool)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 6 }, { QMetaType::Bool, 7 },
        }}),
        // Slot 'startButtonClickedEvent'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'widgetBtnClickedEvent'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'wideBandBtnClickedEvent'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'radioStateChange'
        QtMocHelpers::SlotData<void(RadioState)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 12, 13 },
        }}),
        // Slot 'alexBtnClickedEvent'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'muteBtnClickedEvent'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'moxBtnClickedEvent'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'tunBtnClickedEvent'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'showWidgetEvent'
        QtMocHelpers::SlotData<void(QObject *)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 6 },
        }}),
        // Slot 'closeWidgetEvent'
        QtMocHelpers::SlotData<void(QObject *)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 6 },
        }}),
        // Slot 'suspendSignal'
        QtMocHelpers::SlotData<void(QObject *)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 6 },
        }}),
        // Slot 'showWarningDialog'
        QtMocHelpers::SlotData<void(const QString &)>(21, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 22 },
        }}),
        // Slot 'systemStateChanged'
        QtMocHelpers::SlotData<void(QObject *, QSDR::_Error, QSDR::_HWInterfaceMode, QSDR::_ServerMode, QSDR::_DataEngineState)>(23, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 6 }, { 0x80000000 | 24, 25 }, { 0x80000000 | 26, 27 }, { 0x80000000 | 28, 29 },
            { 0x80000000 | 30, 13 },
        }}),
        // Slot 'ctrlDisplayBtnClickedEvent'
        QtMocHelpers::SlotData<void()>(31, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'closeMainWindow'
        QtMocHelpers::SlotData<void()>(32, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'maximizeMainWindow'
        QtMocHelpers::SlotData<void()>(33, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'setMainWindowGeometry'
        QtMocHelpers::SlotData<void()>(34, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'updateTitle'
        QtMocHelpers::SlotData<void()>(35, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'updateStatusBar'
        QtMocHelpers::SlotData<void(short)>(36, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Short, 37 },
        }}),
        // Slot 'setFullScreen'
        QtMocHelpers::SlotData<void()>(38, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'getRegion'
        QtMocHelpers::SlotData<void()>(39, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'cusdr_setup'
        QtMocHelpers::SlotData<void()>(40, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'setServerMode'
        QtMocHelpers::SlotData<void(QSDR::_ServerMode)>(41, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 28, 29 },
        }}),
        // Slot 'setTxAllowed'
        QtMocHelpers::SlotData<void(QObject *, bool)>(42, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 6 }, { QMetaType::Bool, 3 },
        }}),
        // Slot 'setCurrentReceiver'
        QtMocHelpers::SlotData<void(QObject *, int)>(43, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 6 }, { QMetaType::Int, 44 },
        }}),
        // Slot 'setNumberOfReceivers'
        QtMocHelpers::SlotData<void(QObject *, int)>(45, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 6 }, { QMetaType::Int, 3 },
        }}),
        // Slot 'setSDRMode'
        QtMocHelpers::SlotData<void(bool)>(46, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 2 },
        }}),
        // Slot 'getNetworkInterfaces'
        QtMocHelpers::SlotData<void()>(47, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'setMainVolume'
        QtMocHelpers::SlotData<void(int)>(48, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Slot 'setMicLevel'
        QtMocHelpers::SlotData<void(int)>(49, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Slot 'setDriveLevel'
        QtMocHelpers::SlotData<void(int)>(50, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Slot 'setADCMode'
        QtMocHelpers::SlotData<void(QObject *, int, ADCMode)>(51, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 6 }, { QMetaType::Int, 44 }, { 0x80000000 | 52, 29 },
        }}),
        // Slot 'setAGCMode'
        QtMocHelpers::SlotData<void(QObject *, int, AGCMode, bool)>(53, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 6 }, { QMetaType::Int, 44 }, { 0x80000000 | 54, 29 }, { QMetaType::Bool, 55 },
        }}),
        // Slot 'setAGCGain'
        QtMocHelpers::SlotData<void(int)>(56, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Slot 'setAGCGain'
        QtMocHelpers::SlotData<void(QObject *, int, qreal)>(56, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 6 }, { QMetaType::Int, 44 }, { QMetaType::QReal, 3 },
        }}),
        // Slot 'getLastFrequency'
        QtMocHelpers::SlotData<void()>(57, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'addReceiver'
        QtMocHelpers::SlotData<void()>(58, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'alexPresenceChanged'
        QtMocHelpers::SlotData<void(bool)>(59, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 3 },
        }}),
        // Slot 'alexConfigurationChanged'
        QtMocHelpers::SlotData<void(quint16)>(60, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::UShort, 61 },
        }}),
        // Slot 'alexStateChanged'
        QtMocHelpers::SlotData<void(HamBand, const QList<int> &)>(62, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 63, 64 }, { 0x80000000 | 65, 66 },
        }}),
        // Slot 'setAttenuator'
        QtMocHelpers::SlotData<void()>(67, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'mercuryAttenuatorChanged'
        QtMocHelpers::SlotData<void(QObject *, HamBand, int)>(68, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 6 }, { 0x80000000 | 63, 64 }, { QMetaType::Int, 3 },
        }}),
        // Slot 'showStatusBarMessage'
        QtMocHelpers::SlotData<void(const QString &, int)>(69, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 22 }, { QMetaType::Int, 70 },
        }}),
        // Slot 'clearStatusBarMessage'
        QtMocHelpers::SlotData<void()>(71, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'showNetworkIODialog'
        QtMocHelpers::SlotData<void()>(72, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'addNetworkIOComboBoxEntry'
        QtMocHelpers::SlotData<void(QString)>(73, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 74 },
        }}),
        // Slot 'clearNetworkIOComboBoxEntry'
        QtMocHelpers::SlotData<void()>(75, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'widebandVisibilityChanged'
        QtMocHelpers::SlotData<void(bool)>(76, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 3 },
        }}),
        // Slot 'showRadioPopup'
        QtMocHelpers::SlotData<void(bool)>(77, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 3 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MainWindow, qt_meta_tag_ZN10MainWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10MainWindowE_t>.metaTypes,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MainWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->setAGCSliderValue((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->update(); break;
        case 2: _t->masterSwitchChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 3: _t->startButtonClickedEvent(); break;
        case 4: _t->widgetBtnClickedEvent(); break;
        case 5: _t->wideBandBtnClickedEvent(); break;
        case 6: _t->radioStateChange((*reinterpret_cast< std::add_pointer_t<RadioState>>(_a[1]))); break;
        case 7: _t->alexBtnClickedEvent(); break;
        case 8: _t->muteBtnClickedEvent(); break;
        case 9: _t->moxBtnClickedEvent(); break;
        case 10: _t->tunBtnClickedEvent(); break;
        case 11: _t->showWidgetEvent((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1]))); break;
        case 12: _t->closeWidgetEvent((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1]))); break;
        case 13: _t->suspendSignal((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1]))); break;
        case 14: _t->showWarningDialog((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 15: _t->systemStateChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QSDR::_Error>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QSDR::_HWInterfaceMode>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QSDR::_ServerMode>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<QSDR::_DataEngineState>>(_a[5]))); break;
        case 16: _t->ctrlDisplayBtnClickedEvent(); break;
        case 17: _t->closeMainWindow(); break;
        case 18: _t->maximizeMainWindow(); break;
        case 19: _t->setMainWindowGeometry(); break;
        case 20: _t->updateTitle(); break;
        case 21: _t->updateStatusBar((*reinterpret_cast< std::add_pointer_t<short>>(_a[1]))); break;
        case 22: _t->setFullScreen(); break;
        case 23: _t->getRegion(); break;
        case 24: _t->cusdr_setup(); break;
        case 25: _t->setServerMode((*reinterpret_cast< std::add_pointer_t<QSDR::_ServerMode>>(_a[1]))); break;
        case 26: _t->setTxAllowed((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 27: _t->setCurrentReceiver((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 28: _t->setNumberOfReceivers((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 29: _t->setSDRMode((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 30: _t->getNetworkInterfaces(); break;
        case 31: _t->setMainVolume((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 32: _t->setMicLevel((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 33: _t->setDriveLevel((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 34: _t->setADCMode((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<ADCMode>>(_a[3]))); break;
        case 35: _t->setAGCMode((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<AGCMode>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[4]))); break;
        case 36: _t->setAGCGain((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 37: _t->setAGCGain((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 38: _t->getLastFrequency(); break;
        case 39: _t->addReceiver(); break;
        case 40: _t->alexPresenceChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 41: _t->alexConfigurationChanged((*reinterpret_cast< std::add_pointer_t<quint16>>(_a[1]))); break;
        case 42: _t->alexStateChanged((*reinterpret_cast< std::add_pointer_t<HamBand>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QList<int>>>(_a[2]))); break;
        case 43: _t->setAttenuator(); break;
        case 44: _t->mercuryAttenuatorChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<HamBand>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 45: _t->showStatusBarMessage((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 46: _t->clearStatusBarMessage(); break;
        case 47: _t->showNetworkIODialog(); break;
        case 48: _t->addNetworkIOComboBoxEntry((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 49: _t->clearNetworkIOComboBoxEntry(); break;
        case 50: _t->widebandVisibilityChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 51: _t->showRadioPopup((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 15:
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
        case 25:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QSDR::_ServerMode >(); break;
            }
            break;
        case 34:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 2:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< ADCMode >(); break;
            }
            break;
        case 35:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 2:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< AGCMode >(); break;
            }
            break;
        case 42:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< HamBand >(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<int> >(); break;
            }
            break;
        case 44:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< HamBand >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (MainWindow::*)(int )>(_a, &MainWindow::setAGCSliderValue, 0))
            return;
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 52)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 52;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 52)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 52;
    }
    return _id;
}

// SIGNAL 0
void MainWindow::setAGCSliderValue(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}
namespace {
struct qt_meta_tag_ZN15NetworkIODialogE_t {};
} // unnamed namespace

template <> constexpr inline auto NetworkIODialog::qt_create_metaobjectdata<qt_meta_tag_ZN15NetworkIODialogE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "NetworkIODialog",
        "addDeviceComboBoxItem",
        "",
        "str",
        "clearDeviceComboBoxItem",
        "okBtnClicked"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'addDeviceComboBoxItem'
        QtMocHelpers::SlotData<void(QString)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Slot 'clearDeviceComboBoxItem'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'okBtnClicked'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<NetworkIODialog, qt_meta_tag_ZN15NetworkIODialogE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject NetworkIODialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15NetworkIODialogE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15NetworkIODialogE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15NetworkIODialogE_t>.metaTypes,
    nullptr
} };

void NetworkIODialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<NetworkIODialog *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->addDeviceComboBoxItem((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->clearDeviceComboBoxItem(); break;
        case 2: _t->okBtnClicked(); break;
        default: ;
        }
    }
}

const QMetaObject *NetworkIODialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *NetworkIODialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15NetworkIODialogE_t>.strings))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int NetworkIODialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
namespace {
struct qt_meta_tag_ZN13WarningDialogE_t {};
} // unnamed namespace

template <> constexpr inline auto WarningDialog::qt_create_metaobjectdata<qt_meta_tag_ZN13WarningDialogE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "WarningDialog",
        "setWarningMessage",
        "",
        "msg",
        "okBtnClicked"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'setWarningMessage'
        QtMocHelpers::SlotData<void(const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Slot 'okBtnClicked'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<WarningDialog, qt_meta_tag_ZN13WarningDialogE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject WarningDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13WarningDialogE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13WarningDialogE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN13WarningDialogE_t>.metaTypes,
    nullptr
} };

void WarningDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<WarningDialog *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->setWarningMessage((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->okBtnClicked(); break;
        default: ;
        }
    }
}

const QMetaObject *WarningDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WarningDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13WarningDialogE_t>.strings))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int WarningDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 2;
    }
    return _id;
}
QT_WARNING_POP
