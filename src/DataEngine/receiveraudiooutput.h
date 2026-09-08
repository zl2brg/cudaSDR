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
#include <atomic>

#include "Util/SpscRingBuffer.h"

class ReceiverAudioOutput : public QObject
{
    Q_OBJECT
public:
    explicit ReceiverAudioOutput(QObject *parent = nullptr);
    ~ReceiverAudioOutput();

    void start();
    void stop();
    void writeAudio(const QVector<float>& audioBuffer);
    void writeAudio(const float* data, int size);

    void setSampleRate(int rate);

    size_t ringBufferAvailableRead() const { return m_ringBuffer.availableRead(); }
    size_t ringBufferCapacity() const { return m_ringBuffer.capacity(); }

private slots:
    void onAudioOutputsChanged();
    void onSinkStateChanged(QAudio::State state);
    void reopenOutput();
    void pumpAudio();

private:
    bool isSinkHealthy() const;
    /** Mute without touching the sink; safe from the audio thread. */
    void markSinkLostLocked(const char *reason);
    void handleDeviceLostLocked(const char *reason);
    void openSinkLocked();
    void startLocked();
    void stopLocked();
    /** True when the caller may touch the sink; otherwise the call was re-posted. */
    bool onOwnThread() const;
    void scheduleReopen();

    QAudioSink* m_audioSink = nullptr;
    QIODevice* m_device = nullptr;
    QAudioFormat m_format;
    int m_sampleRate = 48000;
    QMutex m_mutex;
    SpscRingBuffer<float> m_ringBuffer{131072};
    std::atomic<bool> m_wantRunning{false};
    bool m_reopenPending = false;
    QTimer m_reopenTimer;
    QTimer m_pumpTimer;
};
