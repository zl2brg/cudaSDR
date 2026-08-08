#ifndef AUDIOCONFIG_H
#define AUDIOCONFIG_H

#include <QObject>
#include <QString>
#include <QJsonObject>

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
    Q_PROPERTY(double amCarrierLevel READ amCarrierLevel WRITE setAmCarrierLevel NOTIFY amCarrierLevelChanged)
    Q_PROPERTY(int audioCompression READ audioCompression WRITE setAudioCompression NOTIFY audioCompressionChanged)
    Q_PROPERTY(double fmDeviation READ fmDeviation WRITE setFmDeviation NOTIFY fmDeviationChanged)
    Q_PROPERTY(float mainVolume READ mainVolume WRITE setMainVolume NOTIFY mainVolumeChanged)

public:
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

    double amCarrierLevel() const { return m_amCarrierLevel; }
    void setAmCarrierLevel(double level);

    int audioCompression() const { return m_audioCompression; }
    void setAudioCompression(int val);

    double fmDeviation() const { return m_fmDeviation; }
    void setFmDeviation(double dev);

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
    void amCarrierLevelChanged(double level);
    void audioCompressionChanged(int val);
    void fmDeviationChanged(double dev);
    void mainVolumeChanged(float vol);

private:
    int m_micSource;
    int m_micInputDev;
    int m_digitalAudioInputDev;
    QString m_micInputSourceName;
    QString m_digitalInputSourceName;
    double m_micGain;
    int m_driveLevel;
    int m_fmPreemphasis;
    int m_phaseRotator;
    double m_amCarrierLevel;
    int m_audioCompression;
    double m_fmDeviation;
    float m_mainVolume;
};

#endif // AUDIOCONFIG_H
