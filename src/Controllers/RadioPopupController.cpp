#include "RadioPopupController.h"
#include "cusdr_settings.h"
#include "Models/SliceModel.h"
#include "cusdr_radioPopupWidget.h"

RadioPopupController::RadioPopupController(QObject* parent)
    : QObject(parent)
{
}

void RadioPopupController::bind(RadioPopupWidget* view, SliceModel* sliceModel, Settings* model)
{
    m_view = view;
    m_sliceModel = sliceModel;
    m_model = model;

    if (!m_view || !m_model) {
        return;
    }

    int rx = m_view->getReceiver();

    // Initial setup loading
    m_view->setSingleAdcDevice((m_model->getHWInterface() == QSDR::Hermes) || (m_model->getHPSDRHardware() == 1));
    m_view->setBandFrequencyList(m_model->getBandFrequencyList());

    m_view->setHamBand(m_model->getCurrentHamBand(rx));
    m_view->setDSPModeList(m_model->getDSPModeList(rx));
    m_view->setADCMode(m_model->getADCMode(rx));
    m_view->setAGCMode(m_model->getAGCMode(rx));
    m_view->setDefaultFilterMode(m_model->getDefaultFilterMode(rx));
    m_view->setFilterFrequencies(m_model->getFilterLo(rx), m_model->getFilterHi(rx));
    m_view->setSpectrumAveraging(m_model->getSpectrumAveraging(rx));
    m_view->setPanGrid(m_model->getPanGridStatus(rx));
    m_view->setPeakHold(m_model->getPeakHoldStatus(rx));
    m_view->setPanLocked(m_model->getPanLockedStatus(rx));
    m_view->setClickVFO(m_model->getClickVFOStatus(rx));
    m_view->setHairCross(m_model->getHairCrossStatus(rx));
    m_view->setPanadapterMode(m_model->getPanadapterMode(rx));
    m_view->setWaterfallColorMode(m_model->getWaterfallColorMode(rx));
    m_view->setLastFrequencies(m_model->getLastCenterFrequencyList(rx), m_model->getLastVfoFrequencyList(rx));
    m_view->setFreeDVMode(m_model->getFreeDVMode(rx));
    m_view->setAGCShowLines(m_model->getAgcLines(rx));

    // View -> Model
    connect(m_view, &RadioPopupWidget::hamBandRequested, this, [this](int r, HamBand band) {
        m_model->setHamBand(r, true, band);
    });

    connect(m_view, &RadioPopupWidget::vfoFrequencyRequested, this, [this](int r, qint64 val) {
        m_model->setVFOFrequency(2, r, val);
    });

    connect(m_view, &RadioPopupWidget::freeDVModeRequested, this, [this](int r, int mode) {
        m_model->setFreeDVMode(r, mode);
    });

    connect(m_view, &RadioPopupWidget::dspModeRequested, this, [this](int r, DSPMode mode) {
        m_model->setDSPMode(r, mode);
    });

    connect(m_view, &RadioPopupWidget::filterFrequenciesRequested, this, [this](int r, qreal low, qreal high) {
        if (m_sliceModel && r == m_sliceModel->id()) {
            m_sliceModel->setFilterLow(static_cast<float>(low));
            m_sliceModel->setFilterHigh(static_cast<float>(high));
        } else {
            m_model->setRXFilter(r, low, high);
        }
    });

    connect(m_view, &RadioPopupWidget::adcModeRequested, this, [this](int r, ADCMode mode) {
        m_model->setADCMode(r, mode);
    });

    connect(m_view, &RadioPopupWidget::agcModeRequested, this, [this](int r, AGCMode mode) {
        m_model->setAGCMode(r, mode);
    });

    connect(m_view, &RadioPopupWidget::agcShowLinesRequested, this, [this](int r, bool enabled) {
        m_model->setAGCShowLines(r, enabled);
    });

    connect(m_view, &RadioPopupWidget::spectrumAveragingRequested, this, [this](int r, bool enabled) {
        m_model->setSpectrumAveraging(r, enabled);
    });

    connect(m_view, &RadioPopupWidget::panGridRequested, this, [this](int r, bool enabled) {
        m_model->setPanGrid(enabled, r);
    });

    connect(m_view, &RadioPopupWidget::peakHoldRequested, this, [this](int r, bool enabled) {
        m_model->setPeakHold(enabled, r);
    });

    connect(m_view, &RadioPopupWidget::panLockedRequested, this, [this](int r, bool enabled) {
        m_model->setPanLocked(enabled, r);
    });

    connect(m_view, &RadioPopupWidget::clickVFORequested, this, [this](int r, bool enabled) {
        m_model->setClickVFO(enabled, r);
    });

    connect(m_view, &RadioPopupWidget::hairCrossRequested, this, [this](int r, bool enabled) {
        m_model->setHairCross(enabled, r);
    });

    connect(m_view, &RadioPopupWidget::graphicsStateRequested, this, [this](int r, PanGraphicsMode panMode, WaterfallColorMode waterMode) {
        m_model->setGraphicsState(r, panMode, waterMode);
    });

    // Model -> View
    connect(m_model, &Settings::systemStateChanged, m_view, &RadioPopupWidget::systemStateChanged);

    connect(m_model, &Settings::hamBandChanged, this, [this](int r, bool byBtn, HamBand band) {
        Q_UNUSED(byBtn)
        if (m_view->getReceiver() == r) {
            m_view->setHamBand(band);
        }
    });

    connect(m_model, &Settings::freeDVModeChanged, this, [this](int r, int mode) {
        if (m_view->getReceiver() == r) {
            m_view->setFreeDVMode(mode);
        }
    });

    connect(m_model, &Settings::freeDVStatusChanged, this, [this](int r, bool sync, float snr, quint64 rxFrames, quint64 txFrames) {
        if (m_view->getReceiver() == r) {
            m_view->setFreeDVStatus(sync, snr, rxFrames, txFrames);
        }
    });

    connect(m_model, &Settings::adcModeChanged, this, [this](int r, ADCMode mode) {
        if (m_view->getReceiver() == r) {
            m_view->setADCMode(mode);
        }
    });

    connect(m_model, &Settings::filterFrequenciesChanged, this, [this](int r, qreal low, qreal high) {
        if (m_view->getReceiver() == r) {
            m_view->setFilterFrequencies(low, high);
        }
    });
}
