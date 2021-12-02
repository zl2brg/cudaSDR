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
    Setup();
    CHECKED_CONNECT(set,
                    SIGNAL(micInputChanged(int)),
                    this,
                    SLOT(MicInputChanged(int)));

    device = set->getMicInputDev();

    /* device 0 is Hermes mic input */
    if (device > 0) device--;
}



PAudioInput::~PAudioInput()
{
    Stop();
}

void PAudioInput::Setup() {

    error = Pa_Initialize();
    bzero( &inputParameters, sizeof( inputParameters ) ); //not necessary if you are filling in all the fields
    inputParameters.channelCount = 1;
    inputParameters.device = device;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(device)->defaultLowInputLatency ;
    inputParameters.hostApiSpecificStreamInfo = NULL; //See you specific host's API docs for info on using this field
    ;
//    PaStreamCallback  *callback = audioCallback;




    AUDIO_INPUT_DEBUG << "PA Open stream " << error;

}

void PAudioInput::MicInputChanged(int value){
/* Index 0 is hpsdr local mic input */
    if (value > 0)
        device =value -1;

    Stop();
    if (device > 0) {
        Setup();
        Start();
        AUDIO_INPUT_DEBUG << "Mic Input Changed" << value;

    } else {
       AUDIO_INPUT_DEBUG << "Local HPSDR Mic Mode Selected" << value;

    }

}

void PAudioInput::Stop(){
    m_ThreadQuit = true;
    error = Pa_CloseStream( stream );
    msleep(100);

    AUDIO_INPUT_DEBUG << "PA Close stream " << error;
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


}

void PAudioInput::run() {
    unsigned char temp[AUDIO_IN_PACKET_SIZE];
    QByteArray data;

    while(!m_ThreadQuit )	//execute loop until quit flag set

    {
        if (Pa_IsStreamActive( stream ) ) {


            if (Pa_GetStreamReadAvailable(stream) >= AUDIO_FRAMESIZE) {
                Pa_ReadStream(stream, temp, AUDIO_FRAMESIZE);
                m_audioInQueue.enqueue(QByteArray((const char *)temp, AUDIO_IN_PACKET_SIZE));
//                qDebug() << "Data  Queued !";
            } else msleep(5);
        }
    }

}

int  PAudioInput::callbackProcess(unsigned long framesPerBuffer, float *output, short *in, PADeviceCallbackStuff* callbackStuff){
//    qDebug() << "PortAudio Callback " << framesPerBuffer;
    const float *rptr = (const float*)in;
    const char *ptr = (const char *)in;

    if (framesPerBuffer == AUDIO_FRAMESIZE)
    {

        for (int i = 0; i < AUDIO_IN_PACKET_SIZE;i++) {

            callbackStuff->buffer[i] = *ptr++;
        }
        callbackStuff->data_ready = true;
        qDebug() << "Data !";
    }

return paContinue;
}


int audioCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                  const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags,
                  void *_callbackStuff) {
    return ((PADeviceCallbackStuff*)_callbackStuff)->soundDevice->callbackProcess(framesPerBuffer, (float *)outputBuffer, (short*)inputBuffer, (PADeviceCallbackStuff*) _callbackStuff);


}
