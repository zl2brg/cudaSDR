#ifndef TRANSMITMODEL_H
#define TRANSMITMODEL_H

#include <QObject>
#include <QString>
#include <QVector>

class TransmitModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(int amCarrierLevel READ amCarrierLevel WRITE setAmCarrierLevel NOTIFY amCarrierLevelChanged)
    Q_PROPERTY(int audioCompression READ audioCompression WRITE setAudioCompression NOTIFY audioCompressionChanged)
    Q_PROPERTY(int fmDeviation READ fmDeviation WRITE setFmDeviation NOTIFY fmDeviationChanged)
    Q_PROPERTY(bool fmPreEmphasis READ fmPreEmphasis WRITE setFmPreEmphasis NOTIFY fmPreEmphasisChanged)
    Q_PROPERTY(bool phaseRotator READ phaseRotator WRITE setPhaseRotator NOTIFY phaseRotatorChanged)
    Q_PROPERTY(bool phaseRotatorAuto READ phaseRotatorAuto WRITE setPhaseRotatorAuto NOTIFY phaseRotatorAutoChanged)
    Q_PROPERTY(QString phaseRotatorStatus READ phaseRotatorStatus WRITE setPhaseRotatorStatus NOTIFY phaseRotatorStatusChanged)
    Q_PROPERTY(int ctcssToneHz READ ctcssToneHz WRITE setCtcssToneHz NOTIFY ctcssToneHzChanged)
    Q_PROPERTY(bool txEqEnabled READ txEqEnabled WRITE setTxEqEnabled NOTIFY txEqChanged)
    Q_PROPERTY(QVector<int> txEqBands READ txEqBands WRITE setTxEqBands NOTIFY txEqChanged)
    Q_PROPERTY(int txEqCurveDeg READ txEqCurveDeg WRITE setTxEqCurveDeg NOTIFY txEqChanged)
    Q_PROPERTY(bool cfcEnabled READ cfcEnabled WRITE setCfcEnabled NOTIFY cfcChanged)
    Q_PROPERTY(bool cfcPeqEnabled READ cfcPeqEnabled WRITE setCfcPeqEnabled NOTIFY cfcChanged)
    Q_PROPERTY(double cfcPrecomp READ cfcPrecomp WRITE setCfcPrecomp NOTIFY cfcChanged)
    Q_PROPERTY(double cfcPrePeq READ cfcPrePeq WRITE setCfcPrePeq NOTIFY cfcChanged)
    Q_PROPERTY(int cfcCurveDeg READ cfcCurveDeg WRITE setCfcCurveDeg NOTIFY cfcChanged)
    Q_PROPERTY(QVector<double> cfcLevels READ cfcLevels WRITE setCfcLevels NOTIFY cfcChanged)
    Q_PROPERTY(QVector<double> cfcPost READ cfcPost WRITE setCfcPost NOTIFY cfcChanged)
    Q_PROPERTY(int micInputDev READ micInputDev WRITE setMicInputDev NOTIFY micInputDevChanged)
    Q_PROPERTY(QString micInputSourceName READ micInputSourceName WRITE setMicInputSourceName NOTIFY micInputSourceNameChanged)
    Q_PROPERTY(int digitalAudioInputDev READ digitalAudioInputDev WRITE setDigitalAudioInputDev NOTIFY digitalAudioInputDevChanged)
    Q_PROPERTY(QString digitalInputSourceName READ digitalInputSourceName WRITE setDigitalInputSourceName NOTIFY digitalInputSourceNameChanged)
    Q_PROPERTY(int cwKeyerMode READ cwKeyerMode WRITE setCwKeyerMode NOTIFY cwKeyerModeChanged)
    Q_PROPERTY(bool internalCw READ internalCw WRITE setInternalCw NOTIFY internalCwChanged)
    Q_PROPERTY(bool cwKeyReversed READ cwKeyReversed WRITE setCwKeyReversed NOTIFY cwKeyReversedChanged)
    Q_PROPERTY(bool cwKeyerSpacing READ cwKeyerSpacing WRITE setCwKeyerSpacing NOTIFY cwKeyerSpacingChanged)
    Q_PROPERTY(int cwKeyerSpeed READ cwKeyerSpeed WRITE setCwKeyerSpeed NOTIFY cwKeyerSpeedChanged)
    Q_PROPERTY(int cwPttDelay READ cwPttDelay WRITE setCwPttDelay NOTIFY cwPttDelayChanged)
    Q_PROPERTY(int cwSidetoneFreq READ cwSidetoneFreq WRITE setCwSidetoneFreq NOTIFY cwSidetoneFreqChanged)
    Q_PROPERTY(int cwSidetoneVolume READ cwSidetoneVolume WRITE setCwSidetoneVolume NOTIFY cwSidetoneVolumeChanged)
    Q_PROPERTY(int cwHangTime READ cwHangTime WRITE setCwHangTime NOTIFY cwHangTimeChanged)
    Q_PROPERTY(int cwKeyerWeight READ cwKeyerWeight WRITE setCwKeyerWeight NOTIFY cwKeyerWeightChanged)

