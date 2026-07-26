#ifndef SLICEMODEL_H
#define SLICEMODEL_H

#include <QObject>
#include "cusdr_hamDatabase.h"
#include "Settings/SettingsTypes.h"

class SliceModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(int id READ id CONSTANT)
    Q_PROPERTY(qint64 frequency READ frequency WRITE setFrequency NOTIFY frequencyChanged)
    Q_PROPERTY(qint64 centerFrequency READ centerFrequency WRITE setCenterFrequency NOTIFY centerFrequencyChanged)
    Q_PROPERTY(DSPMode dspMode READ dspMode WRITE setDspMode NOTIFY dspModeChanged)
    Q_PROPERTY(float filterLow READ filterLow WRITE setFilterLow NOTIFY filterChanged)
    Q_PROPERTY(float filterHigh READ filterHigh WRITE setFilterHigh NOTIFY filterChanged)
    Q_PROPERTY(int filterPreset READ filterPreset WRITE setFilterPreset NOTIFY filterPresetChanged)
    Q_PROPERTY(int filterSlope READ filterSlope WRITE setFilterSlope NOTIFY filterSlopeChanged)
    Q_PROPERTY(float volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool mute READ mute WRITE setMute NOTIFY muteChanged)
    Q_PROPERTY(float pan READ pan WRITE setPan NOTIFY panChanged)
    Q_PROPERTY(AGCMode agcMode READ agcMode WRITE setAgcMode NOTIFY agcModeChanged)
    Q_PROPERTY(int agcGain READ agcGain WRITE setAgcGain NOTIFY agcGainChanged)
    Q_PROPERTY(int agcMaxGain READ agcMaxGain WRITE setAgcMaxGain NOTIFY agcMaxGainChanged)
    Q_PROPERTY(int agcFixedGain READ agcFixedGain WRITE setAgcFixedGain NOTIFY agcFixedGainChanged)
    Q_PROPERTY(int agcHangThreshold READ agcHangThreshold WRITE setAgcHangThreshold NOTIFY agcHangThresholdChanged)
    Q_PROPERTY(int agcSlope READ agcSlope WRITE setAgcSlope NOTIFY agcSlopeChanged)
    Q_PROPERTY(int nbMode READ nbMode WRITE setNbMode NOTIFY nbModeChanged)
    Q_PROPERTY(int nrMode READ nrMode WRITE setNrMode NOTIFY nrModeChanged)
    Q_PROPERTY(int nr2GainMethod READ nr2GainMethod WRITE setNr2GainMethod NOTIFY nr2GainMethodChanged)
    Q_PROPERTY(int nr2NpeMethod READ nr2NpeMethod WRITE setNr2NpeMethod NOTIFY nr2NpeMethodChanged)
    Q_PROPERTY(bool nr2Ae READ nr2Ae WRITE setNr2Ae NOTIFY nr2AeChanged)
    Q_PROPERTY(int nrAgc READ nrAgc WRITE setNrAgc NOTIFY nrAgcChanged)
    Q_PROPERTY(bool anf READ anf WRITE setAnf NOTIFY anfChanged)
    Q_PROPERTY(bool snb READ snb WRITE setSnb NOTIFY snbChanged)
    Q_PROPERTY(double sMeterValue READ sMeterValue WRITE setSMeterValue NOTIFY sMeterValueChanged)
    Q_PROPERTY(int sMeterHoldTime READ sMeterHoldTime WRITE setSMeterHoldTime NOTIFY sMeterHoldTimeChanged)
    Q_PROPERTY(int fftSize READ fftSize WRITE setFftSize NOTIFY fftSizeChanged)
    Q_PROPERTY(bool spectrumAveraging READ spectrumAveraging WRITE setSpectrumAveraging NOTIFY spectrumAveragingChanged)
    Q_PROPERTY(int spectrumAveragingCnt READ spectrumAveragingCnt WRITE setSpectrumAveragingCnt NOTIFY spectrumAveragingCntChanged)
    Q_PROPERTY(PanAveragingMode panAveragingMode READ panAveragingMode WRITE setPanAveragingMode NOTIFY panAveragingModeChanged)
    Q_PROPERTY(PanGraphicsMode panMode READ panMode WRITE setPanMode NOTIFY panModeChanged)
    Q_PROPERTY(PanDetectorMode panDetectorMode READ panDetectorMode WRITE setPanDetectorMode NOTIFY panDetectorModeChanged)
    Q_PROPERTY(WaterfallColorMode waterfallMode READ waterfallMode WRITE setWaterfallMode NOTIFY waterfallModeChanged)
    Q_PROPERTY(int waterfallOffsetLo READ waterfallOffsetLo WRITE setWaterfallOffsetLo NOTIFY waterfallOffsetChanged)
    Q_PROPERTY(int waterfallOffsetHi READ waterfallOffsetHi WRITE setWaterfallOffsetHi NOTIFY waterfallOffsetChanged)
    Q_PROPERTY(bool panGrid READ panGrid WRITE setPanGrid NOTIFY panGridChanged)
    Q_PROPERTY(bool peakHold READ peakHold WRITE setPeakHold NOTIFY peakHoldChanged)
    Q_PROPERTY(double dBmPanScaleMin READ dBmPanScaleMin WRITE setDBmPanScaleMin NOTIFY panScaleChanged)
    Q_PROPERTY(double dBmPanScaleMax READ dBmPanScaleMax WRITE setDBmPanScaleMax NOTIFY panScaleChanged)
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)

