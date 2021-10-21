//
// Created by simon on 14/09/21.
//

#include <QObject>
#include <QList>
#include <QtMultimedia/QAudioInput>
#include <QBuffer>
#include "portaudio.h"

#include "cusdr_settings.h"

#ifndef CUDASDR_CUSDR_AUDIO_INPUT_H
#define CUDASDR_CUSDR_AUDIO_INPUT_H

#define AUDIO_IN_PACKET_SIZE 4096//2048
class PAudioInput;

struct PADeviceCallbackStuff
{
        PAudioInput* soundDevice;
        int devIndex;
        int          frameIndex;  /* Index into sample array. */
        int          maxFrameIndex;
        float       *inputSamples;
};


#ifdef LOG_AUDIO_INPUT
#   define AUDIO_INPUT_DEBUG qDebug().nospace() << "AudioInput::\t"
#else
#   define AUDIO_INPUT_DEBUG nullDebug()
#endif

QT_FORWARD_DECLARE_CLASS(QAudioInput)
QT_FORWARD_DECLARE_CLASS(QAudioOutput)
QT_FORWARD_DECLARE_CLASS(QFile)

class AudioInput : public QThread {
Q_OBJECT
public:
AudioInput(QObject *parent = 0);
~AudioInput();
void Stop();
bool Start();
void run();

    const QList<QAudioDeviceInfo>& availableAudioInputDevices() const
    { return m_availableAudioInputDevices; }

    QAudio::Mode mode() const       { return m_mode; }
    QAudio::State state() const     { return m_state; }
    const QAudioFormat& format() const  { return m_format; }
    QHQueue<QByteArray> m_audioInQueue;


    QAudio::Mode m_mode;
    QAudio::State m_state;
    QAudioFormat m_format;
    QAudioInput* m_AudioIn;
    QByteArray temp;

    QIODevice * out;
   QIODevice *m_in = nullptr;

public slots:
private:
    Settings				*set;
    bool m_BlockingMode;
    bool m_ThreadQuit;
    bool m_Startup;
    const QList<QAudioDeviceInfo> m_availableAudioInputDevices;
    QAudioDeviceInfo    m_audioInputDevice;
    qint64              m_recordPosition;
    volatile bool m_stopped;

};


class PAudioInput : public QThread {
Q_OBJECT
public:
PAudioInput(QObject *parent = 0);
~PAudioInput();
void Stop();
bool Start();
void run();
public:
    QStringList paDeviceList;
    int callbackProcess(unsigned long framesPerBuffer, float *output, short *in, PADeviceCallbackStuff* callbackstuff);


private:
    Settings		*set;
    PaError         error;
    PaStreamParameters inputParameters;
    PaStream        *stream;
    PaDeviceIndex   device;
    PADeviceCallbackStuff m_callbackStuff;

     bool m_BlockingMode;
    bool m_ThreadQuit;
    bool m_Startup;
    const QList<QAudioDeviceInfo> m_availableAudioInputDevices;
    QAudioDeviceInfo    m_audioInputDevice;
    qint64              m_recordPosition;
    volatile bool m_stopped;


};

int audioCallback(const void *inputBuffer, void *outputBuffer,
                  unsigned long framesPerBuffer,
                  const PaStreamCallbackTimeInfo* timeInfo,
                  PaStreamCallbackFlags statusFlags,
                  void *_callbackStuff);

#endif //CUDASDR_CUSDR_AUDIO_INPUT_H

