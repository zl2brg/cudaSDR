#ifndef EXTCTRLSETTINGSCONTROLLER_H
#define EXTCTRLSETTINGSCONTROLLER_H

#include <QObject>

class Settings;
class ExtCtrlWidget;

class ExtCtrlSettingsController : public QObject {
    Q_OBJECT

public:
    explicit ExtCtrlSettingsController(QObject* parent = nullptr);

    void bind(ExtCtrlWidget* view, Settings* model);

private:
    ExtCtrlWidget* m_view = nullptr;
    Settings* m_model = nullptr;
};

#endif // EXTCTRLSETTINGSCONTROLLER_H
