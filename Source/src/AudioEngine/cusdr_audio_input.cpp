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

AudioInput::AudioInput(QObject *parent) : QThread(parent)



  , set(Settings::instance())
  , m_availableAudioInputDevices(
        QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
  , m_audioInputDevice(QAudioDeviceInfo::defaultInputDevice())
{


}

AudioInput::~AudioInput()
{
    Stop();
    if (m_AudioIn) delete m_AudioIn;
}


bool AudioInput::Start()
{
    QAudioFormat format;
    // Set up the desired format, for example:
    format.setSampleRate(48000);
    format.setChannelCount(1);
    format.setSampleSize(32);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::Float);

    QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    if (!info.isFormatSupported(format)) {
        format = info.nearestFormat(format);
        AUDIO_INPUT_DEBUG << "Default format not supported, trying to use the nearest." << info.nearestFormat(format);
    }

    m_AudioIn = new QAudioInput(format, this);
  //  connect(m_AudioIn, SIGNAL(stateChanged(QAudio::State)), this, SLOT(stateChangeAudioIn(QAudio::State)));
    m_AudioOut = new QAudioOutput(format,this);
    AUDIO_INPUT_DEBUG << "Audio input buffer size " << m_AudioIn->bufferSize();
    temp.fill(0,BUFFER_SIZE * 2);

    if (QAudio::NoError == m_AudioIn->error())
    {
        AUDIO_INPUT_DEBUG << "Start" << m_AudioOut->error();

        m_ThreadQuit = false;
        start(QThread::HighestPriority);	//start worker thread and set its priority
        //		start(QThread::TimeCriticalPriority);	//start worker thread and set its priority
        m_AudioIn->setBufferSize(8192);
        m_in = m_AudioIn->start();
        out = m_AudioOut->start();
        return true;
    }
    else
    {
        qDebug()<<"Soundcard output error";
        return false;
    }
}


void AudioInput::run()
{
    qint64 len;
    qint64 count = 0;
    QMutex mutex;
    QByteArray a;


//    m_AudioIn->reset();

    while(!m_ThreadQuit )	//execute loop until quit flag set
     {
            if( (QAudio::IdleState == m_AudioIn->state() ) ||
                (QAudio::ActiveState == m_AudioIn->state() ) )
        {
             len   =  m_AudioIn->bytesReady();
             if (len > AUDIO_IN_PACKET_SIZE)
                {
                 temp.resize(AUDIO_IN_PACKET_SIZE);
                 temp = m_in->read(AUDIO_IN_PACKET_SIZE);
          //       AUDIO_INPUT_DEBUG << "Audio len" << len ;
                 if (temp.count() == AUDIO_IN_PACKET_SIZE)
                 {
                    out->write(temp.data(),AUDIO_IN_PACKET_SIZE);
                    m_audioInQueue.enqueue(temp);
                 }
                 else   AUDIO_INPUT_DEBUG << "Audio Queue size error " << temp.count() << len << m_AudioIn->periodSize();
                 }
                else {
                 msleep(50);
             }


                    //    AUDIO_INPUT_DEBUG << "Audio Queue " << m_audioInQueue.count();
                }


            }

    }


void AudioInput::Stop(){

    if(!m_ThreadQuit)
        {
            m_ThreadQuit = true;
            m_AudioIn->stop();
            m_AudioOut->stop();
            wait(500);
        }
        if(m_AudioIn)
        {
            delete m_AudioIn;
            m_AudioIn = NULL;
        }

        if(m_AudioOut)
        {
            delete m_AudioOut;
            m_AudioOut = NULL;
        }


}