public:
    explicit SliceModel(int id, QObject *parent = nullptr);

    int id() const { return m_id; }

    qint64 frequency() const { return m_frequency; }
    void setFrequency(qint64 freq);

    qint64 centerFrequency() const { return m_centerFrequency; }
    void setCenterFrequency(qint64 freq);

    DSPMode dspMode() const { return m_dspMode; }
    void setDspMode(DSPMode mode);

    float filterLow() const { return m_filterLow; }
    void setFilterLow(float low);

    float filterHigh() const { return m_filterHigh; }
    void setFilterHigh(float high);

    int filterPreset() const { return m_filterPreset; }
    void setFilterPreset(int preset);

    int filterSlope() const { return m_filterSlope; }
    void setFilterSlope(int slope);

    float volume() const { return m_volume; }
    void setVolume(float vol);

    bool mute() const { return m_mute; }
    void setMute(bool muted);

    float pan() const { return m_pan; }
    void setPan(float pan);

    AGCMode agcMode() const { return m_agcMode; }
    void setAgcMode(AGCMode mode);

    int agcGain() const { return m_agcGain; }
    void setAgcGain(int gain);

    int agcMaxGain() const { return m_agcMaxGain; }
    void setAgcMaxGain(int gain);

    int agcFixedGain() const { return m_agcFixedGain; }
    void setAgcFixedGain(int gain);

    int agcHangThreshold() const { return m_agcHangThreshold; }
    void setAgcHangThreshold(int threshold);

    int agcSlope() const { return m_agcSlope; }
    void setAgcSlope(int slope);

    int nbMode() const { return m_nbMode; }
    void setNbMode(int mode);

    int nrMode() const { return m_nrMode; }
    void setNrMode(int mode);

    int nr2GainMethod() const { return m_nr2GainMethod; }
    void setNr2GainMethod(int method);

    int nr2NpeMethod() const { return m_nr2NpeMethod; }
    void setNr2NpeMethod(int method);

    bool nr2Ae() const { return m_nr2Ae; }
    void setNr2Ae(bool enabled);

    int nrAgc() const { return m_nrAgc; }
    void setNrAgc(int mode);

    bool anf() const { return m_anf; }
    void setAnf(bool enabled);

    bool snb() const { return m_snb; }
    void setSnb(bool enabled);

    double sMeterValue() const { return m_sMeterValue; }
    void setSMeterValue(double value);

    int sMeterHoldTime() const { return m_sMeterHoldTime; }
    void setSMeterHoldTime(int time);

    int fftSize() const { return m_fftSize; }
    void setFftSize(int size);

    bool spectrumAveraging() const { return m_spectrumAveraging; }
    void setSpectrumAveraging(bool enabled);

    int spectrumAveragingCnt() const { return m_spectrumAveragingCnt; }
    void setSpectrumAveragingCnt(int count);

    PanAveragingMode panAveragingMode() const { return m_panAveragingMode; }
    void setPanAveragingMode(PanAveragingMode mode);

    PanGraphicsMode panMode() const { return m_panMode; }
    void setPanMode(PanGraphicsMode mode);

    PanDetectorMode panDetectorMode() const { return m_panDetectorMode; }
    void setPanDetectorMode(PanDetectorMode mode);

    WaterfallColorMode waterfallMode() const { return m_waterfallMode; }
    void setWaterfallMode(WaterfallColorMode mode);

    int waterfallOffsetLo() const { return m_waterfallOffsetLo; }
    void setWaterfallOffsetLo(int offset);

    int waterfallOffsetHi() const { return m_waterfallOffsetHi; }
    void setWaterfallOffsetHi(int offset);

    bool panGrid() const { return m_panGrid; }
    void setPanGrid(bool enabled);

    bool peakHold() const { return m_peakHold; }
    void setPeakHold(bool enabled);

    double dBmPanScaleMin() const { return m_dBmPanScaleMin; }
    void setDBmPanScaleMin(double val);

    double dBmPanScaleMax() const { return m_dBmPanScaleMax; }
    void setDBmPanScaleMax(double val);

    bool active() const { return m_active; }
    void setActive(bool active);

