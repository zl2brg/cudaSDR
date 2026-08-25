#include "cusdr_iambic.h"
#include <time.h>
#include <QtGlobal>
#include <QDebug>

iambic::iambic(QObject *parent)
    : QThread(parent)
    , m_settings(Settings::instance())
{
    if (m_settings) {
        m_cwKeyerMode = m_settings->getCwKeyerMode();
        m_cwKeyerSpeed = m_settings->getCwKeyerSpeed();
        m_cwKeyerWeight = m_settings->getCwKeyerWeight();
        m_cwKeyerSpacing = m_settings->getCwKeyerSpacing();
        m_cwKeysReversed = m_settings->isCwKeyReversed();
        m_cwHangTime = m_settings->getCwHangTime();

        connect(m_settings, &Settings::CwKeyerModeChanged, this, &iambic::setKeyerMode);
        connect(m_settings, &Settings::CwKeyerSpeedChanged, this, &iambic::setKeyerSpeed);
        connect(m_settings, &Settings::CwKeyerWeightChanged, this, &iambic::setKeyerWeight);
        connect(m_settings, &Settings::CwKeyerSpacingChanged, this, &iambic::setKeyerSpacing);
        connect(m_settings, &Settings::CwKeyReversedChanged, this, &iambic::setKeyReversed);
        connect(m_settings, &Settings::CwHangTimeChanged, this, &iambic::setHangTime);
    }
    updateLengths();
    setupPointers();
}

iambic::~iambic() {
    Stop();
}

void iambic::Start() {
    start(QThread::TimeCriticalPriority);
}

void iambic::Stop() {
    {
        QMutexLocker locker(&m_mutex);
        m_threadQuit = true;
        m_waitCond.wakeAll();
    }
    wait(500);
}

void iambic::updateLengths() {
    const int spd = qBound(1, m_cwKeyerSpeed, 100);
    m_dotLengthMs = 1200 / spd;
    const int wt = qBound(10, m_cwKeyerWeight, 90);
    m_dashLengthMs = (m_dotLengthMs * 3 * wt) / 50;
}

void iambic::setupPointers() {
    // keyer_event: channel 0 -> kcwr (dash line), channel 1 -> kcwl (dot line).
    if (m_cwKeysReversed) {
        m_kdot = &m_kcwr;
        m_kdash = &m_kcwl;
        m_kmemLeft = &m_dashMemory;
        m_kmemRight = &m_dotMemory;
    } else {
        m_kdot = &m_kcwl;
        m_kdash = &m_kcwr;
        m_kmemLeft = &m_dotMemory;
        m_kmemRight = &m_dashMemory;
    }
}

void iambic::setKeyerMode(int mode) {
    QMutexLocker locker(&m_mutex);
    m_cwKeyerMode = mode;
}

void iambic::setKeyerSpeed(int wpm) {
    QMutexLocker locker(&m_mutex);
    m_cwKeyerSpeed = wpm;
    updateLengths();
}

void iambic::setKeyerWeight(int weight) {
    QMutexLocker locker(&m_mutex);
    m_cwKeyerWeight = weight;
    updateLengths();
}

void iambic::setKeyReversed(int reversed) {
    QMutexLocker locker(&m_mutex);
    m_cwKeysReversed = (reversed != 0);
    setupPointers();
}

void iambic::setHangTime(int hangTime) {
    QMutexLocker locker(&m_mutex);
    m_cwHangTime = hangTime;
}

void iambic::setKeyerSpacing(int spacing) {
    QMutexLocker locker(&m_mutex);
    m_cwKeyerSpacing = spacing;
}

void iambic::keyer_event(int channel, int state) {
    QMutexLocker locker(&m_mutex);
    if (channel) {
        m_kcwl = state;
        if (state && m_kmemLeft) *m_kmemLeft = 1;
    } else {
        m_kcwr = state;
        if (state && m_kmemRight) *m_kmemRight = 1;
    }
    if (state) {
        m_cwEvent = true;
        m_waitCond.wakeOne();
    }
}

void iambic::set_keyer_out(int state) {
    emit key_down(state);
}

