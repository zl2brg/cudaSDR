#ifndef ALEXSETTINGSCONTROLLER_H
#define ALEXSETTINGSCONTROLLER_H

#include <QObject>

class Settings;
class AlexTabWidget;
class AlexAntennaWidget;
class AlexFilterWidget;

class AlexSettingsController : public QObject {
    Q_OBJECT

public:
    explicit AlexSettingsController(QObject* parent = nullptr);

    void bind(AlexTabWidget* view, Settings* model);

private:
    AlexTabWidget* m_container = nullptr;
    AlexAntennaWidget* m_antennaView = nullptr;
    AlexFilterWidget* m_filterView = nullptr;
    Settings* m_model = nullptr;
};

#endif // ALEXSETTINGSCONTROLLER_H
