#ifndef CWENGINE_H
#define CWENGINE_H
//
// Created by Simon Eatough, ZL2BRG on 24/03/22
//

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include "cusdr_settings.h"

enum KeyerState {
    CHECK = 0,
    SENDDOT,
    SENDDASH,
    DOTDELAY,
    DASHDELAY,
    LETTERSPACE,
    EXITLOOP
};

enum KeyerMode {
    KEYER_STRAIGHT = 0,
    KEYER_MODE_A = 1,
    KEYER_MODE_B = 2,
    KEYER_ULTIMATIC = 3
};

class iambic : public QThread
{
    Q_OBJECT
public:
    explicit iambic(QObject *parent = nullptr);
    ~iambic() override;
    void Stop();
    void Start();
    void run() override;

signals:
    void key_down(int state);

public slots:
    /** Channel 0 = dash line, channel 1 = dot line (HPSDR ccRx convention). */
    void keyer_event(int channel, int state);
    void setKeyerMode(int mode);
    void setKeyerSpeed(int wpm);
    void setKeyerWeight(int weight);
    void setKeyReversed(int reversed);
    void setHangTime(int hangTime);
    void setKeyerSpacing(int spacing);

private:
    void updateLengths();
    void setupPointers();
    void set_keyer_out(int state);

    Settings*           m_settings;
    QMutex              m_mutex;
    QWaitCondition      m_waitCond;

    volatile bool       m_threadQuit = false;
    volatile bool       m_cwEvent = false;

    int                 m_dotMemory = 0;
    int                 m_dashMemory = 0;
    int                 m_dotHeld = 0;
    int                 m_dashHeld = 0;
    int                 m_keyState = CHECK;
    int                 m_dotLengthMs = 60;
    int                 m_dashLengthMs = 180;

    volatile int        m_kcwl = 0;
    volatile int        m_kcwr = 0;

    volatile int*       m_kdot = nullptr;
    volatile int*       m_kdash = nullptr;
    int*                m_kmemLeft = nullptr;
    int*                m_kmemRight = nullptr;

    int                 m_cwHangTime = 10;
    int                 m_cwKeyerMode = KEYER_MODE_B;
    int                 m_cwKeyerSpeed = 20;
    int                 m_cwKeyerWeight = 50;
    int                 m_cwKeyerSpacing = 0;
    bool                m_cwKeysReversed = false;
    volatile int        m_cwvox = 0;
};

#endif // CWENGINE_H
