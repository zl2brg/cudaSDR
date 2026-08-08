#ifndef AUDIO_DIALOG_H
#define AUDIO_DIALOG_H

#include <QWidget>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QComboBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QLabel>
#include <functional>

namespace Ui {
class tx_settings_dialog;
}

class tx_settings_dialog : public QWidget
{
    Q_OBJECT

public:
    explicit tx_settings_dialog(QWidget *parent = nullptr);
    ~tx_settings_dialog();

    // MVC View Interface Setters
    void setAvailableCodec2Modes(const QList<int>& modes);
    void setCodec2ModeStringResolver(std::function<QString(int)> resolver);
    void setAmCarrierLevel(double level);
    void setAudioCompression(double compression);
    void setFmDeviation(int dev);
    void setFmPreEmphasis(bool enabled);
    void setPhaseRotator(bool enabled);
    void setCwSidetoneFreq(int freq);
    void setCwSidetoneVolume(int vol);
    void setCwHangTime(int time);
    void setCwKeyerMode(int mode);
    void setInternalCw(bool val);
    void setCwKeyReversed(bool val);
    void setCwKeyerSpacing(bool val);
    void setCwKeyerSpeed(int speed);
    void setCwPttDelay(int delay);
    void setCwKeyerWeight(int weight);
    void setCurrentReceiver(int rx);
    void setFreeDVMode(int rx, int mode);
    void refreshAudioDevices(const QString& savedMicName, const QString& savedDigitalName);

signals:
    // MVC View Interface Signals
    void audioDevicesRefreshRequested();
    void micInputDevChanged(int dev);
    void micInputSourceNameChanged(const QString& name);
    void digitalAudioInputDevChanged(int dev);
    void digitalInputSourceNameChanged(const QString& name);
    void freeDVModeRequested(int rx, int mode);
    void audioCompressionRequested(int val);
    void amCarrierLevelRequested(int val);
    void fmDeviationRequested(int val);
    void fmPreEmphasisRequested(bool enabled);
    void phaseRotatorRequested(bool enabled);
    void cwKeyerModeRequested(int val);
    void internalCwRequested(bool val);
    void cwKeyReversedRequested(bool val);
    void cwKeyerSpacingRequested(bool val);
    void cwKeyerSpeedRequested(int val);
    void cwPttDelayRequested(int val);
    void cwSidetoneFreqRequested(int val);
    void cwSidetoneVolumeRequested(int val);
    void cwHangTimeRequested(int val);
    void cwKeyerWeightRequested(int val);

private slots:
    void triggerRefreshDevices();

private:
    Ui::tx_settings_dialog *ui;
    QAudioDevice m_inputDevice;
    QAudioDevice m_outputDevice;
    double      m_amCarrierLevel;
    double      m_audioCompressionLevel;
    QFont			m_titleFont;
    QComboBox*      m_codec2ModeCombo;  // FreeDV mode selector
    int             m_currentReceiver;
    std::function<QString(int)> m_codec2ModeStringResolver;
};

#endif // AUDIO_DIALOG_H
