#ifndef ALEXSETTINGSCONTROLLER_H
#define ALEXSETTINGSCONTROLLER_H

#include <QObject>

class Settings;
class RadioModel;
class AlexTabWidget;
class AlexAntennaWidget;
class AlexFilterWidget;

class AlexSettingsController : public QObject {
    Q_OBJECT

public:
    explicit AlexSettingsController(QObject* parent = nullptr);

    void bind(AlexTabWidget* view, Settings* model);
    void bind(AlexTabWidget* view, RadioModel* radioModel, Settings* model);

private:
    AlexTabWidget* m_container = nullptr;
    AlexAntennaWidget* m_antennaView = nullptr;
    AlexFilterWidget* m_filterView = nullptr;
    RadioModel* m_radioModel = nullptr;
    Settings* m_model = nullptr;
};

#endif // ALEXSETTINGSCONTROLLER_H
