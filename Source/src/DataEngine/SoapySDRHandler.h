/**
 * @file  SoapySDRHandler.h
 * @brief SoapySDR hardware transport stub.
 *
 * SoapySDR is a vendor-neutral C++ API that supports RTL-SDR, SDRplay,
 * HackRF, PlutoSDR, LimeSDR and many others.  This handler wraps the
 * SoapySDR stream API behind IHardwareIO so DataEngine and DataProcessor
 * can use any supported device with no protocol-specific changes.
 *
 * This is a stub ready for implementation.  When complete it should:
 *  - Call SoapySDR::Device::make() during initIO() using the device
 *    args stored in TNetworkDevicecard.soapy.
 *  - Open an RX stream (SOAPY_SDR_CF32) and push decoded samples onto
 *    de->io.iq_queue, emitting readydata() per batch.
 *  - For TX-capable devices, implement sendAudio() via a TX stream.
 *  - Map sendCommand(0)/sendCommand(1) to stream activate/deactivate.
 *
 * Reference: https://github.com/pothosware/SoapySDR/wiki
 */

#ifndef SOAPYSDRHANDLER_H
#define SOAPYSDRHANDLER_H

#include "IHardwareIO.h"
#include "cusdr_settings.h"

class SoapySDRHandler : public IHardwareIO {
    Q_OBJECT

public:
    explicit SoapySDRHandler(THPSDRParameter* ioData, QObject* parent = nullptr);
    ~SoapySDRHandler() override;

public slots:
    // IHardwareIO interface
    void initIO()                      override;
    void stop()                        override;
    void writeData()                   override;
    void sendAudio(unsigned char* buf) override;
    void sendInitFrames(int rx)        override;
    void sendCommand(char cmd)         override;
    void set_wbBuffers(int val)        override;

private:
    Settings*        set;
    THPSDRParameter* io;
    volatile bool    m_stopped;

    // TODO: SoapySDR::Device* m_device;
    // TODO: SoapySDR::Stream* m_rxStream;
    // TODO: SoapySDR::Stream* m_txStream;

private slots:
    void setSampleRate(QObject* sender, int value);
};

#endif // SOAPYSDRHANDLER_H
