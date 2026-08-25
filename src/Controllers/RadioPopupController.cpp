#include "RadioPopupController.h"
#include "cusdr_settings.h"
#include "Models/SliceModel.h"
#include "Models/RadioModel.h"
#include "Models/BandPlanManager.h"
#include "Util/DxClusterClient.h"
#include "cusdr_radioPopupWidget.h"
#include "cusdr_agcWidget.h"
#include "UI/noisefilterwidget.h"
#include "cusdr_displayWidget.h"

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

    const HamBand hamBand = m_model->getCurrentHamBand(rx);
    QList<DSPMode> dspModes = m_model->getDSPModeList(rx);
    const DSPMode liveMode = m_model->getDSPMode(rx);
    if (static_cast<int>(hamBand) >= 0 && static_cast<int>(hamBand) < dspModes.size()) {
        dspModes[static_cast<int>(hamBand)] = liveMode;
    }
    m_view->setDSPModeList(dspModes);
    m_view->setHamBand(hamBand);
    m_view->setDSPMode(liveMode);
    m_view->setADCMode(m_model->getADCMode(rx));
    m_view->setAGCMode(m_model->getAGCMode(rx));
    m_view->setDefaultFilterMode(m_model->getDefaultFilterMode(rx));
    m_view->setFilterFrequencies(m_model->getFilterLo(rx), m_model->getFilterHi(rx));
    m_view->setSpectrumAveraging(m_model->getPanAveragingMode(rx) != AV_MODE_NONE);
    m_view->setPanGrid(m_model->getPanGridStatus(rx));
    m_view->setPeakHold(m_model->getPeakHoldStatus(rx));
    m_view->setPanLocked(m_model->getPanLockedStatus(rx));
    m_view->setClickVFO(m_model->getClickVFOStatus(rx));
    m_view->setHairCross(m_model->getHairCrossStatus(rx));
    m_view->setPanadapterMode(m_model->getPanadapterMode(rx));
    m_view->setWaterfallColorMode(m_model->getWaterfallColorMode(rx));
    m_view->setLastFrequencies(m_model->getLastCenterFrequencyList(rx), m_model->getLastVfoFrequencyList(rx));
    m_view->setCtrFrequency(m_model->getCtrFrequency(rx));
    m_view->setVfoFrequency(m_model->getVfoFrequency(rx));
    m_view->setFreeDVMode(m_model->getFreeDVMode(rx));
    m_view->setAGCShowLines(m_model->getAgcLines(rx));

    m_lastPanAvMode = m_model->getPanAveragingMode(rx);
    if (m_lastPanAvMode == AV_MODE_NONE)
        m_lastPanAvMode = AV_MODE_RECURSIVE;

    // View -> Model
    connect(m_view, &RadioPopupWidget::hamBandRequested, this, [this](int r, HamBand band) {
        m_model->setHamBand(r, true, band);
        // Restore last-used VFO for this band from Settings (authoritative), not a stale view cache.
        const QList<qint64> lasts = m_model->getLastVfoFrequencyList(r);
        const int bandIdx = static_cast<int>(band);
        if (bandIdx >= 0 && bandIdx < lasts.size()) {
            m_model->setVFOFrequency(2, r, lasts.at(bandIdx));
        }
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
        }
        m_model->setRXFilter(r, low, high);
    });

    connect(m_view, &RadioPopupWidget::filterSlopeRequested, this, [this](int r, int slope) {
        if (m_sliceModel && r == m_sliceModel->id()) {
            m_sliceModel->setFilterSlope(slope);
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
        // Pan Avg toggles WDSP pan averaging (Display Options "Averaging Mode").
        if (enabled) {
            PanAveragingMode mode = m_lastPanAvMode;
            if (mode == AV_MODE_NONE)
                mode = AV_MODE_RECURSIVE;
            m_model->setPanAveragingMode(r, mode);
            m_model->setSpectrumAveraging(r, true);
        } else {
            const PanAveragingMode cur = m_model->getPanAveragingMode(r);
            if (cur != AV_MODE_NONE)
                m_lastPanAvMode = cur;
            m_model->setPanAveragingMode(r, AV_MODE_NONE);
            m_model->setSpectrumAveraging(r, false);
        }
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

    RadioModel* radioModel = qobject_cast<RadioModel*>(m_sliceModel ? m_sliceModel->parent() : nullptr);
    if (radioModel && radioModel->dxClusterClient()) {
        DxClusterClient* client = radioModel->dxClusterClient();
        m_view->setDxCluster(client->isConnected() || client->state() == DxClusterClient::Connecting || client->state() == DxClusterClient::WaitingForLogin);

        connect(m_view, &RadioPopupWidget::dxClusterRequested, this, [client, radioModel, this](bool enabled) {
            if (enabled) {
                QString call = m_model ? m_model->getCallsign() : QString();
                if (call.trimmed().isEmpty() || call == QStringLiteral("Your Call sign"))
                    call = QStringLiteral("ZL2BRG");
                client->connectTelnet(QStringLiteral("telnet.reversebeacon.net"), 7000, call);
            } else {
                client->disconnectFromCluster();
                if (radioModel && radioModel->bandPlan()) {
                    radioModel->bandPlan()->clearDynamicSpots();
                }
            }
        });

        connect(client, &DxClusterClient::stateChanged, this, [this](DxClusterClient::State state) {
            if (m_view) {
                m_view->setDxCluster(state == DxClusterClient::Connected ||
                                     state == DxClusterClient::Connecting ||
                                     state == DxClusterClient::WaitingForLogin);
            }
        });
    }

    if (m_sliceModel) {
        m_view->setCwDecodeEnabled(m_sliceModel->cwDecodeEnabled());
        connect(m_view, &RadioPopupWidget::cwDecodeRequested, this, [this](bool enabled) {
            if (m_sliceModel) {
                m_sliceModel->setCwDecodeEnabled(enabled);
                if (m_model)
                    m_model->setCwDecode(m_sliceModel->id(), enabled);
            }
        });
        connect(m_sliceModel, &SliceModel::cwDecodeEnabledChanged, this, [this](bool enabled) {
            if (m_view)
                m_view->setCwDecodeEnabled(enabled);
            if (m_model && m_sliceModel)
                m_model->setCwDecode(m_sliceModel->id(), enabled);
        });
    }

    // Model -> View
    connect(m_model, &Settings::systemStateChanged, m_view, &RadioPopupWidget::systemStateChanged);

    connect(m_model, &Settings::hamBandChanged, this, [this](int r, bool byBtn, HamBand band) {
        Q_UNUSED(byBtn)
        if (m_view->getReceiver() == r) {
            m_view->setHamBand(band);
        }
    });

    connect(m_model, &Settings::dspModeChanged, this, [this](int r, DSPMode mode) {
        if (m_view->getReceiver() == r) {
            m_view->setDSPMode(mode);
        }
    });

    connect(m_model, &Settings::ctrFrequencyChanged, this, [this](int mode, int r, qint64 freq) {
        Q_UNUSED(mode)
        if (m_view->getReceiver() == r) {
            m_view->setCtrFrequency(freq);
        }
    });

    connect(m_model, &Settings::vfoFrequencyChanged, this, [this](int mode, int r, qint64 freq) {
        Q_UNUSED(mode)
        if (m_view->getReceiver() == r) {
            m_view->setVfoFrequency(freq);
        }
    });

    connect(m_model, &Settings::graphicModeChanged, this, [this](int r, PanGraphicsMode panMode, WaterfallColorMode waterMode) {
        if (m_view->getReceiver() == r) {
            m_view->setPanadapterMode(panMode);
            m_view->setWaterfallColorMode(waterMode);
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

    if (m_sliceModel) {
        connect(m_sliceModel, &SliceModel::dspModeChanged, this, [this](DSPMode mode) {
            m_view->setDSPMode(mode);
        });
        connect(m_sliceModel, &SliceModel::frequencyChanged, this, [this](qint64 freq) {
            m_view->setVfoFrequency(freq);
        });
        connect(m_sliceModel, &SliceModel::centerFrequencyChanged, this, [this](qint64 freq) {
            m_view->setCtrFrequency(freq);
        });
        connect(m_sliceModel, &SliceModel::panModeChanged, this, [this](PanGraphicsMode mode) {
            m_view->setPanadapterMode(mode);
        });
        connect(m_sliceModel, &SliceModel::waterfallModeChanged, this, [this](WaterfallColorMode mode) {
            m_view->setWaterfallColorMode(mode);
        });
        connect(m_sliceModel, &SliceModel::filterChanged, this, [this]() {
            m_view->setFilterFrequencies(m_sliceModel->filterLow(), m_sliceModel->filterHigh());
        });
        connect(m_sliceModel, &SliceModel::filterSlopeChanged, this, [this](int slope) {
            m_view->setFilterSlope(slope);
        });
        connect(m_sliceModel, &SliceModel::panAveragingModeChanged, this, [this](PanAveragingMode mode) {
            m_view->setSpectrumAveraging(mode != AV_MODE_NONE);
            if (mode != AV_MODE_NONE)
                m_lastPanAvMode = mode;
        });
        connect(m_sliceModel, &SliceModel::peakHoldChanged, this, [this](bool enabled) {
            m_view->setPeakHold(enabled);
        });
    }

    // AGCOptionsWidget binding
    AGCOptionsWidget* agcView = m_view->agcOptionsWidget();
    if (agcView) {
        agcView->setReceiver(rx);
        agcView->setAGCMode(m_model->getAGCMode(rx));
        agcView->setAGCSlope(m_model->getAGCSlope(rx));
        agcView->setAGCMaximumGain(m_sliceModel ? m_sliceModel->agcMaxGain() : static_cast<int>(m_model->getAGCMaximumGain_dB(rx)));
        agcView->setAGCAttackTime(static_cast<int>(m_model->getAGCAttackTime(rx) * 1000));
        agcView->setAGCDecayTime(static_cast<int>(m_model->getAGCDecayTime(rx) * 1000));
        agcView->setAGCHangTime(static_cast<int>(m_model->getAGCHangTime(rx) * 1000));
        agcView->setAGCFixedGain(m_sliceModel ? m_sliceModel->agcFixedGain() : static_cast<int>(m_model->getAGCFixedGain_dB(rx)));
        agcView->setAGCHangThreshold(m_sliceModel ? m_sliceModel->agcHangThreshold() : static_cast<int>(m_model->getAGCHangThreshold(rx)));

        // AGCOptionsWidget View -> Model
        connect(agcView, &AGCOptionsWidget::agcModeRequested, this, [this](int r, AGCMode mode) {
            m_model->setAGCMode(r, mode);
        });

        connect(agcView, &AGCOptionsWidget::agcSlopeRequested, this, [this](int r, int val) {
            if (m_sliceModel && r == m_sliceModel->id()) {
                m_sliceModel->setAgcSlope(val);
            } else {
                m_model->setAGCVariableGain_dB(r, static_cast<qreal>(val));
            }
        });

        connect(agcView, &AGCOptionsWidget::agcMaximumGainRequested, this, [this](int r, int val) {
            if (m_sliceModel && r == m_sliceModel->id()) {
                m_sliceModel->setAgcMaxGain(val);
            } else {
                m_model->setAGCMaximumGain_dB(r, static_cast<qreal>(val));
            }
        });

        connect(agcView, &AGCOptionsWidget::agcFixedGainRequested, this, [this](int r, int val) {
            if (m_sliceModel && r == m_sliceModel->id()) {
                m_sliceModel->setAgcFixedGain(val);
            } else {
                m_model->setAGCFixedGain_dB(r, static_cast<qreal>(val));
            }
        });

        connect(agcView, &AGCOptionsWidget::agcAttackTimeRequested, this, [this](int r, int val) {
            m_model->setAGCAttackTime(r, val / 1000.0);
        });

        connect(agcView, &AGCOptionsWidget::agcDecayTimeRequested, this, [this](int r, int val) {
            m_model->setAGCDecayTime(r, val / 1000.0);
        });

        connect(agcView, &AGCOptionsWidget::agcHangTimeRequested, this, [this](int r, int val) {
            m_model->setAGCHangTime(r, val / 1000.0);
        });

        connect(agcView, &AGCOptionsWidget::agcHangThresholdRequested, this, [this](int r, int val) {
            if (m_sliceModel && r == m_sliceModel->id()) {
                m_sliceModel->setAgcHangThreshold(val);
            } else {
                m_model->setAGCHangThreshold(r, val);
            }
        });

        // AGCOptionsWidget Model -> View
        connect(m_model, &Settings::agcModeChanged, this, [this, agcView](int r, AGCMode mode) {
            if (m_view->getReceiver() == r) {
                m_view->setAGCMode(mode);
                agcView->setAGCMode(mode);
            }
        });

        connect(m_model, &Settings::agcMaximumGainChanged_dB, this, [this, agcView](int r, qreal val) {
            if (m_view->getReceiver() == r) {
                agcView->setAGCMaximumGain(static_cast<int>(val));
            }
        });

        connect(m_model, &Settings::agcFixedGainChanged_dB, this, [this, agcView](int r, qreal val) {
            if (m_view->getReceiver() == r) {
                agcView->setAGCFixedGain(static_cast<int>(val));
            }
        });

        connect(m_model, &Settings::agcHangThresholdSliderChanged, this, [this, agcView](int r, qreal val) {
            if (m_view->getReceiver() == r) {
                agcView->setAGCHangThreshold(static_cast<int>(val));
            }
        });

        if (m_sliceModel) {
            connect(m_sliceModel, &SliceModel::agcModeChanged, this, [this, agcView](AGCMode mode) {
                m_view->setAGCMode(mode);
                agcView->setAGCMode(mode);
            });
            connect(m_sliceModel, &SliceModel::agcMaxGainChanged, this, [this, agcView](int gain) {
                agcView->setAGCMaximumGain(gain);
            });
            connect(m_sliceModel, &SliceModel::agcFixedGainChanged, this, [this, agcView](int gain) {
                agcView->setAGCFixedGain(gain);
            });
            connect(m_sliceModel, &SliceModel::agcHangThresholdChanged, this, [this, agcView](int val) {
                agcView->setAGCHangThreshold(val);
            });
            connect(m_sliceModel, &SliceModel::agcSlopeChanged, this, [this, agcView](int val) {
                agcView->setAGCSlope(val);
            });
        }
    }

    // NoiseFilterWidget binding
    NoiseFilterWidget* nfView = m_view->noiseFilterWidget();
    if (nfView && m_sliceModel) {
        nfView->setReceiver(rx);
        nfView->setNrMode(m_sliceModel->nrMode());
        nfView->setNbMode(m_sliceModel->nbMode());
        nfView->setNr2GainMethod(m_sliceModel->nr2GainMethod());
        nfView->setNr2NpeMethod(m_sliceModel->nr2NpeMethod());
        nfView->setNrAgc(m_sliceModel->nrAgc());
        nfView->setNr2Ae(m_sliceModel->nr2Ae());
        nfView->setSnb(m_sliceModel->snb());
        nfView->setAnf(m_sliceModel->anf());
        if (m_model) {
            nfView->setEmnrPost2Enabled(m_model->getEmnrPost2Enabled());
            nfView->setEmnrPost2Factor(m_model->getEmnrPost2Factor());
            nfView->setEmnrPost2Nlevel(m_model->getEmnrPost2Nlevel());
            nfView->setEmnrPost2Taper(m_model->getEmnrPost2Taper());
            nfView->setEmnrPost2Rate(m_model->getEmnrPost2Rate());
        }

        // Connect View -> Model (SliceModel)
        connect(nfView, &NoiseFilterWidget::nrModeRequested, this, [this](int val) {
            if (m_sliceModel) m_sliceModel->setNrMode(val);
        });
        connect(nfView, &NoiseFilterWidget::nbModeRequested, this, [this](int val) {
            if (m_sliceModel) m_sliceModel->setNbMode(val);
        });
        connect(nfView, &NoiseFilterWidget::nr2GainMethodRequested, this, [this](int val) {
            if (m_sliceModel) m_sliceModel->setNr2GainMethod(val);
        });
        connect(nfView, &NoiseFilterWidget::nr2NpeMethodRequested, this, [this](int val) {
            if (m_sliceModel) m_sliceModel->setNr2NpeMethod(val);
        });
        connect(nfView, &NoiseFilterWidget::nrAgcRequested, this, [this](int val) {
            if (m_sliceModel) m_sliceModel->setNrAgc(val);
        });
        connect(nfView, &NoiseFilterWidget::nr2AeRequested, this, [this](bool val) {
            if (m_sliceModel) m_sliceModel->setNr2Ae(val);
        });
        connect(nfView, &NoiseFilterWidget::snbRequested, this, [this](bool val) {
            if (m_sliceModel) m_sliceModel->setSnb(val);
        });
        connect(nfView, &NoiseFilterWidget::anfRequested, this, [this](bool val) {
            if (m_sliceModel) m_sliceModel->setAnf(val);
        });
        connect(nfView, &NoiseFilterWidget::emnrPost2EnabledRequested, this, [this](bool enabled) {
            if (m_model) m_model->setEmnrPost2Enabled(enabled);
        });
        connect(nfView, &NoiseFilterWidget::emnrPost2FactorRequested, this, [this](double pct) {
            if (m_model) m_model->setEmnrPost2Factor(pct);
        });
        connect(nfView, &NoiseFilterWidget::emnrPost2NlevelRequested, this, [this](double pct) {
            if (m_model) m_model->setEmnrPost2Nlevel(pct);
        });
        connect(nfView, &NoiseFilterWidget::emnrPost2TaperRequested, this, [this](double pct) {
            if (m_model) m_model->setEmnrPost2Taper(pct);
        });
        connect(nfView, &NoiseFilterWidget::emnrPost2RateRequested, this, [this](double seconds) {
            if (m_model) m_model->setEmnrPost2Rate(seconds);
        });

        // Connect Model (SliceModel) -> View
        connect(m_sliceModel, &SliceModel::nrModeChanged, this, [nfView](int val) {
            nfView->setNrMode(val);
        });
        connect(m_sliceModel, &SliceModel::nbModeChanged, this, [nfView](int val) {
            nfView->setNbMode(val);
        });
        connect(m_sliceModel, &SliceModel::nr2GainMethodChanged, this, [nfView](int val) {
            nfView->setNr2GainMethod(val);
        });
        connect(m_sliceModel, &SliceModel::nr2NpeMethodChanged, this, [nfView](int val) {
            nfView->setNr2NpeMethod(val);
        });
        connect(m_sliceModel, &SliceModel::nrAgcChanged, this, [nfView](int val) {
            nfView->setNrAgc(val);
        });
        connect(m_sliceModel, &SliceModel::nr2AeChanged, this, [nfView](bool val) {
            nfView->setNr2Ae(val);
        });
        connect(m_sliceModel, &SliceModel::snbChanged, this, [nfView](bool val) {
            nfView->setSnb(val);
        });
        connect(m_sliceModel, &SliceModel::anfChanged, this, [nfView](bool val) {
            nfView->setAnf(val);
        });
        if (m_model) {
            connect(m_model, &Settings::emnrPost2Changed, this, [this, nfView]() {
                nfView->setEmnrPost2Enabled(m_model->getEmnrPost2Enabled());
                nfView->setEmnrPost2Factor(m_model->getEmnrPost2Factor());
                nfView->setEmnrPost2Nlevel(m_model->getEmnrPost2Nlevel());
                nfView->setEmnrPost2Taper(m_model->getEmnrPost2Taper());
                nfView->setEmnrPost2Rate(m_model->getEmnrPost2Rate());
            });
        }
    }

    // DisplayOptionsWidget binding (Spectrum & Display DSP controls)
    DisplayOptionsWidget* dispView = m_view->displayOptionsWidget();
    if (dispView && m_sliceModel) {
        dispView->setCurrentReceiver(rx);
        dispView->setFramesPerSecond(m_model ? m_model->getFramesPerSecond(rx) : 30);
        dispView->setSpectrumAveragingCnt(m_sliceModel->spectrumAveragingCnt());
        dispView->setWaterfallOffsetLo(m_sliceModel->waterfallOffsetLo());
        dispView->setWaterfallOffsetHi(m_sliceModel->waterfallOffsetHi());
        dispView->setPanadapterMode(m_sliceModel->panMode());
        dispView->setWaterfallColorMode(m_sliceModel->waterfallMode());
        dispView->setPanAveragingMode(m_sliceModel->panAveragingMode());
        dispView->setPanDetectorMode(m_sliceModel->panDetectorMode());
        dispView->setfftSize(m_sliceModel->fftSize());
        if (m_model) {
            dispView->setSMeterHoldTime(m_model->getSMeterHoldTime());
            dispView->setCallsign(m_model->getCallsign());
            dispView->setWidebandAveragingCnt(m_model->getSpectrumAveragingCnt(-1));
        }

        // View -> Model
        connect(dispView, &DisplayOptionsWidget::framesPerSecondRequested, this, [this](int r, int val) {
            if (m_model) m_model->setFramesPerSecond(r, val);
        });

        connect(dispView, &DisplayOptionsWidget::spectrumAveragingCntRequested, this, [this](int r, int val) {
            if (r == -1) {
                if (m_model) m_model->setSpectrumAveragingCnt(-1, val);
            } else if (m_sliceModel) {
                m_sliceModel->setSpectrumAveragingCnt(val);
                if (m_model) m_model->setSpectrumAveragingCnt(r, val);
            }
        });

        connect(dispView, &DisplayOptionsWidget::waterfallTimeRequested, this, [this](int r, int val) {
            if (m_model) m_model->setWaterfallTime(r, val);
        });

        connect(dispView, &DisplayOptionsWidget::waterfallOffsetLoRequested, this, [this](int r, int val) {
            if (m_sliceModel) {
                m_sliceModel->setWaterfallOffsetLo(val);
                if (m_model) m_model->setWaterfallOffesetLo(r, val);
            }
        });

        connect(dispView, &DisplayOptionsWidget::waterfallOffsetHiRequested, this, [this](int r, int val) {
            if (m_sliceModel) {
                m_sliceModel->setWaterfallOffsetHi(val);
                if (m_model) m_model->setWaterfallOffesetHi(r, val);
            }
        });

        connect(dispView, &DisplayOptionsWidget::sMeterHoldTimeRequested, this, [this](int val) {
            if (m_model) m_model->setSMeterHoldTime(val);
        });

        connect(dispView, &DisplayOptionsWidget::callsignRequested, this, [this](const QString& val) {
            if (m_model) m_model->setCallsign(val);
        });

        connect(dispView, &DisplayOptionsWidget::graphicsStateRequested, this, [this](int r, int panadapterMode, int waterColorMode) {
            if (r >= 0 && m_sliceModel) {
                m_sliceModel->setPanMode(static_cast<PanGraphicsMode>(panadapterMode));
                m_sliceModel->setWaterfallMode(static_cast<WaterfallColorMode>(waterColorMode));
                if (m_model) m_model->setGraphicsState(r, static_cast<PanGraphicsMode>(panadapterMode), static_cast<WaterfallColorMode>(waterColorMode));
            } else if (r == -1 && m_model) {
                m_model->setGraphicsState(-1, static_cast<PanGraphicsMode>(panadapterMode), static_cast<WaterfallColorMode>(waterColorMode));
            }
        });

        connect(dispView, &DisplayOptionsWidget::panAveragingModeRequested, this, [this](int r, int mode) {
            if (m_sliceModel) {
                m_sliceModel->setPanAveragingMode(static_cast<PanAveragingMode>(mode));
                if (m_model) m_model->setPanAveragingMode(r, static_cast<PanAveragingMode>(mode));
            }
        });

        connect(dispView, &DisplayOptionsWidget::panDetectorModeRequested, this, [this](int r, int mode) {
            if (m_sliceModel) {
                m_sliceModel->setPanDetectorMode(static_cast<PanDetectorMode>(mode));
                if (m_model) m_model->setPanDetectorMode(r, static_cast<PanDetectorMode>(mode));
            }
        });

        connect(dispView, &DisplayOptionsWidget::fftSizeRequested, this, [this](int r, int size) {
            if (m_sliceModel) {
                m_sliceModel->setFftSize(size);
                if (m_model) m_model->setfftSize(r, size);
            }
        });

        connect(dispView, &DisplayOptionsWidget::fmsqLevelRequested, this, [this](int r, int val) {
            if (m_model) m_model->setfmsqLevel(r, val);
        });

        // Model (SliceModel) -> View
        connect(m_sliceModel, &SliceModel::spectrumAveragingCntChanged, this, [dispView](int val) {
            dispView->setSpectrumAveragingCnt(val);
        });
        connect(m_sliceModel, &SliceModel::waterfallOffsetChanged, this, [this, dispView]() {
            if (m_sliceModel) {
                dispView->setWaterfallOffsetLo(m_sliceModel->waterfallOffsetLo());
                dispView->setWaterfallOffsetHi(m_sliceModel->waterfallOffsetHi());
            }
        });
        connect(m_sliceModel, &SliceModel::panModeChanged, this, [dispView](PanGraphicsMode mode) {
            dispView->setPanadapterMode(mode);
        });
        connect(m_sliceModel, &SliceModel::waterfallModeChanged, this, [dispView](WaterfallColorMode mode) {
            dispView->setWaterfallColorMode(mode);
        });
        connect(m_sliceModel, &SliceModel::panAveragingModeChanged, this, [dispView](PanAveragingMode mode) {
            dispView->setPanAveragingMode(mode);
        });
        connect(m_sliceModel, &SliceModel::panDetectorModeChanged, this, [dispView](PanDetectorMode mode) {
            dispView->setPanDetectorMode(mode);
        });
        connect(m_sliceModel, &SliceModel::fftSizeChanged, this, [dispView](int size) {
            dispView->setfftSize(size);
        });
        if (m_model) {
            connect(m_model, &Settings::spectrumAveragingCntChanged, this, [this, dispView](int r, int val) {
                if (r == -1) {
                    dispView->setWidebandAveragingCnt(val);
                } else if (dispView->currentReceiver() == r) {
                    dispView->setSpectrumAveragingCnt(val);
                }
            });
        }
    }
}