void iambic::run() {
    const int checkIntervalUs = 1000; // 1 ms sampling resolution during delays
    int kdelay = 0;
    m_cwvox = 0;

    while (!m_threadQuit) {
        {
            QMutexLocker locker(&m_mutex);
            while (!m_threadQuit && !m_cwEvent && m_kcwl == 0 && m_kcwr == 0 && m_dotMemory == 0 && m_dashMemory == 0 && m_cwvox == 0) {
                m_waitCond.wait(&m_mutex);
            }
            m_cwEvent = false;
        }

        if (m_threadQuit) break;

        m_keyState = CHECK;

        while (m_keyState != EXITLOOP || m_cwvox > 0) {
            if (m_threadQuit) break;

            if (m_cwvox > 0 && m_keyState != EXITLOOP && m_keyState != CHECK) {
                m_cwvox = m_cwHangTime;
            }

            switch (m_keyState) {
            case EXITLOOP:
                if (m_cwvox > 0) {
                    m_cwvox--;
                    if (m_cwvox == 0) {
                        // Hang time expired
                    } else {
                        struct timespec ts = {0, checkIntervalUs * 1000};
                        nanosleep(&ts, nullptr);
                        if (*m_kdot || *m_kdash || m_dotMemory || m_dashMemory) {
                            m_keyState = CHECK;
                        }
                    }
                }
                break;

            case CHECK:
                m_keyState = EXITLOOP;
                if (m_cwvox > 1) m_cwvox--;

                if (m_cwKeyerMode == KEYER_STRAIGHT) {
                    // Straight / Bug mode
                    if (*m_kdash) {
                        set_keyer_out(1);
                        while (*m_kdash && !m_threadQuit) {
                            struct timespec ts = {0, checkIntervalUs * 1000};
                            nanosleep(&ts, nullptr);
                        }
                        set_keyer_out(0);
                        m_cwvox = m_cwHangTime;
                    }
                    if (*m_kdot) {
                        m_keyState = SENDDOT;
                    }
                } else {
                    // Iambic Paddle (Mode A, Mode B, Ultimatic)
                    if (m_dotMemory || *m_kdot) {
                        m_keyState = SENDDOT;
                    } else if (m_dashMemory || *m_kdash) {
                        m_keyState = SENDDASH;
                    }
                }
                break;

            case SENDDOT: {
                m_dotMemory = 0;
                m_dashHeld = *m_kdash;
                set_keyer_out(1);

                int remainingMs = m_dotLengthMs;
                while (remainingMs > 0 && !m_threadQuit) {
                    int sleepMs = qMin(remainingMs, 2);
                    struct timespec ts = {0, sleepMs * 1000000};
                    nanosleep(&ts, nullptr);
                    remainingMs -= sleepMs;
                    if (*m_kdash) m_dashMemory = 1;
                }

                set_keyer_out(0);
                m_keyState = DOTDELAY;
                kdelay = 0;
                break;
            }

            case DOTDELAY: {
                int remainingMs = m_dotLengthMs;
                while (remainingMs > 0 && !m_threadQuit) {
                    int sleepMs = qMin(remainingMs, 2);
                    struct timespec ts = {0, sleepMs * 1000000};
                    nanosleep(&ts, nullptr);
                    remainingMs -= sleepMs;
                    if (*m_kdash) m_dashMemory = 1;
                    if (*m_kdot) m_dotMemory = 1;
                }

                if (m_cwKeyerMode == KEYER_STRAIGHT) {
                    m_keyState = (*m_kdot) ? SENDDOT : EXITLOOP;
                } else {
                    if (m_cwKeyerMode == KEYER_MODE_A && !*m_kdot && !*m_kdash) {
                        m_dashHeld = 0;
                    }

                    if (m_dashMemory || *m_kdash || m_dashHeld) {
                        m_dashHeld = 0;
                        m_keyState = SENDDASH;
                    } else if (*m_kdot || m_dotMemory) {
                        m_keyState = SENDDOT;
                    } else if (m_cwKeyerSpacing > 0) {
                        m_dotMemory = m_dashMemory = 0;
                        m_keyState = LETTERSPACE;
                        kdelay = 0;
                    } else {
                        m_keyState = EXITLOOP;
                    }
                }
                break;
            }

            case SENDDASH: {
                m_dashMemory = 0;
                m_dotHeld = *m_kdot;
                set_keyer_out(1);

                int remainingMs = m_dashLengthMs;
                while (remainingMs > 0 && !m_threadQuit) {
                    int sleepMs = qMin(remainingMs, 2);
                    struct timespec ts = {0, sleepMs * 1000000};
                    nanosleep(&ts, nullptr);
                    remainingMs -= sleepMs;
                    if (*m_kdot) m_dotMemory = 1;
                }

                set_keyer_out(0);
                m_keyState = DASHDELAY;
                kdelay = 0;
                break;
            }

            case DASHDELAY: {
                int remainingMs = m_dotLengthMs;
                while (remainingMs > 0 && !m_threadQuit) {
                    int sleepMs = qMin(remainingMs, 2);
                    struct timespec ts = {0, sleepMs * 1000000};
                    nanosleep(&ts, nullptr);
                    remainingMs -= sleepMs;
                    if (*m_kdot) m_dotMemory = 1;
                    if (*m_kdash) m_dashMemory = 1;
                }

                if (m_cwKeyerMode == KEYER_MODE_A && !*m_kdot && !*m_kdash) {
                    m_dotHeld = 0;
                }

                if (m_dotMemory || *m_kdot || m_dotHeld) {
                    m_dotHeld = 0;
                    m_keyState = SENDDOT;
                } else if (*m_kdash || m_dashMemory) {
                    m_keyState = SENDDASH;
                } else if (m_cwKeyerSpacing > 0) {
                    m_dotMemory = m_dashMemory = 0;
                    m_keyState = LETTERSPACE;
                    kdelay = 0;
                } else {
                    m_keyState = EXITLOOP;
                }
                break;
            }

            case LETTERSPACE: {
                int remainingMs = 2 * m_dotLengthMs;
                while (remainingMs > 0 && !m_threadQuit) {
                    int sleepMs = qMin(remainingMs, 2);
                    struct timespec ts = {0, sleepMs * 1000000};
                    nanosleep(&ts, nullptr);
                    remainingMs -= sleepMs;
                    if (*m_kdot) m_dotMemory = 1;
                    if (*m_kdash) m_dashMemory = 1;
                }

                if (m_dotMemory || *m_kdot) {
                    m_keyState = SENDDOT;
                } else if (m_dashMemory || *m_kdash) {
                    m_keyState = SENDDASH;
                } else {
                    m_keyState = EXITLOOP;
                }
                break;
            }

            default:
                m_keyState = EXITLOOP;
                break;
            }
        }
    }
}
