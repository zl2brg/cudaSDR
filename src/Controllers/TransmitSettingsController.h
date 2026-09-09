#ifndef TRANSMITSETTINGSCONTROLLER_H
#define TRANSMITSETTINGSCONTROLLER_H

#include <QObject>
#include <QString>

class Settings;
class TransmitModel;
class TransmitOptionsWidget;
class tx_settings_dialog;

class TransmitSettingsController : public QObject {
    Q_OBJECT

public:
    explicit TransmitSettingsController(QObject* parent = nullptr);

    void bind(tx_settings_dialog* view, Settings* model);
    void bind(tx_settings_dialog* view, TransmitModel* txModel, Settings* model);
    void bindOptions(TransmitOptionsWidget* options, TransmitModel* txModel);

private:
    void applyMicInputDev(int dev);
    void applyMicInputSourceName(const QString& name);
    void applyDigitalAudioInputDev(int dev);
    void applyDigitalInputSourceName(const QString& name);

    tx_settings_dialog* m_view = nullptr;
    TransmitOptionsWidget* m_optionsView = nullptr;
    TransmitModel* m_txModel = nullptr;
    Settings* m_model = nullptr;
};

#endif // TRANSMITSETTINGSCONTROLLER_H
