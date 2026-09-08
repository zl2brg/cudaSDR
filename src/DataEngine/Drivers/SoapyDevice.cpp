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

#include "SoapyDevice.h"
#include "cusdr_settings.h"
#include "DataEngine/cusdr_dataEngine.h"
#include "DataEngine/cusdr_dataIO.h"

#ifdef HAVE_SOAPYSDR
#include "DataEngine/SoapySDRDataSource.h"
#endif

SoapyDevice::SoapyDevice(DataEngine* engine, SoapySDRDataSource* source, QObject* parent)
    : QObject(parent)
    , m_engine(engine)
    , m_source(source)
{
    m_rxFrequencies[0] = 14200000;
}

SoapyDevice::~SoapyDevice()
{
    stop();
}

QString SoapyDevice::deviceName() const
{
    Settings* set = Settings::instance();
    if (set && !set->getSoapyHardwareKey().isEmpty()) {
        return set->getSoapyHardwareKey();
    }
    return QStringLiteral("SoapySDR Device");
}

DeviceCapabilities SoapyDevice::capabilities() const
{
    DeviceCapabilities caps;
    Settings* set = Settings::instance();
    caps.supportsTx = true;
    caps.supportsFullDuplex = set ? set->getTxFullDuplex() : false;
    caps.supportsWideband = true;
    caps.maxReceivers = 1;
    caps.supportedSampleRates = {48000, 96000, 192000, 384000, 768000, 1536000, 2048000};
    caps.minFrequencyHz = 100000;
    caps.maxFrequencyHz = 2000000000;
    return caps;
}

bool SoapyDevice::start()
{
    m_running.store(true, std::memory_order_release);
    return true;
}

void SoapyDevice::stop()
{
    m_running.store(false, std::memory_order_release);
    m_ptt.store(false, std::memory_order_release);
}

bool SoapyDevice::setSampleRate(int rate)
{
    if (rate <= 0) return false;
    m_sampleRate = rate;
#ifdef HAVE_SOAPYSDR
    if (m_source) {
        m_source->setSampleRate(rate);
    }
#endif
    return true;
}

bool SoapyDevice::setFrequency(int rx, qint64 freqHz)
{
    if (freqHz < 0) return false;
    m_rxFrequencies[rx] = freqHz;
#ifdef HAVE_SOAPYSDR
    if (m_source) {
        m_source->setFrequency(rx, freqHz);
    }
#endif
    return true;
}

qint64 SoapyDevice::frequency(int rx) const
{
    return m_rxFrequencies.value(rx, 14200000);
}

bool SoapyDevice::setTxFrequency(qint64 freqHz)
{
    if (freqHz < 0) return false;
    m_txFrequency = freqHz;
    return true;
}

bool SoapyDevice::setTxGain(double gainDb)
{
    m_txGain = gainDb;
    return true;
}

bool SoapyDevice::setRxGain(int rx, double gainDb)
{
    m_rxGains[rx] = gainDb;
    return true;
}

bool SoapyDevice::setPtt(bool active)
{
    m_ptt.store(active, std::memory_order_release);
    return true;
}

void SoapyDevice::sendTxIq(const float* buffer, int count)
{
    if (!buffer || count <= 0 || !m_engine || !m_engine->m_dataIO) {
        return;
    }

    if (m_engine->m_dataIO->soapy_tx_iq_queue.isFull()) {
        return;
    }

    Settings* set = Settings::instance();
    const bool negateQ = set ? set->getSoapyHardwareKey().contains(QStringLiteral("Lime"), Qt::CaseInsensitive) : false;

    QVector<float> soapyTxIq(count * 2);
    for (int j = 0; j < count; ++j) {
        soapyTxIq[j * 2]     = buffer[j * 2];
        soapyTxIq[j * 2 + 1] = negateQ ? -buffer[j * 2 + 1] : buffer[j * 2 + 1];
    }

    m_engine->m_dataIO->soapy_tx_iq_queue.enqueue(soapyTxIq);
}

void SoapyDevice::setRxIqCallback(RxIqCallback callback)
{
    m_rxCallback = std::move(callback);
}

int SoapyDevice::readRxIq(int rx, float* destination, int maxSamples)
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

    if (!m_engine || !m_engine->m_dataIO) {
        return 0;
    }

    if (m_engine->m_dataIO->soapy_iq_queue.isEmpty()) {
        return 0;
    }

    QVector<float> samples = m_engine->m_dataIO->soapy_iq_queue.dequeue();
    const int available = samples.size() / 2;
    const int toCopy = std::min(maxSamples, available);
    std::memcpy(destination, samples.constData(), toCopy * 2 * sizeof(float));

    if (m_rxCallback) {
        m_rxCallback(rx, destination, toCopy);
    }
    return toCopy;
}

void SoapyDevice::notifyRxIq(int rx, const float* buffer, int count)
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
