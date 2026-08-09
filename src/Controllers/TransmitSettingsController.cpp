#include "TransmitSettingsController.h"
#include "cusdr_settings.h"
#include "UI/tx_settings_dialog.h"

TransmitSettingsController::TransmitSettingsController(QObject* parent)
    : QObject(parent)
{
}

void TransmitSettingsController::bind(tx_settings_dialog* view, Settings* model)
{
    m_view = view;
    m_model = model;

    if (!m_view || !m_model) {
        return;
    }

    // --- Initial Values Setup ---
    m_view->setAvailableCodec2Modes(m_model->availableCodec2Modes());
    m_view->setCodec2ModeStringResolver([this](int mode) { return m_model->getCodec2ModeString(mode); });

    m_view->setAmCarrierLevel(m_model->getAMCarrierLevel());
    m_view->setAudioCompression(m_model->getAudioCompression());
    m_view->setFmDeviation(m_model->getFMDeveation());
    m_view->setFmPreEmphasis(m_model->getFMpreemphesis() != 0.0);
    m_view->setPhaseRotator(m_model->getPhaseRotator() != 0);
    m_view->setPhaseRotatorAuto(m_model->getPhaseRotatorAuto());
    m_view->setTxEqEnabled(m_model->getTxEqEnabled());
    m_view->setTxEqBands(m_model->getTxEqBands());
    m_view->setTxEqCurveDeg(m_model->getTxEqCurveDeg());
    m_view->setCfcEnabled(m_model->getCfcEnabled());
    m_view->setCfcPeqEnabled(m_model->getCfcPeqEnabled());
    m_view->setCfcPrecomp(m_model->getCfcPrecomp());
    m_view->setCfcPrePeq(m_model->getCfcPrePeq());
    m_view->setCfcCurveDeg(m_model->getCfcCurveDeg());
    m_view->setCfcLevels(m_model->getCfcLevels());
    m_view->setCfcPost(m_model->getCfcPost());
    m_view->refreshEqCurvePlots();
    m_view->setCtcssToneHz(m_model->getCtcssToneHz());
    m_view->setCwSidetoneFreq(m_model->getCwSidetoneFreq());
    m_view->setCwSidetoneVolume(m_model->getCwSidetoneVolume());
    m_view->setCwHangTime(m_model->getCwHangTime());
    m_view->setCwKeyerMode(m_model->getCwKeyerMode());
    m_view->setInternalCw(m_model->isInternalCw());
    m_view->setCwKeyReversed(m_model->isCwKeyReversed());
    m_view->setCwKeyerSpacing(m_model->getCwKeyerSpacing());
    m_view->setCwKeyerSpeed(m_model->getCwKeyerSpeed());
    m_view->setCwPttDelay(m_model->getCwPttDelay());
    m_view->setCwKeyerWeight(m_model->getCwKeyerWeight());
    m_view->setCurrentReceiver(m_model->getCurrentReceiver());
    m_view->setFreeDVMode(m_model->getCurrentReceiver(), m_model->getFreeDVMode(m_model->getCurrentReceiver()));

    m_view->refreshAudioDevices(m_model->getMicInputSourceName(), m_model->getDigitalInputSourceName());

    // --- View -> Model ---
    connect(m_view, &tx_settings_dialog::audioDevicesRefreshRequested, this, [this]() {
        m_view->refreshAudioDevices(m_model->getMicInputSourceName(), m_model->getDigitalInputSourceName());
    });

    connect(m_view, &tx_settings_dialog::micInputDevChanged, this, [this](int dev) {
        m_model->setMicInputDev(dev);
    });

    connect(m_view, &tx_settings_dialog::micInputSourceNameChanged, this, [this](const QString& name) {
        m_model->setMicInputSourceName(name);
    });

    connect(m_view, &tx_settings_dialog::digitalAudioInputDevChanged, this, [this](int dev) {
        m_model->setDigitalAudioInputDev(dev);
    });

    connect(m_view, &tx_settings_dialog::digitalInputSourceNameChanged, this, [this](const QString& name) {
        m_model->setDigitalInputSourceName(name);
    });

    connect(m_view, &tx_settings_dialog::freeDVModeRequested, this, [this](int rx, int mode) {
        m_model->setFreeDVMode(rx, mode);
    });

    connect(m_view, &tx_settings_dialog::audioCompressionRequested, this, [this](int val) {
        m_model->setAudioCompression(val);
    });

    connect(m_view, &tx_settings_dialog::amCarrierLevelRequested, this, [this](int val) {
        m_model->setAMCarrierLevel(val);
    });

    connect(m_view, &tx_settings_dialog::fmDeviationRequested, this, [this](int val) {
        m_model->setFmDeveation(val);
    });

    connect(m_view, &tx_settings_dialog::fmPreEmphasisRequested, this, [this](bool enabled) {
        m_model->setFMPreEmphasize(enabled ? 1 : 0);
    });

    connect(m_view, &tx_settings_dialog::phaseRotatorRequested, this, [this](bool enabled) {
        m_model->setPhaseRotator(enabled ? 1 : 0);
    });

    connect(m_view, &tx_settings_dialog::phaseRotatorAutoRequested, this, [this](bool enabled) {
        m_model->setPhaseRotatorAuto(enabled);
    });

    connect(m_view, &tx_settings_dialog::phaseRotatorAutoResetRequested, this, [this]() {
        m_model->requestPhaseRotatorAutoReset();
    });

    connect(m_view, &tx_settings_dialog::txEqEnabledRequested, this, [this](bool enabled) {
        m_model->setTxEqEnabled(enabled);
    });

    connect(m_view, &tx_settings_dialog::txEqBandRequested, this, [this](int index, int gainDb) {
        m_model->setTxEqBand(index, gainDb);
    });

    connect(m_view, &tx_settings_dialog::txEqCurveDegRequested, this, [this](int deg) {
        m_model->setTxEqCurveDeg(deg);
    });

    connect(m_view, &tx_settings_dialog::cfcEnabledRequested, this, [this](bool enabled) {
        m_model->setCfcEnabled(enabled);
    });
    connect(m_view, &tx_settings_dialog::cfcPeqEnabledRequested, this, [this](bool enabled) {
        m_model->setCfcPeqEnabled(enabled);
    });
    connect(m_view, &tx_settings_dialog::cfcPrecompRequested, this, [this](double db) {
        m_model->setCfcPrecomp(db);
    });
    connect(m_view, &tx_settings_dialog::cfcPrePeqRequested, this, [this](double db) {
        m_model->setCfcPrePeq(db);
    });
    connect(m_view, &tx_settings_dialog::cfcCurveDegRequested, this, [this](int deg) {
        m_model->setCfcCurveDeg(deg);
    });
    connect(m_view, &tx_settings_dialog::cfcLevelRequested, this, [this](int index, double db) {
        m_model->setCfcLevel(index, db);
    });
    connect(m_view, &tx_settings_dialog::cfcPostRequested, this, [this](int index, double db) {
        m_model->setCfcPostBand(index, db);
    });

    connect(m_view, &tx_settings_dialog::ctcssToneHzRequested, this, [this](int hz) {
        m_model->setCtcssToneHz(hz);
    });

    connect(m_view, &tx_settings_dialog::cwKeyerModeRequested, this, [this](int val) {
        m_model->setCwKeyerMode(val);
    });

    connect(m_view, &tx_settings_dialog::internalCwRequested, this, [this](bool val) {
        m_model->setInternalCw(val ? 1 : 0);
    });

    connect(m_view, &tx_settings_dialog::cwKeyReversedRequested, this, [this](bool val) {
        m_model->setCwKeyReversed(val ? 1 : 0);
    });

    connect(m_view, &tx_settings_dialog::cwKeyerSpacingRequested, this, [this](bool val) {
        m_model->setCwKeyerSpacing(val ? 1 : 0);
    });

    connect(m_view, &tx_settings_dialog::cwKeyerSpeedRequested, this, [this](int val) {
        m_model->setCwKeyerSpeed(val);
    });

    connect(m_view, &tx_settings_dialog::cwPttDelayRequested, this, [this](int val) {
        m_model->setCwPttDelay(val);
    });

    connect(m_view, &tx_settings_dialog::cwSidetoneFreqRequested, this, [this](int val) {
        m_model->setCwSidetoneFreq(val);
    });

    connect(m_view, &tx_settings_dialog::cwSidetoneVolumeRequested, this, [this](int val) {
        m_model->setCwSidetoneVolume(val);
    });

    connect(m_view, &tx_settings_dialog::cwHangTimeRequested, this, [this](int val) {
        m_model->setCwHangTime(val);
    });

    connect(m_view, &tx_settings_dialog::cwKeyerWeightRequested, this, [this](int val) {
        m_model->setCwKeyerWeight(val);
    });

    connect(m_model, &Settings::fmPremphasizechanged, this, [this](double value) {
        m_view->setFmPreEmphasis(value != 0.0);
    });

    connect(m_model, &Settings::phaseRotatorChanged, this, [this](int value) {
        m_view->setPhaseRotator(value != 0);
    });

    connect(m_model, &Settings::phaseRotatorAutoChanged, this, [this](bool enabled) {
        m_view->setPhaseRotatorAuto(enabled);
    });

    connect(m_model, &Settings::phaseRotatorStatusChanged, this, [this](const QString &status) {
        m_view->setPhaseRotatorStatus(status);
    });

    connect(m_model, &Settings::txEqChanged, this, [this]() {
        m_view->setTxEqEnabled(m_model->getTxEqEnabled());
        m_view->setTxEqBands(m_model->getTxEqBands());
        m_view->setTxEqCurveDeg(m_model->getTxEqCurveDeg());
        m_view->refreshEqCurvePlots();
    });

    connect(m_model, &Settings::cfcChanged, this, [this]() {
        m_view->setCfcEnabled(m_model->getCfcEnabled());
        m_view->setCfcPeqEnabled(m_model->getCfcPeqEnabled());
        m_view->setCfcPrecomp(m_model->getCfcPrecomp());
        m_view->setCfcPrePeq(m_model->getCfcPrePeq());
        m_view->setCfcCurveDeg(m_model->getCfcCurveDeg());
        m_view->setCfcLevels(m_model->getCfcLevels());
        m_view->setCfcPost(m_model->getCfcPost());
        m_view->refreshEqCurvePlots();
    });

    // --- Model -> View ---
    connect(m_model, &Settings::currentReceiverChanged, this, [this](int rx) {
        m_view->setCurrentReceiver(rx);
        m_view->setFreeDVMode(rx, m_model->getFreeDVMode(rx));
    });

    connect(m_model, &Settings::freeDVModeChanged, this, [this](int rx, int mode) {
        m_view->setFreeDVMode(rx, mode);
    });
}
