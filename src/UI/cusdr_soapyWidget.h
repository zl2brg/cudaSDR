/*
 *   SoapySDR radio parameter widget — antenna and gain controls.
 *   Shown as the "Radio" tab inside cusdr_SetupWidget.
 */

#ifndef CUSDR_SOAPYWIDGET_H
#define CUSDR_SOAPYWIDGET_H

#ifdef HAVE_SOAPYSDR

#include <QWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QSlider>
#include <QGroupBox>
#include <QLabel>
#include "cusdr_settings.h"

class SoapyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SoapyWidget(QWidget *parent = nullptr);

private:
    Settings   *set;

    QGroupBox  *m_antennaGroup;
    QComboBox  *m_antennaCombo;

    QGroupBox  *m_limeGainGroup;
    QCheckBox  *m_autoGainCheck;
    QSlider    *m_lnaSlider;
    QSpinBox   *m_lnaSpinBox;
    QComboBox  *m_tiaCombo;
    QSlider    *m_pgaSlider;
    QSpinBox   *m_pgaSpinBox;

    QGroupBox  *m_overallGainGroup;
    QSlider    *m_overallGainSlider;
    QSpinBox   *m_overallGainSpinBox;

    QGroupBox  *m_dspRateGroup;
    QComboBox  *m_dspRateCombo;
    QLabel     *m_hwRateLabel;
    QLabel     *m_decimLabel;

    void buildUi();
    void populateFromSettings();
    void updateGainGroupVisibility();

private slots:
    void onSoapyAntennaListChanged(QStringList list);
    void onSoapyHardwareKeyChanged(QString key);
    void onSampleRateChanged(int rate);
    void onDspRateChanged(int index);
    void onAutoCalToggled(bool enabled);
    void onLnaSliderChanged(int value);
    void onLnaSpinBoxChanged(int value);
    void onTiaComboChanged(int index);
    void onPgaSliderChanged(int value);
    void onPgaSpinBoxChanged(int value);
    void onOverallGainSliderChanged(int value);
    void onOverallGainSpinBoxChanged(int value);
};

#endif // HAVE_SOAPYSDR
#endif // CUSDR_SOAPYWIDGET_H
