#include "ReceiverConfig.h"
#include "cusdr_settings.h"
#include "Util/settings_utils.h"
#include <QSettings>

namespace {

template<typename T>
QList<T> paddedList(const QList<T> &src, int count, T fill)
{
    QList<T> out = src;
    while (out.size() < count)
        out.append(fill);
    while (out.size() > count)
        out.removeLast();
    return out;
}

bool validMouseWheelStep(int value)
{
    switch (value) {
    case 1: case 5: case 10: case 50: case 100: case 500:
    case 1000: case 5000: case 10000: case 50000: case 100000: case 500000:
        return true;
    default:
        return false;
    }
}

} // namespace

static_assert(ReceiverConfig::kBandCount == MAX_BANDS, "ReceiverConfig band count must match MAX_BANDS");

ReceiverConfig::ReceiverConfig(int id, QObject *parent)
    : QObject(parent)
    , m_id(id)
    , m_dspCore(QSDR::QtDSP)
    , m_hamBand(gen)
    , m_dspMode(USB)
    , m_adcMode(adc1)
    , m_agcMode(agcMED)
    , m_ctrFrequency(7050000)
    , m_vfoFrequency(7050000)
    , m_vfoAFrequency(7050000)
    , m_vfoBFrequency(7050000)
    , m_activeVfo(0)
    , m_filterSlope(1)
{
    ensureBandLists();
}

void ReceiverConfig::ensureBandLists()
{
    m_lastCenterFrequencyList = paddedList(m_lastCenterFrequencyList, kBandCount, static_cast<qint64>(7050000));
    m_lastVfoFrequencyList = paddedList(m_lastVfoFrequencyList, kBandCount, static_cast<qint64>(7050000));
    m_mercuryAttenuators = paddedList(m_mercuryAttenuators, kBandCount, 0);
    m_dBmPanScaleMinList = paddedList(m_dBmPanScaleMinList, kBandCount, -120.0);
    m_dBmPanScaleMaxList = paddedList(m_dBmPanScaleMaxList, kBandCount, -10.0);
    m_dspModeList = paddedList(m_dspModeList, kBandCount, LSB);
}

void ReceiverConfig::setDspCore(QSDR::_DSPCore core) {
    if (m_dspCore != core) {
        m_dspCore = core;
        emit dspCoreChanged(m_dspCore);
    }
}

void ReceiverConfig::setHamBand(HamBand band) {
    if (m_hamBand != band) {
        m_hamBand = band;
        emit hamBandChanged(m_hamBand);
    }
}

void ReceiverConfig::setDspMode(DSPMode mode) {
    if (m_dspMode != mode) {
        m_dspMode = mode;
        emit dspModeChanged(m_dspMode);
    }
}

void ReceiverConfig::setAdcMode(ADCMode mode) {
    if (m_adcMode != mode) {
        m_adcMode = mode;
        emit adcModeChanged(m_adcMode);
    }
}

void ReceiverConfig::setAgcMode(AGCMode mode) {
    if (m_agcMode != mode) {
        m_agcMode = mode;
        emit agcModeChanged(m_agcMode);
    }
}

void ReceiverConfig::setCtrFrequency(qint64 freq) {
    if (m_ctrFrequency != freq) {
        m_ctrFrequency = freq;
        emit ctrFrequencyChanged(m_ctrFrequency);
    }
}

void ReceiverConfig::setVfoFrequency(qint64 freq) {
    if (m_vfoFrequency != freq) {
        m_vfoFrequency = freq;
        emit vfoFrequencyChanged(m_vfoFrequency);
    }
}

void ReceiverConfig::setVfoAFrequency(qint64 freq) {
    if (m_vfoAFrequency != freq) {
        m_vfoAFrequency = freq;
        emit vfoAFrequencyChanged(m_vfoAFrequency);
    }
}

void ReceiverConfig::setVfoBFrequency(qint64 freq) {
    if (m_vfoBFrequency != freq) {
        m_vfoBFrequency = freq;
        emit vfoBFrequencyChanged(m_vfoBFrequency);
    }
}

void ReceiverConfig::setActiveVfo(int vfo) {
    const int clamped = (vfo == 1) ? 1 : 0;
    if (m_activeVfo != clamped) {
        m_activeVfo = clamped;
        emit activeVfoChanged(m_activeVfo);
    }
}

