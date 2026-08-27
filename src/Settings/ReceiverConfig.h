#ifndef RECEIVERCONFIG_H
#define RECEIVERCONFIG_H

#include <QObject>
#include <QJsonObject>
#include <QList>
#include "SettingsTypes.h"
#include "cusdr_hamDatabase.h"

class QSettings;
struct _receiver;
typedef struct _receiver TReceiver;

/**
 * Persistence DTO for per-receiver fields that the INI currently stores.
 *
 * Not a live runtime store: DSP/UI use SliceModel (and TReceiver for residual
 * fields). Load paths parse into this DTO then applyTo(TReceiver); save paths
 * populate from TReceiver (after syncSettingsWithSlices) then write.
 *
 * INI loadIni/saveIni still cover only the original subset so they do not
 * duplicate keys that Settings::loadSettings/saveSettings still write.
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
    /** Matches MAX_BANDS / HamBand count. */
    static constexpr int kBandCount = 22;

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

    PanGraphicsMode panMode() const { return m_panMode; }
    void setPanMode(PanGraphicsMode mode);
    WaterfallColorMode waterfallMode() const { return m_waterfallMode; }
    void setWaterfallMode(WaterfallColorMode mode);
    PanAveragingMode panAvMode() const { return m_panAvMode; }
    void setPanAvMode(PanAveragingMode mode);
    PanDetectorMode panDetMode() const { return m_panDetMode; }
    void setPanDetMode(PanDetectorMode mode);

    int fftSize() const { return m_fftSize; }
    void setFftSize(int size);
    int framesPerSecond() const { return m_framesPerSecond; }
    void setFramesPerSecond(int fps);
    int averagingCnt() const { return m_averagingCnt; }
    void setAveragingCnt(int count);
    int waterfallOffsetLo() const { return m_waterfallOffsetLo; }
    void setWaterfallOffsetLo(int offset);
    int waterfallOffsetHi() const { return m_waterfallOffsetHi; }
    void setWaterfallOffsetHi(int offset);
    int filterIndex() const { return m_filterIndex; }
    void setFilterIndex(int index);

    float freqRulerPosition() const { return m_freqRulerPosition; }
    void setFreqRulerPosition(float pos);
    float audioVolume() const { return m_audioVolume; }
    void setAudioVolume(float volume);
    qreal mouseWheelFreqStep() const { return m_mouseWheelFreqStep; }
    void setMouseWheelFreqStep(qreal step);
    qreal filterLo() const { return m_filterLo; }
    void setFilterLo(qreal freq);
    qreal filterHi() const { return m_filterHi; }
    void setFilterHi(qreal freq);

    qreal agcGain() const { return m_agcGain; }
    void setAgcGain(qreal gain);
    qreal agcFixedGain() const { return m_agcFixedGain; }
    void setAgcFixedGain(qreal gain);
    int agcMaximumGain() const { return m_agcMaximumGain; }
    void setAgcMaximumGain(int gain);
    int agcSlope() const { return m_agcSlope; }
    void setAgcSlope(int slope);
    qreal agcAttackTime() const { return m_agcAttackTime; }
    void setAgcAttackTime(qreal seconds);
    qreal agcDecayTime() const { return m_agcDecayTime; }
    void setAgcDecayTime(qreal seconds);
    qreal agcHangTime() const { return m_agcHangTime; }
    void setAgcHangTime(qreal seconds);

    int nr() const { return m_nr; }
    void setNr(int mode);
    int nrAgc() const { return m_nrAgc; }
    void setNrAgc(int mode);
    int nbMode() const { return m_nbMode; }
    void setNbMode(int mode);
    int nr2GainMethod() const { return m_nr2GainMethod; }
    void setNr2GainMethod(int method);
    int nr2NpeMethod() const { return m_nr2NpeMethod; }
    void setNr2NpeMethod(int method);
    bool nr2Ae() const { return m_nr2Ae; }
    void setNr2Ae(bool enabled);
    bool anf() const { return m_anf; }
    void setAnf(bool enabled);
    bool snb() const { return m_snb; }
    void setSnb(bool enabled);

    bool agcLines() const { return m_agcLines; }
    void setAgcLines(bool enabled);
    bool panLocked() const { return m_panLocked; }
    void setPanLocked(bool locked);
    bool spectrumAveraging() const { return m_spectrumAveraging; }
    void setSpectrumAveraging(bool enabled);
    bool hairCross() const { return m_hairCross; }
    void setHairCross(bool enabled);
    bool panGrid() const { return m_panGrid; }
    void setPanGrid(bool enabled);
    bool peakHold() const { return m_peakHold; }
    void setPeakHold(bool enabled);
    bool clickVFO() const { return m_clickVFO; }
    void setClickVFO(bool enabled);
    bool cwDecode() const { return m_cwDecode; }
    void setCwDecode(bool enabled);

    QList<qint64> lastCenterFrequencyList() const { return m_lastCenterFrequencyList; }
    void setLastCenterFrequencyList(const QList<qint64> &values);
    QList<qint64> lastVfoFrequencyList() const { return m_lastVfoFrequencyList; }
    void setLastVfoFrequencyList(const QList<qint64> &values);
    QList<int> mercuryAttenuators() const { return m_mercuryAttenuators; }
    void setMercuryAttenuators(const QList<int> &values);
    QList<qreal> dBmPanScaleMinList() const { return m_dBmPanScaleMinList; }
    void setdBmPanScaleMinList(const QList<qreal> &values);
    QList<qreal> dBmPanScaleMaxList() const { return m_dBmPanScaleMaxList; }
    void setdBmPanScaleMaxList(const QList<qreal> &values);
    QList<DSPMode> dspModeList() const { return m_dspModeList; }
    void setDspModeList(const QList<DSPMode> &values);

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
    void ensureBandLists();

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

    PanGraphicsMode m_panMode = Line;
    WaterfallColorMode m_waterfallMode = Enhanced;
    PanAveragingMode m_panAvMode = AV_MODE_RECURSIVE;
    PanDetectorMode m_panDetMode = DET_MODE_ROSENFELL;
    int m_fftSize = 1;
    int m_framesPerSecond = 25;
    int m_averagingCnt = 5;
    int m_waterfallOffsetLo = -5;
    int m_waterfallOffsetHi = 20;
    int m_filterIndex = 0;
    float m_freqRulerPosition = 0.5f;
    float m_audioVolume = 0.10f;
    qreal m_mouseWheelFreqStep = 100.0;
    qreal m_filterLo = -3050.0;
    qreal m_filterHi = -150.0;
    qreal m_agcGain = 100.0;
    qreal m_agcFixedGain = 30.0;
    int m_agcMaximumGain = 30;
    int m_agcSlope = 0;
    qreal m_agcAttackTime = 0.001;
    qreal m_agcDecayTime = 0.250;
    qreal m_agcHangTime = 0.100;
    int m_nr = 0;
    int m_nrAgc = 0;
    int m_nbMode = 0;
    int m_nr2GainMethod = 0;
    int m_nr2NpeMethod = 0;
    bool m_nr2Ae = false;
    bool m_anf = false;
    bool m_snb = false;
    bool m_agcLines = true;
    bool m_panLocked = false;
    bool m_spectrumAveraging = true;
    bool m_hairCross = false;
    bool m_panGrid = true;
    bool m_peakHold = false;
    bool m_clickVFO = false;
    bool m_cwDecode = false;

    QList<qint64> m_lastCenterFrequencyList;
    QList<qint64> m_lastVfoFrequencyList;
    QList<int> m_mercuryAttenuators;
    QList<qreal> m_dBmPanScaleMinList;
    QList<qreal> m_dBmPanScaleMaxList;
    QList<DSPMode> m_dspModeList;
};

#endif // RECEIVERCONFIG_H
