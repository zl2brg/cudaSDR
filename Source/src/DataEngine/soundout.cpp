/////////////////////////////////////////////////////////////////////
// soundout.cpp: implementation of the CSoundOut class.
//
// This class implements a class to output data to a soundcard.
// A fractional resampler is used to convert the users input rate to
// the sound card rate and also perform frequency lock between the
// two clock domains.
//
// History:
//  2010-09-15  Initial creation MSW
//  2011-03-27  Initial release
//  2011-08-07  Changed some debug output
//  2015-01-24  RRK, Minor mods, adding to cudaSDR
//  2025-09-08  Updated for Qt 6 audio APIs
/////////////////////////////////////////////////////////////////////

//==========================================================================================
// + + +   This Software is released under the "Simplified BSD License"  + + +
//Copyright 2010 Moe Wheatley. All rights reserved.
//
//Redistribution and use in source and binary forms, with or without modification, are
//permitted provided that the following conditions are met:
//
//   1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
//
//   2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other materials
//    provided with the distribution.
//
//THIS SOFTWARE IS PROVIDED BY Moe Wheatley ``AS IS'' AND ANY EXPRESS OR IMPLIED
//WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Moe Wheatley OR
//CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//The views and conclusions contained in the software and documentation are those of the
//authors and should not be interpreted as representing official policies, either expressed
//or implied, of Moe Wheatley.
//==========================================================================================
#include "soundout.h"
#include <QDebug>
#include <QMediaDevices>
#include <QAudioSink>
#include <math.h>
#include <atomic>
#include <algorithm>
#define SOUNDCARD_RATE 48000    // output soundcard sample rate
#define FILTERQLEVEL_ALPHA 0.001
#define P_GAIN 2.38e-7      // Proportional gain

#define TRUE 1
#define FALSE 0

#define TEST_ERROR 1.0

// Thread-safe quit flag
static std::atomic_bool threadQuit{true};

/////////////////////////////////////////////////////////////////////
//   constructor/destructor
/////////////////////////////////////////////////////////////////////
CSoundOut::CSoundOut(QObject *parent) :
    QThread(parent),
    m_pParent(parent),
    m_pAudioSink(nullptr),
    m_pOutput(nullptr),
    m_UserDataRate(SOUNDCARD_RATE),
    m_OutRatio(1.0),
    m_RateCorrection(0.0),
    m_Gain(1.0),
    m_Startup(true),
    m_BlockingMode(false)
{
    m_OutResampler.Init(8192);
}

CSoundOut::~CSoundOut()
{
    Stop();
}

void GetAlsaMasterVolume(long *volume)
{
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    const char *selem_name = "Master";

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, volume);

    snd_mixer_close(handle);
}

void SetAlsaMasterVolume(long volume)
{
    long min, max;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    const char *selem_name = "Master";

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_set_playback_volume_all(elem, volume * max / 100);

    snd_mixer_close(handle);
}

