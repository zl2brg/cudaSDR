/**
* @file  cusdr_spectrumBinWorker.cpp
* @brief Off-GUI FFT→pan/waterfall max-hold binning worker
*/

#include "cusdr_spectrumBinWorker.h"

SpectrumBinWorker::SpectrumBinWorker(QObject* parent)
    : QThread(parent)
{
    qRegisterMetaType<SpectrumBinWorker::Result>("SpectrumBinWorker::Result");
}

SpectrumBinWorker::~SpectrumBinWorker()
{
    stop();
    wait();
}

void SpectrumBinWorker::submit(const Request& request)
{
    QMutexLocker lock(&m_mutex);
    m_request = request;
    m_pending = true;
    m_condition.wakeOne();
}

void SpectrumBinWorker::stop()
{
    QMutexLocker lock(&m_mutex);
    m_stop = true;
    m_pending = false;
    m_condition.wakeOne();
}

bool SpectrumBinWorker::isBusy() const
{
    QMutexLocker lock(&m_mutex);
    return m_busy;
}

void SpectrumBinWorker::run()
{
    for (;;) {
        Request req;
        {
            QMutexLocker lock(&m_mutex);
            while (!m_stop && !m_pending)
                m_condition.wait(&m_mutex);
            if (m_stop)
                return;
            req = std::move(m_request);
            m_pending = false;
            m_busy = true;
        }

        Result out;
        out.generation = req.generation;
        out.bins = DisplayUtils::binPanadapterSpectrum(
            req.spectrum, req.spectrum, req.params, req.peakHoldBins);

        {
            QMutexLocker lock(&m_mutex);
            m_busy = false;
        }

        emit binsReady(out);
    }
}
