#include "RadioController.h"

#include "Models/RadioModel.h"
#include "Models/SliceModel.h"
#include "DataEngine/cusdr_dataEngine.h"
#include "DataEngine/cusdr_receiver.h"
#include "QtWDSP/qtwdsp_dspEngine.h"
#include "cusdr_settings.h"

RadioController::RadioController(QObject* parent)
    : QObject(parent)
{
}

void RadioController::bind(RadioModel* model, DataEngine* engine)
{
    if (!model || !engine) {
        return;
    }

    const QList<SliceModel*>& slices = model->slices();
    for (int i = 0; i < slices.size(); ++i) {
        SliceModel* slice = slices.at(i);
        if (!slice || i >= engine->RX.size()) {
            continue;
        }
        bindSlice(slice, engine, engine->RX.at(i));
    }
}

void RadioController::bindSlice(SliceModel* slice, DataEngine* engine, Receiver* receiver)
{
    if (!slice || !engine || !receiver) {
        return;
    }

    const int rx = slice->id();

    // VFO -> hardware NCO / RX center tracking (Protocol 1 rx_freq_change).
    connect(slice, &SliceModel::frequencyChanged, engine,
            [engine, rx](long freq) { engine->setFrequency(0, rx, freq); });

    // Panadapter center -> RX + protocol.
    connect(slice, &SliceModel::centerFrequencyChanged, engine,
            [engine, receiver, rx](long freq) {
                receiver->setCtrFrequency(freq);
                engine->io.rx_freq_change = rx;
            });

    // Mode -> TX CC + WDSP (slice is authoritative; Settings::dspMode no longer required).
    connect(slice, &SliceModel::dspModeChanged, engine,
            [engine, receiver, slice, rx](DSPMode mode) {
                engine->applySliceDspMode(rx, mode);
                receiver->applyDspModeFromSlice(mode, slice->centerFrequency());
            });

    // Filters -> WDSP (Receiver also mirrors m_receiverData for legacy readers).
    connect(slice, &SliceModel::filterChanged, receiver,
            [receiver, slice]() {
                receiver->applyFilterFromSlice(
                    static_cast<double>(slice->filterLow()),
                    static_cast<double>(slice->filterHigh()));
            });
}
