#ifndef AUDIOCONFIG_H
#define AUDIOCONFIG_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QVector>

class QSettings;

class AudioConfig : public QObject {
    Q_OBJECT
    Q_PROPERTY(int micSource READ micSource WRITE setMicSource NOTIFY micSourceChanged)
    Q_PROPERTY(int micInputDev READ micInputDev WRITE setMicInputDev NOTIFY micInputDevChanged)
    Q_PROPERTY(int digitalAudioInputDev READ digitalAudioInputDev WRITE setDigitalAudioInputDev NOTIFY digitalAudioInputDevChanged)
    Q_PROPERTY(QString micInputSourceName READ micInputSourceName WRITE setMicInputSourceName NOTIFY micInputSourceNameChanged)
    Q_PROPERTY(QString digitalInputSourceName READ digitalInputSourceName WRITE setDigitalInputSourceName NOTIFY digitalInputSourceNameChanged)
    Q_PROPERTY(double micGain READ micGain WRITE setMicGain NOTIFY micGainChanged)
    Q_PROPERTY(int driveLevel READ driveLevel WRITE setDriveLevel NOTIFY driveLevelChanged)
    Q_PROPERTY(int fmPreemphasis READ fmPreemphasis WRITE setFmPreemphasis NOTIFY fmPreemphasisChanged)
    Q_PROPERTY(int phaseRotator READ phaseRotator WRITE setPhaseRotator NOTIFY phaseRotatorChanged)
    Q_PROPERTY(bool phaseRotatorAuto READ phaseRotatorAuto WRITE setPhaseRotatorAuto NOTIFY phaseRotatorAutoChanged)
    Q_PROPERTY(double amCarrierLevel READ amCarrierLevel WRITE setAmCarrierLevel NOTIFY amCarrierLevelChanged)
    Q_PROPERTY(int audioCompression READ audioCompression WRITE setAudioCompression NOTIFY audioCompressionChanged)
    Q_PROPERTY(double fmDeviation READ fmDeviation WRITE setFmDeviation NOTIFY fmDeviationChanged)
    Q_PROPERTY(int ctcssToneHz READ ctcssToneHz WRITE setCtcssToneHz NOTIFY ctcssToneHzChanged)
    Q_PROPERTY(bool rxEqEnabled READ rxEqEnabled WRITE setRxEqEnabled NOTIFY rxEqEnabledChanged)
    Q_PROPERTY(bool txEqEnabled READ txEqEnabled WRITE setTxEqEnabled NOTIFY txEqEnabledChanged)
    Q_PROPERTY(float mainVolume READ mainVolume WRITE setMainVolume NOTIFY mainVolumeChanged)

public:
    static constexpr int kCfcBands = 10;
    static constexpr int kEqDrawPoints = 1024;

    explicit AudioConfig(QObject *parent = nullptr);

    int micSource() const { return m_micSource; }
    void setMicSource(int source);

    int micInputDev() const { return m_micInputDev; }
    void setMicInputDev(int dev);

    int digitalAudioInputDev() const { return m_digitalAudioInputDev; }
    void setDigitalAudioInputDev(int dev);

    QString micInputSourceName() const { return m_micInputSourceName; }
    void setMicInputSourceName(const QString &name);

    QString digitalInputSourceName() const { return m_digitalInputSourceName; }
    void setDigitalInputSourceName(const QString &name);

    double micGain() const { return m_micGain; }
    void setMicGain(double gain);

    int driveLevel() const { return m_driveLevel; }
    void setDriveLevel(int level);

    int fmPreemphasis() const { return m_fmPreemphasis; }
    void setFmPreemphasis(int val);

    int phaseRotator() const { return m_phaseRotator; }
    void setPhaseRotator(int val);

    bool phaseRotatorAuto() const { return m_phaseRotatorAuto; }
    void setPhaseRotatorAuto(bool enabled);

    double amCarrierLevel() const { return m_amCarrierLevel; }
    void setAmCarrierLevel(double level);

    int audioCompression() const { return m_audioCompression; }
    void setAudioCompression(int val);

    double fmDeviation() const { return m_fmDeviation; }
    void setFmDeviation(double dev);

    /** CTCSS tone in Hz; 0 = disabled. */
    int ctcssToneHz() const { return m_ctcssToneHz; }
    void setCtcssToneHz(int hz);

    bool rxEqEnabled() const { return m_rxEqEnabled; }
    void setRxEqEnabled(bool enabled);

