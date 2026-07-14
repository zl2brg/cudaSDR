#ifndef RADIOSETTINGSCONTROLLER_H
#define RADIOSETTINGSCONTROLLER_H

#include <QObject>

#ifdef HAVE_SOAPYSDR

class Settings;
class cusdr_radioSettingsWidget;

class RadioSettingsController : public QObject {
    Q_OBJECT

public:
    explicit RadioSettingsController(QObject* parent = nullptr);

    void bind(cusdr_radioSettingsWidget* view, Settings* model);

private:
    cusdr_radioSettingsWidget* m_view = nullptr;
    Settings* m_model = nullptr;
};

#endif // HAVE_SOAPYSDR

#endif // RADIOSETTINGSCONTROLLER_H
