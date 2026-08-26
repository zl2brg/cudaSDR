#ifndef RECEIVERCONFIG_H
#define RECEIVERCONFIG_H

#include <QObject>
#include <QJsonObject>
#include "SettingsTypes.h"
#include "cusdr_hamDatabase.h"

class QSettings;
struct _receiver;
typedef struct _receiver TReceiver;

/**
 * Ephemeral persistence DTO for a subset of per-receiver fields.
 *
 * Not a live runtime store: DSP/UI use SliceModel (and TReceiver for residual
 * fields). Load paths parse into this DTO then applyTo(TReceiver); save paths
 * populate from TReceiver (after syncSettingsWithSlices) then write.
 */
class ReceiverConfig : public QObject {
    Q_OBJECT
    Q_PROPERTY(int id READ id CONSTANT)
    Q_PROPERTY(QSDR::_DSPCore dspCore READ dspCore WRITE setDspCore NOTIFY dspCoreChanged)
    Q_PROPERTY(HamBand hamBand READ hamBand WRITE setHamBand NOTIFY hamBandChanged)
    Q_PROPERTY(DSPMode dspMode READ dspMode WRITE setDspMode NOTIFY dspModeChanged)
    Q_PROPERTY(ADCMode adcMode READ adcMode WRITE setAdcMode NOTIFY adcModeChanged)
    Q_PROPERTY(AGCMode agcMode READ agcMode WRITE setAgcMode NOTIFY agcModeChanged)
    Q_PROPERTY(qint64 ctrFrequency READ ctrFrequency WRITE setCtrFrequency NOTIFY ctrFrequencyChanged)
    Q_PROPERTY(qint64 vfoFrequency READ vfoFrequency WRITE setVfoFrequency NOTIFY vfoFrequencyChanged)
    Q_PROPERTY(qint64 vfoAFrequency READ vfoAFrequency WRITE setVfoAFrequency NOTIFY vfoAFrequencyChanged)
    Q_PROPERTY(qint64 vfoBFrequency READ vfoBFrequency WRITE setVfoBFrequency NOTIFY vfoBFrequencyChanged)
    Q_PROPERTY(int activeVfo READ activeVfo WRITE setActiveVfo NOTIFY activeVfoChanged)
    Q_PROPERTY(int filterSlope READ filterSlope WRITE setFilterSlope NOTIFY filterSlopeChanged)

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

    qint64 ctrFrequency() const { return m_ctrFrequency; }
    void setCtrFrequency(qint64 freq);

    qint64 vfoFrequency() const { return m_vfoFrequency; }
    void setVfoFrequency(qint64 freq);

    qint64 vfoAFrequency() const { return m_vfoAFrequency; }
    void setVfoAFrequency(qint64 freq);

    qint64 vfoBFrequency() const { return m_vfoBFrequency; }
    void setVfoBFrequency(qint64 freq);

    int activeVfo() const { return m_activeVfo; }
    void setActiveVfo(int vfo);

    int filterSlope() const { return m_filterSlope; }
    void setFilterSlope(int slope);

    /** Push DTO fields into the residual TReceiver channel (not a live copy). */
    void applyTo(TReceiver &rx) const;
    /** Populate DTO from TReceiver after syncSettingsWithSlices(). */
    void fromReceiver(const TReceiver &rx);

    void load(const QJsonObject &json);
    void save(QJsonObject &json) const;

    void loadIni(QSettings *settings);
    void saveIni(QSettings *settings) const;

signals:
    void dspCoreChanged(QSDR::_DSPCore core);
    void hamBandChanged(HamBand band);
    void dspModeChanged(DSPMode mode);
    void adcModeChanged(ADCMode mode);
    void agcModeChanged(AGCMode mode);
    void ctrFrequencyChanged(qint64 freq);
    void vfoFrequencyChanged(qint64 freq);
    void vfoAFrequencyChanged(qint64 freq);
    void vfoBFrequencyChanged(qint64 freq);
    void activeVfoChanged(int vfo);
    void filterSlopeChanged(int slope);

private:
    int m_id;
    QSDR::_DSPCore m_dspCore;
    HamBand m_hamBand;
    DSPMode m_dspMode;
    ADCMode m_adcMode;
    AGCMode m_agcMode;
    qint64 m_ctrFrequency;
    qint64 m_vfoFrequency;
    qint64 m_vfoAFrequency;
    qint64 m_vfoBFrequency;
    int m_activeVfo = 0;
    int m_filterSlope = 1;
};

#endif // RECEIVERCONFIG_H
