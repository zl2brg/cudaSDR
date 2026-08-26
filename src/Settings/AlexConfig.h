#ifndef ALEXCONFIG_H
#define ALEXCONFIG_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>
#include <QStringList>

class QSettings;

class AlexConfig : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool manualFilterSelect READ manualFilterSelect WRITE setManualFilterSelect NOTIFY manualFilterSelectChanged)
    Q_PROPERTY(bool bypassAll READ bypassAll WRITE setBypassAll NOTIFY bypassAllChanged)
    Q_PROPERTY(bool amp6m READ amp6m WRITE setAmp6m NOTIFY amp6mChanged)
    Q_PROPERTY(bool hpf1_5MHz READ hpf1_5MHz WRITE setHpf1_5MHz NOTIFY hpf1_5MHzChanged)
    Q_PROPERTY(bool hpf6_5MHz READ hpf6_5MHz WRITE setHpf6_5MHz NOTIFY hpf6_5MHzChanged)
    Q_PROPERTY(bool hpf9_5MHz READ hpf9_5MHz WRITE setHpf9_5MHz NOTIFY hpf9_5MHzChanged)
    Q_PROPERTY(bool hpf13MHz READ hpf13MHz WRITE setHpf13MHz NOTIFY hpf13MHzChanged)
    Q_PROPERTY(bool hpf20MHz READ hpf20MHz WRITE setHpf20MHz NOTIFY hpf20MHzChanged)
    Q_PROPERTY(quint16 alexConfig READ alexConfig WRITE setAlexConfig NOTIFY alexConfigChanged)
    Q_PROPERTY(int attenuation READ attenuation WRITE setAttenuation NOTIFY attenuationChanged)

public:
    static constexpr int kHpfFilterCount = 6;
    static constexpr int kLpfFilterCount = 7;
    static constexpr int kAlexStateCount = 22;

    /** KISS Konsole encoding: RX/TX antenna selectors of 0 are invalid. */
    static int normalizedState(int state);

    explicit AlexConfig(QObject *parent = nullptr);

    bool manualFilterSelect() const { return m_manualFilterSelect; }
    void setManualFilterSelect(bool manual);

    bool bypassAll() const { return m_bypassAll; }
    void setBypassAll(bool bypass);

    bool amp6m() const { return m_amp6m; }
    void setAmp6m(bool enabled);

    bool hpf1_5MHz() const { return m_hpf1_5MHz; }
    void setHpf1_5MHz(bool enabled);

    bool hpf6_5MHz() const { return m_hpf6_5MHz; }
    void setHpf6_5MHz(bool enabled);

    bool hpf9_5MHz() const { return m_hpf9_5MHz; }
    void setHpf9_5MHz(bool enabled);

    bool hpf13MHz() const { return m_hpf13MHz; }
    void setHpf13MHz(bool enabled);

    bool hpf20MHz() const { return m_hpf20MHz; }
    void setHpf20MHz(bool enabled);

    bool lpf160m() const { return m_lpf160m; }
    void setLpf160m(bool enabled);
    bool lpf80m() const { return m_lpf80m; }
    void setLpf80m(bool enabled);
    bool lpf60_40m() const { return m_lpf60_40m; }
    void setLpf60_40m(bool enabled);
    bool lpf30_20m() const { return m_lpf30_20m; }
    void setLpf30_20m(bool enabled);
    bool lpf17_15m() const { return m_lpf17_15m; }
    void setLpf17_15m(bool enabled);
    bool lpf12_10m() const { return m_lpf12_10m; }
    void setLpf12_10m(bool enabled);
    bool lpf6m() const { return m_lpf6m; }
    void setLpf6m(bool enabled);

    quint16 alexConfig() const { return m_alexConfig; }
    void setAlexConfig(quint16 config);

    int attenuation() const { return m_attenuation; }
    void setAttenuation(int attn);

    QList<int> alexStates() const { return m_alexStates; }
    void setAlexStates(const QList<int> &states);
    void setAlexState(int pos, int value);

    QList<long> hpfLoFrequencies() const { return m_hpfLoFreqs; }
    QList<long> hpfHiFrequencies() const { return m_hpfHiFreqs; }
    void setHpfLoFrequency(int filter, long freq);
    void setHpfHiFrequency(int filter, long freq);

    QList<long> lpfLoFrequencies() const { return m_lpfLoFreqs; }
    QList<long> lpfHiFrequencies() const { return m_lpfHiFreqs; }
    void setLpfLoFrequency(int filter, long freq);
    void setLpfHiFrequency(int filter, long freq);

    void load(const QJsonObject &json);
    void save(QJsonObject &json) const;

    void loadIni(QSettings *settings);
    void saveIni(QSettings *settings) const;

    void loadStates(QSettings *settings, const QStringList &bandKeys);
    void saveStates(QSettings *settings, const QStringList &bandKeys) const;

signals:
    void manualFilterSelectChanged(bool manual);
    void bypassAllChanged(bool bypass);
    void amp6mChanged(bool enabled);
    void hpf1_5MHzChanged(bool enabled);
    void hpf6_5MHzChanged(bool enabled);
    void hpf9_5MHzChanged(bool enabled);
    void hpf13MHzChanged(bool enabled);
    void hpf20MHzChanged(bool enabled);
    void lpf160mChanged(bool enabled);
    void lpf80mChanged(bool enabled);
    void lpf60_40mChanged(bool enabled);
    void lpf30_20mChanged(bool enabled);
    void lpf17_15mChanged(bool enabled);
    void lpf12_10mChanged(bool enabled);
    void lpf6mChanged(bool enabled);
    void alexConfigChanged(quint16 config);
    void attenuationChanged(int attn);
    void alexStatesChanged();
    void hpfFrequenciesChanged();
    void lpfFrequenciesChanged();

private:
    void updateBitmask();
    void applyBitmask(quint16 config, bool emitFlags);
    void setFlag(bool &field, bool enabled, void (AlexConfig::*signal)(bool));

    bool m_manualFilterSelect;
    bool m_bypassAll;
    bool m_amp6m;
    bool m_hpf1_5MHz;
    bool m_hpf6_5MHz;
    bool m_hpf9_5MHz;
    bool m_hpf13MHz;
    bool m_hpf20MHz;
    bool m_lpf160m;
    bool m_lpf80m;
    bool m_lpf60_40m;
    bool m_lpf30_20m;
    bool m_lpf17_15m;
    bool m_lpf12_10m;
    bool m_lpf6m;
    quint16 m_alexConfig;
    int m_attenuation;

    QList<int> m_alexStates;
    QList<long> m_hpfLoFreqs;
    QList<long> m_hpfHiFreqs;
    QList<long> m_lpfLoFreqs;
    QList<long> m_lpfHiFreqs;
};

#endif // ALEXCONFIG_H
