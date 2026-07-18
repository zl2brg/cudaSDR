#include "HpsdrSettingsController.h"
#include "cusdr_settings.h"
#include "cusdr_hpsdrWidget.h"

HpsdrSettingsController::HpsdrSettingsController(QObject* parent)
    : QObject(parent)
{
}

void HpsdrSettingsController::bind(HPSDRWidget* view, Settings* model)
{
    m_view = view;
    m_model = model;

    if (!m_view || !m_model) {
        return;
    }

    // --- 1. Populate initial View state from Model ---
    m_view->setHwInterface(m_model->getHWInterface());
    m_view->setHpsdrHardware(m_model->getHPSDRHardware());
    m_view->setNumberOfReceivers(m_model->getNumberOfReceivers());
    m_view->setFirmwareCheck(m_model->getFirmwareVersionCheck());
    m_view->set10MhzSource(m_model->get10MHzSource());
    m_view->set122_88MhzSource(m_model->get122_8MHzSource());
    m_view->setSampleRate(m_model->getSampleRate());
    m_view->setMercuryPresence(m_model->getMercuryPresence());
    m_view->setPenelopePresence(m_model->getPenelopePresence());
    m_view->setPennyLanePresence(m_model->getPennyLanePresence());
    m_view->setAlexPresence(m_model->getAlexPresence());
    m_view->setExcaliburPresence(m_model->getExcaliburPresence());
    m_view->setCurrentMetisCard(m_model->getCurrentMetisCard());
    m_view->setDataEngineRunning(m_model->getDataEngineState() == QSDR::DataEngineUp);

    auto normalizeHermesHardware = [this](int hw) {
        // Hermes is integrated: clear Atlas-module presence and neutralize 10 MHz source.
        if (hw != 1)
            return;
        if (m_model->getPenelopePresence())
            m_model->setPenelopePresence(false);
        if (m_model->getPennyLanePresence())
            m_model->setPennyLanePresence(false);
        if (m_model->getMercuryPresence())
            m_model->setMercuryPresence(false);
        if (m_model->getExcaliburPresence())
            m_model->setExcaliburPresence(false);
        // Pre-MVC used source index 2 when selecting Hermes (clock group hidden).
        if (m_model->get10MHzSource() != 2)
            m_model->set10MhzSource(2);
    };
    normalizeHermesHardware(m_model->getHPSDRHardware());

    // --- 2. View -> Model (User interaction events) ---
    connect(m_view, &HPSDRWidget::hwInterfaceRequested, this, [this](QSDR::_HWInterfaceMode mode) {
        if (m_model->getHWInterface() != mode) {
            m_model->setSystemState(QSDR::NoError, mode, m_model->getCurrentServerMode(), m_model->getDataEngineState());
        }
    });

    connect(m_view, &HPSDRWidget::hpsdrHardwareRequested, this, [this, normalizeHermesHardware](int hw) {
        if (m_model->getHPSDRHardware() != hw) {
            m_model->setHPSDRHardware(hw);
        }
        normalizeHermesHardware(hw);
    });

    connect(m_view, &HPSDRWidget::numberOfReceiversRequested, this, [this](int count) {
        if (m_model->getNumberOfReceivers() != count) {
            m_model->setReceivers(count);
        }
    });

    connect(m_view, &HPSDRWidget::firmwareCheckRequested, this, [this](bool check) {
        if (m_model->getFirmwareVersionCheck() != check) {
            m_model->setCheckFirmwareVersion(check);
        }
    });

    connect(m_view, &HPSDRWidget::src10MhzRequested, this, [this](int src) {
        if (m_model->get10MHzSource() != src) {
            m_model->set10MhzSource(src);
        }
    });

    connect(m_view, &HPSDRWidget::src122_88MhzRequested, this, [this](int src) {
        if (m_model->get122_8MHzSource() != src) {
            m_model->set122_88MhzSource(src);
        }
    });

    connect(m_view, &HPSDRWidget::sampleRateRequested, this, [this](int rate) {
        if (m_model->getSampleRate() != rate) {
            m_model->setSampleRate(rate);
        }
    });

    connect(m_view, &HPSDRWidget::mercuryPresenceRequested, this, [this](bool pres) {
        if (m_model->getMercuryPresence() != pres) {
            m_model->setMercuryPresence(pres);
        }
    });

    connect(m_view, &HPSDRWidget::penelopePresenceRequested, this, [this](bool pres) {
        if (m_model->getPenelopePresence() != pres) {
            m_model->setPenelopePresence(pres);
        }
    });

    connect(m_view, &HPSDRWidget::pennyLanePresenceRequested, this, [this](bool pres) {
        if (m_model->getPennyLanePresence() != pres) {
            m_model->setPennyLanePresence(pres);
        }
    });

    connect(m_view, &HPSDRWidget::alexPresenceRequested, this, [this](bool pres) {
        if (m_model->getAlexPresence() != pres) {
            m_model->setAlexPresence(pres);
        }
    });

    connect(m_view, &HPSDRWidget::excaliburPresenceRequested, this, [this](bool pres) {
        if (m_model->getExcaliburPresence() != pres) {
            m_model->setExcaliburPresence(pres);
        }
    });

    // --- 3. Model -> View (Model update notifications) ---
    connect(m_model, &Settings::systemStateChanged, this, [this](QSDR::_Error, QSDR::_HWInterfaceMode mode, QSDR::_ServerMode, QSDR::_DataEngineState state) {
        m_view->setHwInterface(mode);
        // Re-apply device card so 768/1536 enablement tracks protocol after interface changes.
        m_view->setCurrentMetisCard(m_model->getCurrentMetisCard());
        m_view->setDataEngineRunning(state == QSDR::DataEngineUp);
    });

    connect(m_model, &Settings::numberOfRXChanged, this, [this](int count) {
        m_view->setNumberOfReceivers(count);
    });

    connect(m_model, &Settings::mercuryPresenceChanged, this, [this](bool pres) {
        m_view->setMercuryPresence(pres);
    });

    connect(m_model, &Settings::penelopePresenceChanged, this, [this](bool pres) {
        m_view->setPenelopePresence(pres);
    });

    connect(m_model, &Settings::pennyLanePresenceChanged, this, [this](bool pres) {
        m_view->setPennyLanePresence(pres);
    });

    connect(m_model, &Settings::alexPresenceChanged, this, [this](bool pres) {
        m_view->setAlexPresence(pres);
    });

    connect(m_model, &Settings::excaliburPresenceChanged, this, [this](bool pres) {
        m_view->setExcaliburPresence(pres);
    });

    connect(m_model, &Settings::checkFirmwareVersionChanged, this, [this](bool check) {
        m_view->setFirmwareCheck(check);
    });

    connect(m_model, &Settings::src10MhzChanged, this, [this](int src) {
        m_view->set10MhzSource(src);
    });

    connect(m_model, &Settings::src122_88MhzChanged, this, [this](int src) {
        m_view->set122_88MhzSource(src);
    });

    connect(m_model, &Settings::sampleRateChanged, this, [this](int rate) {
        m_view->setSampleRate(rate);
    });

    connect(m_model, &Settings::hpsdrHardwareChanged, this, [this](int hw) {
        m_view->setHpsdrHardware(hw);
    });

    connect(m_model, &Settings::hpsdrNetworkDeviceChanged, this, [this](const TNetworkDevicecard& card) {
        m_view->setCurrentMetisCard(card);
    });
}
