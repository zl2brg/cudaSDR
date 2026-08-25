#ifndef DISPLAYSETTINGSCONTROLLER_H
#define DISPLAYSETTINGSCONTROLLER_H

#include <QObject>

class Settings;
class RadioModel;
class DisplayTabWidget;
class DisplayOptionsWidget;
class ColorOptionsWidget;

class DisplaySettingsController : public QObject {
    Q_OBJECT

public:
    explicit DisplaySettingsController(QObject* parent = nullptr);

    void bind(DisplayTabWidget* container, Settings* model);
    void bind(DisplayTabWidget* container, RadioModel* radioModel, Settings* model);

private:
    DisplayTabWidget* m_container = nullptr;
    DisplayOptionsWidget* m_displayView = nullptr;
    ColorOptionsWidget* m_colorView = nullptr;
    RadioModel* m_radioModel = nullptr;
    Settings* m_model = nullptr;
};

#endif // DISPLAYSETTINGSCONTROLLER_H
