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
    PAudioInput *pin= new PAudioInput();
    AUDIO_INPUT_DEBUG << "Start";
    QAudioFormat format;
    // Set up the desired format, for example:
    format.setSampleRate(48000);
    format.setChannelCount(1);
    format.setSampleSize(32);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::Float);

    QAudioDeviceInfo info =m_availableAudioInputDevices.at(set->getMicInputDev());
    if (!info.isFormatSupported(format)) {
        format = info.nearestFormat(format);
        AUDIO_INPUT_DEBUG << "Default format not supported, trying to use the nearest." << info.nearestFormat(format);
    }


    AUDIO_INPUT_DEBUG << format << info.deviceName();


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
//        m_AudioIn->setBufferSize(8192);
        m_AudioIn->setVolume(1.0);
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
    return;

//    m_AudioIn->reset();

    while(!m_ThreadQuit )	//execute loop until quit flag set
     {
            if( (QAudio::IdleState == m_AudioIn->state() ) ||
                (QAudio::ActiveState == m_AudioIn->state() ) )
        {
             len   =  m_AudioIn->bytesReady();
             if (len > AUDIO_IN_PACKET_SIZE)
                {
                    temp = m_in->read(AUDIO_IN_PACKET_SIZE);
                    if (temp.size() != AUDIO_IN_PACKET_SIZE)
                    {
                        qDebug() << "Audio Error" << temp.size();
                        temp.fill(0,AUDIO_IN_PACKET_SIZE);
                    }

                    double test1 = (double)(temp.at(40) << 24);
                    test1 +=  (double)(temp.at(41) << 16);
                    test1 +=  (double)(temp.at(42) << 8);
                    test1 +=  (double)(temp.at(43));
//                    qDebug() << "addio read" << test1;


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
AUDIO_INPUT_DEBUG << "run thhread exit";
    }


void AudioInput::Stop(){

    if(!m_ThreadQuit)
        {
            AUDIO_INPUT_DEBUG << "Stop";
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


PAudioInput::PAudioInput(QObject *parent) : QThread(parent)
        , set(Settings::instance())
{
    error = Pa_Initialize();
    int numDevices;
    numDevices = Pa_GetDeviceCount();
    if( numDevices < 0 )
    {
        printf( "ERROR: Pa_CountDevices returned 0x%x\n", numDevices );
        error = numDevices;

    }
    const   PaDeviceInfo *deviceInfo;
    for( int i=0; i<numDevices; i++ )
    {
        deviceInfo = Pa_GetDeviceInfo( i );
        paDeviceList.append(QString(deviceInfo->name));
    }

    device = 17;
    bzero( &inputParameters, sizeof( inputParameters ) ); //not necessary if you are filling in all the fields
    inputParameters.channelCount = 1;
    inputParameters.device = device;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(17)->defaultLowInputLatency ;
    inputParameters.hostApiSpecificStreamInfo = NULL; //See you specific host's API docs for info on using this field
    PaStreamCallback  *callback = audioCallback;
    error = Pa_OpenStream(
            &stream,
            &inputParameters,
            NULL,
            48000,
            paFramesPerBufferUnspecified,
            paNoFlag, //flags that can be used to define dither, clip settings and more
            callback, //your callback function
            (void *) &m_callbackStuff );

    AUDIO_INPUT_DEBUG << "PA Open stream " << error;
}

PAudioInput::~PAudioInput()
{
    Stop();
}

void PAudioInput::Stop(){
    error = Pa_CloseStream( stream );
    AUDIO_INPUT_DEBUG << "PA Close stream " << error;
}


bool PAudioInput::Start() {

    error = Pa_StartStream(stream);
    AUDIO_INPUT_DEBUG << "PA Start stream " << error;

}

void PAudioInput::run() {

}

int  PAudioInput::callbackProcess(unsigned long framesPerBuffer, float *output, short *in, PADeviceCallbackStuff* callbackStuff){

    qDebug() << "PortAudio Callback " << framesPerBuffer;
    const float *rptr = (const float*)in;
    float *wptr = &callbackStuff->inputSamples[callbackStuff->frameIndex * 1];

    for (unsigned int i = 0; i < framesPerBuffer;i++)
    {
        qDebug() << "data" << *in++;
    }
return paContinue;
}


int audioCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                  const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags,
                  void *_callbackStuff) {
    return ((PADeviceCallbackStuff*)_callbackStuff)->soundDevice->callbackProcess(framesPerBuffer, (float *)outputBuffer, (short*)inputBuffer, (PADeviceCallbackStuff*) _callbackStuff);


}