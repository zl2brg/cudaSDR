#ifndef HPSDRSETTINGSCONTROLLER_H
#define HPSDRSETTINGSCONTROLLER_H

#include <QObject>

class Settings;
class HPSDRWidget;

class HpsdrSettingsController : public QObject {
    Q_OBJECT

public:
    explicit HpsdrSettingsController(QObject* parent = nullptr);

    void bind(HPSDRWidget* view, Settings* model);

private:
    HPSDRWidget* m_view = nullptr;
    Settings* m_model = nullptr;
};

#endif // HPSDRSETTINGSCONTROLLER_H