void ReceiverConfig::setFilterSlope(int slope) {
    if (slope < 0)
        slope = 0;
    if (m_filterSlope != slope) {
        m_filterSlope = slope;
        emit filterSlopeChanged(m_filterSlope);
    }
}

void ReceiverConfig::setPanMode(PanGraphicsMode mode)
{
    if (mode < Line || mode > Solid)
        mode = Line;
    m_panMode = mode;
}

void ReceiverConfig::setWaterfallMode(WaterfallColorMode mode)
{
    if (mode < Simple || mode > Enhanced)
        mode = Enhanced;
    m_waterfallMode = mode;
}

void ReceiverConfig::setPanAvMode(PanAveragingMode mode)
{
    if (mode < AV_MODE_NONE || mode > AV_MODE_LOG_RECURSIVE)
        mode = AV_MODE_RECURSIVE;
    m_panAvMode = mode;
}

void ReceiverConfig::setPanDetMode(PanDetectorMode mode)
{
    if (mode < DET_MODE_PEAK || mode > DET_MODE_SAMPLE)
        mode = DET_MODE_ROSENFELL;
    m_panDetMode = mode;
}

void ReceiverConfig::setFftSize(int size)
{
    m_fftSize = SettingsUtils::clampIntRange(size, 0, 7, 1);
}

void ReceiverConfig::setFramesPerSecond(int fps)
{
    m_framesPerSecond = SettingsUtils::clampIntRange(fps, 0, 200, 25);
}

void ReceiverConfig::setAveragingCnt(int count)
{
    if (count < 1)
        count = 5;
    m_averagingCnt = count;
}

void ReceiverConfig::setWaterfallOffsetLo(int offset)
{
    m_waterfallOffsetLo = SettingsUtils::clampIntRange(offset, -50, 50, -5);
}

void ReceiverConfig::setWaterfallOffsetHi(int offset)
{
    m_waterfallOffsetHi = SettingsUtils::clampIntRange(offset, -50, 50, 20);
}

void ReceiverConfig::setFilterIndex(int index)
{
    m_filterIndex = index;
}

void ReceiverConfig::setFreqRulerPosition(float pos)
{
    m_freqRulerPosition = qBound(0.0f, pos, 1.0f);
}

void ReceiverConfig::setAudioVolume(float volume)
{
    m_audioVolume = qBound(0.0f, volume, 1.0f);
}

void ReceiverConfig::setMouseWheelFreqStep(qreal step)
{
    const int value = static_cast<int>(step);
    m_mouseWheelFreqStep = validMouseWheelStep(value) ? static_cast<qreal>(value) : 100.0;
}

void ReceiverConfig::setFilterLo(qreal freq)
{
    if (freq < -20000.0 || freq > 20000.0)
        freq = -3050.0;
    m_filterLo = freq;
}

void ReceiverConfig::setFilterHi(qreal freq)
{
    if (freq < -20000.0 || freq > 20000.0)
        freq = -150.0;
    m_filterHi = freq;
}

void ReceiverConfig::setAgcGain(qreal gain)
{
    m_agcGain = qBound(-20.0, gain, 120.0);
}

void ReceiverConfig::setAgcFixedGain(qreal gain)
{
    m_agcFixedGain = qBound(-20.0, gain, 50.0);
}

void ReceiverConfig::setAgcMaximumGain(int gain)
{
    m_agcMaximumGain = SettingsUtils::clampIntRange(gain, -20, 150, 30);
}

void ReceiverConfig::setAgcSlope(int slope)
{
    m_agcSlope = SettingsUtils::clampIntRange(slope, 0, 20, 0);
}

void ReceiverConfig::setAgcAttackTime(qreal seconds)
{
    m_agcAttackTime = qMax(0.0, seconds);
}

void ReceiverConfig::setAgcDecayTime(qreal seconds)
{
    m_agcDecayTime = qMax(0.0, seconds);
}

void ReceiverConfig::setAgcHangTime(qreal seconds)
{
    m_agcHangTime = qMax(0.0, seconds);
}

