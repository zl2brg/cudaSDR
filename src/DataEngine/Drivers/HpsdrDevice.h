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

#ifndef CUDASDR_HPSDR_DEVICE_H
#define CUDASDR_HPSDR_DEVICE_H

#include "DataEngine/ISdrDevice.h"
#include <QObject>
#include <QMap>
#include <atomic>

class DataIO;
class IHPSDRProtocol;

/**
 * @class HpsdrDevice
 * @brief Driver adapter encapsulating OpenHPSDR Protocol 1 and Protocol 2 network devices.
 */
class HpsdrDevice : public QObject, public ISdrDevice {
    Q_OBJECT

public:
    explicit HpsdrDevice(DataIO* dataIO, IHPSDRProtocol* protocol, bool isProtocol2 = false, QObject* parent = nullptr);
    ~HpsdrDevice() override;

    // ISdrDevice interface
    QString deviceName() const override;
    DeviceType deviceType() const override { return m_isProtocol2 ? DeviceType::HpsdrP2 : DeviceType::HpsdrP1; }
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

    void setProtocol(IHPSDRProtocol* protocol) { m_protocol = protocol; }
    void setDeviceName(const QString& name) { m_deviceName = name; }

private:
    DataIO* m_dataIO = nullptr;
    IHPSDRProtocol* m_protocol = nullptr;
    bool m_isProtocol2 = false;
    QString m_deviceName = QStringLiteral("OpenHPSDR Device");

    std::atomic<bool> m_running{false};
    std::atomic<bool> m_ptt{false};
    int m_sampleRate = 48000;
    qint64 m_txFrequency = 14200000;
    QMap<int, qint64> m_rxFrequencies;
    QMap<int, double> m_rxGains;
    double m_txGain = 0.0;
};

#endif // CUDASDR_HPSDR_DEVICE_H