/////////////////////////////////////////////////////////////////////
// Starts up soundcard output thread using soundcard at list OutDevIndx
/////////////////////////////////////////////////////////////////////
bool CSoundOut::Start(int OutDevIndx, bool StereoOut, double UsrDataRate, bool BlockingMode)
{
    long mvolume = 0;
    m_StereoOut = StereoOut;
    m_BlockingMode = BlockingMode;

    // Get available audio output devices using Qt6 API
    const QList<QAudioDevice> outputDevices = QMediaDevices::audioOutputs();

    if (outputDevices.isEmpty()) {
        qDebug() << "No audio output devices found";
        return false;
    }

    // Select the specified device or default
    if (OutDevIndx == -1) {
        GetAlsaMasterVolume(&mvolume);
        qDebug() << "Soundcard volume" << mvolume;
        m_OutDeviceInfo = QMediaDevices::defaultAudioOutput();
    } else if (OutDevIndx >= 0 && OutDevIndx < outputDevices.size()) {
        m_OutDeviceInfo = outputDevices.at(OutDevIndx);
    } else {
        m_OutDeviceInfo = QMediaDevices::defaultAudioOutput();
    }

    // Debug: List all available audio devices
    for (const QAudioDevice &device : outputDevices) {
        qDebug() << "Available audio device:" << device.description();
    }
    qDebug() << "Selected device:" << m_OutDeviceInfo.description();

    // Setup audio format for output
    QAudioFormat format;
    format.setSampleRate(SOUNDCARD_RATE);
    format.setChannelCount(m_StereoOut ? 2 : 1);
    format.setSampleFormat(QAudioFormat::Int16);

    // Verify the format is supported
    if (!m_OutDeviceInfo.isFormatSupported(format)) {
        qDebug() << "Audio format not supported by device - using nearest supported format";
        format = m_OutDeviceInfo.preferredFormat();
    }

    // Create QAudioSink (Qt6 replacement for QAudioOutput)
    m_pAudioSink = new QAudioSink(m_OutDeviceInfo, format, this);

    if (!m_pAudioSink) {
        qDebug() << "Failed to create audio sink";
        return false;
    }

    if (m_pAudioSink->state() != QAudio::StoppedState &&
        m_pAudioSink->state() != QAudio::IdleState) {
        qDebug() << "Audio sink in invalid state";
        return false;
    }

    // Initialize the data queue variables
    m_UserDataRate = 1; // Force user data rate to be changed
    ChangeUserDataRate(UsrDataRate);

    // Start the audio output
    m_pOutput = m_pAudioSink->start();

    // Set system volume if using default device
    if (OutDevIndx == -1) SetAlsaMasterVolume(50);

    // Calculate block time for thread sleep
    m_BlockTime = (250 * m_pAudioSink->bufferSize()) /
                  (SOUNDCARD_RATE * format.channelCount());

    qDebug() << "Buffer size:" << m_pAudioSink->bufferSize();
    qDebug() << "Block time:" << m_BlockTime;

    // Start worker thread
    threadQuit = false;
    start(QThread::HighestPriority);
    return true;
}

/////////////////////////////////////////////////////////////////////
// Closes down sound card output thread
/////////////////////////////////////////////////////////////////////
void CSoundOut::Stop()
{
    if (!threadQuit) {
        threadQuit = true;
        if (m_pAudioSink) {
            m_pAudioSink->stop();
        }
        wait(500);
    }

    if (m_pAudioSink) {
        delete m_pAudioSink;
        m_pAudioSink = nullptr;
    }
}

/////////////////////////////////////////////////////////////////////
// Sets/changes user data input rate
/////////////////////////////////////////////////////////////////////
void CSoundOut::ChangeUserDataRate(double UsrDataRate)
{
    if (m_UserDataRate != UsrDataRate) {
        m_UserDataRate = UsrDataRate;
        for (int i = 0; i < OUTQSIZE; i++) {
            m_OutQueueMono[i] = 0;
            m_OutQueueStereo[i].re = 0;
            m_OutQueueStereo[i].im = 0;
        }
        m_OutRatio = m_UserDataRate / SOUNDCARD_RATE;
        m_OutQHead = 0;
        m_OutQTail = 0;
        m_OutQLevel = 0;
        m_AveOutQLevel = OUTQSIZE / 2;
        m_Startup = true;
    }
    qDebug() << "SoundOutRatio Rate" << (1.0 / m_OutRatio) << m_UserDataRate;
}

/////////////////////////////////////////////////////////////////////
// Sets/changes volume control gain  0 <= vol <= 99
// Range scales to attenuation(gain) of -50dB to 0dB
/////////////////////////////////////////////////////////////////////
void CSoundOut::SetVolume(qint32 vol)
{
    QMutexLocker locker(&m_Mutex);
    if (vol == 0) {
        m_Gain = 0.0;
    } else if (vol <= 99) {
        m_Gain = pow(10.0, ((double)vol - 99.0) / 39.2);
    }

    // Also set QAudioSink volume if available
    if (m_pAudioSink) {
        float sinkVol = vol / 99.0f;
        m_pAudioSink->setVolume(sinkVol);
    }
}