void ReceiverConfig::setNr(int mode) { m_nr = mode; }
void ReceiverConfig::setNrAgc(int mode) { m_nrAgc = mode; }
void ReceiverConfig::setNbMode(int mode) { m_nbMode = mode; }
void ReceiverConfig::setNr2GainMethod(int method) { m_nr2GainMethod = method; }
void ReceiverConfig::setNr2NpeMethod(int method) { m_nr2NpeMethod = method; }
void ReceiverConfig::setNr2Ae(bool enabled) { m_nr2Ae = enabled; }
void ReceiverConfig::setAnf(bool enabled) { m_anf = enabled; }
void ReceiverConfig::setSnb(bool enabled) { m_snb = enabled; }
void ReceiverConfig::setAgcLines(bool enabled) { m_agcLines = enabled; }
void ReceiverConfig::setPanLocked(bool locked) { m_panLocked = locked; }
void ReceiverConfig::setSpectrumAveraging(bool enabled) { m_spectrumAveraging = enabled; }
void ReceiverConfig::setHairCross(bool enabled) { m_hairCross = enabled; }
void ReceiverConfig::setPanGrid(bool enabled) { m_panGrid = enabled; }
void ReceiverConfig::setPeakHold(bool enabled) { m_peakHold = enabled; }
void ReceiverConfig::setClickVFO(bool enabled) { m_clickVFO = enabled; }
void ReceiverConfig::setCwDecode(bool enabled) { m_cwDecode = enabled; }

void ReceiverConfig::setLastCenterFrequencyList(const QList<qint64> &values)
{
    m_lastCenterFrequencyList = paddedList(values, kBandCount, static_cast<qint64>(7050000));
}

void ReceiverConfig::setLastVfoFrequencyList(const QList<qint64> &values)
{
    m_lastVfoFrequencyList = paddedList(values, kBandCount, static_cast<qint64>(7050000));
}

void ReceiverConfig::setMercuryAttenuators(const QList<int> &values)
{
    QList<int> next = paddedList(values, kBandCount, 0);
    for (int &v : next)
        v = qBound(0, v, 3);
    m_mercuryAttenuators = next;
}

void ReceiverConfig::setdBmPanScaleMinList(const QList<qreal> &values)
{
    QList<qreal> next = paddedList(values, kBandCount, -120.0);
    for (qreal &v : next) {
        if (v < MINDBM || v > MAXDBM)
            v = -120.0;
    }
    m_dBmPanScaleMinList = next;
}

void ReceiverConfig::setdBmPanScaleMaxList(const QList<qreal> &values)
{
    QList<qreal> next = paddedList(values, kBandCount, -10.0);
    for (qreal &v : next) {
        if (v < MINDBM || v > MAXDBM)
            v = -10.0;
    }
    m_dBmPanScaleMaxList = next;
}

void ReceiverConfig::setDspModeList(const QList<DSPMode> &values)
{
    m_dspModeList = paddedList(values, kBandCount, LSB);
}

void ReceiverConfig::applyTo(TReceiver &rx) const {
    rx.dspCore = m_dspCore;
    rx.hamBand = m_hamBand;
    rx.dspMode = m_dspMode;
    rx.adcMode = m_adcMode;
    rx.agcMode = m_agcMode;
    rx.hangEnabled = SettingsUtils::agcHangEnabledForMode(m_agcMode);
    rx.ctrFrequency = m_ctrFrequency;
    rx.vfoFrequency = m_vfoFrequency;
    rx.vfoAFrequency = m_vfoAFrequency;
    rx.vfoBFrequency = m_vfoBFrequency;
    rx.activeVfo = m_activeVfo;
    rx.filterSlope = m_filterSlope;
    rx.panMode = m_panMode;
    rx.waterfallMode = m_waterfallMode;
    rx.panAvMode = m_panAvMode;
    rx.panDetMode = m_panDetMode;
    rx.fftsize = m_fftSize;
    rx.framesPerSecond = m_framesPerSecond;
    rx.averagingCnt = m_averagingCnt;
    rx.waterfallOffsetLo = m_waterfallOffsetLo;
    rx.waterfallOffsetHi = m_waterfallOffsetHi;
    rx.m_filterIndex = m_filterIndex;
    rx.freqRulerPosition = m_freqRulerPosition;
    rx.audioVolume = m_audioVolume;
    rx.mouseWheelFreqStep = m_mouseWheelFreqStep;
    rx.filterLo = m_filterLo;
    rx.filterHi = m_filterHi;
    rx.acgGain = m_agcGain;
    rx.agcFixedGain_dB = m_agcFixedGain;
    rx.agcMaximumGain_dB = m_agcMaximumGain;
    rx.agcSlope = m_agcSlope;
    rx.agcAttackTime = m_agcAttackTime;
    rx.agcDecayTime = m_agcDecayTime;
    rx.agcHangTime = m_agcHangTime;
    rx.nr = m_nr;
    rx.nr_agc = m_nrAgc;
    rx.nbMode = m_nbMode;
    rx.nr2_gain_method = m_nr2GainMethod;
    rx.nr2_npe_method = m_nr2NpeMethod;
    rx.nr2_ae = m_nr2Ae;
    rx.anf = m_anf;
    rx.snb = m_snb;
    rx.agcLines = m_agcLines;
    rx.panLocked = m_panLocked;
    rx.spectrumAveraging = m_spectrumAveraging;
    rx.hairCross = m_hairCross;
    rx.panGrid = m_panGrid;
    rx.peakHold = m_peakHold;
    rx.clickVFO = m_clickVFO;
    rx.cwDecode = m_cwDecode;
    rx.lastCenterFrequencyList = m_lastCenterFrequencyList;
    rx.lastVfoFrequencyList = m_lastVfoFrequencyList;
    rx.mercuryAttenuators = m_mercuryAttenuators;
    rx.dBmPanScaleMinList = m_dBmPanScaleMinList;
    rx.dBmPanScaleMaxList = m_dBmPanScaleMaxList;
    rx.dspModeList = m_dspModeList;
    if (m_hamBand >= 0 && m_hamBand < rx.dspModeList.size())
        rx.dspModeList[m_hamBand] = m_dspMode;
}

