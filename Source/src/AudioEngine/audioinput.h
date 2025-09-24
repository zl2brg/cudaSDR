#ifndef AUDIOINPUT_H
#define AUDIOINPUT_H

#include <QObject>
#include <QAudioSource>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QAudioFormat>
#include <QIODevice>
#include <QQueue>
#include <QMutex>
#include <QByteArray>
#include "cusdr_settings.h"

// Define the AUDIOBUF as in your original code
typedef QVector<double> AUDIOBUF;

class AudioInput : public QObject
{
    Q_OBJECT
public:
    explicit AudioInput(QObject *parent = nullptr);
    ~AudioInput();

    void Start();
    void Stop();

    // Queue for the audio data that can be accessed by DataProcessor
    QQueue<AUDIOBUF> m_faudioInQueue;

signals:
    void tx_mic_data_ready();

private slots:
    void handleReadyRead();

private:
    void setupAudioSource();
    void processAudioData(const QByteArray &data);

    QAudioSource *m_audioSource;
    QIODevice *m_audioInputDevice;
    QAudioFormat m_format;
    QMutex m_mutex;
    bool m_running;
    int m_sampleRate;
    int m_bufferSize;
    Settings *set;
};

#endif // AUDIOINPUT_H
