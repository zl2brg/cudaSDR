#ifndef RECEIVERCONFIG_H
#define RECEIVERCONFIG_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include "SettingsTypes.h"
#include "cusdr_hamDatabase.h"

class ReceiverConfig : public QObject {
    Q_OBJECT
    Q_PROPERTY(int id READ id CONSTANT)
    Q_PROPERTY(QSDR::_DSPCore dspCore READ dspCore WRITE setDspCore NOTIFY dspCoreChanged)
    Q_PROPERTY(HamBand hamBand READ hamBand WRITE setHamBand NOTIFY hamBandChanged)
    Q_PROPERTY(DSPMode dspMode READ dspMode WRITE setDspMode NOTIFY dspModeChanged)
    Q_PROPERTY(ADCMode adcMode READ adcMode WRITE setAdcMode NOTIFY adcModeChanged)
    Q_PROPERTY(AGCMode agcMode READ agcMode WRITE setAgcMode NOTIFY agcModeChanged)
    Q_PROPERTY(long ctrFrequency READ ctrFrequency WRITE setCtrFrequency NOTIFY ctrFrequencyChanged)
    Q_PROPERTY(long vfoFrequency READ vfoFrequency WRITE setVfoFrequency NOTIFY vfoFrequencyChanged)

public:
    explicit ReceiverConfig(int id, QObject *parent = nullptr);

    int id() const { return m_id; }

    QSDR::_DSPCore dspCore() const { return m_dspCore; }
    void setDspCore(QSDR::_DSPCore core);

    HamBand hamBand() const { return m_hamBand; }
    void setHamBand(HamBand band);

    DSPMode dspMode() const { return m_dspMode; }
    void setDspMode(DSPMode mode);

    ADCMode adcMode() const { return m_adcMode; }
    void setAdcMode(ADCMode mode);

    AGCMode agcMode() const { return m_agcMode; }
    void setAgcMode(AGCMode mode);

    long ctrFrequency() const { return m_ctrFrequency; }
    void setCtrFrequency(long freq);

    long vfoFrequency() const { return m_vfoFrequency; }
    void setVfoFrequency(long freq);

    void load(const QJsonObject &json);
    void save(QJsonObject &json) const;

signals:
    void dspCoreChanged(QSDR::_DSPCore core);
    void hamBandChanged(HamBand band);
    void dspModeChanged(DSPMode mode);
    void adcModeChanged(ADCMode mode);
    void agcModeChanged(AGCMode mode);
    void ctrFrequencyChanged(long freq);
    void vfoFrequencyChanged(long freq);

private:
    int m_id;
    QSDR::_DSPCore m_dspCore;
    HamBand m_hamBand;
    DSPMode m_dspMode;
    ADCMode m_adcMode;
    AGCMode m_agcMode;
    long m_ctrFrequency;
    long m_vfoFrequency;
};

#endif // RECEIVERCONFIG_H