void ReceiverConfig::fromReceiver(const TReceiver &rx) {
    setDspCore(rx.dspCore);
    setHamBand(rx.hamBand);
    setDspMode(rx.dspMode);
    setAdcMode(rx.adcMode);
    setAgcMode(rx.agcMode);
    setCtrFrequency(rx.ctrFrequency);
    setVfoFrequency(rx.vfoFrequency);
    setVfoAFrequency(rx.vfoAFrequency);
    setVfoBFrequency(rx.vfoBFrequency);
    setActiveVfo(rx.activeVfo);
    setFilterSlope(rx.filterSlope);
    setPanMode(rx.panMode);
    setWaterfallMode(rx.waterfallMode);
    setPanAvMode(rx.panAvMode);
    setPanDetMode(rx.panDetMode);
    setFftSize(rx.fftsize);
    setFramesPerSecond(rx.framesPerSecond);
    setAveragingCnt(rx.averagingCnt);
    setWaterfallOffsetLo(rx.waterfallOffsetLo);
    setWaterfallOffsetHi(rx.waterfallOffsetHi);
    setFilterIndex(rx.m_filterIndex);
    setFreqRulerPosition(rx.freqRulerPosition);
    setAudioVolume(rx.audioVolume);
    setMouseWheelFreqStep(rx.mouseWheelFreqStep);
    setFilterLo(rx.filterLo);
    setFilterHi(rx.filterHi);
    setAgcGain(rx.acgGain);
    setAgcFixedGain(rx.agcFixedGain_dB);
    setAgcMaximumGain(rx.agcMaximumGain_dB);
    setAgcSlope(rx.agcSlope);
    setAgcAttackTime(rx.agcAttackTime);
    setAgcDecayTime(rx.agcDecayTime);
    setAgcHangTime(rx.agcHangTime);
    setNr(rx.nr);
    setNrAgc(rx.nr_agc);
    setNbMode(rx.nbMode);
    setNr2GainMethod(rx.nr2_gain_method);
    setNr2NpeMethod(rx.nr2_npe_method);
    setNr2Ae(rx.nr2_ae);
    setAnf(rx.anf);
    setSnb(rx.snb);
    setAgcLines(rx.agcLines);
    setPanLocked(rx.panLocked);
    setSpectrumAveraging(rx.spectrumAveraging);
    setHairCross(rx.hairCross);
    setPanGrid(rx.panGrid);
    setPeakHold(rx.peakHold);
    setClickVFO(rx.clickVFO);
    setCwDecode(rx.cwDecode);
    setLastCenterFrequencyList(rx.lastCenterFrequencyList);
    setLastVfoFrequencyList(rx.lastVfoFrequencyList);
    setMercuryAttenuators(rx.mercuryAttenuators);
    setdBmPanScaleMinList(rx.dBmPanScaleMinList);
    setdBmPanScaleMaxList(rx.dBmPanScaleMaxList);
    setDspModeList(rx.dspModeList);
}

