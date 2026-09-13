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
#include "Models/RadioModel.h"
#include "cusdr_settings.h"

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
    // Protocol 1/2 encode LO from Settings / SliceModel; DataEngine listens to
    // ctrFrequencyChanged and pins which RX to send (including P2 HP).
    Settings::instance()->setCtrFrequency(1, rx, freqHz);
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

    // Protocol 1/2 encode TX LO from RadioModel::effectiveTxFrequency()
    // (TX slice when split, otherwise the current receiver). DataEngine
    // listens to ctrFrequencyChanged and pushes Protocol 2 HP.
    Settings* set = Settings::instance();
    int txRx = set->getCurrentReceiver();
    if (RadioModel* rm = set->radioModel()) {
        if (rm->txSliceIndex() >= 0)
            txRx = rm->txSliceIndex();
        rm->txParams().txFrequency = static_cast<double>(freqHz);
    }
    set->setCtrFrequency(1, txRx, freqHz);
    return true;
}

bool HpsdrDevice::setTxGain(double gainDb)
{
    m_txGain = gainDb;
    // HPSDR TX "gain" is Penelope/Hermes drive 0-100. Protocol 1 writes it
    // as C1; Protocol 2 scales it to 0-255 on DUC0. DataEngine copies
    // driveLevelChanged into txParams().drivelevel.
    const int drive = qBound(0, qRound(gainDb), 100);
    if (RadioModel* rm = Settings::instance()->radioModel())
        rm->txParams().drivelevel = static_cast<uchar>(drive);
    Settings::instance()->setDriveLevel(drive);
    return true;
}

bool HpsdrDevice::setRxGain(int rx, double gainDb)
{
    m_rxGains[rx] = gainDb;
    // Mercury/Hermes attenuator: 0/1/2/3 → 0/10/20/30 dB. Gain is the inverse.
    const int att = qBound(0, qRound(-gainDb / 10.0), 3);
    Settings::instance()->setMercuryAttenuator(att);
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
    m_rxIngest.setCallback(std::move(callback));
}

int HpsdrDevice::readRxIq(int rx, float* destination, int maxSamples)
{
    return m_rxIngest.read(rx, destination, maxSamples);
}

void HpsdrDevice::notifyRxIq(int rx, const float* buffer, int count)
{
    m_rxIngest.notify(rx, buffer, count);
}
