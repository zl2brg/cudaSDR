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

#include "SimulatedDevice.h"
#include <cmath>
#include <random>

SimulatedDevice::SimulatedDevice(QObject* parent)
    : QObject(parent)
{
    m_rxFrequencies[0] = 14200000;
}

SimulatedDevice::~SimulatedDevice()
{
    stop();
}

DeviceCapabilities SimulatedDevice::capabilities() const
{
    DeviceCapabilities caps;
    caps.supportsTx = true;
    caps.supportsFullDuplex = true;
    caps.supportsWideband = false;
    caps.maxReceivers = 4;
    caps.supportedSampleRates = {48000, 96000, 192000, 384000};
    caps.minFrequencyHz = 100000;
    caps.maxFrequencyHz = 60000000;
    return caps;
}

bool SimulatedDevice::start()
{
    m_running.store(true, std::memory_order_release);
    return true;
}

void SimulatedDevice::stop()
{
    m_running.store(false, std::memory_order_release);
    m_ptt.store(false, std::memory_order_release);
}

bool SimulatedDevice::setSampleRate(int rate)
{
    if (rate <= 0) return false;
    if (!capabilities().supportedSampleRates.contains(rate)) return false;
    m_sampleRate = rate;
    return true;
}

bool SimulatedDevice::setFrequency(int rx, qint64 freqHz)
{
    if (freqHz < capabilities().minFrequencyHz || freqHz > capabilities().maxFrequencyHz) return false;
    m_rxFrequencies[rx] = freqHz;
    return true;
}

qint64 SimulatedDevice::frequency(int rx) const
{
    return m_rxFrequencies.value(rx, 14200000);
}

bool SimulatedDevice::setTxFrequency(qint64 freqHz)
{
    if (freqHz < capabilities().minFrequencyHz || freqHz > capabilities().maxFrequencyHz) return false;
    m_txFrequency = freqHz;
    return true;
}

bool SimulatedDevice::setTxGain(double gainDb)
{
    m_txGain = gainDb;
    return true;
}

bool SimulatedDevice::setRxGain(int rx, double gainDb)
{
    m_rxGains[rx] = gainDb;
    return true;
}

bool SimulatedDevice::setPtt(bool active)
{
    m_ptt.store(active, std::memory_order_release);
    return true;
}

void SimulatedDevice::sendTxIq(const float* buffer, int count)
{
    if (!buffer || count <= 0) return;
    m_txSamplesCount.fetch_add(static_cast<quint64>(count), std::memory_order_relaxed);
}

void SimulatedDevice::generateSamples(float* buffer, int count)
{
    if (!buffer || count <= 0) return;

    const double phaseInc = 2.0 * M_PI * m_toneOffsetHz / static_cast<double>(m_sampleRate);
    static thread_local std::mt19937 gen(1337);
    std::normal_distribution<float> dist(0.0f, static_cast<float>(m_noiseFloor));

    for (int i = 0; i < count; ++i) {
        float toneI = static_cast<float>(m_toneAmplitude * std::cos(m_phase));
        float toneQ = static_cast<float>(m_toneAmplitude * std::sin(m_phase));

        buffer[2 * i]     = toneI + dist(gen);
        buffer[2 * i + 1] = toneQ + dist(gen);

        m_phase += phaseInc;
        if (m_phase > 2.0 * M_PI) {
            m_phase -= 2.0 * M_PI;
        }
    }
}

void SimulatedDevice::setRxIqCallback(RxIqCallback callback)
{
    m_rxCallback = std::move(callback);
}

int SimulatedDevice::readRxIq(int rx, float* destination, int maxSamples)
{
    if (!destination || maxSamples <= 0) {
        return 0;
    }

    {
        QMutexLocker locker(&m_rxBufferMutex);
        auto it = m_rxBuffers.find(rx);
        if (it != m_rxBuffers.end() && !it.value().isEmpty()) {
            auto& buf = it.value();
            const int available = buf.size() / 2;
            const int toCopy = std::min(maxSamples, available);
            std::memcpy(destination, buf.constData(), toCopy * 2 * sizeof(float));
            buf.remove(0, toCopy * 2);
            if (m_rxCallback) {
                m_rxCallback(rx, destination, toCopy);
            }
            return toCopy;
        }
    }

    generateSamples(destination, maxSamples);
    if (m_rxCallback) {
        m_rxCallback(rx, destination, maxSamples);
    }
    return maxSamples;
}

void SimulatedDevice::notifyRxIq(int rx, const float* buffer, int count)
{
    if (!buffer || count <= 0) return;

    {
        QMutexLocker locker(&m_rxBufferMutex);
        auto& q = m_rxBuffers[rx];
        if (q.size() > 16384 * 2) {
            q.remove(0, count * 2);
        }
        const int oldSize = q.size();
        q.resize(oldSize + count * 2);
        std::memcpy(q.data() + oldSize, buffer, count * 2 * sizeof(float));
    }

    if (m_rxCallback) {
        m_rxCallback(rx, buffer, count);
    }
}
