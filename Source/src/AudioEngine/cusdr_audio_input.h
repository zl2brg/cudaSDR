//
// Created by simon on 14/09/21.
//

#include <QObject>
#include <QList>
#include <QtMultimedia/QAudioInput>
#include <QBuffer>
#include <QVector>
#include "portaudio.h"

#include "cusdr_settings.h"

#ifndef CUDASDR_CUSDR_AUDIO_INPUT_H
#define CUDASDR_CUSDR_AUDIO_INPUT_H


#define AUDIO_FRAMESIZE  1024
#define AUDIO_IN_PACKET_SIZE 4096//2048
class PAudioInput;

struct PADeviceCallbackStuff
{
        PAudioInput* soundDevice;
        int devIndex;
        int          frameIndex;  /* Index into sample array. */
        int          maxFrameIndex;
        unsigned char buffer[AUDIO_IN_PACKET_SIZE];
        bool data_ready;
};


#ifdef LOG_AUDIO_INPUT
#   define AUDIO_INPUT_DEBUG qDebug().nospace() << "AudioInput::\t"
#else
#   define AUDIO_INPUT_DEBUG nullDebug()
#endif


typedef QVector<double> AUDIOBUF;

Q_DECLARE_METATYPE (AUDIOBUF)

class PAudioInput : public QThread {
Q_OBJECT
public:
PAudioInput(QObject *parent = 0);
~PAudioInput();
void Setup();
void Stop();
bool Start();
void run();
public:
    QStringList paDeviceList;
    QHQueue<QByteArray> m_audioInQueue;
    AUDIOBUF  audioinputBuffer;
    QHQueue<AUDIOBUF> m_faudioInQueue;

signals:
    void tx_mic_data_ready();



private:
    Settings		*set;
    PaError         error;
    PaStreamParameters inputParameters;
    PaStream        *stream =NULL;

    bool m_BlockingMode;
    bool m_ThreadQuit;
    bool m_Startup;
    qint64              m_recordPosition;
    volatile bool m_stopped;

private slots:
    void MicInputChanged(int source);

};

#endif //CUDASDR_CUSDR_AUDIO_INPUT_H

