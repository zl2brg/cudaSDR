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

#ifndef CUDASDR_ISDR_DEVICE_H
#define CUDASDR_ISDR_DEVICE_H

#include <QString>
#include <QList>
#include <QtGlobal>

/**
 * @enum DeviceType
 * @brief Categorizes the underlying SDR hardware protocol or subsystem.
 */
enum class DeviceType {
    None,
    HpsdrP1,
    HpsdrP2,
    SoapySDR,
    Simulated
};

/**
 * @struct DeviceCapabilities
 * @brief Describes hardware capabilities to decouple core DSP from driver-specific checks.
 */
struct DeviceCapabilities {
    bool supportsTx = false;
    bool supportsFullDuplex = false;
    bool supportsWideband = false;
    int maxReceivers = 1;
    QList<int> supportedSampleRates = {48000};
    qint64 minFrequencyHz = 0;
    qint64 maxFrequencyHz = 60000000;
};

/**
 * @class ISdrDevice
 * @brief Unified hardware abstraction interface for physical, networked, and simulated SDR devices.
 */
class ISdrDevice {
public:
    virtual ~ISdrDevice() = default;

    virtual QString deviceName() const = 0;
    virtual DeviceType deviceType() const = 0;
    virtual DeviceCapabilities capabilities() const = 0;

    virtual bool isRunning() const = 0;
    virtual bool start() = 0;
    virtual void stop() = 0;

    virtual bool setSampleRate(int rate) = 0;
    virtual int sampleRate() const = 0;

    virtual bool setFrequency(int rx, qint64 freqHz) = 0;
    virtual qint64 frequency(int rx) const = 0;

    virtual bool setTxFrequency(qint64 freqHz) = 0;
    virtual qint64 txFrequency() const = 0;

    virtual bool setTxGain(double gainDb) = 0;
    virtual bool setRxGain(int rx, double gainDb) = 0;

    virtual bool setPtt(bool active) = 0;
    virtual bool isPtt() const = 0;

    virtual void sendTxIq(const float* buffer, int count) = 0;
};

#endif // CUDASDR_ISDR_DEVICE_H
