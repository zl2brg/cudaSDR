/**
 * @file  SoapySDRHandler.cpp
 * @brief SoapySDR hardware transport — stub implementation.
 *
 * TODO: Implement SoapySDR stream open, readStream loop, and TX path.
 */

#include "SoapySDRHandler.h"
#include <QDebug>

SoapySDRHandler::SoapySDRHandler(THPSDRParameter* ioData, QObject* parent)
    : IHardwareIO(parent)
    , set(Settings::instance())
    , io(ioData)
    , m_stopped(false)
{
    connect(set, &Settings::sampleRateChanged,
            this, &SoapySDRHandler::setSampleRate);
}

SoapySDRHandler::~SoapySDRHandler()
{
    if (!m_stopped) stop();
}

void SoapySDRHandler::initIO()
{
    // TODO: SoapySDR::Device* dev = SoapySDR::Device::make(args)
    // TODO: dev->setSampleRate(SOAPY_SDR_RX, 0, samplerate)
    // TODO: m_rxStream = dev->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32)
    // TODO: dev->activateStream(m_rxStream)
    // TODO: start readStream loop (push CF32 samples onto io->iq_queue, emit readydata())
    qDebug() << "SoapySDRHandler::initIO() — stub, not yet implemented";
    emit messageEvent("SoapySDR: initIO() stub — not implemented");
}

void SoapySDRHandler::stop()
{
    m_stopped = true;
    // TODO: dev->deactivateStream(m_rxStream)
    // TODO: dev->closeStream(m_rxStream)
    // TODO: SoapySDR::Device::unmake(m_device)
    qDebug() << "SoapySDRHandler::stop() — stub";
}

void SoapySDRHandler::writeData()
{
    // TODO: write TX IQ samples via dev->writeStream(m_txStream, ...)
}

void SoapySDRHandler::sendAudio(unsigned char* buf)
{
    Q_UNUSED(buf)
    // SoapySDR devices typically handle audio out-of-band or not at all.
    // For devices with TX capability, route mic audio to TX stream here.
}

void SoapySDRHandler::sendInitFrames(int rx)
{
    Q_UNUSED(rx)
    // TODO: dev->setFrequency(SOAPY_SDR_RX, rx, frequency)
}

void SoapySDRHandler::sendCommand(char cmd)
{
    // 0 = stop, non-zero = start
    if (cmd == 0) stop();
    else          initIO();
}

void SoapySDRHandler::set_wbBuffers(int val)
{
    Q_UNUSED(val)
    // SoapySDR does not use the Metis wideband buffer concept.
    // Implement wideband spectrum capture separately if needed.
}

void SoapySDRHandler::setSampleRate(QObject* sender, int value)
{
    Q_UNUSED(sender)
    Q_UNUSED(value)
    // TODO: dev->setSampleRate(SOAPY_SDR_RX, 0, value)
}
