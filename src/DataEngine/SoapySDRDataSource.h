#ifndef SOAPYSDRDATASOURCE_H
#define SOAPYSDRDATASOURCE_H

#ifdef HAVE_SOAPYSDR

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QElapsedTimer>
#include <QVector>
#include <QMutex>
#include <atomic>
#include <complex>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Types.hpp>
#include <liquid/liquid.h>
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
    void setFrequency(int rx, qint64 frequency);

private:
    struct RfRatePlan {
        int rfSampleRate = 48000;
        int decimRatio = 1;
        int effectiveMinHz = 0;
    };

    RfRatePlan chooseRfSampleRate(int dspRate) const;
    bool syncRfRateFromHardware(int dspRate);
    bool restartRxStream();
    bool restartTxStream();
    void applyBandwidthForRfRate(int rfSampleRate);
    void applyLimeAutoCalibrate(bool enabled);
    void requestHardwareRetune();
    bool isLimeHardware() const;
    static bool isSampleRateCompatible(double rfHz, int dspHz);
    static int hardwareMinSampleRateHz(const std::string& hwKey, int driverReportedMin);
    void publishWidebandSpectrum(const float* interleavedIQ, int complexSamples);
    void publishWidebandFrequencyRange();
    void drainSoapyTxIqQueue();
    bool fillTxBufferFromRing(float *txBuff, int numComplexSamples);
    void clearTxIqRing();
    void configureTxSampleRate();

    void setupResamplers(int rxRfRate, int rxDspRate, int txRfRate, int txDspRate);

    Settings* set;
    THPSDRParameter* io;
    SoapySDR::Device* m_device;
    SoapySDR::Stream* m_rxStream;
    SoapySDR::Stream* m_txStream;

    volatile bool m_stopped;
    int m_sampleRate;      // DSP/WDSP processing rate (from Settings)
    int m_rfSampleRate;    // hardware RF sample rate (exact multiple of m_sampleRate)
    int m_decimRatio;      // m_rfSampleRate / m_sampleRate (averaging decimation factor)
    int m_minSampleRate;   // device hardware minimum, read-only after init()
    size_t m_numChannels;
    qint64  m_minFrequency;
    qint64  m_maxFrequency;

    // Pending hardware changes — written by slots (UI thread via DirectConnection),
    // consumed by runStream() loop so they take effect between readStream() calls.
    std::atomic<double> m_pendingFreq;
    std::atomic<bool>   m_freqPending;
    std::atomic<int>    m_pendingDspSampleRate;
    std::atomic<int>    m_pendingRfSampleRate;
    std::atomic<int>    m_pendingDecimRatio;
    std::atomic<bool>   m_sampleRatePending;

    // Last VFO seen by runStream() — used for polling-based frequency tracking
    // as a fallback when the signal/slot path doesn't fire.
    qint64 m_lastKnownVfo;
    int m_streamTimeouts;
    int m_txUnderrunCount;
    int m_txErrorCount;
    QElapsedTimer m_txLogTimer;
    std::complex<float> m_txTonePhase;
    bool m_txCapable;
    std::atomic<bool> m_txActive;
    std::atomic<int> m_radioStateValue;
    qint64 m_lastTxSetFrequency;
    bool m_txDebugPrimed;
    static constexpr int kTxIqSampleRate = 48000;
    int m_txSampleRate = kTxIqSampleRate;
    QVector<float> m_txIqRing;
    QMutex m_txIqMutex;

    // RX Resampler (RF -> DSP) — multi-stage for large decimation ratios (e.g. 125:1)
    msresamp_crcf m_rxResampler;
    liquid_float_complex* m_rxResampIn;
    liquid_float_complex* m_rxResampOut;

    // RX DC blocker state — single-pole IIR highpass on complex stream
    float m_dcBlockXprevI = 0.0f;
    float m_dcBlockXprevQ = 0.0f;
    float m_dcBlockYprevI = 0.0f;
    float m_dcBlockYprevQ = 0.0f;

    // TX Resampler (DSP -> RF) — multi-stage for large interpolation ratios
    msresamp_crcf m_txResampler;
    liquid_float_complex* m_txResampIn;
    liquid_float_complex* m_txResampOut;

signals:
    void messageEvent(QString message);
    void readydata();
    void widebandSpectrumReady(const qVectorFloat& buffer);
    void widebandSpectrumReset();
    void widebandFrequencyRangeReady(qreal lowHz, qreal highHz);
};

#endif // HAVE_SOAPYSDR

#endif // SOAPYSDRDATASOURCE_H