void ReceiverConfig::load(const QJsonObject &json) {
    if (json.contains(QLatin1String("dspCore")))
        setDspCore(static_cast<QSDR::_DSPCore>(json.value(QLatin1String("dspCore")).toInt()));
    if (json.contains(QLatin1String("hamBand")))
        setHamBand(static_cast<HamBand>(json.value(QLatin1String("hamBand")).toInt()));
    if (json.contains(QLatin1String("dspMode")))
        setDspMode(static_cast<DSPMode>(json.value(QLatin1String("dspMode")).toInt()));
    if (json.contains(QLatin1String("adcMode")))
        setAdcMode(static_cast<ADCMode>(json.value(QLatin1String("adcMode")).toInt()));
    if (json.contains(QLatin1String("agcMode")))
        setAgcMode(static_cast<AGCMode>(json.value(QLatin1String("agcMode")).toInt()));
    if (json.contains(QLatin1String("ctrFrequency")))
        setCtrFrequency(static_cast<qint64>(json.value(QLatin1String("ctrFrequency")).toDouble()));
    if (json.contains(QLatin1String("vfoFrequency")))
        setVfoFrequency(static_cast<qint64>(json.value(QLatin1String("vfoFrequency")).toDouble()));
    if (json.contains(QLatin1String("vfoAFrequency")))
        setVfoAFrequency(static_cast<qint64>(json.value(QLatin1String("vfoAFrequency")).toDouble()));
    else
        setVfoAFrequency(vfoFrequency());
    if (json.contains(QLatin1String("vfoBFrequency")))
        setVfoBFrequency(static_cast<qint64>(json.value(QLatin1String("vfoBFrequency")).toDouble()));
    else
        setVfoBFrequency(vfoFrequency());
    if (json.contains(QLatin1String("activeVfo")))
        setActiveVfo(json.value(QLatin1String("activeVfo")).toInt());
    else
        setActiveVfo(0);
    if (json.contains(QLatin1String("filterSlope")))
        setFilterSlope(json.value(QLatin1String("filterSlope")).toInt(1));

    if (json.contains(QLatin1String("panMode")))
        setPanMode(static_cast<PanGraphicsMode>(json.value(QLatin1String("panMode")).toInt()));
    if (json.contains(QLatin1String("waterfallMode")))
        setWaterfallMode(static_cast<WaterfallColorMode>(json.value(QLatin1String("waterfallMode")).toInt()));
    if (json.contains(QLatin1String("panAvMode")))
        setPanAvMode(static_cast<PanAveragingMode>(json.value(QLatin1String("panAvMode")).toInt()));
    if (json.contains(QLatin1String("panDetMode")))
        setPanDetMode(static_cast<PanDetectorMode>(json.value(QLatin1String("panDetMode")).toInt()));
    if (json.contains(QLatin1String("fftSize")))
        setFftSize(json.value(QLatin1String("fftSize")).toInt());
    if (json.contains(QLatin1String("framesPerSecond")))
        setFramesPerSecond(json.value(QLatin1String("framesPerSecond")).toInt());
    if (json.contains(QLatin1String("averagingCnt")))
        setAveragingCnt(json.value(QLatin1String("averagingCnt")).toInt());
    if (json.contains(QLatin1String("waterfallOffsetLo")))
        setWaterfallOffsetLo(json.value(QLatin1String("waterfallOffsetLo")).toInt());
    if (json.contains(QLatin1String("waterfallOffsetHi")))
        setWaterfallOffsetHi(json.value(QLatin1String("waterfallOffsetHi")).toInt());
    if (json.contains(QLatin1String("filterIndex")))
        setFilterIndex(json.value(QLatin1String("filterIndex")).toInt());
    if (json.contains(QLatin1String("freqRulerPosition")))
        setFreqRulerPosition(static_cast<float>(json.value(QLatin1String("freqRulerPosition")).toDouble()));
    if (json.contains(QLatin1String("audioVolume")))
        setAudioVolume(static_cast<float>(json.value(QLatin1String("audioVolume")).toDouble()));
    if (json.contains(QLatin1String("mouseWheelFreqStep")))
        setMouseWheelFreqStep(json.value(QLatin1String("mouseWheelFreqStep")).toDouble());
    if (json.contains(QLatin1String("filterLo")))
        setFilterLo(json.value(QLatin1String("filterLo")).toDouble());
    if (json.contains(QLatin1String("filterHi")))
        setFilterHi(json.value(QLatin1String("filterHi")).toDouble());
    if (json.contains(QLatin1String("agcGain")))
        setAgcGain(json.value(QLatin1String("agcGain")).toDouble());
    if (json.contains(QLatin1String("agcFixedGain")))
        setAgcFixedGain(json.value(QLatin1String("agcFixedGain")).toDouble());
    if (json.contains(QLatin1String("agcMaximumGain")))
        setAgcMaximumGain(json.value(QLatin1String("agcMaximumGain")).toInt());
    if (json.contains(QLatin1String("agcSlope")))
        setAgcSlope(json.value(QLatin1String("agcSlope")).toInt());
    if (json.contains(QLatin1String("agcAttackTime")))
        setAgcAttackTime(json.value(QLatin1String("agcAttackTime")).toDouble());
    if (json.contains(QLatin1String("agcDecayTime")))
        setAgcDecayTime(json.value(QLatin1String("agcDecayTime")).toDouble());
    if (json.contains(QLatin1String("agcHangTime")))
        setAgcHangTime(json.value(QLatin1String("agcHangTime")).toDouble());
    if (json.contains(QLatin1String("nr")))
        setNr(json.value(QLatin1String("nr")).toInt());
    if (json.contains(QLatin1String("nrAgc")))
        setNrAgc(json.value(QLatin1String("nrAgc")).toInt());
    if (json.contains(QLatin1String("nbMode")))
        setNbMode(json.value(QLatin1String("nbMode")).toInt());
    if (json.contains(QLatin1String("nr2GainMethod")))
        setNr2GainMethod(json.value(QLatin1String("nr2GainMethod")).toInt());
    if (json.contains(QLatin1String("nr2NpeMethod")))
        setNr2NpeMethod(json.value(QLatin1String("nr2NpeMethod")).toInt());
    if (json.contains(QLatin1String("nr2Ae")))
        setNr2Ae(json.value(QLatin1String("nr2Ae")).toBool());
    if (json.contains(QLatin1String("anf")))
        setAnf(json.value(QLatin1String("anf")).toBool());
    if (json.contains(QLatin1String("snb")))
        setSnb(json.value(QLatin1String("snb")).toBool());
    if (json.contains(QLatin1String("agcLines")))
        setAgcLines(json.value(QLatin1String("agcLines")).toBool());
    if (json.contains(QLatin1String("panLocked")))
        setPanLocked(json.value(QLatin1String("panLocked")).toBool());
    if (json.contains(QLatin1String("spectrumAveraging")))
        setSpectrumAveraging(json.value(QLatin1String("spectrumAveraging")).toBool());
    if (json.contains(QLatin1String("hairCross")))
        setHairCross(json.value(QLatin1String("hairCross")).toBool());
    if (json.contains(QLatin1String("panGrid")))
        setPanGrid(json.value(QLatin1String("panGrid")).toBool());
    if (json.contains(QLatin1String("peakHold")))
        setPeakHold(json.value(QLatin1String("peakHold")).toBool());
    if (json.contains(QLatin1String("clickVFO")))
        setClickVFO(json.value(QLatin1String("clickVFO")).toBool());
    if (json.contains(QLatin1String("cwDecode")))
        setCwDecode(json.value(QLatin1String("cwDecode")).toBool());

    if (json.contains(QLatin1String("lastCenterFrequencyList"))) {
        const auto vec = SettingsUtils::jsonArrayToVector<qint64>(
            json, QStringLiteral("lastCenterFrequencyList"), kBandCount, static_cast<qint64>(7050000));
        setLastCenterFrequencyList(QList<qint64>(vec.begin(), vec.end()));
    }
    if (json.contains(QLatin1String("lastVfoFrequencyList"))) {
        const auto vec = SettingsUtils::jsonArrayToVector<qint64>(
            json, QStringLiteral("lastVfoFrequencyList"), kBandCount, static_cast<qint64>(7050000));
        setLastVfoFrequencyList(QList<qint64>(vec.begin(), vec.end()));
    }
    if (json.contains(QLatin1String("mercuryAttenuators"))) {
        const auto vec = SettingsUtils::jsonArrayToVector<int>(
            json, QStringLiteral("mercuryAttenuators"), kBandCount, 0);
        setMercuryAttenuators(QList<int>(vec.begin(), vec.end()));
    }
    if (json.contains(QLatin1String("dBmPanScaleMinList"))) {
        const auto vec = SettingsUtils::jsonArrayToVector<double>(
            json, QStringLiteral("dBmPanScaleMinList"), kBandCount, -120.0);
        setdBmPanScaleMinList(QList<qreal>(vec.begin(), vec.end()));
    }
    if (json.contains(QLatin1String("dBmPanScaleMaxList"))) {
        const auto vec = SettingsUtils::jsonArrayToVector<double>(
            json, QStringLiteral("dBmPanScaleMaxList"), kBandCount, -10.0);
        setdBmPanScaleMaxList(QList<qreal>(vec.begin(), vec.end()));
    }
    if (json.contains(QLatin1String("dspModeList"))) {
        const auto vec = SettingsUtils::jsonArrayToVector<int>(
            json, QStringLiteral("dspModeList"), kBandCount, static_cast<int>(LSB));
        QList<DSPMode> modes;
        modes.reserve(kBandCount);
        for (int v : vec)
            modes.append(static_cast<DSPMode>(v));
        setDspModeList(modes);
    }
}