public:
    explicit TransmitModel(QObject *parent = nullptr);

    int amCarrierLevel() const { return m_amCarrierLevel; }
    void setAmCarrierLevel(int level);

    int audioCompression() const { return m_audioCompression; }
    void setAudioCompression(int val);

    int fmDeviation() const { return m_fmDeviation; }
    void setFmDeviation(int val);

    bool fmPreEmphasis() const { return m_fmPreEmphasis; }
    void setFmPreEmphasis(bool enabled);

    bool phaseRotator() const { return m_phaseRotator; }
    void setPhaseRotator(bool enabled);

    bool phaseRotatorAuto() const { return m_phaseRotatorAuto; }
    void setPhaseRotatorAuto(bool enabled);

    QString phaseRotatorStatus() const { return m_phaseRotatorStatus; }
    void setPhaseRotatorStatus(const QString &status);

    int ctcssToneHz() const { return m_ctcssToneHz; }
    void setCtcssToneHz(int hz);

    bool txEqEnabled() const { return m_txEqEnabled; }
    void setTxEqEnabled(bool enabled);

    QVector<int> txEqBands() const { return m_txEqBands; }
    void setTxEqBands(const QVector<int> &bands);
    void setTxEqBand(int index, int gainDb);

    int txEqCurveDeg() const { return m_txEqCurveDeg; }
    void setTxEqCurveDeg(int deg);

    bool cfcEnabled() const { return m_cfcEnabled; }
    void setCfcEnabled(bool enabled);

    bool cfcPeqEnabled() const { return m_cfcPeqEnabled; }
    void setCfcPeqEnabled(bool enabled);

    double cfcPrecomp() const { return m_cfcPrecomp; }
    void setCfcPrecomp(double db);

    double cfcPrePeq() const { return m_cfcPrePeq; }
    void setCfcPrePeq(double db);

    int cfcCurveDeg() const { return m_cfcCurveDeg; }
    void setCfcCurveDeg(int deg);

    QVector<double> cfcLevels() const { return m_cfcLevels; }
    void setCfcLevels(const QVector<double> &levels);
    void setCfcLevel(int index, double db);

    QVector<double> cfcPost() const { return m_cfcPost; }
    void setCfcPost(const QVector<double> &post);
    void setCfcPostBand(int index, double db);

    int micInputDev() const { return m_micInputDev; }
    void setMicInputDev(int dev);

    QString micInputSourceName() const { return m_micInputSourceName; }
    void setMicInputSourceName(const QString &name);

    int digitalAudioInputDev() const { return m_digitalAudioInputDev; }
    void setDigitalAudioInputDev(int dev);

    QString digitalInputSourceName() const { return m_digitalInputSourceName; }
    void setDigitalInputSourceName(const QString &name);

    int cwKeyerMode() const { return m_cwKeyerMode; }
    void setCwKeyerMode(int val);

    bool internalCw() const { return m_internalCw; }
    void setInternalCw(bool val);

    bool cwKeyReversed() const { return m_cwKeyReversed; }
    void setCwKeyReversed(bool val);

    bool cwKeyerSpacing() const { return m_cwKeyerSpacing; }
    void setCwKeyerSpacing(bool val);

    int cwKeyerSpeed() const { return m_cwKeyerSpeed; }
    void setCwKeyerSpeed(int val);

    int cwPttDelay() const { return m_cwPttDelay; }
    void setCwPttDelay(int val);

    int cwSidetoneFreq() const { return m_cwSidetoneFreq; }
    void setCwSidetoneFreq(int val);

    int cwSidetoneVolume() const { return m_cwSidetoneVolume; }
    void setCwSidetoneVolume(int val);

    int cwHangTime() const { return m_cwHangTime; }
    void setCwHangTime(int val);

    int cwKeyerWeight() const { return m_cwKeyerWeight; }
    void setCwKeyerWeight(int val);

signals:
    void amCarrierLevelChanged(int level);
    void audioCompressionChanged(int val);
    void fmDeviationChanged(int val);
    void fmPreEmphasisChanged(bool enabled);
    void phaseRotatorChanged(bool enabled);
    void phaseRotatorAutoChanged(bool enabled);
    void phaseRotatorStatusChanged(const QString &status);
    void phaseRotatorAutoResetRequested();
    void ctcssToneHzChanged(int hz);
    void txEqChanged();
    void cfcChanged();
    void micInputDevChanged(int dev);
    void micInputSourceNameChanged(const QString &name);
    void digitalAudioInputDevChanged(int dev);
    void digitalInputSourceNameChanged(const QString &name);
    void cwKeyerModeChanged(int val);
    void internalCwChanged(bool val);
    void cwKeyReversedChanged(bool val);
    void cwKeyerSpacingChanged(bool val);
    void cwKeyerSpeedChanged(int val);
    void cwPttDelayChanged(int val);
    void cwSidetoneFreqChanged(int val);
    void cwSidetoneVolumeChanged(int val);
    void cwHangTimeChanged(int val);
    void cwKeyerWeightChanged(int val);

private:
    int m_amCarrierLevel = 100;
    int m_audioCompression = 0;
    int m_fmDeviation = 5000;
    bool m_fmPreEmphasis = false;
    bool m_phaseRotator = false;
    bool m_phaseRotatorAuto = false;
    QString m_phaseRotatorStatus;
    int m_ctcssToneHz = 0;

    bool m_txEqEnabled = false;
    QVector<int> m_txEqBands{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int m_txEqCurveDeg = 0;

    bool m_cfcEnabled = false;
    bool m_cfcPeqEnabled = false;
    double m_cfcPrecomp = 0.0;
    double m_cfcPrePeq = 0.0;
    int m_cfcCurveDeg = 0;
    QVector<double> m_cfcLevels{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    QVector<double> m_cfcPost{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

    int m_micInputDev = 0;
    QString m_micInputSourceName;
    int m_digitalAudioInputDev = 0;
    QString m_digitalInputSourceName;

    int m_cwKeyerMode = 2;
    bool m_internalCw = true;
    bool m_cwKeyReversed = false;
    bool m_cwKeyerSpacing = false;
    int m_cwKeyerSpeed = 20;
    int m_cwPttDelay = 20;
    int m_cwSidetoneFreq = 700;
    int m_cwSidetoneVolume = 50;
    int m_cwHangTime = 500;
    int m_cwKeyerWeight = 50;
};

#endif // TRANSMITMODEL_H