////////////////////////////////////////////////////////////////
// Called by application to put COMPLEX input into
// STEREO 2 channel soundcard output queue
////////////////////////////////////////////////////////////////
// ... [includes and preamble unchanged] ...

// At the top, add if needed:
#include <QDateTime>

// ... [rest of your code] ...

void CSoundOut::PutOutQueue(int numsamples, TYPECPX* pData)
{
    TYPESTEREO16 RData[OUTQSIZE];
    int i;
    bool overflow = false;

    if ((0 == numsamples) || threadQuit)
        return;

    // ------- DEBUG: Log input to PutOutQueue -------
  //  qDebug() << "numsamples in:" << numsamples << "Queue level before:" << m_OutQLevel;

    // Resample
    int resampled_count = m_OutResampler.Resample(numsamples, TEST_ERROR * m_OutRatio * (1.0 + m_RateCorrection),
                                                  pData, RData, m_Gain);

    // ------- DEBUG: Resampler output stats -------
    double maxval = 0;
    for (int j = 0; j < resampled_count; ++j) {
        double abs_re = static_cast<double>(std::abs(RData[j].re));
        double abs_im = static_cast<double>(std::abs(RData[j].im));
        maxval = std::max(maxval, std::max(abs_re, abs_im));
    }
//    qDebug() << "[PutOutQueue:CPX] Resampler output count:" << resampled_count << "Max abs:" << maxval;

    if (m_BlockingMode) {
        for (i = 0; i < resampled_count; i++) {
            while (((m_OutQHead + 1) & (OUTQSIZE - 1)) == m_OutQTail)
                msleep(10);
            m_OutQueueStereo[m_OutQHead++] = RData[i];
            m_OutQHead &= (OUTQSIZE - 1);
            m_OutQLevel++;
        }
    } else {
        QMutexLocker locker(&m_Mutex);
        for (i = 0; i < resampled_count; i++) {
            m_OutQueueStereo[m_OutQHead++] = RData[i];
            m_OutQHead &= (OUTQSIZE - 1);
            m_OutQLevel++;
            if (m_OutQHead == m_OutQTail) {
                m_OutQTail += OUTQSIZE / 4;
                m_OutQTail &= (OUTQSIZE - 1);
                m_OutQLevel -= OUTQSIZE / 4;
                i = resampled_count;
                overflow = true;
            }
        }
        if (overflow) {
            qWarning() << "[PutOutQueue:CPX] Snd Overflow";
            m_AveOutQLevel = m_OutQLevel;
        }
        m_AveOutQLevel = (1.0 - FILTERQLEVEL_ALPHA) * m_AveOutQLevel + FILTERQLEVEL_ALPHA * (double)m_OutQLevel;
    }

    // ------- DEBUG: Queue level after PutOutQueue -------
//    qDebug() << "[PutOutQueue:CPX] Queue level after:" << m_OutQLevel;
}

