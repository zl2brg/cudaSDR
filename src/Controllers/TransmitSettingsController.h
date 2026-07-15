#ifndef TRANSMITSETTINGSCONTROLLER_H
#define TRANSMITSETTINGSCONTROLLER_H

#include <QObject>

class Settings;
class tx_settings_dialog;

class TransmitSettingsController : public QObject {
    Q_OBJECT

public:
    explicit TransmitSettingsController(QObject* parent = nullptr);

    void bind(tx_settings_dialog* view, Settings* model);

private:
    tx_settings_dialog* m_view = nullptr;
    Settings* m_model = nullptr;
};

#endif // TRANSMITSETTINGSCONTROLLER_H
