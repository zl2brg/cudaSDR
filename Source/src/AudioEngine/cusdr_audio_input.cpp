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


    AUDIO_INPUT_DEBUG << format;
    m_AudioIn = new QAudioInput(format, this);
  //  connect(m_AudioIn, SIGNAL(stateChanged(QAudio::State)), this, SLOT(stateChangeAudioIn(QAudio::State)));
 //   m_AudioOut = new QAudioOutput(format,this);
    AUDIO_INPUT_DEBUG << "Audio input buffer size " << m_AudioIn->bufferSize();
    temp.fill(0,BUFFER_SIZE * 2);

    if (QAudio::NoError == m_AudioIn->error())
    {

        m_ThreadQuit = false;
        start(QThread::HighestPriority);	//start worker thread and set its priority
        //		start(QThread::TimeCriticalPriority);	//start worker thread and set its priority
        m_AudioIn->setBufferSize(8192);
        m_in = m_AudioIn->start();
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
                    long *test = (long*) temp.data_ptr();
                    double test1 = (unsigned char)(temp.at(0) << 24);
                    test1 +=  (double)(unsigned char)(temp.at(1) << 16);
                    test1 +=  (double)(unsigned char)(temp.at(2) << 8);
                    test1 +=  (double)(unsigned char)(temp.at(3));
                    float test2 = (float(test1));
                    //fprintf(stderr,"float  %f dpuble %f\n",test1,test2);
   //                 AUDIO_INPUT_DEBUG << *test << " " << (float) test2  ;
   //                 AUDIO_INPUT_DEBUG "data "<< hex <<  temp.at(0)<< hex  << temp.at(1) << hex << temp.at(2) << hex << temp.at(3) ;

                 if (temp.count() == AUDIO_IN_PACKET_SIZE)
                 {
                    m_audioInQueue.enqueue(temp);
                 }
//                 else   AUDIO_INPUT_DEBUG << "Audio Queue size error " << temp.count() << len << m_AudioIn->periodSize();
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
            wait(500);
        }
        if(m_AudioIn)
        {
            delete m_AudioIn;
            m_AudioIn = NULL;
        }
}

