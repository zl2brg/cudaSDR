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
#include <QList>
#include <cmath>
#include <random>
#include <vector>

SimulatedDevice::SimulatedDevice(QObject* parent)
    : QObject(parent)
{
    m_rxFrequencies[0] = 14200000;
}

SimulatedDevice::~SimulatedDevice()
{
    stop();
}

int SimulatedDevice::blockIntervalMs() const
{
    const int rate = qMax(1, m_sampleRate);
    return qMax(1, (kIqBlockSize * 1000) / rate);
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
    if (m_running.load(std::memory_order_acquire))
        return true;
    if (!m_rxTimer) {
        m_rxTimer = new QTimer(this);
        m_rxTimer->setTimerType(Qt::PreciseTimer);
        connect(m_rxTimer, &QTimer::timeout, this, &SimulatedDevice::emitSyntheticRx);
    }
    m_running.store(true, std::memory_order_release);
    m_rxTimer->start(blockIntervalMs());
    return true;
}

void SimulatedDevice::stop()
{
    if (m_rxTimer)
        m_rxTimer->stop();
    m_running.store(false, std::memory_order_release);
    m_ptt.store(false, std::memory_order_release);
}

bool SimulatedDevice::setSampleRate(int rate)
{
    if (rate <= 0) return false;
    if (!capabilities().supportedSampleRates.contains(rate)) return false;
    m_sampleRate = rate;
    if (m_rxTimer && m_rxTimer->isActive())
        m_rxTimer->setInterval(blockIntervalMs());
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
    m_rxIngest.setCallback(std::move(callback));
}

int SimulatedDevice::readRxIq(int rx, float* destination, int maxSamples)
{
    if (!destination || maxSamples <= 0) {
        return 0;
    }

    const int fromBuffer = m_rxIngest.read(rx, destination, maxSamples);
    if (fromBuffer > 0) {
        return fromBuffer;
    }

    generateSamples(destination, maxSamples);
    return maxSamples;
}

void SimulatedDevice::notifyRxIq(int rx, const float* buffer, int count)
{
    m_rxIngest.notify(rx, buffer, count);
}

void SimulatedDevice::emitSyntheticRx()
{
    if (!m_running.load(std::memory_order_acquire))
        return;

    std::vector<float> buffer(static_cast<size_t>(kIqBlockSize) * 2);
    generateSamples(buffer.data(), kIqBlockSize);

    QList<int> rxs = m_rxFrequencies.keys();
    if (rxs.isEmpty())
        rxs.append(0);
    for (int rx : rxs)
        notifyRxIq(rx, buffer.data(), kIqBlockSize);
}
