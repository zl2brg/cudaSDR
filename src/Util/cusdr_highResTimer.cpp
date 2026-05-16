#include "cusdr_highResTimer.h"

HResTimer::HResTimer() : m_elapsedNs(0), m_stopped(false) {
    m_timer.start();
}

HResTimer::~HResTimer() {
}

void HResTimer::start() {
    m_stopped = false;
    m_timer.start();
}

void HResTimer::stop() {
    if (!m_stopped) {
        m_elapsedNs = m_timer.nsecsElapsed();
        m_stopped = true;
    }
}

double HResTimer::getElapsedTimeInMicroSec() {
    if (!m_stopped) {
        return m_timer.nsecsElapsed() / 1000.0;
    }
    return m_elapsedNs / 1000.0;
}

double HResTimer::getElapsedTimeInMilliSec() {
    return getElapsedTimeInMicroSec() * 0.001;
}

double HResTimer::getElapsedTimeInSec() {
    return getElapsedTimeInMicroSec() * 0.000001;
}

double HResTimer::getElapsedTime() {
    return getElapsedTimeInSec();
}
