#include "TransmitModel.h"

TransmitModel::TransmitModel(QObject *parent)
    : QObject(parent)
{
}

void TransmitModel::setAmCarrierLevel(int level) {
    if (m_amCarrierLevel == level) return;
    m_amCarrierLevel = level;
    emit amCarrierLevelChanged(m_amCarrierLevel);
}

void TransmitModel::setAudioCompression(int val) {
    if (m_audioCompression == val) return;
    m_audioCompression = val;
    emit audioCompressionChanged(m_audioCompression);
}

void TransmitModel::setFmDeviation(int val) {
    if (m_fmDeviation == val) return;
    m_fmDeviation = val;
    emit fmDeviationChanged(m_fmDeviation);
}

void TransmitModel::setFmPreEmphasis(bool enabled) {
    if (m_fmPreEmphasis == enabled) return;
    m_fmPreEmphasis = enabled;
    emit fmPreEmphasisChanged(m_fmPreEmphasis);
}

void TransmitModel::setPhaseRotator(bool enabled) {
    if (m_phaseRotator == enabled) return;
    m_phaseRotator = enabled;
    emit phaseRotatorChanged(m_phaseRotator);
}

void TransmitModel::setPhaseRotatorAuto(bool enabled) {
    if (m_phaseRotatorAuto == enabled) return;
    m_phaseRotatorAuto = enabled;
    emit phaseRotatorAutoChanged(m_phaseRotatorAuto);
}

void TransmitModel::setPhaseRotatorStatus(const QString &status) {
    if (m_phaseRotatorStatus == status) return;
    m_phaseRotatorStatus = status;
    emit phaseRotatorStatusChanged(m_phaseRotatorStatus);
}

void TransmitModel::setCtcssToneHz(int hz) {
    if (m_ctcssToneHz == hz) return;
    m_ctcssToneHz = hz;
    emit ctcssToneHzChanged(m_ctcssToneHz);
}

void TransmitModel::setTxEqEnabled(bool enabled) {
    if (m_txEqEnabled == enabled) return;
    m_txEqEnabled = enabled;
    emit txEqChanged();
}

void TransmitModel::setTxEqBands(const QVector<int> &bands) {
    if (m_txEqBands == bands) return;
    m_txEqBands = bands;
    emit txEqChanged();
}

void TransmitModel::setTxEqBand(int index, int gainDb) {
    if (index < 0 || index >= m_txEqBands.size()) return;
    if (m_txEqBands[index] == gainDb) return;
    m_txEqBands[index] = gainDb;
    emit txEqChanged();
}

void TransmitModel::setTxEqCurveDeg(int deg) {
    if (m_txEqCurveDeg == deg) return;
    m_txEqCurveDeg = deg;
    emit txEqChanged();
}

void TransmitModel::setCfcEnabled(bool enabled) {
    if (m_cfcEnabled == enabled) return;
    m_cfcEnabled = enabled;
    emit cfcChanged();
}

void TransmitModel::setCfcPeqEnabled(bool enabled) {
    if (m_cfcPeqEnabled == enabled) return;
    m_cfcPeqEnabled = enabled;
    emit cfcChanged();
}

void TransmitModel::setCfcPrecomp(double db) {
    if (qFuzzyCompare(m_cfcPrecomp, db)) return;
    m_cfcPrecomp = db;
    emit cfcChanged();
}

void TransmitModel::setCfcPrePeq(double db) {
    if (qFuzzyCompare(m_cfcPrePeq, db)) return;
    m_cfcPrePeq = db;
    emit cfcChanged();
}

void TransmitModel::setCfcCurveDeg(int deg) {
    if (m_cfcCurveDeg == deg) return;
    m_cfcCurveDeg = deg;
    emit cfcChanged();
}

void TransmitModel::setCfcLevels(const QVector<double> &levels) {
    if (m_cfcLevels == levels) return;
    m_cfcLevels = levels;
    emit cfcChanged();
}

void TransmitModel::setCfcLevel(int index, double db) {
    if (index < 0 || index >= m_cfcLevels.size()) return;
    if (qFuzzyCompare(m_cfcLevels[index], db)) return;
    m_cfcLevels[index] = db;
    emit cfcChanged();
}

