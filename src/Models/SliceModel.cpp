#include "SliceModel.h"

SliceModel::SliceModel(int id, QObject *parent)
    : QObject(parent), m_id(id)
{
}

void SliceModel::setFrequency(long freq) {
    if (m_frequency == freq) return;
    m_frequency = freq;
    emit frequencyChanged(m_frequency);
}

void SliceModel::setCenterFrequency(long freq) {
    if (m_centerFrequency == freq) return;
    m_centerFrequency = freq;
    emit centerFrequencyChanged(m_centerFrequency);
}

void SliceModel::setDspMode(DSPMode mode) {
    if (m_dspMode == mode) return;
    m_dspMode = mode;
    emit dspModeChanged(m_dspMode);
}

void SliceModel::setFilterLow(float low) {
    if (m_filterLow == low) return;
    m_filterLow = low;
    emit filterChanged();
}

void SliceModel::setFilterHigh(float high) {
    if (m_filterHigh == high) return;
    m_filterHigh = high;
    emit filterChanged();
}

void SliceModel::setFilterPreset(int preset) {
    if (m_filterPreset == preset) return;
    m_filterPreset = preset;
    emit filterPresetChanged(m_filterPreset);
}

void SliceModel::setVolume(float vol) {
    if (m_volume == vol) return;
    m_volume = vol;
    emit volumeChanged(m_volume);
}

void SliceModel::setMute(bool muted) {
    if (m_mute == muted) return;
    m_mute = muted;
    emit muteChanged(m_mute);
}

void SliceModel::setPan(float pan) {
    if (m_pan == pan) return;
    m_pan = pan;
    emit panChanged(m_pan);
}

void SliceModel::setAgcMode(AGCMode mode) {
    if (m_agcMode == mode) return;
    m_agcMode = mode;
    emit agcModeChanged(m_agcMode);
}

void SliceModel::setAgcGain(int gain) {
    if (m_agcGain == gain) return;
    m_agcGain = gain;
    emit agcGainChanged(m_agcGain);
}

void SliceModel::setAgcMaxGain(int gain) {
    if (m_agcMaxGain == gain) return;
    m_agcMaxGain = gain;
    emit agcMaxGainChanged(m_agcMaxGain);
}

void SliceModel::setAgcFixedGain(int gain) {
    if (m_agcFixedGain == gain) return;
    m_agcFixedGain = gain;
    emit agcFixedGainChanged(m_agcFixedGain);
}

void SliceModel::setAgcHangThreshold(int threshold) {
    if (m_agcHangThreshold == threshold) return;
    m_agcHangThreshold = threshold;
    emit agcHangThresholdChanged(m_agcHangThreshold);
}

void SliceModel::setAgcSlope(int slope) {
    if (m_agcSlope == slope) return;
    m_agcSlope = slope;
    emit agcSlopeChanged(m_agcSlope);
}

void SliceModel::setNbMode(int mode) {
    if (m_nbMode == mode) return;
    m_nbMode = mode;
    emit nbModeChanged(m_nbMode);
}

void SliceModel::setNrMode(int mode) {
    if (m_nrMode == mode) return;
    m_nrMode = mode;
    emit nrModeChanged(m_nrMode);
}

void SliceModel::setNr2GainMethod(int method) {
    if (m_nr2GainMethod == method) return;
    m_nr2GainMethod = method;
    emit nr2GainMethodChanged(m_nr2GainMethod);
}

void SliceModel::setNr2NpeMethod(int method) {
    if (m_nr2NpeMethod == method) return;
    m_nr2NpeMethod = method;
    emit nr2NpeMethodChanged(m_nr2NpeMethod);
}

void SliceModel::setNr2Ae(bool enabled) {
    if (m_nr2Ae == enabled) return;
    m_nr2Ae = enabled;
    emit nr2AeChanged(m_nr2Ae);
}

void SliceModel::setNrAgc(int mode) {
    if (m_nrAgc == mode) return;
    m_nrAgc = mode;
    emit nrAgcChanged(m_nrAgc);
}

void SliceModel::setAnf(bool enabled) {
    if (m_anf == enabled) return;
    m_anf = enabled;
    emit anfChanged(m_anf);
}

void SliceModel::setSnb(bool enabled) {
    if (m_snb == enabled) return;
    m_snb = enabled;
    emit snbChanged(m_snb);
}

void SliceModel::setSMeterValue(double value) {
    if (m_sMeterValue == value) return;
    m_sMeterValue = value;
    emit sMeterValueChanged(m_sMeterValue);
}

void SliceModel::setSMeterHoldTime(int time) {
    if (m_sMeterHoldTime == time) return;
    m_sMeterHoldTime = time;
    emit sMeterHoldTimeChanged(m_sMeterHoldTime);
}

void SliceModel::setFftSize(int size) {
    if (m_fftSize == size) return;
    m_fftSize = size;
    emit fftSizeChanged(m_fftSize);
}

void SliceModel::setSpectrumAveraging(bool enabled) {
    if (m_spectrumAveraging == enabled) return;
    m_spectrumAveraging = enabled;
    emit spectrumAveragingChanged(m_spectrumAveraging);
}

void SliceModel::setSpectrumAveragingCnt(int count) {
    if (m_spectrumAveragingCnt == count) return;
    m_spectrumAveragingCnt = count;
    emit spectrumAveragingCntChanged(m_spectrumAveragingCnt);
}

void SliceModel::setPanAveragingMode(PanAveragingMode mode) {
    if (m_panAveragingMode == mode) return;
    m_panAveragingMode = mode;
    emit panAveragingModeChanged(m_panAveragingMode);
}

void SliceModel::setPanMode(PanGraphicsMode mode) {
    if (m_panMode == mode) return;
    m_panMode = mode;
    emit panModeChanged(m_panMode);
}

void SliceModel::setPanDetectorMode(PanDetectorMode mode) {
    if (m_panDetectorMode == mode) return;
    m_panDetectorMode = mode;
    emit panDetectorModeChanged(m_panDetectorMode);
}

void SliceModel::setWaterfallMode(WaterfallColorMode mode) {
    if (m_waterfallMode == mode) return;
    m_waterfallMode = mode;
    emit waterfallModeChanged(m_waterfallMode);
}

void SliceModel::setWaterfallOffsetLo(int offset) {
    if (m_waterfallOffsetLo == offset) return;
    m_waterfallOffsetLo = offset;
    emit waterfallOffsetChanged();
}

void SliceModel::setWaterfallOffsetHi(int offset) {
    if (m_waterfallOffsetHi == offset) return;
    m_waterfallOffsetHi = offset;
    emit waterfallOffsetChanged();
}

void SliceModel::setPanGrid(bool enabled) {
    if (m_panGrid == enabled) return;
    m_panGrid = enabled;
    emit panGridChanged(m_panGrid);
}

void SliceModel::setPeakHold(bool enabled) {
    if (m_peakHold == enabled) return;
    m_peakHold = enabled;
    emit peakHoldChanged(m_peakHold);
}

void SliceModel::setDBmPanScaleMin(double val) {
    if (m_dBmPanScaleMin == val) return;
    m_dBmPanScaleMin = val;
    emit panScaleChanged();
}

void SliceModel::setDBmPanScaleMax(double val) {
    if (m_dBmPanScaleMax == val) return;
    m_dBmPanScaleMax = val;
    emit panScaleChanged();
}

void SliceModel::setActive(bool active) {
    if (m_active == active) return;
    m_active = active;
    emit activeChanged(m_active);
}
