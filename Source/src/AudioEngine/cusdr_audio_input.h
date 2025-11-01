//
// Created by simon on 14/09/21.
//

#include <QObject>
#include <QList>
#include <QtMultimedia/QAudioSource>
#include <QtMultimedia/QAudioDevice>
#include <QtMultimedia/QMediaDevices>
#include <QBuffer>
#include <QVector>
#include <QMutex>
#include <QIODevice>

#include "cusdr_settings.h"

#ifndef CUDASDR_CUSDR_AUDIO_INPUT_H
#define CUDASDR_CUSDR_AUDIO_INPUT_H


#define AUDIO_FRAMESIZE  1024
#define AUDIO_IN_PACKET_SIZE 4096//2048

#define LOG_AUDIO_INPUT

#ifdef LOG_AUDIO_INPUT
#   define AUDIO_INPUT_DEBUG qDebug().nospace() << "AudioInput::\t"
#else
#   define AUDIO_INPUT_DEBUG nullDebug()
#endif


typedef QVector<double> AUDIOBUF;

Q_DECLARE_METATYPE (AUDIOBUF)

class PAudioInput : public QObject {
Q_OBJECT
public:
    PAudioInput(QObject *parent = nullptr);
    ~PAudioInput();
    void Setup();
    void Stop();
    bool Start();
    
public:
    QStringList paDeviceList;
    QHQueue<QByteArray> m_audioInQueue;
    AUDIOBUF  audioinputBuffer;
    QHQueue<AUDIOBUF> m_faudioInQueue;

signals:
    void tx_mic_data_ready();

private:
    void setupAudioSource();
    void processAudioData(const QByteArray &data);
    
private slots:
    void MicInputChanged(int source);
    void handleReadyRead();

private:
    Settings*           set;
    QAudioSource*       m_audioSource;
    QIODevice*          m_audioInputDevice;
    QAudioFormat        m_format;
    QMutex              m_mutex;
    bool                m_running;
    int                 m_sampleRate;
    int                 m_bufferSize;
    int                 m_deviceIndex;
};

#endif //CUDASDR_CUSDR_AUDIO_INPUT_H