void TransmitModel::setCfcPost(const QVector<double> &post) {
    if (m_cfcPost == post) return;
    m_cfcPost = post;
    emit cfcChanged();
}

void TransmitModel::setCfcPostBand(int index, double db) {
    if (index < 0 || index >= m_cfcPost.size()) return;
    if (qFuzzyCompare(m_cfcPost[index], db)) return;
    m_cfcPost[index] = db;
    emit cfcChanged();
}

void TransmitModel::setMicInputDev(int dev) {
    if (m_micInputDev == dev) return;
    m_micInputDev = dev;
    emit micInputDevChanged(m_micInputDev);
}

void TransmitModel::setMicInputSourceName(const QString &name) {
    if (m_micInputSourceName == name) return;
    m_micInputSourceName = name;
    emit micInputSourceNameChanged(m_micInputSourceName);
}

void TransmitModel::setDigitalAudioInputDev(int dev) {
    if (m_digitalAudioInputDev == dev) return;
    m_digitalAudioInputDev = dev;
    emit digitalAudioInputDevChanged(m_digitalAudioInputDev);
}

void TransmitModel::setDigitalInputSourceName(const QString &name) {
    if (m_digitalInputSourceName == name) return;
    m_digitalInputSourceName = name;
    emit digitalInputSourceNameChanged(m_digitalInputSourceName);
}

void TransmitModel::setCwKeyerMode(int val) {
    if (m_cwKeyerMode == val) return;
    m_cwKeyerMode = val;
    emit cwKeyerModeChanged(m_cwKeyerMode);
}

void TransmitModel::setInternalCw(bool val) {
    if (m_internalCw == val) return;
    m_internalCw = val;
    emit internalCwChanged(m_internalCw);
}

void TransmitModel::setCwKeyReversed(bool val) {
    if (m_cwKeyReversed == val) return;
    m_cwKeyReversed = val;
    emit cwKeyReversedChanged(m_cwKeyReversed);
}

void TransmitModel::setCwKeyerSpacing(bool val) {
    if (m_cwKeyerSpacing == val) return;
    m_cwKeyerSpacing = val;
    emit cwKeyerSpacingChanged(m_cwKeyerSpacing);
}

void TransmitModel::setCwKeyerSpeed(int val) {
    if (m_cwKeyerSpeed == val) return;
    m_cwKeyerSpeed = val;
    emit cwKeyerSpeedChanged(m_cwKeyerSpeed);
}

void TransmitModel::setCwPttDelay(int val) {
    if (m_cwPttDelay == val) return;
    m_cwPttDelay = val;
    emit cwPttDelayChanged(m_cwPttDelay);
}

void TransmitModel::setCwSidetoneFreq(int val) {
    if (m_cwSidetoneFreq == val) return;
    m_cwSidetoneFreq = val;
    emit cwSidetoneFreqChanged(m_cwSidetoneFreq);
}

void TransmitModel::setCwSidetoneVolume(int val) {
    if (m_cwSidetoneVolume == val) return;
    m_cwSidetoneVolume = val;
    emit cwSidetoneVolumeChanged(m_cwSidetoneVolume);
}

void TransmitModel::setCwHangTime(int val) {
    if (m_cwHangTime == val) return;
    m_cwHangTime = val;
    emit cwHangTimeChanged(m_cwHangTime);
}

void TransmitModel::setCwKeyerWeight(int val) {
    if (m_cwKeyerWeight == val) return;
    m_cwKeyerWeight = val;
    emit cwKeyerWeightChanged(m_cwKeyerWeight);
}

void TransmitModel::setTxFilterLow(int val) {
    if (m_txFilterLow == val) return;
    m_txFilterLow = val;
    emit txFilterLowChanged(m_txFilterLow);
}

void TransmitModel::setTxFilterHigh(int val) {
    if (m_txFilterHigh == val) return;
    m_txFilterHigh = val;
    emit txFilterHighChanged(m_txFilterHigh);
}

void TransmitModel::setTxUseRxFilter(bool enabled) {
    if (m_txUseRxFilter == enabled) return;
    m_txUseRxFilter = enabled;
    emit txUseRxFilterChanged(m_txUseRxFilter);
}
