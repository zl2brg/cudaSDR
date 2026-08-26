#ifndef FREEDVCONFIG_H
#define FREEDVCONFIG_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>

class QSettings;

class FreeDVConfig : public QObject {
    Q_OBJECT
    Q_PROPERTY(int defaultMode READ defaultMode WRITE setDefaultMode NOTIFY defaultModeChanged)
    Q_PROPERTY(float sqThreshold READ sqThreshold WRITE setSqThreshold NOTIFY sqThresholdChanged)
    Q_PROPERTY(bool autoSync READ autoSync WRITE setAutoSync NOTIFY autoSyncChanged)
    Q_PROPERTY(bool clipAudio READ clipAudio WRITE setClipAudio NOTIFY clipAudioChanged)
    Q_PROPERTY(bool txBandpass READ txBandpass WRITE setTxBandpass NOTIFY txBandpassChanged)

public:
    static constexpr int kMaxReceivers = 8;

    explicit FreeDVConfig(QObject *parent = nullptr);

    int defaultMode() const { return m_defaultMode; }
    void setDefaultMode(int mode);

    float sqThreshold() const { return m_sqThreshold; }
    void setSqThreshold(float threshold);

    bool autoSync() const { return m_autoSync; }
    void setAutoSync(bool enabled);

    bool clipAudio() const { return m_clipAudio; }
    void setClipAudio(bool enabled);

    bool txBandpass() const { return m_txBandpass; }
    void setTxBandpass(bool enabled);

    int rxMode(int rx) const;
    void setRxMode(int rx, int mode);

    QList<int> rxModes() const { return m_rxModes; }
    void setRxModes(const QList<int> &modes);

    void load(const QJsonObject &json);
    void save(QJsonObject &json) const;

    void loadIni(QSettings *settings);
    void saveIni(QSettings *settings) const;

signals:
    void defaultModeChanged(int mode);
    void sqThresholdChanged(float threshold);
    void autoSyncChanged(bool enabled);
    void clipAudioChanged(bool enabled);
    void txBandpassChanged(bool enabled);
    void rxModeChanged(int rx, int mode);

private:
    int m_defaultMode;
    float m_sqThreshold;
    bool m_autoSync;
    bool m_clipAudio;
    bool m_txBandpass;
    QList<int> m_rxModes;
};

#endif // FREEDVCONFIG_H