void CSoundOut::PutOutQueue(int numsamples, TYPEREAL* pData)
{
    TYPEMONO16 RData[OUTQSIZE];
    int i;
    bool overflow = false;

    if ((0 == numsamples) || threadQuit)
        return;

    // ------- DEBUG: Log input to PutOutQueue -------
 //   qDebug() << "[PutOutQueue:REAL] << "numsamples in:" << numsamples << "Queue level before:" << m_OutQLevel;

    // Resample
    int resampled_count = m_OutResampler.Resample(numsamples, TEST_ERROR * m_OutRatio * (1.0 + m_RateCorrection),
                                                  pData, RData, m_Gain);

    // ------- DEBUG: Resampler output stats -------
    double maxval = 0;
    for (int j = 0; j < resampled_count; ++j)
        maxval = std::max(maxval, std::abs(static_cast<double>(RData[j])));
    // qDebug() << "[PutOutQueue:REAL] Resampler output count:" << resampled_count << "Max abs:" << maxval;

    if (m_BlockingMode) {
        for (i = 0; i < resampled_count; i++) {
            while (((m_OutQHead + 1) & (OUTQSIZE - 1)) == m_OutQTail)
                msleep(10);
            m_OutQueueMono[m_OutQHead++] = RData[i];
            m_OutQHead &= (OUTQSIZE - 1);
            m_OutQLevel++;
        }
    } else {
        QMutexLocker locker(&m_Mutex);
        for (i = 0; i < resampled_count; i++) {
            m_OutQueueMono[m_OutQHead++] = RData[i];
            m_OutQHead &= (OUTQSIZE - 1);
            m_OutQLevel++;
            if (m_OutQHead == m_OutQTail) {
                m_OutQTail += OUTQSIZE / 4;
                m_OutQTail &= (OUTQSIZE - 1);
                m_OutQLevel -= OUTQSIZE / 4;
                i = resampled_count;
                overflow = true;
            }
        }
        if (overflow) {
            qWarning() << "[PutOutQueue:REAL] Snd Overflow";
            m_AveOutQLevel = m_OutQLevel;
        }
        m_AveOutQLevel = (1.0 - FILTERQLEVEL_ALPHA) * m_AveOutQLevel + FILTERQLEVEL_ALPHA * (double)m_OutQLevel;
    }

    // ------- DEBUG: Queue level after PutOutQueue -------
//    qDebug() << "[PutOutQueue:REAL] Queue level after:" << m_OutQLevel;
}

void CSoundOut::GetOutQueue(int numsamples, TYPEMONO16* pData)
{
    int i;
    bool underflow = false;

    QMutexLocker locker(&m_Mutex);

    if (m_Startup) {
        for (i = 0; i < numsamples; i++)
            pData[i] = 0;

        if (m_OutQLevel > OUTQSIZE / 2) {
            m_Startup = false;
            m_RateUpdateCount = -5 * SOUNDCARD_RATE;
            m_PpmError = 0;
            m_AveOutQLevel = m_OutQLevel;
            m_UpdateToggle = true;
        } else {
            return;
        }
    }

    for (i = 0; i < numsamples; i++) {
        if (m_OutQHead != m_OutQTail) {
            pData[i] = m_OutQueueMono[m_OutQTail++];
            m_OutQTail &= (OUTQSIZE - 1);
            m_OutQLevel--;
        } else {
            // ------- DEBUG: Underflow event -------
            qWarning() << "[GetOutQueue:MONO] Snd Underflow at" << QDateTime::currentDateTime().toString()
                       << "Queue level:" << m_OutQLevel;
            m_OutQTail -= (OUTQSIZE / 4);
            m_OutQTail &= (OUTQSIZE - 1);
            pData[i] = m_OutQueueMono[m_OutQTail];
            m_OutQLevel += (OUTQSIZE / 4);
            underflow = true;
        }
    }

    // ------- DEBUG: Queue status after GetOutQueue -------
  //  qDebug() << "[GetOutQueue:MONO] Queue level after:" << m_OutQLevel;

    if (m_BlockingMode) return;

    m_AveOutQLevel = (1.0 - FILTERQLEVEL_ALPHA) * m_AveOutQLevel + FILTERQLEVEL_ALPHA * m_OutQLevel;

    if (underflow) {
        // Additional underflow marker
        qWarning() << "[GetOutQueue:MONO] Underflow - m_AveOutQLevel reset to" << m_AveOutQLevel;
    }

    m_RateUpdateCount += numsamples;
    if (m_RateUpdateCount >= SOUNDCARD_RATE) {
        CalcError();
        m_RateUpdateCount = 0;
    }
}