void ReceiverConfig::save(QJsonObject &json) const {
    json[QLatin1String("dspCore")] = static_cast<int>(m_dspCore);
    json[QLatin1String("hamBand")] = static_cast<int>(m_hamBand);
    json[QLatin1String("dspMode")] = static_cast<int>(m_dspMode);
    json[QLatin1String("adcMode")] = static_cast<int>(m_adcMode);
    json[QLatin1String("agcMode")] = static_cast<int>(m_agcMode);
    json[QLatin1String("ctrFrequency")] = static_cast<double>(m_ctrFrequency);
    json[QLatin1String("vfoFrequency")] = static_cast<double>(m_vfoFrequency);
    json[QLatin1String("vfoAFrequency")] = static_cast<double>(m_vfoAFrequency);
    json[QLatin1String("vfoBFrequency")] = static_cast<double>(m_vfoBFrequency);
    json[QLatin1String("activeVfo")] = m_activeVfo;
    json[QLatin1String("filterSlope")] = m_filterSlope;
    json[QLatin1String("panMode")] = static_cast<int>(m_panMode);
    json[QLatin1String("waterfallMode")] = static_cast<int>(m_waterfallMode);
    json[QLatin1String("panAvMode")] = static_cast<int>(m_panAvMode);
    json[QLatin1String("panDetMode")] = static_cast<int>(m_panDetMode);
    json[QLatin1String("fftSize")] = m_fftSize;
    json[QLatin1String("framesPerSecond")] = m_framesPerSecond;
    json[QLatin1String("averagingCnt")] = m_averagingCnt;
    json[QLatin1String("waterfallOffsetLo")] = m_waterfallOffsetLo;
    json[QLatin1String("waterfallOffsetHi")] = m_waterfallOffsetHi;
    json[QLatin1String("filterIndex")] = m_filterIndex;
    json[QLatin1String("freqRulerPosition")] = static_cast<double>(m_freqRulerPosition);
    json[QLatin1String("audioVolume")] = static_cast<double>(m_audioVolume);
    json[QLatin1String("mouseWheelFreqStep")] = m_mouseWheelFreqStep;
    json[QLatin1String("filterLo")] = m_filterLo;
    json[QLatin1String("filterHi")] = m_filterHi;
    json[QLatin1String("agcGain")] = m_agcGain;
    json[QLatin1String("agcFixedGain")] = m_agcFixedGain;
    json[QLatin1String("agcMaximumGain")] = m_agcMaximumGain;
    json[QLatin1String("agcSlope")] = m_agcSlope;
    json[QLatin1String("agcAttackTime")] = m_agcAttackTime;
    json[QLatin1String("agcDecayTime")] = m_agcDecayTime;
    json[QLatin1String("agcHangTime")] = m_agcHangTime;
    json[QLatin1String("nr")] = m_nr;
    json[QLatin1String("nrAgc")] = m_nrAgc;
    json[QLatin1String("nbMode")] = m_nbMode;
    json[QLatin1String("nr2GainMethod")] = m_nr2GainMethod;
    json[QLatin1String("nr2NpeMethod")] = m_nr2NpeMethod;
    json[QLatin1String("nr2Ae")] = m_nr2Ae;
    json[QLatin1String("anf")] = m_anf;
    json[QLatin1String("snb")] = m_snb;
    json[QLatin1String("agcLines")] = m_agcLines;
    json[QLatin1String("panLocked")] = m_panLocked;
    json[QLatin1String("spectrumAveraging")] = m_spectrumAveraging;
    json[QLatin1String("hairCross")] = m_hairCross;
    json[QLatin1String("panGrid")] = m_panGrid;
    json[QLatin1String("peakHold")] = m_peakHold;
    json[QLatin1String("clickVFO")] = m_clickVFO;
    json[QLatin1String("cwDecode")] = m_cwDecode;
    json[QLatin1String("lastCenterFrequencyList")] = SettingsUtils::toJsonArray(m_lastCenterFrequencyList);
    json[QLatin1String("lastVfoFrequencyList")] = SettingsUtils::toJsonArray(m_lastVfoFrequencyList);
    json[QLatin1String("mercuryAttenuators")] = SettingsUtils::toJsonArray(m_mercuryAttenuators);
    json[QLatin1String("dBmPanScaleMinList")] = SettingsUtils::toJsonArray(m_dBmPanScaleMinList);
    json[QLatin1String("dBmPanScaleMaxList")] = SettingsUtils::toJsonArray(m_dBmPanScaleMaxList);
    json[QLatin1String("dspModeList")] = SettingsUtils::toJsonArray(m_dspModeList);
}

