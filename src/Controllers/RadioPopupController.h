#ifndef RADIOPOPUPCONTROLLER_H
#define RADIOPOPUPCONTROLLER_H

#include <QObject>
#include "Settings/SettingsTypes.h"

class Settings;
class SliceModel;
class RadioPopupWidget;

class RadioPopupController : public QObject {
    Q_OBJECT

public:
    explicit RadioPopupController(QObject* parent = nullptr);

    void bind(RadioPopupWidget* view, SliceModel* sliceModel, Settings* model);

private:
    RadioPopupWidget* m_view = nullptr;
    SliceModel* m_sliceModel = nullptr;
    Settings* m_model = nullptr;
    PanAveragingMode m_lastPanAvMode = AV_MODE_RECURSIVE;
};

#endif // RADIOPOPUPCONTROLLER_H
