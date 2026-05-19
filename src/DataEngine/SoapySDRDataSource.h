#ifndef SOAPYSDRDATASOURCE_H
#define SOAPYSDRDATASOURCE_H

#ifdef HAVE_SOAPYSDR

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QElapsedTimer>
#include <atomic>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Types.hpp>
#include "cusdr_settings.h"
#include "Util/cusdr_queue.h"

class SoapySDRDataSource : public QObject {
    Q_OBJECT

public:
    explicit SoapySDRDataSource(THPSDRParameter *ioData);
    ~SoapySDRDataSource();

public slots:
    void init();
    void stop();
    void runStream();

private slots:
    void setSampleRate(int value);
    void setFrequency(int rx, long frequency);

private:
    Settings* set;
    THPSDRParameter* io;
    SoapySDR::Device* m_device;
    SoapySDR::Stream* m_rxStream;

    volatile bool m_stopped;
    int m_sampleRate;      // DSP/WDSP processing rate (from Settings)
    int m_rfSampleRate;    // hardware RF sample rate (exact multiple of m_sampleRate)
    int m_decimRatio;      // m_rfSampleRate / m_sampleRate (averaging decimation factor)
    int m_minSampleRate;   // device hardware minimum, read-only after init()
    size_t m_numChannels;
    long m_minFrequency;
    long m_maxFrequency;

    // Pending hardware changes — written by slots (UI thread via DirectConnection),
    // consumed by runStream() loop so they take effect between readStream() calls.
    std::atomic<double> m_pendingFreq;
    std::atomic<bool>   m_freqPending;
    std::atomic<int>    m_pendingRfSampleRate;
    std::atomic<int>    m_pendingDecimRatio;
    std::atomic<bool>   m_sampleRatePending;

    // Last VFO seen by runStream() — used for polling-based frequency tracking
    // as a fallback when the signal/slot path doesn't fire.
    long m_lastKnownVfo;

signals:
    void messageEvent(QString message);
    void readydata();
};

#endif // HAVE_SOAPYSDR

#endif // SOAPYSDRDATASOURCE_H
