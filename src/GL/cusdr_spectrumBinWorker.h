/**
* @file  cusdr_spectrumBinWorker.h
* @brief Off-GUI FFT→pan/waterfall max-hold binning worker
*/

#ifndef CUSDR_SPECTRUM_BIN_WORKER_H
#define CUSDR_SPECTRUM_BIN_WORKER_H

#include "Util/display_utils.h"

#include <QMetaType>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>

/**
 * Runs DisplayUtils::binPanadapterSpectrum on a dedicated thread so paintGL
 * only uploads already-binned pan/waterfall pixels.
 *
 * submit() coalesces: while busy, the latest request replaces any pending one.
 */
class SpectrumBinWorker : public QThread {
    Q_OBJECT

public:
    struct Request {
        QVector<float> spectrum;
        DisplayUtils::PanBinParams params;
        QVector<float> peakHoldBins;
        quint64 generation = 0;
    };

    struct Result {
        DisplayUtils::PanBinResult bins;
        quint64 generation = 0;
    };

    explicit SpectrumBinWorker(QObject* parent = nullptr);
    ~SpectrumBinWorker() override;

    void submit(const Request& request);
    void stop();
    bool isBusy() const;

signals:
    void binsReady(SpectrumBinWorker::Result result);

protected:
    void run() override;

private:
    mutable QMutex m_mutex;
    QWaitCondition m_condition;
    bool m_stop = false;
    bool m_pending = false;
    bool m_busy = false;
    Request m_request;
};

Q_DECLARE_METATYPE(SpectrumBinWorker::Result)

#endif // CUSDR_SPECTRUM_BIN_WORKER_H
