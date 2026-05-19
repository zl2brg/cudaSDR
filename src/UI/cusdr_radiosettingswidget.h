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

private:
    Ui::cusdr_radioSettingsWidget *ui;
    Settings *set;

    void setupRadioTab();
    void updateGainGroupVisibility();

private slots:
#ifdef HAVE_SOAPYSDR
    void onSoapyAntennaListChanged(QStringList list);
    void onSoapyHardwareKeyChanged(QString key);
    void onAntennaComboChanged(int index);
    void onLnaSpinBoxChanged(int value);
    void onTiaComboChanged(int index);
    void onPgaSpinBoxChanged(int value);
    void onOverallGainSliderChanged(int value);
    void onOverallGainSpinBoxChanged(int value);
#endif
};

#endif // CUSDR_RADIOSETTINGSWIDGET_H
