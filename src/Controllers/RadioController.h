#ifndef RADIOCONTROLLER_H
#define RADIOCONTROLLER_H

#include <QObject>

class RadioModel;
class DataEngine;
class SliceModel;
class Receiver;

/**
 * Binds SliceModel signals to DataEngine / Receiver / WDSP (no Settings relay).
 * Slice state is the source of truth for tuning, mode, and filters.
 */
class RadioController : public QObject {
    Q_OBJECT

public:
    explicit RadioController(QObject* parent = nullptr);

    void bind(RadioModel* model, DataEngine* engine);

private:
    void bindSlice(SliceModel* slice, DataEngine* engine, Receiver* receiver);
};

#endif  // RADIOCONTROLLER_H
