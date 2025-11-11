/****************************************************************************
** Meta object code from reading C++ file 'cusdr_ogl3DPanel.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/GL/cusdr_ogl3DPanel.h"
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cusdr_ogl3DPanel.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN10QGL3DPanelE_t {};
} // unnamed namespace

template <> constexpr inline auto QGL3DPanel::qt_create_metaobjectdata<qt_meta_tag_ZN10QGL3DPanelE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "QGL3DPanel",
        "mousePositionChanged",
        "",
        "position",
        "frequencySelected",
        "frequency",
        "setSpectrumBuffer",
        "rx",
        "qVectorFloat",
        "buffer",
        "setCtrFrequency",
        "sender",
        "mode",
        "freq",
        "setVFOFrequency",
        "setHeightScale",
        "scale",
        "setFrequencyScale",
        "setTimeScale",
        "setUpdateRate",
        "intervalMs",
        "setShowGrid",
        "show",
        "setShowAxes",
        "setWireframeMode",
        "wireframe",
        "setWaterfallOffset",
        "offset",
        "performUpdate",
        "calculateFPS",
        "spectrumDataChanged",
        "QList<float>",
        "data",
        "updateDisplay",
        "onUpdateTimer"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'mousePositionChanged'
        QtMocHelpers::SignalData<void(QPoint)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QPoint, 3 },
        }}),
        // Signal 'frequencySelected'
        QtMocHelpers::SignalData<void(float)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 5 },
        }}),
        // Slot 'setSpectrumBuffer'
        QtMocHelpers::SlotData<void(int, const qVectorFloat &)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 7 }, { 0x80000000 | 8, 9 },
        }}),
        // Slot 'setCtrFrequency'
        QtMocHelpers::SlotData<void(QObject *, int, int, long)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 11 }, { QMetaType::Int, 12 }, { QMetaType::Int, 7 }, { QMetaType::Long, 13 },
        }}),
        // Slot 'setVFOFrequency'
        QtMocHelpers::SlotData<void(QObject *, int, int, long)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QObjectStar, 11 }, { QMetaType::Int, 12 }, { QMetaType::Int, 7 }, { QMetaType::Long, 13 },
        }}),
        // Slot 'setHeightScale'
        QtMocHelpers::SlotData<void(float)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 16 },
        }}),
        // Slot 'setFrequencyScale'
        QtMocHelpers::SlotData<void(float)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 16 },
        }}),
        // Slot 'setTimeScale'
        QtMocHelpers::SlotData<void(float)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 16 },
        }}),
        // Slot 'setUpdateRate'
        QtMocHelpers::SlotData<void(int)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 20 },
        }}),
        // Slot 'setShowGrid'
        QtMocHelpers::SlotData<void(bool)>(21, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 22 },
        }}),
        // Slot 'setShowAxes'
        QtMocHelpers::SlotData<void(bool)>(23, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 22 },
        }}),
        // Slot 'setWireframeMode'
        QtMocHelpers::SlotData<void(bool)>(24, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 25 },
        }}),
        // Slot 'setWaterfallOffset'
        QtMocHelpers::SlotData<void(float)>(26, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Float, 27 },
        }}),
        // Slot 'performUpdate'
        QtMocHelpers::SlotData<void()>(28, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'calculateFPS'
        QtMocHelpers::SlotData<void()>(29, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'spectrumDataChanged'
        QtMocHelpers::SlotData<void(const QVector<float> &)>(30, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 31, 32 },
        }}),
        // Slot 'updateDisplay'
        QtMocHelpers::SlotData<void()>(33, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onUpdateTimer'
        QtMocHelpers::SlotData<void()>(34, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<QGL3DPanel, qt_meta_tag_ZN10QGL3DPanelE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject QGL3DPanel::staticMetaObject = { {
    QMetaObject::SuperData::link<QOpenGLWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10QGL3DPanelE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10QGL3DPanelE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10QGL3DPanelE_t>.metaTypes,
    nullptr
} };

void QGL3DPanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<QGL3DPanel *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->mousePositionChanged((*reinterpret_cast< std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 1: _t->frequencySelected((*reinterpret_cast< std::add_pointer_t<float>>(_a[1]))); break;
        case 2: _t->setSpectrumBuffer((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qVectorFloat>>(_a[2]))); break;
        case 3: _t->setCtrFrequency((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<long>>(_a[4]))); break;
        case 4: _t->setVFOFrequency((*reinterpret_cast< std::add_pointer_t<QObject*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<long>>(_a[4]))); break;
        case 5: _t->setHeightScale((*reinterpret_cast< std::add_pointer_t<float>>(_a[1]))); break;
        case 6: _t->setFrequencyScale((*reinterpret_cast< std::add_pointer_t<float>>(_a[1]))); break;
        case 7: _t->setTimeScale((*reinterpret_cast< std::add_pointer_t<float>>(_a[1]))); break;
        case 8: _t->setUpdateRate((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 9: _t->setShowGrid((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 10: _t->setShowAxes((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 11: _t->setWireframeMode((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 12: _t->setWaterfallOffset((*reinterpret_cast< std::add_pointer_t<float>>(_a[1]))); break;
        case 13: _t->performUpdate(); break;
        case 14: _t->calculateFPS(); break;
        case 15: _t->spectrumDataChanged((*reinterpret_cast< std::add_pointer_t<QList<float>>>(_a[1]))); break;
        case 16: _t->updateDisplay(); break;
        case 17: _t->onUpdateTimer(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 15:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<float> >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (QGL3DPanel::*)(QPoint )>(_a, &QGL3DPanel::mousePositionChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (QGL3DPanel::*)(float )>(_a, &QGL3DPanel::frequencySelected, 1))
            return;
    }
}

const QMetaObject *QGL3DPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QGL3DPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10QGL3DPanelE_t>.strings))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "QOpenGLFunctions"))
        return static_cast< QOpenGLFunctions*>(this);
    return QOpenGLWidget::qt_metacast(_clname);
}

int QGL3DPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QOpenGLWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    return _id;
}

// SIGNAL 0
void QGL3DPanel::mousePositionChanged(QPoint _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void QGL3DPanel::frequencySelected(float _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}
QT_WARNING_POP
