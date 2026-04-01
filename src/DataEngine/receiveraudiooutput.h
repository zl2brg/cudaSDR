#pragma once

#include <QObject>
#include <QAudioSink>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QAudioFormat>
#include <QIODevice>
#include <QMutex>
#include <QVector>
#include <QByteArray>

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

private:
    QAudioSink* m_audioSink = nullptr;
    QIODevice* m_device = nullptr;
    QAudioFormat m_format;
    int m_sampleRate = 48000;
    QMutex m_mutex;
    QByteArray m_pending;  // carry-forward for partial writes
};
