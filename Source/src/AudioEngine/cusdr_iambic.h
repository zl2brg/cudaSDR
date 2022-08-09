#ifndef CWENGINE_H
#define CWENGINE_H
//
// Created by Simon Eatough, ZL2BRG on 24/03/22
//

#include <QObject>
#include <QList>
#include "QMutex"
#include "stdint.h"
#include "cusdr_settings.h"


enum {
    CHECK = 0,
    SENDDOT,
    SENDDASH,
    DOTDELAY,
    DASHDELAY,
    LETTERSPACE,
    EXITLOOP
};

class iambic : public QThread
{
    Q_OBJECT
public:
    explicit  iambic(QObject *parent = NULL);
    ~iambic();
    void Setup();
    void Stop();
    void Start();
    void run();
    public:


    private:
    Settings		*set;

        bool m_BlockingMode = false;
        volatile bool m_ThreadQuit = false;
        bool m_Startup = false;
        volatile bool m_running = false;
        volatile bool cw_event = false;
        int dot_memory = 0;
        int dash_memory = 0;
        int dot_held = 0;
        int dash_held = 0;
        int key_state = 0;
        int dot_length = 1000;
        int dash_length = 0;
        volatile int kcwl = 0;
        volatile int kcwr = 0;

        volatile int *kdot;
        volatile int *kdash;
        volatile int *kmemr;
        volatile int *kmeml;
        RadioState radioState;
        DSPMode txmode;
        volatile int cwvox;
        int cw_keyer_hang_time=10;
        int cw_keyer_mode;
        int cw_keyer_speed = 12;
        int cw_keyer_weight = 50;

public:

signals:
    void key_down(int count);

public slots:
    void keyer_event(int left, int state);

    private slots:

    int get_tx_mode();

    void ext_mox_update();

    void set_keyer_out(int i);
};




#endif // CWENGINE_H
