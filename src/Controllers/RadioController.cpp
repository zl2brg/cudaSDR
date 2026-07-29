#include "RadioController.h"

#include "Models/RadioModel.h"
#include "Models/SliceModel.h"
#include "DataEngine/cusdr_dataEngine.h"

RadioController::RadioController(QObject* parent)
    : QObject(parent)
{
}

void RadioController::bind(RadioModel* model, DataEngine* engine)
{
    if (!model || !engine) {
        return;
    }

    // Disconnect any connections from a previous bind() call to prevent
    // duplicate signal firings after an engine stop/restart.
    for (const QMetaObject::Connection& c : std::as_const(m_connections))
        disconnect(c);
    m_connections.clear();

    const QList<SliceModel*>& slices = model->slices();
    for (int i = 0; i < slices.size(); ++i) {
        SliceModel* slice = slices.at(i);
        if (!slice || i >= engine->RX.size()) {
            continue;
        }
        bindSlice(slice, engine);
    }
}

void RadioController::bindSlice(SliceModel* slice, DataEngine* engine)
{
    if (!slice || !engine) {
        return;
    }

    const int rx = slice->id();

    // Panadapter center -> tell Protocol 1/2 which RX had a center-frequency change.
    // (WDSP NCO / filters / mode: SliceModel -> QWDSPEngine directly.)
    m_connections.append(connect(slice, &SliceModel::centerFrequencyChanged, engine,
            [engine, rx](long) {
                engine->rx_freq_change = rx;
            }));

    // Mode -> TX control bytes (WDSP mode is driven by SliceModel -> QWDSPEngine).
    m_connections.append(connect(slice, &SliceModel::dspModeChanged, engine,
            [engine, rx](DSPMode mode) {
                engine->applySliceDspMode(rx, mode);
            }));
}
