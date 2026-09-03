#ifndef TRANSMITCONFIG_H
#define TRANSMITCONFIG_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>

class QSettings;

class TransmitConfig : public QObject {
    Q_OBJECT
    Q_PROPERTY(int micSource READ micSource WRITE setMicSource NOTIFY micSourceChanged)
    Q_PROPERTY(int micInputDev READ micInputDev WRITE setMicInputDev NOTIFY micInputDevChanged)
    Q_PROPERTY(int digitalAudioInputDev READ digitalAudioInputDev WRITE setDigitalAudioInputDev NOTIFY digitalAudioInputDevChanged)
    Q_PROPERTY(QString micInputSourceName READ micInputSourceName WRITE setMicInputSourceName NOTIFY micInputSourceNameChanged)
    Q_PROPERTY(QString digitalInputSourceName READ digitalInputSourceName WRITE setDigitalInputSourceName NOTIFY digitalInputSourceNameChanged)
    Q_PROPERTY(double micGain READ micGain WRITE setMicGain NOTIFY micGainChanged)
    Q_PROPERTY(int driveLevel READ driveLevel WRITE setDriveLevel NOTIFY driveLevelChanged)
    Q_PROPERTY(int tunePower READ tunePower WRITE setTunePower NOTIFY tunePowerChanged)
    Q_PROPERTY(bool paEnabled READ paEnabled WRITE setPaEnabled NOTIFY paEnabledChanged)
    Q_PROPERTY(int fmPreemphasis READ fmPreemphasis WRITE setFmPreemphasis NOTIFY fmPreemphasisChanged)
    Q_PROPERTY(int phaseRotator READ phaseRotator WRITE setPhaseRotator NOTIFY phaseRotatorChanged)
    Q_PROPERTY(bool phaseRotatorAuto READ phaseRotatorAuto WRITE setPhaseRotatorAuto NOTIFY phaseRotatorAutoChanged)
    Q_PROPERTY(QString phaseRotatorStatus READ phaseRotatorStatus WRITE setPhaseRotatorStatus NOTIFY phaseRotatorStatusChanged)
    Q_PROPERTY(double amCarrierLevel READ amCarrierLevel WRITE setAmCarrierLevel NOTIFY amCarrierLevelChanged)
    Q_PROPERTY(int audioCompression READ audioCompression WRITE setAudioCompression NOTIFY audioCompressionChanged)
    Q_PROPERTY(double fmDeviation READ fmDeviation WRITE setFmDeviation NOTIFY fmDeviationChanged)
    Q_PROPERTY(int ctcssToneHz READ ctcssToneHz WRITE setCtcssToneHz NOTIFY ctcssToneHzChanged)
    Q_PROPERTY(bool txEqEnabled READ txEqEnabled WRITE setTxEqEnabled NOTIFY txEqEnabledChanged)
    Q_PROPERTY(int txEqCurveDeg READ txEqCurveDeg WRITE setTxEqCurveDeg NOTIFY txEqCurveDegChanged)
    Q_PROPERTY(bool cfcEnabled READ cfcEnabled WRITE setCfcEnabled NOTIFY cfcEnabledChanged)
    Q_PROPERTY(bool cfcPeqEnabled READ cfcPeqEnabled WRITE setCfcPeqEnabled NOTIFY cfcPeqEnabledChanged)
    Q_PROPERTY(double cfcPrecomp READ cfcPrecomp WRITE setCfcPrecomp NOTIFY cfcPrecompChanged)
    Q_PROPERTY(double cfcPrePeq READ cfcPrePeq WRITE setCfcPrePeq NOTIFY cfcPrePeqChanged)
    Q_PROPERTY(int cfcCurveDeg READ cfcCurveDeg WRITE setCfcCurveDeg NOTIFY cfcCurveDegChanged)
    Q_PROPERTY(bool txFullDuplex READ txFullDuplex WRITE setTxFullDuplex NOTIFY txFullDuplexChanged)
    Q_PROPERTY(double repeaterOffset READ repeaterOffset WRITE setRepeaterOffset NOTIFY repeaterOffsetChanged)
    Q_PROPERTY(int txFilterLow READ txFilterLow WRITE setTxFilterLow NOTIFY txFilterLowChanged)
    Q_PROPERTY(int txFilterHigh READ txFilterHigh WRITE setTxFilterHigh NOTIFY txFilterHighChanged)
    Q_PROPERTY(bool txUseRxFilter READ txUseRxFilter WRITE setTxUseRxFilter NOTIFY txUseRxFilterChanged)

public:
    static constexpr int kCfcBands = 10;
    static constexpr int kEqBands = 11;

