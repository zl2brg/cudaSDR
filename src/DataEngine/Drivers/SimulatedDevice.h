/* Copyright (C)
 *
 * Simon Eatough ZL2BRG
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef CUDASDR_SIMULATED_DEVICE_H
#define CUDASDR_SIMULATED_DEVICE_H

#include "DataEngine/ISdrDevice.h"
#include <QObject>
#include <QVector>
#include <QMap>
#include <QMutex>
#include <atomic>

/**
 * @class SimulatedDevice
 * @brief Software-simulated SDR device for testing, simulation, and offline operation.
 *
 * Generates synthetic IQ test signals (tone + noise) and receives TX IQ without physical hardware.
 */
class SimulatedDevice : public QObject, public ISdrDevice {
    Q_OBJECT

public:
    explicit SimulatedDevice(QObject* parent = nullptr);
    ~SimulatedDevice() override;

    // ISdrDevice interface
    QString deviceName() const override { return QStringLiteral("Simulated SDR Device"); }
    DeviceType deviceType() const override { return DeviceType::Simulated; }
    DeviceCapabilities capabilities() const override;

    bool isRunning() const override { return m_running.load(std::memory_order_acquire); }
    bool start() override;
    void stop() override;

    bool setSampleRate(int rate) override;
    int sampleRate() const override { return m_sampleRate; }

    bool setFrequency(int rx, qint64 freqHz) override;
    qint64 frequency(int rx) const override;

    bool setTxFrequency(qint64 freqHz) override;
    qint64 txFrequency() const override { return m_txFrequency; }

    bool setTxGain(double gainDb) override;
    bool setRxGain(int rx, double gainDb) override;

    bool setPtt(bool active) override;
    bool isPtt() const override { return m_ptt.load(std::memory_order_acquire); }

    void sendTxIq(const float* buffer, int count) override;
    void setRxIqCallback(RxIqCallback callback) override;
    int readRxIq(int rx, float* destination, int maxSamples) override;
    void notifyRxIq(int rx, const float* buffer, int count) override;

    // Simulation controls
    void setToneOffsetHz(double hz) { m_toneOffsetHz = hz; }
    double toneOffsetHz() const { return m_toneOffsetHz; }
    void setToneAmplitude(double amp) { m_toneAmplitude = amp; }
    void setNoiseFloor(double noise) { m_noiseFloor = noise; }

    /**
     * @brief Generate synthetic interleaved I/Q samples into a caller-provided buffer.
     * @param buffer Output float buffer (must hold at least count * 2 elements)
     * @param count Number of complex I/Q sample pairs to generate
     */
    void generateSamples(float* buffer, int count);

    quint64 txSampleCount() const { return m_txSamplesCount.load(std::memory_order_acquire); }

signals:
    void samplesReady(const QVector<float>& interleavedIq);

private:
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_ptt{false};
    int m_sampleRate = 48000;
    qint64 m_txFrequency = 14200000; // 20m default
    QMap<int, qint64> m_rxFrequencies;
    QMap<int, double> m_rxGains;
    double m_txGain = 0.0;

    double m_toneOffsetHz = 1000.0; // 1 kHz test tone
    double m_toneAmplitude = 0.5;
    double m_noiseFloor = 0.001;
    double m_phase = 0.0;

    std::atomic<quint64> m_txSamplesCount{0};
    RxIqCallback m_rxCallback;
    mutable QMutex m_rxBufferMutex;
    QMap<int, QVector<float>> m_rxBuffers;
};

#endif // CUDASDR_SIMULATED_DEVICE_H
