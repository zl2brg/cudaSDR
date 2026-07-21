#pragma once

#include <QObject>
#include <QAudioSink>
#include <QAudioDevice>
#include <QAudioFormat>
#include <QIODevice>
#include <QMutex>
#include <QVector>
#include <QByteArray>
#include <QTimer>

class ReceiverAudioOutput : public QObject
{
    Q_OBJECT
public:
    explicit ReceiverAudioOutput(QObject *parent = nullptr);
    ~ReceiverAudioOutput();

    void start();
    void stop();
    void writeAudio(const QVector<float>& audioBuffer);

    void setSampleRate(int rate);

private slots:
    void onAudioOutputsChanged();
    void onSinkStateChanged(QAudio::State state);
    void reopenOutput();

private:
    bool isSinkHealthy() const;
    void handleDeviceLostLocked(const char *reason);
    void openSinkLocked();
    void startLocked();
    void stopLocked();

    QAudioSink* m_audioSink = nullptr;
    QIODevice* m_device = nullptr;
    QAudioFormat m_format;
    int m_sampleRate = 48000;
    QMutex m_mutex;
    QByteArray m_pending;  // carry-forward for partial writes
    bool m_wantRunning = false;
    bool m_reopenPending = false;
    QTimer m_reopenTimer;
};
