#ifndef AUDIO_DIALOG_H
#define AUDIO_DIALOG_H

#include <QWidget>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QVector>
#include <functional>

class EqCurvePlot;

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
    void setPhaseRotatorAuto(bool enabled);
    void setPhaseRotatorStatus(const QString &status);
    void setTxEqEnabled(bool enabled);
    void setTxEqBands(const QVector<int> &bands);
    void setTxEqCurveDeg(int deg);
    void setCfcEnabled(bool enabled);
    void setCfcPeqEnabled(bool enabled);
    void setCfcPrecomp(double db);
    void setCfcPrePeq(double db);
    void setCfcCurveDeg(int deg);
    void setCfcLevels(const QVector<double> &levels);
    void setCfcPost(const QVector<double> &post);
    void refreshEqCurvePlots();
    void setCtcssToneHz(int hz);
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
    void updateTxEqPassband();

protected:
    void showEvent(QShowEvent *event) override;

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
    void phaseRotatorAutoRequested(bool enabled);
    void phaseRotatorAutoResetRequested();
    void txEqEnabledRequested(bool enabled);
    void txEqBandRequested(int index, int gainDb);
    void txEqCurveDegRequested(int deg);
    void cfcEnabledRequested(bool enabled);
    void cfcPeqEnabledRequested(bool enabled);
    void cfcPrecompRequested(double db);
    void cfcPrePeqRequested(double db);
    void cfcCurveDegRequested(int deg);
    void cfcLevelRequested(int index, double db);
    void cfcPostRequested(int index, double db);
    void ctcssToneHzRequested(int hz);
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
    QCheckBox*      m_phaseRotatorAuto = nullptr;
    QPushButton*    m_phaseRotatorReset = nullptr;
    QLabel*         m_phaseRotatorStatus = nullptr;
    QCheckBox*      m_txEqEnable = nullptr;
    QSpinBox*       m_txEqCurveDeg = nullptr;
    EqCurvePlot*    m_txEqPlot = nullptr;
    QCheckBox*      m_cfcEnable = nullptr;
    QCheckBox*      m_cfcPeqEnable = nullptr;
    QDoubleSpinBox* m_cfcPrecomp = nullptr;
    QDoubleSpinBox* m_cfcPrePeq = nullptr;
    QSpinBox*       m_cfcCurveDeg = nullptr;
    EqCurvePlot*    m_cfcCompPlot = nullptr;
    EqCurvePlot*    m_cfcPeqPlot = nullptr;
    int             m_currentReceiver;
    std::function<QString(int)> m_codec2ModeStringResolver;
};

#endif // AUDIO_DIALOG_H
