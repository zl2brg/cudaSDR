#ifndef EXTCTRLSETTINGSCONTROLLER_H
#define EXTCTRLSETTINGSCONTROLLER_H

#include <QObject>

class Settings;
class RadioModel;
class ExtCtrlWidget;

class ExtCtrlSettingsController : public QObject {
    Q_OBJECT

public:
    explicit ExtCtrlSettingsController(QObject* parent = nullptr);

    void bind(ExtCtrlWidget* view, Settings* model);
    void bind(ExtCtrlWidget* view, RadioModel* radioModel, Settings* model);

private:
    ExtCtrlWidget* m_view = nullptr;
    RadioModel* m_radioModel = nullptr;
    Settings* m_model = nullptr;
};

#endif // EXTCTRLSETTINGSCONTROLLER_H
