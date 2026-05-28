#ifndef CUSDR_RADIOSETTINGSWIDGET_H
#define CUSDR_RADIOSETTINGSWIDGET_H

#include <QDialog>
#include "cusdr_settings.h"

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
#endif

private:
    Ui::cusdr_radioSettingsWidget *ui;
    Settings *set;

    void setupRadioTab();
    void updateGainGroupVisibility();

private slots:
#ifdef HAVE_SOAPYSDR
    void onSoapyAntennaListChanged(QStringList list);
    void onSoapyTxAntennaListChanged(QStringList list);
    void onSoapyHardwareKeyChanged(QString key);
    void onSoapyAutoCalibrateChanged(bool enabled);
    void onAntennaComboChanged(int index);
    void onTxAntennaComboChanged(int index);
    void onAutoCalToggled(bool enabled);
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
