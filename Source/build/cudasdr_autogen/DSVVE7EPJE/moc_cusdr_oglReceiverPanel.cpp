/****************************************************************************
** Meta object code from reading C++ file 'cusdr_oglReceiverPanel.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/GL/cusdr_oglReceiverPanel.h"
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#include <QtNetwork/QSslError>
#include <QtGui/qtextcursor.h>
#include <QtGui/qscreen.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cusdr_oglReceiverPanel.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN16QGLReceiverPanelE_t {};
} // unnamed namespace

template <> constexpr inline auto QGLReceiverPanel::qt_create_metaobjectdata<qt_meta_tag_ZN16QGLReceiverPanelE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "QGLReceiverPanel",
        "showEvent",
        "",
        "sender",
        "closeEvent",
        "messageEvent",
        "msg",
        "coordChanged",
        "x",
        "y",
        "minimumSizeHint",
        "sizeHint",
        "setSpectrumBuffer",
        "rx",
        "qVectorFloat",
        "buffer",
        "setCtrFrequency",
        "mode",
        "freq",
        "setVFOFrequency",
        "systemStateChanged",
        "QSDR::_Error",
        "err",
        "QSDR::_HWInterfaceMode",
        "hwmode",
        "QSDR::_ServerMode",
        "QSDR::_DataEngineState",
        "state",
        "graphicModeChanged",
        "PanGraphicsMode",
        "panMode",
        "WaterfallColorMode",
        "waterfallColorMode",
        "setSpectrumSize",
        "value",
        "setCurrentReceiver",
        "setHamBand",
        "byButton",
        "HamBand",
        "band",
        "setFilterFrequencies",
        "lo",
        "hi",
        "setMercuryAttenuator",
        "setupDisplayRegions",
        "size",
        "setSpectrumAveraging",
        "setSpectrumAveragingCnt",
        "setVfoToMidFrequency",
        "setMidToVfoFrequency",
        "setPanGridStatus",
        "setPeakHoldStatus",
        "setPanLockedStatus",
        "setClickVFOStatus",
        "setHairCrossStatus",
        "setPanadapterColors",
        "getRegion",
        "p",
        "freqRulerPositionChanged",
        "pos",
        "sampleRateChanged",
        "setWaterfallOffesetLo",
        "setWaterfallOffesetHi",
        "setdBmScaleMin",
        "setdBmScaleMax",
        "setMouseWheelFreqStep",
        "setADCStatus",
        "updateADCStatus",
        "setFramesPerSecond",
        "setDSPMode",
        "DSPMode",
        "setADCMode",
        "ADCMode",
        "setAGCMode",
        "AGCMode",
        "hangEnabled",
        "setAGCLineLevels",
        "thresh",
        "hang",
        "setAGCLineFixedLevel",
        "setAGCLinesStatus"
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
        // Signal 'coordChanged'
        QtMocHelpers::SignalData<void(int, int)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 8 }, { QMetaType::Int, 9 },
        }}),
        // Slot 'minimumSizeHint'
        QtMocHelpers::SlotData<QSize() const>(10, 2, QMC::AccessPublic, QMetaType::QSize),
        // Slot 'sizeHint'
        QtMocHelpers::SlotData<QSize() const>(11, 2, QMC::AccessPublic, QMetaType::QSize),
        // Slot 'setSpectrumBuffer'
        QtMocHelpers::SlotData<void(int, const qVectorFloat &)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 13 }, { 0x80000000 | 14, 15 },
        }}),
        // Slot 'setCtrFrequency'
        QtMocHelpers::SlotData<void(QObject *, int, int, long)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 17 }, { QMetaType::Int, 13 }, { QMetaType::Long, 18 },
        }}),
        // Slot 'setVFOFrequency'
        QtMocHelpers::SlotData<void(QObject *, int, int, long)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 17 }, { QMetaType::Int, 13 }, { QMetaType::Long, 18 },
        }}),
        // Slot 'systemStateChanged'
        QtMocHelpers::SlotData<void(QObject *, QSDR::_Error, QSDR::_HWInterfaceMode, QSDR::_ServerMode, QSDR::_DataEngineState)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { 0x80000000 | 21, 22 }, { 0x80000000 | 23, 24 }, { 0x80000000 | 25, 17 },
            { 0x80000000 | 26, 27 },
        }}),
        // Slot 'graphicModeChanged'
        QtMocHelpers::SlotData<void(QObject *, int, PanGraphicsMode, WaterfallColorMode)>(28, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 13 }, { 0x80000000 | 29, 30 }, { 0x80000000 | 31, 32 },
        }}),
        // Slot 'setSpectrumSize'
        QtMocHelpers::SlotData<void(QObject *, int)>(33, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 34 },
        }}),
        // Slot 'setCurrentReceiver'
        QtMocHelpers::SlotData<void(QObject *, int)>(35, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 34 },
        }}),
        // Slot 'setHamBand'
        QtMocHelpers::SlotData<void(QObject *, int, bool, HamBand)>(36, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 13 }, { QMetaType::Bool, 37 }, { 0x80000000 | 38, 39 },
        }}),
        // Slot 'setFilterFrequencies'
        QtMocHelpers::SlotData<void(QObject *, int, qreal, qreal)>(40, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 13 }, { QMetaType::QReal, 41 }, { QMetaType::QReal, 42 },
        }}),
        // Slot 'setMercuryAttenuator'
        QtMocHelpers::SlotData<void(QObject *, HamBand, int)>(43, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { 0x80000000 | 38, 39 }, { QMetaType::Int, 34 },
        }}),
        // Slot 'setupDisplayRegions'
        QtMocHelpers::SlotData<void(QSize)>(44, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QSize, 45 },
        }}),
        // Slot 'setSpectrumAveraging'
        QtMocHelpers::SlotData<void(QObject *, int, bool)>(46, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 13 }, { QMetaType::Bool, 34 },
        }}),
        // Slot 'setSpectrumAveragingCnt'
        QtMocHelpers::SlotData<void(int)>(47, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 34 },
        }}),
        // Slot 'setVfoToMidFrequency'
        QtMocHelpers::SlotData<void()>(48, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'setMidToVfoFrequency'
        QtMocHelpers::SlotData<void()>(49, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'setPanGridStatus'
        QtMocHelpers::SlotData<void(bool, int)>(50, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 34 }, { QMetaType::Int, 13 },
        }}),
        // Slot 'setPeakHoldStatus'
        QtMocHelpers::SlotData<void(bool, int)>(51, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 34 }, { QMetaType::Int, 13 },
        }}),
        // Slot 'setPanLockedStatus'
        QtMocHelpers::SlotData<void(bool, int)>(52, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 34 }, { QMetaType::Int, 13 },
        }}),
        // Slot 'setClickVFOStatus'
        QtMocHelpers::SlotData<void(bool, int)>(53, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 34 }, { QMetaType::Int, 13 },
        }}),
        // Slot 'setHairCrossStatus'
        QtMocHelpers::SlotData<void(bool, int)>(54, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 34 }, { QMetaType::Int, 13 },
        }}),
        // Slot 'setPanadapterColors'
        QtMocHelpers::SlotData<void()>(55, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'getRegion'
        QtMocHelpers::SlotData<void(QPoint)>(56, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QPoint, 57 },
        }}),
        // Slot 'freqRulerPositionChanged'
        QtMocHelpers::SlotData<void(QObject *, int, float)>(58, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 13 }, { QMetaType::Float, 59 },
        }}),
        // Slot 'sampleRateChanged'
        QtMocHelpers::SlotData<void(QObject *, int)>(60, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 34 },
        }}),
        // Slot 'setWaterfallOffesetLo'
        QtMocHelpers::SlotData<void(int, int)>(61, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 13 }, { QMetaType::Int, 34 },
        }}),
        // Slot 'setWaterfallOffesetHi'
        QtMocHelpers::SlotData<void(int, int)>(62, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 13 }, { QMetaType::Int, 34 },
        }}),
        // Slot 'setdBmScaleMin'
        QtMocHelpers::SlotData<void(int, qreal)>(63, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 13 }, { QMetaType::QReal, 34 },
        }}),
        // Slot 'setdBmScaleMax'
        QtMocHelpers::SlotData<void(int, qreal)>(64, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 13 }, { QMetaType::QReal, 34 },
        }}),
        // Slot 'setMouseWheelFreqStep'
        QtMocHelpers::SlotData<void(QObject *, int, qreal)>(65, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 2 }, { QMetaType::Int, 2 }, { QMetaType::QReal, 2 },
        }}),
        // Slot 'setADCStatus'
        QtMocHelpers::SlotData<void(int)>(66, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 34 },
        }}),
        // Slot 'updateADCStatus'
        QtMocHelpers::SlotData<void()>(67, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'setFramesPerSecond'
        QtMocHelpers::SlotData<void(QObject *, int, int)>(68, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 13 }, { QMetaType::Int, 34 },
        }}),
        // Slot 'setDSPMode'
        QtMocHelpers::SlotData<void(QObject *, int, DSPMode)>(69, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 13 }, { 0x80000000 | 70, 17 },
        }}),
        // Slot 'setADCMode'
        QtMocHelpers::SlotData<void(QObject *, int, ADCMode)>(71, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 13 }, { 0x80000000 | 72, 17 },
        }}),
        // Slot 'setAGCMode'
        QtMocHelpers::SlotData<void(QObject *, int, AGCMode, bool)>(73, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 13 }, { 0x80000000 | 74, 17 }, { QMetaType::Bool, 75 },
        }}),
        // Slot 'setAGCLineLevels'
        QtMocHelpers::SlotData<void(QObject *, int, qreal, qreal)>(76, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 13 }, { QMetaType::QReal, 77 }, { QMetaType::QReal, 78 },
        }}),
        // Slot 'setAGCLineFixedLevel'
        QtMocHelpers::SlotData<void(QObject *, int, qreal)>(79, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Int, 13 }, { QMetaType::QReal, 34 },
        }}),
        // Slot 'setAGCLinesStatus'
        QtMocHelpers::SlotData<void(QObject *, bool, int)>(80, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QObjectStar, 3 }, { QMetaType::Bool, 34 }, { QMetaType::Int, 13 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<QGLReceiverPanel, qt_meta_tag_ZN16QGLReceiverPanelE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject QGLReceiverPanel::staticMetaObject = { {
    QMetaObject::SuperData::link<QOpenGLWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16QGLReceiverPanelE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16QGLReceiverPanelE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN16QGLReceiverPanelE_t>.metaTypes,
    nullptr
} };

void QGLReceiverPanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<QGLReceiverPanel *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->showEvent((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1]))); break;
        case 1: _t->closeEvent((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1]))); break;
        case 2: _t->messageEvent((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->coordChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 4: { QSize _r = _t->minimumSizeHint();
            if (_a[0]) *reinterpret_cast< QSize*>(_a[0]) = std::move(_r); }  break;
        case 5: { QSize _r = _t->sizeHint();
            if (_a[0]) *reinterpret_cast< QSize*>(_a[0]) = std::move(_r); }  break;
        case 6: _t->setSpectrumBuffer((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qVectorFloat>>(_a[2]))); break;
        case 7: _t->setCtrFrequency((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<long>>(_a[4]))); break;
        case 8: _t->setVFOFrequency((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<long>>(_a[4]))); break;
        case 9: _t->systemStateChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QSDR::_Error>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QSDR::_HWInterfaceMode>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QSDR::_ServerMode>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<QSDR::_DataEngineState>>(_a[5]))); break;
        case 10: _t->graphicModeChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<PanGraphicsMode>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<WaterfallColorMode>>(_a[4]))); break;
        case 11: _t->setSpectrumSize((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 12: _t->setCurrentReceiver((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 13: _t->setHamBand((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<HamBand>>(_a[4]))); break;
        case 14: _t->setFilterFrequencies((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[4]))); break;
        case 15: _t->setMercuryAttenuator((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<HamBand>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 16: _t->setupDisplayRegions((*reinterpret_cast< std::add_pointer_t<QSize>>(_a[1]))); break;
        case 17: _t->setSpectrumAveraging((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3]))); break;
        case 18: _t->setSpectrumAveragingCnt((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 19: _t->setVfoToMidFrequency(); break;
        case 20: _t->setMidToVfoFrequency(); break;
        case 21: _t->setPanGridStatus((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 22: _t->setPeakHoldStatus((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 23: _t->setPanLockedStatus((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 24: _t->setClickVFOStatus((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 25: _t->setHairCrossStatus((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 26: _t->setPanadapterColors(); break;
        case 27: _t->getRegion((*reinterpret_cast< std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 28: _t->freqRulerPositionChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<float>>(_a[3]))); break;
        case 29: _t->sampleRateChanged((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 30: _t->setWaterfallOffesetLo((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 31: _t->setWaterfallOffesetHi((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 32: _t->setdBmScaleMin((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[2]))); break;
        case 33: _t->setdBmScaleMax((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[2]))); break;
        case 34: _t->setMouseWheelFreqStep((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 35: _t->setADCStatus((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 36: _t->updateADCStatus(); break;
        case 37: _t->setFramesPerSecond((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 38: _t->setDSPMode((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<DSPMode>>(_a[3]))); break;
        case 39: _t->setADCMode((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<ADCMode>>(_a[3]))); break;
        case 40: _t->setAGCMode((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<AGCMode>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[4]))); break;
        case 41: _t->setAGCLineLevels((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[4]))); break;
        case 42: _t->setAGCLineFixedLevel((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 43: _t->setAGCLinesStatus((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 9:
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
        case 13:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 3:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< HamBand >(); break;
            }
            break;
        case 15:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< HamBand >(); break;
            }
            break;
        case 38:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 2:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< DSPMode >(); break;
            }
            break;
        case 39:
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
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (QGLReceiverPanel::*)(QObject * )>(_a, &QGLReceiverPanel::showEvent, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (QGLReceiverPanel::*)(QObject * )>(_a, &QGLReceiverPanel::closeEvent, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (QGLReceiverPanel::*)(QString )>(_a, &QGLReceiverPanel::messageEvent, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (QGLReceiverPanel::*)(int , int )>(_a, &QGLReceiverPanel::coordChanged, 3))
            return;
    }
}

const QMetaObject *QGLReceiverPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QGLReceiverPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16QGLReceiverPanelE_t>.strings))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "QOpenGLFunctions"))
        return static_cast< QOpenGLFunctions*>(this);
    return QOpenGLWidget::qt_metacast(_clname);
}

int QGLReceiverPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QOpenGLWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 44)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 44;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 44)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 44;
    }
    return _id;
}

// SIGNAL 0
void QGLReceiverPanel::showEvent(QObject * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void QGLReceiverPanel::closeEvent(QObject * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void QGLReceiverPanel::messageEvent(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void QGLReceiverPanel::coordChanged(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}
QT_WARNING_POP
