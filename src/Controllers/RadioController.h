#ifndef RADIOCONTROLLER_H
#define RADIOCONTROLLER_H

#include <QObject>
#include <QMetaObject>
#include <QVector>

class RadioModel;
class DataEngine;
class SliceModel;

/**
 * Binds SliceModel protocol/hardware side effects to DataEngine.
 * DSP (WDSP volume, mode, filters, NCO) is wired SliceModel -> QWDSPEngine at RX init.
 */
class RadioController : public QObject {
    Q_OBJECT

public:
    explicit RadioController(QObject* parent = nullptr);

    void bind(RadioModel* model, DataEngine* engine);

private:
    void bindSlice(SliceModel* slice, DataEngine* engine);

    // Tracks live connections so bind() can tear them down before re-binding
    // (prevents duplicate firings when the engine stops/restarts).
    QVector<QMetaObject::Connection> m_connections;
};

#endif  // RADIOCONTROLLER_H