void ReceiverConfig::loadIni(QSettings *settings) {
    // Must match Settings::m_rxStringList ("receiver0", …) used by load/saveSettings.
    const QString prefix = QStringLiteral("receiver%1").arg(m_id);
    // Legacy prefix from the brief period when ReceiverConfig wrote "rxN/…"
    const QString legacyPrefix = QStringLiteral("rx%1").arg(m_id);

    auto keyOrLegacy = [&](const char *suffix) -> QString {
        QString cstr = prefix + QLatin1Char('/') + QLatin1String(suffix);
        if (!settings->contains(cstr))
            cstr = legacyPrefix + QLatin1Char('/') + QLatin1String(suffix);
        return cstr;
    };

    const QString dspCoreStr = settings->value(keyOrLegacy("dspCore"), QStringLiteral("qtdsp")).toString();
    if (dspCoreStr == QLatin1String("qtdsp"))
        setDspCore(QSDR::QtDSP);

    setCtrFrequency(static_cast<qint64>(settings->value(keyOrLegacy("centerFrequency"), 7050000.0).toDouble()));
    setVfoFrequency(static_cast<qint64>(settings->value(keyOrLegacy("vfoFrequency"), 7050000.0).toDouble()));

    {
        const QString cstr = keyOrLegacy("vfoAFrequency");
        if (settings->contains(cstr))
            setVfoAFrequency(static_cast<qint64>(settings->value(cstr).toDouble()));
        else
            setVfoAFrequency(vfoFrequency());
    }
    {
        const QString cstr = keyOrLegacy("vfoBFrequency");
        if (settings->contains(cstr))
            setVfoBFrequency(static_cast<qint64>(settings->value(cstr).toDouble()));
        else
            setVfoBFrequency(vfoFrequency());
    }

    setActiveVfo(settings->value(keyOrLegacy("activeVfo"), 0).toInt());
    setFilterSlope(settings->value(keyOrLegacy("filterSlope"), 1).toInt());
}

void ReceiverConfig::saveIni(QSettings *settings) const {
    const QString prefix = QStringLiteral("receiver%1").arg(m_id);

    if (m_dspCore == QSDR::QtDSP)
        settings->setValue(prefix + QStringLiteral("/dspCore"), QStringLiteral("qtdsp"));

    settings->setValue(prefix + QStringLiteral("/centerFrequency"), m_ctrFrequency);
    settings->setValue(prefix + QStringLiteral("/vfoFrequency"), m_vfoFrequency);
    settings->setValue(prefix + QStringLiteral("/vfoAFrequency"), m_vfoAFrequency);
    settings->setValue(prefix + QStringLiteral("/vfoBFrequency"), m_vfoBFrequency);
    settings->setValue(prefix + QStringLiteral("/activeVfo"), m_activeVfo);
    settings->setValue(prefix + QStringLiteral("/filterSlope"), m_filterSlope);
}
