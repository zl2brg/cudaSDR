#include "cusdr_iambic.h"
//
// Created by Simon Eatough, ZL2BRG on 24/03/22
//

#include <QThread>

#define KEYER_STRAIGHT 0
#define KEYER_MODE_A 1
#define KEYER_MODE_B 2
#define NSEC_PER_SEC   1000000000

#define LOG_CW_ENGINE
#ifdef LOG_CW_ENGINE
#define CW_ENGINE_DEBUG qDebug().nospace() << "CW Engine::\t"
#else
#   define CW_ENGINE_DEBUG nullDebug()
#endif

int cw_keyer_spacing=10;

void iambic::keyer_event(int left, int state) {
    bool cw_key_hit;

    if (state) {
        cw_key_hit = true;
    }
    if (left) {
        // left paddle hit or released
        kcwl = state;
        if (state) *kmeml=1;  // trigger dot/dash memory
    } else {
        // right paddle hit or released
        kcwr = state;
        if (state) *kmemr=1;  // trigger dot/dash memory
    }
    if (state) {
        cw_event = true;
    }

}

iambic::iambic(QObject *parent) : QThread(parent)
  , set(Settings::instance())
{
radioState = set->getRadioState();
txmode = set->getDSPMode(0);
    dot_length = 1200 / cw_keyer_speed;
    // will be 3 * dot length at standard weight
    dash_length = (dot_length * 3 * cw_keyer_weight) / 50;

    bool cw_keys_reversed = 0;
    if (cw_keys_reversed) {
        kdot  = &kcwr;
        kdash = &kcwl;
        kmemr = &dot_memory;
        kmeml = &dash_memory;
    } else {
        kdot  = &kcwl;
        kdash = &kcwr;
        kmeml = &dot_memory;
        kmemr = &dash_memory;
    }

}




iambic::~iambic(){
Stop();
}

void iambic::Start(){
  start(QThread::NormalPriority);
}



void  iambic::Stop(){

    m_ThreadQuit = true;
    msleep(100);

}


