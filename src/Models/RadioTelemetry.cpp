#include "RadioTelemetry.h"

#include "RadioModel.h"
#include "SliceModel.h"
#include "cusdr_settings.h"

RadioTelemetry::RadioTelemetry(RadioModel* radioModel, QObject* parent)
    : QObject(parent)
    , m_radioModel(radioModel)
{
}

SliceModel* RadioTelemetry::sliceForRx(int rx) const
{
    if (!m_radioModel || rx < 0 || rx >= m_radioModel->slices().size()) {
        return nullptr;
    }
    return m_radioModel->slices().at(rx);
}

void RadioTelemetry::setSpectrumBuffer(int rx, const qVectorFloat& buffer)
{
    emit spectrumBufferChanged(rx, buffer);
}

void RadioTelemetry::setPostSpectrumBuffer(int rx, const float* buffer)
{
    emit postSpectrumBufferChanged(rx, buffer);
}

void RadioTelemetry::setSMeterValue(int rx, double value)
{
    if (SliceModel* slice = sliceForRx(rx)) {
        slice->setSMeterValue(value);
    }
}

void RadioTelemetry::setProtocolSync(int value)
{
    emit protocolSyncChanged(value);
}

void RadioTelemetry::setADCOverflow(int value)
{
    emit adcOverflowChanged(value);
}

void RadioTelemetry::setPacketLoss(int value)
{
    emit packetLossChanged(value);
}

void RadioTelemetry::setForwardPower(qreal watts)
{
    emit forwardPowerChanged(watts);
}

void RadioTelemetry::setReversePower(qreal watts)
{
    emit reversePowerChanged(watts);
}

void RadioTelemetry::setSWR(qreal swr)
{
    emit swrChanged(swr);
}

void RadioTelemetry::setSupplyVoltage(qreal volts)
{
    emit supplyVoltageChanged(volts);
}

void RadioTelemetry::setTemperature(qreal temp)
{
    emit temperatureChanged(temp);
}

void RadioTelemetry::setSendIQ(int value)
{
    emit sendIQSignalChanged(value);
}

void RadioTelemetry::setRcveIQ(int value)
{
    emit rcveIQSignalChanged(value);
}

void RadioTelemetry::setWidebandSpectrumBuffer(const qVectorFloat& buffer)
{
    emit widebandSpectrumBufferChanged(buffer);
}

void RadioTelemetry::resetWidebandSpectrumBuffer()
{
    emit widebandSpectrumBufferReset();
}

void RadioTelemetry::setWidebandFrequencyRange(qreal lowHz, qreal highHz)
{
    emit widebandFrequencyRangeChanged(lowHz, highHz);
}

RadioTelemetry* telemetryFromSettings()
{
    Settings* settings = Settings::instance();
    if (!settings || !settings->radioModel()) {
        return nullptr;
    }
    return settings->radioModel()->telemetry();
}