    /** 11 gains for SetRXAGrphEQ10 (preamp + 10 bands), dB. */
    QVector<int> rxEqBands() const { return m_rxEqBands; }
    void setRxEqBands(const QVector<int> &bands);
    void setRxEqBand(int index, int gainDb);

    /** 0 = classic linear GrphEQ10, >0 = NURBS degree for SetRXAEQCurve. */
    int rxEqCurveDeg() const { return m_rxEqCurveDeg; }
    void setRxEqCurveDeg(int deg);

    bool txEqEnabled() const { return m_txEqEnabled; }
    void setTxEqEnabled(bool enabled);

    /** 11 gains for SetTXAGrphEQ10 (preamp + 10 bands), dB. */
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

    /** Fixed voice-band CFC frequencies (Hz), length kCfcBands. */
    QVector<double> cfcFreqs() const { return m_cfcFreqs; }
    QVector<double> cfcLevels() const { return m_cfcLevels; }
    QVector<double> cfcPost() const { return m_cfcPost; }
    void setCfcLevel(int index, double db);
    void setCfcPost(int index, double db);

    /** 0 = stock profile path; >0 enables Comp/Peq NURBS curves. */
    int cfcCurveDeg() const { return m_cfcCurveDeg; }
    void setCfcCurveDeg(int deg);

    bool emnrPost2Enabled() const { return m_emnrPost2Enabled; }
    void setEmnrPost2Enabled(bool enabled);
    double emnrPost2Factor() const { return m_emnrPost2Factor; }
    void setEmnrPost2Factor(double pct);
    double emnrPost2Nlevel() const { return m_emnrPost2Nlevel; }
    void setEmnrPost2Nlevel(double pct);
    double emnrPost2Taper() const { return m_emnrPost2Taper; }
    void setEmnrPost2Taper(double pct);
    double emnrPost2Rate() const { return m_emnrPost2Rate; }
    void setEmnrPost2Rate(double seconds);

    float mainVolume() const { return m_mainVolume; }
    void setMainVolume(float vol);

    void load(const QJsonObject &json);
    void save(QJsonObject &json) const;

    void loadIni(QSettings *settings);
    void saveIni(QSettings *settings) const;

signals:
    void micSourceChanged(int source);
    void micInputDevChanged(int dev);
    void digitalAudioInputDevChanged(int dev);
    void micInputSourceNameChanged(const QString &name);
    void digitalInputSourceNameChanged(const QString &name);
    void micGainChanged(double gain);
    void driveLevelChanged(int level);
    void fmPreemphasisChanged(int val);
    void phaseRotatorChanged(int val);
    void phaseRotatorAutoChanged(bool enabled);
    void amCarrierLevelChanged(double level);
    void audioCompressionChanged(int val);
    void fmDeviationChanged(double dev);
    void ctcssToneHzChanged(int hz);
    void rxEqEnabledChanged(bool enabled);
    void rxEqBandsChanged();
    void rxEqCurveDegChanged(int deg);
    void txEqEnabledChanged(bool enabled);
    void txEqBandsChanged();
    void txEqCurveDegChanged(int deg);
    void cfcChanged();
    void emnrPost2Changed();
    void mainVolumeChanged(float vol);

private:
    void ensureCfcDefaults();

    int m_micSource;
    int m_micInputDev;
    int m_digitalAudioInputDev;
    QString m_micInputSourceName;
    QString m_digitalInputSourceName;
    double m_micGain;
    int m_driveLevel;
    int m_fmPreemphasis;
    int m_phaseRotator;
    bool m_phaseRotatorAuto;
    double m_amCarrierLevel;
    int m_audioCompression;
    double m_fmDeviation;
    int m_ctcssToneHz;
    bool m_rxEqEnabled;
    QVector<int> m_rxEqBands;
    int m_rxEqCurveDeg;
    bool m_txEqEnabled;
    QVector<int> m_txEqBands;
    int m_txEqCurveDeg;
    bool m_cfcEnabled;
    bool m_cfcPeqEnabled;
    double m_cfcPrecomp;
    double m_cfcPrePeq;
    QVector<double> m_cfcFreqs;
    QVector<double> m_cfcLevels;
    QVector<double> m_cfcPost;
    int m_cfcCurveDeg;
    bool m_emnrPost2Enabled;
    double m_emnrPost2Factor;
    double m_emnrPost2Nlevel;
    double m_emnrPost2Taper;
    double m_emnrPost2Rate;
    float m_mainVolume;
};

#endif // AUDIOCONFIG_H
