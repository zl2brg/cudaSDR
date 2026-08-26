#ifndef AUDIOCONFIG_H
#define AUDIOCONFIG_H

#include <QObject>
#include <QJsonObject>
#include <QVector>

class QSettings;

class AudioConfig : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool rxEqEnabled READ rxEqEnabled WRITE setRxEqEnabled NOTIFY rxEqEnabledChanged)
    Q_PROPERTY(float mainVolume READ mainVolume WRITE setMainVolume NOTIFY mainVolumeChanged)

public:
    static constexpr int kEqDrawPoints = 1024;

    explicit AudioConfig(QObject *parent = nullptr);

    bool rxEqEnabled() const { return m_rxEqEnabled; }
    void setRxEqEnabled(bool enabled);

    /** 11 gains for SetRXAGrphEQ10 (preamp + 10 bands), dB. */
    QVector<int> rxEqBands() const { return m_rxEqBands; }
    void setRxEqBands(const QVector<int> &bands);
    void setRxEqBand(int index, int gainDb);

    /** 0 = classic linear GrphEQ10, >0 = NURBS degree for SetRXAEQCurve. */
    int rxEqCurveDeg() const { return m_rxEqCurveDeg; }
    void setRxEqCurveDeg(int deg);

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
    void rxEqEnabledChanged(bool enabled);
    void rxEqBandsChanged();
    void rxEqCurveDegChanged(int deg);
    void emnrPost2Changed();
    void mainVolumeChanged(float vol);

private:
    bool m_rxEqEnabled;
    QVector<int> m_rxEqBands;
    int m_rxEqCurveDeg;
    bool m_emnrPost2Enabled;
    double m_emnrPost2Factor;
    double m_emnrPost2Nlevel;
    double m_emnrPost2Taper;
    double m_emnrPost2Rate;
    float m_mainVolume;
};

#endif // AUDIOCONFIG_H
