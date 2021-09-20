//
// Created by simon on 14/09/21.
//


#include <QCoreApplication>
#include <QMetaObject>
#include <QSet>
#include <QAudioInput>
#include <QAudioOutput>
#include <QDebug>
#include <QThread>
#include <QFile>
#include <QTimer>
#include <QDebug>
#define LOG_AUDIO_INPUT

#include "cusdr_audio_input.h"

AudioInput::AudioInput(QObject *parent)

:QObject(parent)
, set(Settings::instance())
, m_availableAudioInputDevices(
 QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
, m_audioInputDevice(QAudioDeviceInfo::defaultInputDevice())
{



/*

      m_AudioIn->setNotifyInterval(100);
*/
/*
      CHECKED_CONNECT(
                  &mInputBuffer,
                  SIGNAL(readyRead()),
                  this,
                  SLOT(audioUpdate()));
*/

setup();
start();

}

AudioInput::~AudioInput()
{
    stop();
    if (m_AudioIn) delete m_AudioIn;
}

void AudioInput::setup(){

    QAudioFormat format;
    // Set up the desired format, for example:
    format.setSampleRate(48000);
    format.setChannelCount(1);
    format.setSampleSize(24);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::Float);

    QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    if (!info.isFormatSupported(format)) {
        qWarning() << "Default format not supported, trying to use the nearest.";
        format = info.nearestFormat(format);
    }

    m_AudioIn = new QAudioInput(format, this);
    connect(m_AudioIn, SIGNAL(stateChanged(QAudio::State)), this, SLOT(stateChangeAudioIn(QAudio::State)));

}

void AudioInput::start()
{
    m_AudioIn->setNotifyInterval(100);
    m_in = m_AudioIn->start();
    CHECKED_CONNECT(
            m_in,
            SIGNAL(readyRead()),
            this,
            SLOT(audioUpdate()));
}


void AudioInput::stop(){
    m_AudioIn->stop();
    m_audioInQueue.release_queue();
}

void AudioInput::processAudioIn() {



while (!m_stopped)
    {

    }
}

void AudioInput::stateChangeAudioIn(QAudio::State s) {

AUDIO_INPUT_DEBUG << "state changed " << s;
}


void AudioInput::audioUpdate(){
    AUDIO_INPUT_DEBUG << "State " << m_in->bytesAvailable() << "size" << m_in->size();
    m_audioInQueue.enqueue(m_in->readAll());
    AUDIO_INPUT_DEBUG << "Audio Queue size" << m_audioInQueue.count();
}

