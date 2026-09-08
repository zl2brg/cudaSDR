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

#include "HpsdrDevice.h"
#include "DataEngine/cusdr_dataIO.h"
#include "DataEngine/IHPSDRProtocol.h"

HpsdrDevice::HpsdrDevice(DataIO* dataIO, IHPSDRProtocol* protocol, bool isProtocol2, QObject* parent)
    : QObject(parent)
    , m_dataIO(dataIO)
    , m_protocol(protocol)
    , m_isProtocol2(isProtocol2)
{
    m_rxFrequencies[0] = 14200000;
}

HpsdrDevice::~HpsdrDevice()
{
    stop();
}

QString HpsdrDevice::deviceName() const
{
    return m_deviceName;
}

DeviceCapabilities HpsdrDevice::capabilities() const
{
    DeviceCapabilities caps;
    caps.supportsTx = true;
    caps.supportsFullDuplex = m_isProtocol2;
    caps.supportsWideband = true;
    caps.maxReceivers = m_isProtocol2 ? 7 : 4;
    caps.supportedSampleRates = {48000, 96000, 192000, 384000};
    caps.minFrequencyHz = 100000;
    caps.maxFrequencyHz = 60000000;
    return caps;
}

bool HpsdrDevice::start()
{
    if (m_dataIO) {
        m_dataIO->networkDeviceStartStop(1);
    }
    m_running.store(true, std::memory_order_release);
    return true;
}

void HpsdrDevice::stop()
{
    if (m_dataIO) {
        m_dataIO->networkDeviceStartStop(0);
    }
    m_running.store(false, std::memory_order_release);
    m_ptt.store(false, std::memory_order_release);
}

bool HpsdrDevice::setSampleRate(int rate)
{
    if (rate <= 0) return false;
    m_sampleRate = rate;
    if (m_dataIO) {
        m_dataIO->setSampleRate(rate);
    }
    return true;
}

bool HpsdrDevice::setFrequency(int rx, qint64 freqHz)
{
    if (freqHz < 0) return false;
    m_rxFrequencies[rx] = freqHz;
    return true;
}

qint64 HpsdrDevice::frequency(int rx) const
{
    return m_rxFrequencies.value(rx, 14200000);
}

bool HpsdrDevice::setTxFrequency(qint64 freqHz)
{
    if (freqHz < 0) return false;
    m_txFrequency = freqHz;
    return true;
}

bool HpsdrDevice::setTxGain(double gainDb)
{
    m_txGain = gainDb;
    return true;
}

bool HpsdrDevice::setRxGain(int rx, double gainDb)
{
    m_rxGains[rx] = gainDb;
    return true;
}

bool HpsdrDevice::setPtt(bool active)
{
    m_ptt.store(active, std::memory_order_release);
    return true;
}

void HpsdrDevice::sendTxIq(const float* buffer, int count)
{
    Q_UNUSED(buffer)
    Q_UNUSED(count)
    // HPSDR TX samples are marshaled into the UDP transmitter buffer via DataProcessor.
}

void HpsdrDevice::setRxIqCallback(RxIqCallback callback)
{
    m_rxCallback = std::move(callback);
}

int HpsdrDevice::readRxIq(int rx, float* destination, int maxSamples)
{
    if (!destination || maxSamples <= 0) {
        return 0;
    }

    QMutexLocker locker(&m_rxBufferMutex);
    auto it = m_rxBuffers.find(rx);
    if (it == m_rxBuffers.end() || it.value().isEmpty()) {
        return 0;
    }

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

void HpsdrDevice::notifyRxIq(int rx, const float* buffer, int count)
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
