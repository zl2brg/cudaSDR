#ifndef CUSDR_RADIOSETTINGSWIDGET_H
#define CUSDR_RADIOSETTINGSWIDGET_H

#include <QDialog>

namespace Ui {
class cusdr_radioSettingsWidget;
}

class cusdr_radioSettingsWidget : public QDialog
{
    Q_OBJECT

public:
    explicit cusdr_radioSettingsWidget(QWidget *parent = nullptr);
    ~cusdr_radioSettingsWidget();

#ifdef HAVE_SOAPYSDR
    QWidget *detachRadioConfigPage();

    // MVC View Interface Setters
    void setTxFullDuplex(bool enabled);
    void setSoapyIQBalance(bool enabled);
    void setSoapyAutoCalibrate(bool enabled);
    void setSoapyLnaGain(int value);
    void setSoapyTiaGain(int value);
    void setSoapyPgaGain(int value);
    void setSoapyOverallGain(int value);
    void setAntennaList(const QStringList& list, const QString& active);
    void setTxAntennaList(const QStringList& list, const QString& active);
    void updateGainGroupVisibility(const QString& hardwareKey);
#endif

signals:
#ifdef HAVE_SOAPYSDR
    // MVC View Interface Signals
    void txFullDuplexRequested(bool enabled);
    void soapyIQBalanceRequested(bool enabled);
    void soapyAutoCalibrateRequested(bool enabled);
    void soapyLnaGainRequested(int value);
    void soapyTiaGainRequested(int value);
    void soapyPgaGainRequested(int value);
    void soapyOverallGainRequested(int value);
    void soapyRxAntennaRequested(const QString& antenna);
    void soapyTxAntennaRequested(const QString& antenna);
#endif

private:
    Ui::cusdr_radioSettingsWidget *ui;

private slots:
#ifdef HAVE_SOAPYSDR
    // Internal UI wiring slots
    void onFullDuplexToggled(bool enabled);
    void onIQBalanceToggled(bool enabled);
    void onAutoCalToggled(bool enabled);
    void onAntennaComboChanged(int index);
    void onTxAntennaComboChanged(int index);
    void onLnaSliderChanged(int value);
    void onLnaSpinBoxChanged(int value);
    void onTiaSliderChanged(int value);
    void onPgaSliderChanged(int value);
    void onPgaSpinBoxChanged(int value);
    void onOverallGainSliderChanged(int value);
    void onOverallGainSpinBoxChanged(int value);
#endif
};

#endif // CUSDR_RADIOSETTINGSWIDGET_H