    explicit TransmitConfig(QObject *parent = nullptr);

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

    int tunePower() const { return m_tunePower; }
    void setTunePower(int power);

    bool paEnabled() const { return m_paEnabled; }
    void setPaEnabled(bool enabled);

    int fmPreemphasis() const { return m_fmPreemphasis; }
    void setFmPreemphasis(int val);

    int phaseRotator() const { return m_phaseRotator; }
    void setPhaseRotator(int val);

    bool phaseRotatorAuto() const { return m_phaseRotatorAuto; }
    void setPhaseRotatorAuto(bool enabled);

    QString phaseRotatorStatus() const { return m_phaseRotatorStatus; }
    void setPhaseRotatorStatus(const QString &status);

    double amCarrierLevel() const { return m_amCarrierLevel; }
    void setAmCarrierLevel(double level);

    int audioCompression() const { return m_audioCompression; }
    void setAudioCompression(int val);

    double fmDeviation() const { return m_fmDeviation; }
    void setFmDeviation(double dev);

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

    QVector<double> cfcFreqs() const { return m_cfcFreqs; }
    QVector<double> cfcLevels() const { return m_cfcLevels; }
    QVector<double> cfcPost() const { return m_cfcPost; }
    void setCfcLevels(const QVector<double> &levels);
    void setCfcLevel(int index, double db);
    void setCfcPost(const QVector<double> &post);
    void setCfcPostBand(int index, double db);

    int cfcCurveDeg() const { return m_cfcCurveDeg; }
    void setCfcCurveDeg(int deg);

    bool txFullDuplex() const { return m_txFullDuplex; }
    void setTxFullDuplex(bool fullDuplex);

    double repeaterOffset() const { return m_repeaterOffset; }
    void setRepeaterOffset(double offset);

    int txFilterLow() const { return m_txFilterLow; }
    void setTxFilterLow(int val);

    int txFilterHigh() const { return m_txFilterHigh; }
    void setTxFilterHigh(int val);

    bool txUseRxFilter() const { return m_txUseRxFilter; }
    void setTxUseRxFilter(bool enabled);

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
    void tunePowerChanged(int power);
    void paEnabledChanged(bool enabled);
    void fmPreemphasisChanged(int val);
    void phaseRotatorChanged(int val);
    void phaseRotatorAutoChanged(bool enabled);
    void phaseRotatorStatusChanged(const QString &status);
    void amCarrierLevelChanged(double level);
    void audioCompressionChanged(int val);
    void fmDeviationChanged(double dev);
    void ctcssToneHzChanged(int hz);
    void txEqEnabledChanged(bool enabled);
    void txEqBandsChanged();
    void txEqCurveDegChanged(int deg);
    void cfcEnabledChanged(bool enabled);
    void cfcPeqEnabledChanged(bool enabled);
    void cfcPrecompChanged(double db);
    void cfcPrePeqChanged(double db);
    void cfcCurveDegChanged(int deg);
    void cfcLevelsChanged();
    void cfcPostChanged();
    void txFullDuplexChanged(bool fullDuplex);
    void repeaterOffsetChanged(double offset);
    void txFilterLowChanged(int val);
    void txFilterHighChanged(int val);
    void txUseRxFilterChanged(bool enabled);

private:
    void ensureCfcDefaults();

    int m_micSource;
    int m_micInputDev;
    int m_digitalAudioInputDev;
    QString m_micInputSourceName;
    QString m_digitalInputSourceName;
    double m_micGain;
    int m_driveLevel;
    int m_tunePower;
    bool m_paEnabled;
    int m_fmPreemphasis;
    int m_phaseRotator;
    bool m_phaseRotatorAuto;
    QString m_phaseRotatorStatus;
    double m_amCarrierLevel;
    int m_audioCompression;
    double m_fmDeviation;
    int m_ctcssToneHz;
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
    bool m_txFullDuplex;
    double m_repeaterOffset;
    int m_txFilterLow;
    int m_txFilterHigh;
    bool m_txUseRxFilter;
};

#endif // TRANSMITCONFIG_H