void iambic::run()
{
        struct timespec loop_delay;
        int interval = 1000000; // 1 ms
        int i;
        int kdelay;
        int old_volume;
        bool cw_breakin = false;
        bool cw_not_ready = false;
        bool enforce_cw_vox = false;
        cwvox=0;
        while(!m_ThreadQuit) {

            if (cw_event) {
                cw_event = false;
                if (!kcwl && !kcwr) continue;
                // check mode: to not induce RX/TX transition if not in CW mode
                if (!radioState != MOX && cw_breakin && (txmode == CWU || txmode == CWL)) {
                    // Wait for mox, that is, wait for WDSP shutting down the RX and
                    // firing up the TX. This induces a small delay when hitting the key for
                    // the first time, but excludes that the first dot is swallowed.
                    // Note: if out-of-band, mox will never come, therefore
                    // give up after 200 msec.
                    i = 200;
              //      while ((radioState != MOX || cw_not_ready) && i-- > 0) usleep(1000L);
                //    cwvox = (int) cw_keyer_hang_time;
                }
                // Trigger VOX if CAT CW was active and we have interrupted it by hitting a key
                if (enforce_cw_vox) cwvox = (int) cw_keyer_hang_time;

                key_state = CHECK;

                while (key_state != EXITLOOP || cwvox > 0) {
                    //
                    // if key_state == EXITLOOP and cwvox == 0, then
                    // just leave the while-loop without removing MOX
                    //
                    // re-trigger VOX if *not* busy-spinning
                    if (cwvox > 0 && key_state != EXITLOOP && key_state != CHECK) cwvox = (int) cw_keyer_hang_time;
                    switch (key_state) {

                        case EXITLOOP:
                            // If we arrive here, cwvox is greater than zero, since key_state==EXITLOOP
                            // AND cwvox==0 leaves the outer "while" loop.
                            cwvox--;
                            // If CW-vox still hanging, continue "busy-spinning"
                            if (cwvox == 0) {
                                // we have just reduced cwvox from 1 to 0.
                                ext_mox_update();
                            } else {
                                key_state = CHECK;
                            }
                            break;

                        case CHECK: // check for key press
                            key_state = EXITLOOP;  // default next state
                            // Do not decrement cwvox until zero here, otherwise
                            // we won't enter the code 10 lines above that de-activates MOX.
                            if (cwvox > 1) cwvox--;
                            if (cw_keyer_mode == KEYER_STRAIGHT) {       // Straight/External key or bug
                                if (*kdash) {                  // send manual dashes
                                    set_keyer_out(1);
                                    clock_gettime(CLOCK_MONOTONIC, &loop_delay);
                                    // wait until dash is released. Check once a milli-sec
                                    while (*kdash) {


                                        loop_delay.tv_nsec += interval;
                                        while (loop_delay.tv_nsec >= NSEC_PER_SEC) {
                                            loop_delay.tv_nsec -= NSEC_PER_SEC;
                                            loop_delay.tv_sec++;
                                        }
                                        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &loop_delay, NULL);
                                       //msleep(1);
                                    }

                                // dash released.
                                set_keyer_out(0);
                                // since we stay in CHECK mode, re-trigger cwvox here
                                cwvox = cw_keyer_hang_time;
                                }
                                if (*kdot) {
                                    // "bug" mode: dot key activates automatic dots
                                    key_state = SENDDOT;
                                }
                            }
                                // end of KEYER_STRAIGHT case
                             else {
                                // Paddle
                                // If both following if-statements are true, which one should win?
                                // I think a "simultaneous squeeze" means a dot-dash sequence, since in
                                // a dash-dot sequence there is a larger time window to hit the dot.
                                if (*kdash) key_state = SENDDASH;
                                if (*kdot) key_state = SENDDOT;
                            }
                            break;

                        case SENDDOT:
                            dash_memory = 0;
                            dash_held = *kdash;
                            set_keyer_out(1);
                            // Wait one dot length, then key-up
                            clock_gettime(CLOCK_MONOTONIC, &loop_delay);
                            // Wait one dot length, then key-up
                            loop_delay.tv_nsec += 1000000 * dot_length;
                            while (loop_delay.tv_nsec >= NSEC_PER_SEC) {
                                loop_delay.tv_nsec -= NSEC_PER_SEC;
                                loop_delay.tv_sec++;
                            }
                            clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &loop_delay, NULL);
                            set_keyer_out(0);
                            key_state = DOTDELAY;       // add inter-character spacing of one dot length
                            kdelay = 0;
                            break;

                        case DOTDELAY:
                            kdelay++;
            if (kdelay > dot_length) {
                                if (cw_keyer_mode == KEYER_STRAIGHT) {
                                    // bug mode: continue sending dots or exit, depending on current dot key status
                                    key_state = EXITLOOP;
                                    if (*kdot) key_state = SENDDOT;
                                    // end of bug/straight case
                                } else {
//
//                  DL1YCF:
//                  This is my understanding where MODE A comes in:
//                  If at the end of the delay, BOTH keys are
//                  released, then do not start the next element.
//                  However, if  the dash has been hit DURING the preceeding
//                  dot, produce a dash in either case
//
                                    if (cw_keyer_mode == KEYER_MODE_A && !*kdot && !*kdash) dash_held = 0;

                                    if (dash_memory || *kdash || dash_held)
                                        key_state = SENDDASH;
                                    else if (*kdot)                                 // dot still held, so send a dot
                                        key_state = SENDDOT;
                                    else if (cw_keyer_spacing) {
                                        dot_memory = dash_memory = 0;
                                        key_state = LETTERSPACE;
                                        kdelay = 0;
                                    } else
                                        key_state = EXITLOOP;
                                    // end of iambic case
                                }
                            }
                            break;

                        case SENDDASH:
                            dot_memory = 0;
                            dot_held = *kdot;  // remember if dot is still held at beginning of the dash
                            set_keyer_out(1);
                            clock_gettime(CLOCK_MONOTONIC, &loop_delay);
                            // Wait one dash length and then key-up
                            loop_delay.tv_nsec += 1000000L * dash_length;
                            while (loop_delay.tv_nsec >= NSEC_PER_SEC) {
                                loop_delay.tv_nsec -= NSEC_PER_SEC;
                                loop_delay.tv_sec++;
                            }
                            clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME,  &loop_delay, NULL);
                            set_keyer_out(0);
                            key_state = DASHDELAY;       // add inter-character spacing of one dot length
                            kdelay = 0;
                            // do not fall through, update VOX at beginning of loop
                            break;

                        case DASHDELAY:
                            // we never arrive here in STRAIGHT/BUG mode
                            kdelay++;
                            if (kdelay > dot_length) {
//
//                  DL1YCF:
//                  This is my understanding where MODE A comes in:
//                  If at the end of the dash delay, BOTH keys are
//                  released, then do not start the next element.
//                  However, if  the dot has been hit DURING the preceeding
//                  dash, produce a dot in either case
//
                                if (cw_keyer_mode == KEYER_MODE_A && !*kdot && !*kdash) dot_held = 0;
                                if (dot_memory || *kdot || dot_held)
                                    key_state = SENDDOT;
                                else if (*kdash)
                                    key_state = SENDDASH;
                                else if (cw_keyer_spacing) {
                                    dot_memory = dash_memory = 0;
                                    key_state = LETTERSPACE;
                                    kdelay = 0;
                                } else key_state = EXITLOOP;
                            }
                            break;

                        case LETTERSPACE:
                            // Add letter space (3 x dot delay) to end of character and check if a paddle is pressed during this time.
                            // Actually add 2 x dot_length since we already have a dot delay at the end of the character.
                            kdelay++;
                            if (kdelay > 2 * dot_length) {
                                if (dot_memory)         // check if a dot or dash paddle was pressed during the delay.
                                    key_state = SENDDOT;
                                else if (dash_memory)
                                    key_state = SENDDASH;
                                else key_state = EXITLOOP;   // no memories set so restart
                            }
                            break;

                        default:
                            fprintf(stderr, "KEYER THREAD: unknown state=%d", (int) key_state);
                            key_state = EXITLOOP;
                    }

                }


            }

//        CW_ENGINE_DEBUG <<  "CW running";

    }

}



int iambic::get_tx_mode() {
    return 0;
}

void iambic::ext_mox_update() {

}

void iambic::set_keyer_out(int i) {
    emit key_down(i);
}
