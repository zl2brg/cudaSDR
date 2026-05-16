#ifndef CPUMONITOR_H
#define CPUMONITOR_H

#include <QObject>
#include <QTimer>
#include <QThread>

#if defined(Q_OS_WIN32)
#include <windows.h>
#else
#include <sys/types.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/times.h>
#endif

class CPUMonitor : public QObject {
    Q_OBJECT
public:
    explicit CPUMonitor(QObject *parent = nullptr);
    ~CPUMonitor() override;

public slots:
    void start();
    void stop();

signals:
    void cpuLoadChanged(int load);

private slots:
    void update();

private:
    QTimer* m_timer;
    
#if defined(Q_OS_WIN32)
    FILETIME m_ftPrevSysKernel;
    FILETIME m_ftPrevSysUser;
    FILETIME m_ftPrevProcKernel;
    FILETIME m_ftPrevProcUser;
    bool m_firstRun;
    ULONGLONG subtractTimes(const FILETIME& ftA, const FILETIME& ftB);
#else
    clock_t m_ptick;
    double m_ptime;
    int m_clockTick;
#endif
};

#endif // CPUMONITOR_H
