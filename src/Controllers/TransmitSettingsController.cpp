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

    // --- Model -> View ---
    connect(m_model, &Settings::currentReceiverChanged, this, [this](int rx) {
        m_view->setCurrentReceiver(rx);
        m_view->setFreeDVMode(rx, m_model->getFreeDVMode(rx));
    });

    connect(m_model, &Settings::freeDVModeChanged, this, [this](int rx, int mode) {
        m_view->setFreeDVMode(rx, mode);
    });
}
