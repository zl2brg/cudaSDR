#include "TransmitSettingsController.h"
#include "cusdr_settings.h"
#include "Models/TransmitModel.h"
#include "UI/tx_settings_dialog.h"
#include "cusdr_transmitOptionsWidget.h"

TransmitSettingsController::TransmitSettingsController(QObject* parent)
    : QObject(parent)
{
}

void TransmitSettingsController::bind(tx_settings_dialog* view, Settings* model)
{
    bind(view, nullptr, model);
}

void TransmitSettingsController::bind(tx_settings_dialog* view, TransmitModel* txModel, Settings* model)
{
    m_view = view;
    m_txModel = txModel;
    m_model = model;

    if (!m_view || !m_model) {
        return;
    }

    // --- Initial Values Setup ---
    m_view->setAvailableCodec2Modes(m_model->availableCodec2Modes());
    m_view->setCodec2ModeStringResolver([this](int mode) { return m_model->getCodec2ModeString(mode); });

    if (m_txModel) {
        m_view->setAmCarrierLevel(m_txModel->amCarrierLevel());
        m_view->setAudioCompression(m_txModel->audioCompression());
        m_view->setFmDeviation(m_txModel->fmDeviation());
        m_view->setFmPreEmphasis(m_txModel->fmPreEmphasis());
        m_view->setPhaseRotator(m_txModel->phaseRotator());
        m_view->setPhaseRotatorAuto(m_txModel->phaseRotatorAuto());
        m_view->setPhaseRotatorStatus(m_txModel->phaseRotatorStatus());
        m_view->setTxEqEnabled(m_txModel->txEqEnabled());
        m_view->setTxEqBands(m_txModel->txEqBands());
        m_view->setTxEqCurveDeg(m_txModel->txEqCurveDeg());
        m_view->setCfcEnabled(m_txModel->cfcEnabled());
        m_view->setCfcPeqEnabled(m_txModel->cfcPeqEnabled());
        m_view->setCfcPrecomp(m_txModel->cfcPrecomp());
        m_view->setCfcPrePeq(m_txModel->cfcPrePeq());
        m_view->setCfcCurveDeg(m_txModel->cfcCurveDeg());
        m_view->setCfcLevels(m_txModel->cfcLevels());
        m_view->setCfcPost(m_txModel->cfcPost());
        m_view->setCtcssToneHz(m_txModel->ctcssToneHz());
        m_view->setCwSidetoneFreq(m_txModel->cwSidetoneFreq());
        m_view->setCwSidetoneVolume(m_txModel->cwSidetoneVolume());
        m_view->setCwHangTime(m_txModel->cwHangTime());
        m_view->setCwKeyerMode(m_txModel->cwKeyerMode());
        m_view->setInternalCw(m_txModel->internalCw());
        m_view->setCwKeyReversed(m_txModel->cwKeyReversed());
        m_view->setCwKeyerSpacing(m_txModel->cwKeyerSpacing());
        m_view->setCwKeyerSpeed(m_txModel->cwKeyerSpeed());
        m_view->setCwPttDelay(m_txModel->cwPttDelay());
        m_view->setCwKeyerWeight(m_txModel->cwKeyerWeight());
        m_view->setTxFilterLow(m_txModel->txFilterLow());
        m_view->setTxFilterHigh(m_txModel->txFilterHigh());
        m_view->setTxUseRxFilter(m_txModel->txUseRxFilter());
        m_view->refreshAudioDevices(m_txModel->micInputSourceName(),
                                    m_txModel->digitalInputSourceName());
    } else {
        m_view->setAmCarrierLevel(m_model->getAMCarrierLevel());
        m_view->setAudioCompression(m_model->getAudioCompression());
        m_view->setFmDeviation(static_cast<int>(m_model->getFMDeveation()));
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
        m_view->setTxFilterLow(m_model->getTxFilterLow());
        m_view->setTxFilterHigh(m_model->getTxFilterHigh());
        m_view->setTxUseRxFilter(m_model->getTxUseRxFilter());
        m_view->refreshAudioDevices(m_model->getMicInputSourceName(),
                                    m_model->getDigitalInputSourceName());
    }
    m_view->refreshEqCurvePlots();
    m_view->setCurrentReceiver(m_model->getCurrentReceiver());
    m_view->setFreeDVMode(m_model->getCurrentReceiver(), m_model->getFreeDVMode(m_model->getCurrentReceiver()));

    // --- View -> TransmitModel (Settings persistence via syncSettingsWithTransmit on save) ---
    connect(m_view, &tx_settings_dialog::audioDevicesRefreshRequested, this, [this]() {
        if (m_txModel) {
            m_view->refreshAudioDevices(m_txModel->micInputSourceName(),
                                        m_txModel->digitalInputSourceName());
        } else {
            m_view->refreshAudioDevices(m_model->getMicInputSourceName(),
                                        m_model->getDigitalInputSourceName());
        }
    });

    auto requireTx = [this]() { return m_txModel != nullptr; };

    connect(m_view, &tx_settings_dialog::micInputDevChanged, this, [this](int dev) {
        if (m_txModel) m_txModel->setMicInputDev(dev);
        else m_model->setMicInputDev(dev);
    });
    connect(m_view, &tx_settings_dialog::micInputSourceNameChanged, this, [this](const QString& name) {
        if (m_txModel) m_txModel->setMicInputSourceName(name);
        else m_model->setMicInputSourceName(name);
    });
    connect(m_view, &tx_settings_dialog::digitalAudioInputDevChanged, this, [this](int dev) {
        if (m_txModel) m_txModel->setDigitalAudioInputDev(dev);
        else m_model->setDigitalAudioInputDev(dev);
    });
    connect(m_view, &tx_settings_dialog::digitalInputSourceNameChanged, this, [this](const QString& name) {
        if (m_txModel) m_txModel->setDigitalInputSourceName(name);
        else m_model->setDigitalInputSourceName(name);
    });
    connect(m_view, &tx_settings_dialog::freeDVModeRequested, this, [this](int rx, int mode) {
        m_model->setFreeDVMode(rx, mode);
    });
    connect(m_view, &tx_settings_dialog::audioCompressionRequested, this, [this](int val) {
        if (m_txModel) m_txModel->setAudioCompression(val);
        else m_model->setAudioCompression(val);
    });
    connect(m_view, &tx_settings_dialog::amCarrierLevelRequested, this, [this](int val) {
        if (m_txModel) m_txModel->setAmCarrierLevel(val);
        else m_model->setAMCarrierLevel(val);
    });
    connect(m_view, &tx_settings_dialog::fmDeviationRequested, this, [this](int val) {
        if (m_txModel) m_txModel->setFmDeviation(val);
        else m_model->setFmDeveation(val);
    });
    connect(m_view, &tx_settings_dialog::fmPreEmphasisRequested, this, [this](bool enabled) {
        if (m_txModel) m_txModel->setFmPreEmphasis(enabled);
        else m_model->setFMPreEmphasize(enabled ? 1 : 0);
    });
    connect(m_view, &tx_settings_dialog::phaseRotatorRequested, this, [this](bool enabled) {
        if (m_txModel) m_txModel->setPhaseRotator(enabled);
        else m_model->setPhaseRotator(enabled ? 1 : 0);
    });
    connect(m_view, &tx_settings_dialog::phaseRotatorAutoRequested, this, [this](bool enabled) {
        if (m_txModel) m_txModel->setPhaseRotatorAuto(enabled);
        else m_model->setPhaseRotatorAuto(enabled);
    });
    connect(m_view, &tx_settings_dialog::phaseRotatorAutoResetRequested, this, [this]() {
        if (m_txModel) emit m_txModel->phaseRotatorAutoResetRequested();
        else m_model->requestPhaseRotatorAutoReset();
    });
    connect(m_view, &tx_settings_dialog::txEqEnabledRequested, this, [this](bool enabled) {
        if (m_txModel) m_txModel->setTxEqEnabled(enabled);
        else m_model->setTxEqEnabled(enabled);
    });
    connect(m_view, &tx_settings_dialog::txEqBandRequested, this, [this](int index, int gainDb) {
        if (m_txModel) m_txModel->setTxEqBand(index, gainDb);
        else m_model->setTxEqBand(index, gainDb);
    });
    connect(m_view, &tx_settings_dialog::txEqCurveDegRequested, this, [this](int deg) {
        if (m_txModel) m_txModel->setTxEqCurveDeg(deg);
        else m_model->setTxEqCurveDeg(deg);
    });
    connect(m_view, &tx_settings_dialog::cfcEnabledRequested, this, [this](bool enabled) {
        if (m_txModel) m_txModel->setCfcEnabled(enabled);
        else m_model->setCfcEnabled(enabled);
    });
    connect(m_view, &tx_settings_dialog::cfcPeqEnabledRequested, this, [this](bool enabled) {
        if (m_txModel) m_txModel->setCfcPeqEnabled(enabled);
        else m_model->setCfcPeqEnabled(enabled);
    });
    connect(m_view, &tx_settings_dialog::cfcPrecompRequested, this, [this](double db) {
        if (m_txModel) m_txModel->setCfcPrecomp(db);
        else m_model->setCfcPrecomp(db);
    });
    connect(m_view, &tx_settings_dialog::cfcPrePeqRequested, this, [this](double db) {
        if (m_txModel) m_txModel->setCfcPrePeq(db);
        else m_model->setCfcPrePeq(db);
    });
    connect(m_view, &tx_settings_dialog::cfcCurveDegRequested, this, [this](int deg) {
        if (m_txModel) m_txModel->setCfcCurveDeg(deg);
        else m_model->setCfcCurveDeg(deg);
    });
    connect(m_view, &tx_settings_dialog::cfcLevelRequested, this, [this](int index, double db) {
        if (m_txModel) m_txModel->setCfcLevel(index, db);
        else m_model->setCfcLevel(index, db);
    });
    connect(m_view, &tx_settings_dialog::cfcPostRequested, this, [this](int index, double db) {
        if (m_txModel) m_txModel->setCfcPostBand(index, db);
        else m_model->setCfcPostBand(index, db);
    });
    connect(m_view, &tx_settings_dialog::ctcssToneHzRequested, this, [this](int hz) {
        if (m_txModel) m_txModel->setCtcssToneHz(hz);
        else m_model->setCtcssToneHz(hz);
    });
    connect(m_view, &tx_settings_dialog::cwKeyerModeRequested, this, [this](int val) {
        if (m_txModel) m_txModel->setCwKeyerMode(val);
        else m_model->setCwKeyerMode(val);
    });
    connect(m_view, &tx_settings_dialog::internalCwRequested, this, [this](bool val) {
        if (m_txModel) m_txModel->setInternalCw(val);
        else m_model->setInternalCw(val ? 1 : 0);
    });
    connect(m_view, &tx_settings_dialog::cwKeyReversedRequested, this, [this](bool val) {
        if (m_txModel) m_txModel->setCwKeyReversed(val);
        else m_model->setCwKeyReversed(val ? 1 : 0);
    });
    connect(m_view, &tx_settings_dialog::cwKeyerSpacingRequested, this, [this](bool val) {
        if (m_txModel) m_txModel->setCwKeyerSpacing(val);
        else m_model->setCwKeyerSpacing(val ? 1 : 0);
    });
    connect(m_view, &tx_settings_dialog::cwKeyerSpeedRequested, this, [this](int val) {
        if (m_txModel) m_txModel->setCwKeyerSpeed(val);
        else m_model->setCwKeyerSpeed(val);
    });
    connect(m_view, &tx_settings_dialog::cwPttDelayRequested, this, [this](int val) {
        if (m_txModel) m_txModel->setCwPttDelay(val);
        else m_model->setCwPttDelay(val);
    });
    connect(m_view, &tx_settings_dialog::cwSidetoneFreqRequested, this, [this](int val) {
        if (m_txModel) m_txModel->setCwSidetoneFreq(val);
        else m_model->setCwSidetoneFreq(val);
    });
    connect(m_view, &tx_settings_dialog::cwSidetoneVolumeRequested, this, [this](int val) {
        if (m_txModel) m_txModel->setCwSidetoneVolume(val);
        else m_model->setCwSidetoneVolume(val);
    });
    connect(m_view, &tx_settings_dialog::cwHangTimeRequested, this, [this](int val) {
        if (m_txModel) m_txModel->setCwHangTime(val);
        else m_model->setCwHangTime(val);
    });
    connect(m_view, &tx_settings_dialog::cwKeyerWeightRequested, this, [this](int val) {
        if (m_txModel) m_txModel->setCwKeyerWeight(val);
        else m_model->setCwKeyerWeight(val);
    });
    connect(m_view, &tx_settings_dialog::txFilterLowRequested, this, [this](int val) {
        if (m_txModel) m_txModel->setTxFilterLow(val);
        else m_model->setTxFilterLow(val);
    });
    connect(m_view, &tx_settings_dialog::txFilterHighRequested, this, [this](int val) {
        if (m_txModel) m_txModel->setTxFilterHigh(val);
        else m_model->setTxFilterHigh(val);
    });
    connect(m_view, &tx_settings_dialog::txUseRxFilterRequested, this, [this](bool val) {
        if (m_txModel) m_txModel->setTxUseRxFilter(val);
        else m_model->setTxUseRxFilter(val);
    });

    // --- TransmitModel -> View ---
    if (m_txModel) {
        connect(m_txModel, &TransmitModel::fmPreEmphasisChanged, m_view, &tx_settings_dialog::setFmPreEmphasis);
        connect(m_txModel, &TransmitModel::phaseRotatorChanged, m_view, &tx_settings_dialog::setPhaseRotator);
        connect(m_txModel, &TransmitModel::phaseRotatorAutoChanged, m_view, &tx_settings_dialog::setPhaseRotatorAuto);
        connect(m_txModel, &TransmitModel::phaseRotatorStatusChanged, m_view, &tx_settings_dialog::setPhaseRotatorStatus);
        connect(m_txModel, &TransmitModel::txEqChanged, this, [this]() {
            m_view->setTxEqEnabled(m_txModel->txEqEnabled());
            m_view->setTxEqBands(m_txModel->txEqBands());
            m_view->setTxEqCurveDeg(m_txModel->txEqCurveDeg());
            m_view->refreshEqCurvePlots();
        });
        connect(m_txModel, &TransmitModel::cfcChanged, this, [this]() {
            m_view->setCfcEnabled(m_txModel->cfcEnabled());
            m_view->setCfcPeqEnabled(m_txModel->cfcPeqEnabled());
            m_view->setCfcPrecomp(m_txModel->cfcPrecomp());
            m_view->setCfcPrePeq(m_txModel->cfcPrePeq());
            m_view->setCfcCurveDeg(m_txModel->cfcCurveDeg());
            m_view->setCfcLevels(m_txModel->cfcLevels());
            m_view->setCfcPost(m_txModel->cfcPost());
            m_view->refreshEqCurvePlots();
        });
        connect(m_txModel, &TransmitModel::amCarrierLevelChanged, this, [this](int level) {
            m_view->setAmCarrierLevel(level);
        });
        connect(m_txModel, &TransmitModel::audioCompressionChanged, this, [this](int val) {
            m_view->setAudioCompression(val);
        });
        connect(m_txModel, &TransmitModel::txFilterLowChanged, m_view, &tx_settings_dialog::setTxFilterLow);
        connect(m_txModel, &TransmitModel::txFilterHighChanged, m_view, &tx_settings_dialog::setTxFilterHigh);
        connect(m_txModel, &TransmitModel::txUseRxFilterChanged, m_view, &tx_settings_dialog::setTxUseRxFilter);
    } else {
        connect(m_model, &Settings::fmPremphasizechanged, this, [this](double value) {
            m_view->setFmPreEmphasis(value != 0.0);
        });
        connect(m_model, &Settings::phaseRotatorChanged, this, [this](int value) {
            m_view->setPhaseRotator(value != 0);
        });
        connect(m_model, &Settings::phaseRotatorAutoChanged, m_view, &tx_settings_dialog::setPhaseRotatorAuto);
        connect(m_model, &Settings::phaseRotatorStatusChanged, m_view, &tx_settings_dialog::setPhaseRotatorStatus);
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
        connect(m_model, &Settings::txFilterLowChanged, m_view, &tx_settings_dialog::setTxFilterLow);
        connect(m_model, &Settings::txFilterHighChanged, m_view, &tx_settings_dialog::setTxFilterHigh);
        connect(m_model, &Settings::txUseRxFilterChanged, m_view, &tx_settings_dialog::setTxUseRxFilter);
    }

    connect(m_model, &Settings::currentReceiverChanged, this, [this](int rx) {
        m_view->setCurrentReceiver(rx);
        m_view->setFreeDVMode(rx, m_model->getFreeDVMode(rx));
    });
    connect(m_model, &Settings::freeDVModeChanged, this, [this](int rx, int mode) {
        m_view->setFreeDVMode(rx, mode);
    });

    Q_UNUSED(requireTx);
}

void TransmitSettingsController::bindOptions(TransmitOptionsWidget* options, TransmitModel* txModel)
{
    m_optionsView = options;
    if (!m_optionsView)
        return;
    if (txModel)
        m_txModel = txModel;
    if (!m_txModel)
        return;

    m_optionsView->setAmCarrierLevel(m_txModel->amCarrierLevel());
    m_optionsView->setAudioCompression(m_txModel->audioCompression());

    connect(m_optionsView, &TransmitOptionsWidget::amCarrierLevelRequested, this, [this](int val) {
        m_txModel->setAmCarrierLevel(val);
    });
    connect(m_optionsView, &TransmitOptionsWidget::audioCompressionRequested, this, [this](int val) {
        m_txModel->setAudioCompression(val);
    });
    connect(m_txModel, &TransmitModel::amCarrierLevelChanged, m_optionsView, &TransmitOptionsWidget::setAmCarrierLevel);
    connect(m_txModel, &TransmitModel::audioCompressionChanged, m_optionsView, &TransmitOptionsWidget::setAudioCompression);
}
