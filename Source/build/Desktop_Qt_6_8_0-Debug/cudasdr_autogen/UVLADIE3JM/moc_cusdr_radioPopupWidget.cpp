/****************************************************************************
** Meta object code from reading C++ file 'cusdr_radioPopupWidget.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/cusdr_radioPopupWidget.h"
#include <QtGui/qtextcursor.h>
#include <QtGui/qscreen.h>
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cusdr_radioPopupWidget.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN16RadioPopupWidgetE_t {};
} // unnamed namespace

template <> constexpr inline auto RadioPopupWidget::qt_create_metaobjectdata<qt_meta_tag_ZN16RadioPopupWidgetE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "RadioPopupWidget",
        "showEvent",
        "",
        "sender",
        "hideEvent",
        "closeEvent",
        "newMessage",
        "msg",
        "midToVfoBtnEvent",
        "vfoToMidBtnEvent",
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
        "showPopupWidget",
        "position",
        "graphicModeChanged",
        "rx",
        "PanGraphicsMode",
        "panMode",
        "WaterfallColorMode",
        "waterfallColorMode",
        "setSticky",
        "createOptionsBtnGroup",
        "createFFTOptionsGroup",
        "createBandBtnGroup",
        "createAdcBtnGroup",
        "createModeBtnGroup",
        "createAgcBtnGroup",
        "createFilterBtnWidgetA",
        "createFilterBtnWidgetB",
        "createFilterBtnWidgetC",
        "avgBtnClicked",
        "gridBtnClicked",
        "peakHoldBtnClicked",
        "panLockedBtnClicked",
        "clickVfoBtnClicked",
        "hairCrossBtnClicked",
        "midToVfoBtnClicked",
        "vfoToMidBtnClicked",
        "panModeChanged",
        "waterfallModeChanged",
        "setCurrentReceiver",
        "value",
        "ctrFrequencyChanged",
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
        "adcModeChangedByBtn",
        "adcModeChanged",
        "ADCMode",
        "agcModeChangedByBtn",
        "agcModeChanged",
        "AGCMode",
        "hang",
        "agcShowLinesChanged",
        "filterChangedByBtn",
        "filterChanged",
        "low",
        "high",
        "filterGroupChanged"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'showEvent'
        QtMocHelpers::SignalData<void(QObject *)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 },
        }}),
        // Signal 'hideEvent'
        QtMocHelpers::SignalData<void(QObject *)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 },
        }}),
        // Signal 'closeEvent'
        QtMocHelpers::SignalData<void(QObject *)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 },
        }}),
        // Signal 'newMessage'
        QtMocHelpers::SignalData<void(QString)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 7 },
        }}),
        // Signal 'midToVfoBtnEvent'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'vfoToMidBtnEvent'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'minimumSizeHint'
        QtMocHelpers::SlotData<QSize() const>(10, 2, QMC::AccessPublic, QMetaType::QSize),
        // Slot 'systemStateChanged'
        QtMocHelpers::SlotData<void(QObject *, QSDR::_Error, QSDR::_HWInterfaceMode, QSDR::_ServerMode, QSDR::_DataEngineState)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { 0x80000000 | 12, 13 }, { 0x80000000 | 14, 15 }, { 0x80000000 | 16, 17 },
            { 0x80000000 | 18, 19 },
        }}),
        // Slot 'showPopupWidget'
        QtMocHelpers::SlotData<bool(QObject *, QPoint)>(20, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::QPoint, 21 },
        }}),
        // Slot 'graphicModeChanged'
        QtMocHelpers::SlotData<void(QObject *, int, PanGraphicsMode, WaterfallColorMode)>(22, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 23 }, { 0x80000000 | 24, 25 }, { 0x80000000 | 26, 27 },
        }}),
        // Slot 'setSticky'
        QtMocHelpers::SlotData<void()>(28, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'createOptionsBtnGroup'
        QtMocHelpers::SlotData<void()>(29, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'createFFTOptionsGroup'
        QtMocHelpers::SlotData<void()>(30, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'createBandBtnGroup'
        QtMocHelpers::SlotData<void()>(31, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'createAdcBtnGroup'
        QtMocHelpers::SlotData<void()>(32, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'createModeBtnGroup'
        QtMocHelpers::SlotData<void()>(33, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'createAgcBtnGroup'
        QtMocHelpers::SlotData<void()>(34, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'createFilterBtnWidgetA'
        QtMocHelpers::SlotData<void()>(35, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'createFilterBtnWidgetB'
        QtMocHelpers::SlotData<void()>(36, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'createFilterBtnWidgetC'
        QtMocHelpers::SlotData<void()>(37, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'avgBtnClicked'
        QtMocHelpers::SlotData<void()>(38, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'gridBtnClicked'
        QtMocHelpers::SlotData<void()>(39, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'peakHoldBtnClicked'
        QtMocHelpers::SlotData<void()>(40, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'panLockedBtnClicked'
        QtMocHelpers::SlotData<void()>(41, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'clickVfoBtnClicked'
        QtMocHelpers::SlotData<void()>(42, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'hairCrossBtnClicked'
        QtMocHelpers::SlotData<void()>(43, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'midToVfoBtnClicked'
        QtMocHelpers::SlotData<void()>(44, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'vfoToMidBtnClicked'
        QtMocHelpers::SlotData<void()>(45, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'panModeChanged'
        QtMocHelpers::SlotData<void()>(46, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'waterfallModeChanged'
        QtMocHelpers::SlotData<void()>(47, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'setCurrentReceiver'
        QtMocHelpers::SlotData<void(QObject *, int)>(48, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 49 },
        }}),
        // Slot 'ctrFrequencyChanged'
        QtMocHelpers::SlotData<void(QObject *, int, int, long)>(50, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 17 }, { QMetaType::Int, 23 }, { QMetaType::Long, 51 },
        }}),
        // Slot 'vfoFrequencyChanged'
        QtMocHelpers::SlotData<void(QObject *, int, int, long)>(52, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 17 }, { QMetaType::Int, 23 }, { QMetaType::Long, 51 },
        }}),
        // Slot 'bandChangedByBtn'
        QtMocHelpers::SlotData<void()>(53, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'bandChanged'
        QtMocHelpers::SlotData<void(QObject *, int, bool, HamBand)>(54, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 23 }, { QMetaType::Bool, 55 }, { 0x80000000 | 56, 57 },
        }}),
        // Slot 'dspModeChangedByBtn'
        QtMocHelpers::SlotData<void()>(58, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'dspModeChanged'
        QtMocHelpers::SlotData<void(QObject *, int, DSPMode)>(59, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 23 }, { 0x80000000 | 60, 17 },
        }}),
        // Slot 'adcModeChangedByBtn'
        QtMocHelpers::SlotData<void()>(61, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'adcModeChanged'
        QtMocHelpers::SlotData<void(QObject *, int, ADCMode)>(62, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 23 }, { 0x80000000 | 63, 17 },
        }}),
        // Slot 'agcModeChangedByBtn'
        QtMocHelpers::SlotData<void()>(64, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'agcModeChanged'
        QtMocHelpers::SlotData<void(QObject *, int, AGCMode, bool)>(65, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 23 }, { 0x80000000 | 66, 17 }, { QMetaType::Bool, 67 },
        }}),
        // Slot 'agcShowLinesChanged'
        QtMocHelpers::SlotData<void()>(68, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'filterChangedByBtn'
        QtMocHelpers::SlotData<void()>(69, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'filterChanged'
        QtMocHelpers::SlotData<void(QObject *, int, qreal, qreal)>(70, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 23 }, { QMetaType::QReal, 71 }, { QMetaType::QReal, 72 },
        }}),
        // Slot 'filterGroupChanged'
        QtMocHelpers::SlotData<void(DSPMode)>(73, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 60, 17 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<RadioPopupWidget, qt_meta_tag_ZN16RadioPopupWidgetE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject RadioPopupWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16RadioPopupWidgetE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16RadioPopupWidgetE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN16RadioPopupWidgetE_t>.metaTypes,
    nullptr
} };

void RadioPopupWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<RadioPopupWidget *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->showEvent((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1]))); break;
        case 1: _t->hideEvent((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1]))); break;
        case 2: _t->closeEvent((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1]))); break;
        case 3: _t->newMessage((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->midToVfoBtnEvent(); break;
        case 5: _t->vfoToMidBtnEvent(); break;
        case 6: { QSize _r = _t->minimumSizeHint();
            if (_a[0]) *reinterpret_cast< QSize*>(_a[0]) = std::move(_r); }  break;
        case 7: _t->systemStateChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QSDR::_Error>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QSDR::_HWInterfaceMode>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QSDR::_ServerMode>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<QSDR::_DataEngineState>>(_a[5]))); break;
        case 8: { bool _r = _t->showPopupWidget((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QPoint>>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 9: _t->graphicModeChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<PanGraphicsMode>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<WaterfallColorMode>>(_a[4]))); break;
        case 10: _t->setSticky(); break;
        case 11: _t->createOptionsBtnGroup(); break;
        case 12: _t->createFFTOptionsGroup(); break;
        case 13: _t->createBandBtnGroup(); break;
        case 14: _t->createAdcBtnGroup(); break;
        case 15: _t->createModeBtnGroup(); break;
        case 16: _t->createAgcBtnGroup(); break;
        case 17: _t->createFilterBtnWidgetA(); break;
        case 18: _t->createFilterBtnWidgetB(); break;
        case 19: _t->createFilterBtnWidgetC(); break;
        case 20: _t->avgBtnClicked(); break;
        case 21: _t->gridBtnClicked(); break;
        case 22: _t->peakHoldBtnClicked(); break;
        case 23: _t->panLockedBtnClicked(); break;
        case 24: _t->clickVfoBtnClicked(); break;
        case 25: _t->hairCrossBtnClicked(); break;
        case 26: _t->midToVfoBtnClicked(); break;
        case 27: _t->vfoToMidBtnClicked(); break;
        case 28: _t->panModeChanged(); break;
        case 29: _t->waterfallModeChanged(); break;
        case 30: _t->setCurrentReceiver((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 31: _t->ctrFrequencyChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<long>>(_a[4]))); break;
        case 32: _t->vfoFrequencyChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<long>>(_a[4]))); break;
        case 33: _t->bandChangedByBtn(); break;
        case 34: _t->bandChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<HamBand>>(_a[4]))); break;
        case 35: _t->dspModeChangedByBtn(); break;
        case 36: _t->dspModeChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<DSPMode>>(_a[3]))); break;
        case 37: _t->adcModeChangedByBtn(); break;
        case 38: _t->adcModeChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<ADCMode>>(_a[3]))); break;
        case 39: _t->agcModeChangedByBtn(); break;
        case 40: _t->agcModeChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<AGCMode>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[4]))); break;
        case 41: _t->agcShowLinesChanged(); break;
        case 42: _t->filterChangedByBtn(); break;
        case 43: _t->filterChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[4]))); break;
        case 44: _t->filterGroupChanged((*reinterpret_cast< std::add_pointer_t<DSPMode>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 7:
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
        case 34:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 3:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< HamBand >(); break;
            }
            break;
        case 36:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 2:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< DSPMode >(); break;
            }
            break;
        case 38:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 2:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< ADCMode >(); break;
            }
            break;
        case 40:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 2:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< AGCMode >(); break;
            }
            break;
        case 44:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< DSPMode >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (RadioPopupWidget::*)(QObject * )>(_a, &RadioPopupWidget::showEvent, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (RadioPopupWidget::*)(QObject * )>(_a, &RadioPopupWidget::hideEvent, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (RadioPopupWidget::*)(QObject * )>(_a, &RadioPopupWidget::closeEvent, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (RadioPopupWidget::*)(QString )>(_a, &RadioPopupWidget::newMessage, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (RadioPopupWidget::*)()>(_a, &RadioPopupWidget::midToVfoBtnEvent, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (RadioPopupWidget::*)()>(_a, &RadioPopupWidget::vfoToMidBtnEvent, 5))
            return;
    }
}

const QMetaObject *RadioPopupWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RadioPopupWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16RadioPopupWidgetE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int RadioPopupWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 45)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 45;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 45)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 45;
    }
    return _id;
}

// SIGNAL 0
void RadioPopupWidget::showEvent(QObject * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void RadioPopupWidget::hideEvent(QObject * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void RadioPopupWidget::closeEvent(QObject * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void RadioPopupWidget::newMessage(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void RadioPopupWidget::midToVfoBtnEvent()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void RadioPopupWidget::vfoToMidBtnEvent()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}
QT_WARNING_POP
