#pragma once

#include <QThread>
#include <QMutex>
#include <QAudioDevice>
#include <QAudioSink>
#include <QIODevice>
#include "fractresampler.h"
#include <alsa/asoundlib.h>
#include <alsa/mixer.h>
#include <atomic>


#define OUTQSIZE 4096
#define SOUND_WRITEBUFSIZE 4096

// Forward declaration for the resampler class
class CFractResampler;

class CSoundOut : public QThread
{
    Q_OBJECT
public:
    explicit CSoundOut(QObject *parent = nullptr);
    ~CSoundOut();

    bool Start(int OutDevIndx, bool StereoOut, double UsrDataRate, bool BlockingMode);
    void Stop();
    void ChangeUserDataRate(double UsrDataRate);
    void SetVolume(qint32 vol);

    void PutOutQueue(int numsamples, TYPECPX *pData);
    void PutOutQueue(int numsamples, TYPEREAL *pData);

protected:
    void run() override;

private:
    void GetOutQueue(int numsamples, TYPEMONO16 *pData);
    void GetOutQueue(int numsamples, TYPESTEREO16 *pData);
    void CalcError();

    QObject *m_pParent;
    QAudioDevice m_OutDeviceInfo;  // Qt6 device representation
    QAudioSink *m_pAudioSink;      // Changed from QAudioOutput to QAudioSink
    QIODevice *m_pOutput;

    double m_UserDataRate;
    double m_OutRatio;
    double m_RateCorrection;
    double m_Gain;
    double m_AveOutQLevel;
    double m_BlockTime;
    int m_OutQHead, m_OutQTail, m_OutQLevel;
    int m_RateUpdateCount;
    int m_PpmError;
    bool m_Startup;
    bool m_BlockingMode;
    bool m_StereoOut;
    bool m_UpdateToggle;

    QMutex m_Mutex;

    // Output Queues
    TYPEMONO16 m_OutQueueMono[OUTQSIZE];
    TYPESTEREO16 m_OutQueueStereo[OUTQSIZE];

    // Data buffer for audio writes
    char m_pData[SOUND_WRITEBUFSIZE];

    // Resampler
    CFractResampler m_OutResampler;
};