signals:
    void frequencyChanged(qint64 freq);
    void centerFrequencyChanged(qint64 freq);
    void dspModeChanged(DSPMode mode);
    void filterChanged();
    void filterPresetChanged(int preset);
    void filterSlopeChanged(int slope);
    void volumeChanged(float vol);
    void muteChanged(bool muted);
    void panChanged(float pan);
    void agcModeChanged(AGCMode mode);
    void agcGainChanged(int gain);
    void agcMaxGainChanged(int gain);
    void agcFixedGainChanged(int gain);
    void agcHangThresholdChanged(int threshold);
    void agcSlopeChanged(int slope);
    void nbModeChanged(int mode);
    void nrModeChanged(int mode);
    void nr2GainMethodChanged(int method);
    void nr2NpeMethodChanged(int method);
    void nr2AeChanged(bool enabled);
    void nrAgcChanged(int mode);
    void anfChanged(bool enabled);
    void snbChanged(bool enabled);
    void sMeterValueChanged(double value);
    void sMeterHoldTimeChanged(int time);
    void fftSizeChanged(int size);
    void spectrumAveragingChanged(bool enabled);
    void spectrumAveragingCntChanged(int count);
    void panAveragingModeChanged(PanAveragingMode mode);
    void panModeChanged(PanGraphicsMode mode);
    void panDetectorModeChanged(PanDetectorMode mode);
    void waterfallModeChanged(WaterfallColorMode mode);
    void waterfallOffsetChanged();
    void panGridChanged(bool enabled);
    void peakHoldChanged(bool enabled);
    void panScaleChanged();
    void activeChanged(bool active);

private:
    int m_id;
    qint64 m_frequency = 7000000;
    qint64 m_centerFrequency = 7000000;
    DSPMode m_dspMode = LSB;
    float m_filterLow = -3050.0f;
    float m_filterHigh = -150.0f;
    int m_filterPreset = 3; // Default to 2.4k
    int m_filterSlope = 1; // Default to Normal
    float m_volume = 0.5f;
    bool m_mute = false;
    float m_pan = 0.0f;
    AGCMode m_agcMode = agcMED;
    int m_agcGain = 30;
    int m_agcMaxGain = 100;
    int m_agcFixedGain = 0;
    int m_agcHangThreshold = -100;
    int m_agcSlope = 0;
    int m_nbMode = 0;
    int m_nrMode = 0;
    int m_nr2GainMethod = 0;
    int m_nr2NpeMethod = 0;
    bool m_nr2Ae = false;
    int m_nrAgc = 0;
    bool m_anf = false;
    bool m_snb = false;
    double m_sMeterValue = -140.0;
    int m_sMeterHoldTime = 1000;
    int m_fftSize = 1; // combo index: 0=2k … 4=32k (see QWDSPEngine::getfftVal)
    bool m_spectrumAveraging = false;
    int m_spectrumAveragingCnt = 5;
    PanAveragingMode m_panAveragingMode = AV_MODE_NONE;
    PanGraphicsMode m_panMode = FilledLine;
    PanDetectorMode m_panDetectorMode = DET_MODE_PEAK;
    WaterfallColorMode m_waterfallMode = Simple;
    int m_waterfallOffsetLo = -120;
    int m_waterfallOffsetHi = -60;
    bool m_panGrid = true;
    bool m_peakHold = false;
    double m_dBmPanScaleMin = -140.0;
    double m_dBmPanScaleMax = -20.0;
    bool m_active = false;
};

#endif // SLICEMODEL_H