void CSoundOut::GetOutQueue(int numsamples, TYPESTEREO16* pData)
{
    int i;
    bool underflow = false;

    QMutexLocker locker(&m_Mutex);

    if (m_Startup) {
        for (i = 0; i < numsamples; i++) {
            pData[i].re = 0;
            pData[i].im = 0;
        }

        if (m_OutQLevel > OUTQSIZE / 2) {
            m_Startup = false;
            m_RateUpdateCount = -5 * SOUNDCARD_RATE;
            m_PpmError = 0;
            m_AveOutQLevel = m_OutQLevel;
            m_UpdateToggle = true;
        } else {
            return;
        }
    }

    for (i = 0; i < numsamples; i++) {
        if (m_OutQHead != m_OutQTail) {
            pData[i] = m_OutQueueStereo[m_OutQTail++];
            m_OutQTail &= (OUTQSIZE - 1);
            m_OutQLevel--;
        } else {
            // ------- DEBUG: Underflow event -------
            qWarning() << "[GetOutQueue:STEREO] Snd Underflow at" << QDateTime::currentDateTime().toString()
                       << "Queue level:" << m_OutQLevel;
            m_OutQTail -= (OUTQSIZE / 4);
            m_OutQTail &= (OUTQSIZE - 1);
            pData[i] = m_OutQueueStereo[m_OutQTail];
            m_OutQLevel += (OUTQSIZE / 4);
            underflow = true;
        }
    }

    // ------- DEBUG: Queue status after GetOutQueue -------
  // qDebug() << "[GetOutQueue:STEREO] Queue level after:" << m_OutQLevel;

    if (m_BlockingMode) return;

    m_AveOutQLevel = (1.0 - FILTERQLEVEL_ALPHA) * m_AveOutQLevel + FILTERQLEVEL_ALPHA * m_OutQLevel;

    if (underflow) {
        // Additional underflow marker
        qWarning() << "[GetOutQueue:STEREO] Underflow - m_AveOutQLevel reset to" << m_AveOutQLevel;
    }

    m_RateUpdateCount += numsamples;
    if (m_RateUpdateCount >= SOUNDCARD_RATE) {
        CalcError();
        m_RateUpdateCount = 0;
    }
}

////////////////////////////////////////////////////////////////
// Called alternately from the Get routines to update the
// error correction process
////////////////////////////////////////////////////////////////
void CSoundOut::CalcError()
{
    double error;
    error = (double)(m_AveOutQLevel - OUTQSIZE/2 );	//neg==level is too low  pos == level is to high
    error = error * P_GAIN;
    m_RateCorrection = error;
    m_PpmError = (int)( m_RateCorrection*1e6 );
    if( abs(m_PpmError) > 500)
    {
        qDebug()<<"SoundOut "<<m_PpmError << m_AveOutQLevel;
    }
}

// In audio thread (run)
void CSoundOut::run()
{
    int iter_count = 0;
    while (!threadQuit) {
        if (m_pAudioSink->state() == QAudio::IdleState ||
            m_pAudioSink->state() == QAudio::ActiveState) {

            qint64 len = m_pAudioSink->bytesFree();

            if (len > 0) {
                if (len > SOUND_WRITEBUFSIZE)
                    len = SOUND_WRITEBUFSIZE;

                if (m_StereoOut) {
                    len &= ~(0x03);
                    GetOutQueue(len / 4, (TYPESTEREO16*)m_pData);
                } else {
                    len &= ~(0x01);
                    GetOutQueue(len / 2, (TYPEMONO16*)m_pData);
                }

                m_pOutput->write((char*)m_pData, len);
            } else {
                msleep(m_BlockTime);
            }
        } else {
            qWarning() << "[AudioThread] SoundOut Error - State: " << m_pAudioSink->state();
            threadQuit = true;
        }
    }
    qDebug() << "[AudioThread] Sound thread exit";
}
