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
    mInputBuffer.open(QBuffer::ReadWrite);
       QAudioFormat format;
      // Set up the desired format, for example:
      format.setSampleRate(48000);
      format.setChannelCount(1);
      format.setSampleSize(16);
      format.setCodec("audio/pcm");
      format.setByteOrder(QAudioFormat::LittleEndian);
      format.setSampleType(QAudioFormat::UnSignedInt);

      QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
      if (!info.isFormatSupported(format)) {
          qWarning() << "Default format not supported, trying to use the nearest.";
          format = info.nearestFormat(format);
      }

      m_AudioIn = new QAudioInput(format, this);
      connect(m_AudioIn, SIGNAL(stateChanged(QAudio::State)), this, SLOT(stateChangeAudioIn(QAudio::State)));
/*
      CHECKED_CONNECT(
                 m_AudioIn,
                 SIGNAL(notify()),
                 this,
                 SLOT(audioUpdate()));
      m_AudioIn->setNotifyInterval(100);
*/

      CHECKED_CONNECT(
                  &mInputBuffer,
                  SIGNAL(readyRead()),
                  this,
                  SLOT(audioUpdate()));

   //   m_AudioIn->setNotifyInterval(100);
      m_AudioIn->start(&mInputBuffer);


}

AudioInput::~AudioInput()
{
    m_AudioIn->stop();
    stop();
    if (m_AudioIn) delete m_AudioIn;
     m_AudioIn = nullptr;
}

void AudioInput::start()
{

}


void AudioInput::stop(){
    m_AudioIn->stop();
    mInputBuffer.close();
}

void AudioInput::processAudioIn() {



while (!m_stopped)
    {
    mInputBuffer.waitForReadyRead(1000);
    AUDIO_INPUT_DEBUG << "State " << mInputBuffer.bytesAvailable() << "size" << mInputBuffer.size();
    mInputBuffer.readAll();

    }
mInputBuffer.close();
}

void AudioInput::stateChangeAudioIn(QAudio::State s) {

AUDIO_INPUT_DEBUG << "state changed " << s;
}


void AudioInput::audioUpdate(){
    AUDIO_INPUT_DEBUG << "State " << mInputBuffer.bytesAvailable() << "size" << mInputBuffer.size();
    mInputBuffer.readAll();

}

