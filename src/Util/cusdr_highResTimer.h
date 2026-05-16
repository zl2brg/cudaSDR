#ifndef CUSDR_HRES_TIMER_H
#define CUSDR_HRES_TIMER_H

#include <QElapsedTimer>

class HResTimer {

public:
	HResTimer();
   	~HResTimer();

   	void   start();
   	void   stop();
   	double getElapsedTime();
   	double getElapsedTimeInSec();
   	double getElapsedTimeInMilliSec();
   	double getElapsedTimeInMicroSec();

private:
    QElapsedTimer m_timer;
    qint64 m_elapsedNs;
    bool m_stopped;
};

#endif // CUSDR_HRES_TIMER_H
