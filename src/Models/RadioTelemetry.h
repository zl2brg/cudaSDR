#ifndef RADIOTELEMETRY_H
#define RADIOTELEMETRY_H

#include <QObject>
#include <QVector>

#include "cusdr_settings.h"

class RadioModel;
class SliceModel;

/** Live SDR telemetry (spectrum, meters, link/PA status). Not persisted in Settings. */
class RadioTelemetry : public QObject {
    Q_OBJECT

public:
    explicit RadioTelemetry(RadioModel* radioModel, QObject* parent = nullptr);

    void setSpectrumBuffer(int rx, const qVectorFloat& buffer);
    void setPostSpectrumBuffer(int rx, const float* buffer);
    void setSMeterValue(int rx, double value);

    void setProtocolSync(int value);
    void setADCOverflow(int value);
    void setPacketLoss(int value);
    void setForwardPower(qreal watts);
    void setReversePower(qreal watts);
    void setSWR(qreal swr);
    void setSupplyVoltage(qreal volts);
    void setTemperature(qreal temp);
    void setSendIQ(int value);
    void setRcveIQ(int value);

    void setWidebandSpectrumBuffer(const qVectorFloat& buffer);
    void resetWidebandSpectrumBuffer();
    void setWidebandFrequencyRange(qreal lowHz, qreal highHz);

signals:
    void spectrumBufferChanged(int rx, const qVectorFloat& buffer);
    void postSpectrumBufferChanged(int rx, const float* buffer);

    void protocolSyncChanged(int value);
    void adcOverflowChanged(int value);
    void packetLossChanged(int value);
    void forwardPowerChanged(qreal watts);
    void reversePowerChanged(qreal watts);
    void swrChanged(qreal swr);
    void supplyVoltageChanged(qreal volts);
    void temperatureChanged(qreal temp);
    void sendIQSignalChanged(int value);
    void rcveIQSignalChanged(int value);

    void widebandSpectrumBufferChanged(const qVectorFloat& buffer);
    void widebandSpectrumBufferReset();
    void widebandFrequencyRangeChanged(qreal lowHz, qreal highHz);

private:
    SliceModel* sliceForRx(int rx) const;

    RadioModel* m_radioModel = nullptr;
};

/** Convenience for DataEngine / protocol code paths that only have Settings. */
RadioTelemetry* telemetryFromSettings();

#endif // RADIOTELEMETRY_H
