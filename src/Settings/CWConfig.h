#ifndef CWCONFIG_H
#define CWCONFIG_H

#include <QObject>
#include <QJsonObject>

class CWConfig : public QObject {
    Q_OBJECT
    Q_PROPERTY(int internalCw READ internalCw WRITE setInternalCw NOTIFY internalCwChanged)
    Q_PROPERTY(int keyReversed READ keyReversed WRITE setKeyReversed NOTIFY keyReversedChanged)
    Q_PROPERTY(int keyerSpacing READ keyerSpacing WRITE setKeyerSpacing NOTIFY keyerSpacingChanged)
    Q_PROPERTY(int keyerSpeed READ keyerSpeed WRITE setKeyerSpeed NOTIFY keyerSpeedChanged)
    Q_PROPERTY(int keyerMode READ keyerMode WRITE setKeyerMode NOTIFY keyerModeChanged)
    Q_PROPERTY(int sidetoneVolume READ sidetoneVolume WRITE setSidetoneVolume NOTIFY sidetoneVolumeChanged)
    Q_PROPERTY(int sidetoneFreq READ sidetoneFreq WRITE setSidetoneFreq NOTIFY sidetoneFreqChanged)
    Q_PROPERTY(int pttDelay READ pttDelay WRITE setPttDelay NOTIFY pttDelayChanged)
    Q_PROPERTY(int hangTime READ hangTime WRITE setHangTime NOTIFY hangTimeChanged)
    Q_PROPERTY(int keyerWeight READ keyerWeight WRITE setKeyerWeight NOTIFY keyerWeightChanged)

public:
    explicit CWConfig(QObject *parent = nullptr);

    int internalCw() const { return m_internal_cw; }
    void setInternalCw(int val);

    int keyReversed() const { return m_key_reversed; }
    void setKeyReversed(int val);

    int keyerSpacing() const { return m_keyer_spacing; }
    void setKeyerSpacing(int val);

    int keyerSpeed() const { return m_keyer_speed; }
    void setKeyerSpeed(int val);

    int keyerMode() const { return m_keyer_mode; }
    void setKeyerMode(int val);

    int sidetoneVolume() const { return m_sidetone_volume; }
    void setSidetoneVolume(int val);

    int sidetoneFreq() const { return m_sidetone_freq; }
    void setSidetoneFreq(int val);

    int pttDelay() const { return m_ptt_delay; }
    void setPttDelay(int val);

    int hangTime() const { return m_hang_time; }
    void setHangTime(int val);

    int keyerWeight() const { return m_keyer_weight; }
    void setKeyerWeight(int val);

    void load(const QJsonObject &json);
    void save(QJsonObject &json) const;

signals:
    void internalCwChanged(int val);
    void keyReversedChanged(int val);
    void keyerSpacingChanged(int val);
    void keyerSpeedChanged(int val);
    void keyerModeChanged(int val);
    void sidetoneVolumeChanged(int val);
    void sidetoneFreqChanged(int val);
    void pttDelayChanged(int val);
    void hangTimeChanged(int val);
    void keyerWeightChanged(int val);

private:
    int m_internal_cw;
    int m_key_reversed;
    int m_keyer_spacing;
    int m_keyer_speed;
    int m_keyer_mode;
    int m_sidetone_volume;
    int m_sidetone_freq;
    int m_ptt_delay;
    int m_hang_time;
    int m_keyer_weight;
};

#endif // CWCONFIG_H
