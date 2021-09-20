//
// Created by simon on 14/09/21.
//

#include <QObject>
#include <QList>
#include <QtMultimedia/QAudioInput>
#include <QBuffer>

#include "cusdr_settings.h"

#ifndef CUDASDR_CUSDR_AUDIO_INPUT_H
#define CUDASDR_CUSDR_AUDIO_INPUT_H


#ifdef LOG_AUDIO_INPUT
#   define AUDIO_INPUT_DEBUG qDebug().nospace() << "AudioInput::\t"
#else
#   define AUDIO_INPUT_DEBUG nullDebug()
#endif

QT_FORWARD_DECLARE_CLASS(QAudioInput)
QT_FORWARD_DECLARE_CLASS(QAudioOutput)
QT_FORWARD_DECLARE_CLASS(QFile)

class AudioInput : public QObject {
Q_OBJECT
public:
AudioInput(QObject *parent = 0);
~AudioInput();
void setup();
void stop();
void start();

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


public slots:
    void processAudioIn();
    void stateChangeAudioIn(QAudio::State s);
    void audioUpdate();

private:
    Settings				*set;

    const QList<QAudioDeviceInfo> m_availableAudioInputDevices;
    QAudioDeviceInfo    m_audioInputDevice;
    qint64              m_recordPosition;
    volatile bool m_stopped;
    QIODevice *m_in = nullptr;
};


#endif //CUDASDR_CUSDR_AUDIO_INPUT_H
