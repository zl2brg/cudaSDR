//
// Created by Simon Eatough, ZL2BRG on 14/09/21.
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


PAudioInput::PAudioInput(QObject *parent) : QThread(parent)
        , set(Settings::instance())
{

    CHECKED_CONNECT(set,
                    SIGNAL(micInputChanged(int)),
                    this,
                    SLOT(MicInputChanged(int)));

    inputParameters.device = set->getMicInputDev();

    /* device 0 is Hermes mic input */
    if (inputParameters.device > 0) inputParameters.device--;
    Setup();
    audioinputBuffer.resize(1024);
    audioinputBuffer.fill(0.0);
    qDebug() << "Audio Buffer Size"  << audioinputBuffer.size();

}



PAudioInput::~PAudioInput()
{
    Stop();
}

void PAudioInput::Setup() {

    error = Pa_Initialize();
    bzero( &inputParameters, sizeof( inputParameters ) ); //not necessary if you are filling in all the fields
    inputParameters.channelCount = 1;
    inputParameters.hostApiSpecificStreamInfo = 0;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency ;
    inputParameters.hostApiSpecificStreamInfo = 0; //See you specific host's API docs for info on using this field

    AUDIO_INPUT_DEBUG << "PA Open stream " << error;

}

void PAudioInput::MicInputChanged(int value){
/* Index 0 is hpsdr local mic input */
    if (value > 0)
        inputParameters.device =value -1;

    Stop();
    if (inputParameters.device > 0) {
        Setup();
        Start();
        AUDIO_INPUT_DEBUG << "Mic Input Changed" << value;

    } else {
       AUDIO_INPUT_DEBUG << "Local HPSDR Mic Mode Selected" << value;

    }

}

void PAudioInput::Stop(){
        if (stream)
        {
         error = Pa_CloseStream( stream );
         msleep(100);
         AUDIO_INPUT_DEBUG << "PA Close stream " << error;
        }
        m_ThreadQuit = true;


}


bool PAudioInput::Start() {
    error = Pa_OpenStream(
            &stream,
            &inputParameters,
            NULL,
            48000,
            AUDIO_FRAMESIZE,
            paNoFlag, //flags that can be used to define dither, clip settings and more
            NULL, //your callback function
            NULL );
    error = Pa_StartStream(stream);
    AUDIO_INPUT_DEBUG << "PA Start stream " << error;
    m_ThreadQuit = false;
    start(QThread::HighestPriority);
    return true;
}

void PAudioInput::run() {
    unsigned char temp[AUDIO_IN_PACKET_SIZE];
    QByteArray data;
    float *sample;
    qDebug() << "Double" << sizeof(double);

    while(!m_ThreadQuit )	//execute loop until quit flag set
    {
        if (Pa_IsStreamActive( stream ) ) {
            if (Pa_GetStreamReadAvailable(stream) >= AUDIO_FRAMESIZE) {
                Pa_ReadStream(stream, temp, AUDIO_FRAMESIZE);
                sample = (float *) (temp);

                for (int i = 0; i < AUDIO_FRAMESIZE; i++  )
                {
                   audioinputBuffer[i] = (double)(*sample);
                   sample++;
                }
                emit tx_mic_data_ready();
                m_faudioInQueue.enqueue(audioinputBuffer);
                m_audioInQueue.enqueue(QByteArray((const char *)temp, AUDIO_IN_PACKET_SIZE));
            } else msleep(5);
        }
    }

}

