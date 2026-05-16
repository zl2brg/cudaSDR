#include "CPUMonitor.h"
#include <QDebug>

CPUMonitor::CPUMonitor(QObject *parent) : QObject(parent) {
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &CPUMonitor::update);

#if defined(Q_OS_WIN32)
    m_firstRun = true;
    ZeroMemory(&m_ftPrevSysKernel, sizeof(FILETIME));
    ZeroMemory(&m_ftPrevSysUser, sizeof(FILETIME));
    ZeroMemory(&m_ftPrevProcKernel, sizeof(FILETIME));
    ZeroMemory(&m_ftPrevProcUser, sizeof(FILETIME));
#else
    m_ptick = 0;
    m_ptime = 0.0;
    m_clockTick = sysconf(_SC_CLK_TCK);
#endif
}

CPUMonitor::~CPUMonitor() {
    stop();
}

void CPUMonitor::start() {
    m_timer->start(1000);
}

void CPUMonitor::stop() {
    m_timer->stop();
}

void CPUMonitor::update() {
#if defined(Q_OS_WIN32)
    FILETIME ftSysIdle, ftSysKernel, ftSysUser;
    FILETIME ftProcCreation, ftProcExit, ftProcKernel, ftProcUser;

    if (!GetSystemTimes(&ftSysIdle, &ftSysKernel, &ftSysUser) ||
        !GetProcessTimes(GetCurrentProcess(), &ftProcCreation, &ftProcExit, &ftProcKernel, &ftProcUser))
    {
        return;
    }

    if (!m_firstRun) {
        ULONGLONG ftSysKernelDiff = subtractTimes(ftSysKernel, m_ftPrevSysKernel);
        ULONGLONG ftSysUserDiff = subtractTimes(ftSysUser, m_ftPrevSysUser);
        ULONGLONG ftProcKernelDiff = subtractTimes(ftProcKernel, m_ftPrevProcKernel);
        ULONGLONG ftProcUserDiff = subtractTimes(ftProcUser, m_ftPrevProcUser);

        ULONGLONG nTotalSys = ftSysKernelDiff + ftSysUserDiff;
        ULONGLONG nTotalProc = ftProcKernelDiff + ftProcUserDiff;

        if (nTotalSys > 0) {
            int load = static_cast<int>((100.0 * nTotalProc) / nTotalSys);
            emit cpuLoadChanged(load);
        }
    }
    
    m_ftPrevSysKernel = ftSysKernel;
    m_ftPrevSysUser = ftSysUser;
    m_ftPrevProcKernel = ftProcKernel;
    m_ftPrevProcUser = ftProcUser;
    m_firstRun = false;

#else
    clock_t tick;
    double time;
    double load;
    struct rusage usage;
    struct tms systime;

    tick = times(&systime);
    getrusage(RUSAGE_SELF, &usage);

    time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec * 1e-6 +
           usage.ru_stime.tv_sec + usage.ru_stime.tv_usec * 1e-6;

    if (m_ptick && m_ptime) {
        load = ((time - m_ptime) / (tick - m_ptick)) * m_clockTick * 100;
        emit cpuLoadChanged(static_cast<int>(load));
    }

    m_ptick = tick;
    m_ptime = time;
#endif
}

#if defined(Q_OS_WIN32)
ULONGLONG CPUMonitor::subtractTimes(const FILETIME& ftA, const FILETIME& ftB) {
    LARGE_INTEGER a, b;
    a.LowPart = ftA.dwLowDateTime;
    a.HighPart = ftA.dwHighDateTime;
    b.LowPart = ftB.dwLowDateTime;
    b.HighPart = ftB.dwHighDateTime;
    return a.QuadPart - b.QuadPart;
}
#endif
