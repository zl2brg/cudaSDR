#ifndef AUDIO_DIALOG_H
#define AUDIO_DIALOG_H

#include <QDialog>
#include <QAudioDeviceInfo>

#include "cusdr_settings.h"

namespace Ui {
class audio_dialog;
}

class audio_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit audio_dialog(QWidget *parent = nullptr);
    const QAudioDeviceInfo& inputDevice() const     { return m_inputDevice; }
    ~audio_dialog();

private:
    Ui::audio_dialog *ui;
    Settings*		set;

    QFont			m_titleFont;
    //WindowFunction   m_windowFunction;
    QAudioDeviceInfo m_inputDevice;
    QAudioDeviceInfo m_outputDevice;
    const QList<QAudioDeviceInfo> m_availableAudioInputDevices;
    QAudioDeviceInfo    m_audioInputDevice;
signals:
    void micInputChanged(int);

private slots:
    void ok_pressed();
    void cancel_pressed();
    void audio_input_changed(int index);
    void mic_level_changed(int level);

};

#endif // AUDIO_DIALOG_H
