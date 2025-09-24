#ifndef AUDIO_DIALOG_H
#define AUDIO_DIALOG_H

#include <QDialog>
#include <QAudioDevice>
#include <QMediaDevices>
#include "cusdr_settings.h"

namespace Ui {
class tx_settings_dialog;
}

class tx_settings_dialog : public QWidget
{
    Q_OBJECT

public:
    explicit tx_settings_dialog(QWidget *parent = nullptr);
    ~tx_settings_dialog();

private:
    Ui::tx_settings_dialog *ui;
    Settings*		set;
    QAudioDevice m_inputDevice;
    QAudioDevice m_outputDevice;
    PaError         error = paNoError;
    QStringList paDeviceList;
    double      m_amCarrierLevel;
    double      m_audioCompressionLevel;
    QFont			m_titleFont;
    //WindowFunction   m_windowFunction;

signals:
    void micInputChanged(int);

private slots:



};

#endif // AUDIO_DIALOG_H
