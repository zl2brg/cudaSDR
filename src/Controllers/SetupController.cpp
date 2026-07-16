#include "SetupController.h"
#include "UI/cusdr_setupwidget.h"
#include "cusdr_settings.h"

SetupController::SetupController(QObject* parent)
    : QObject(parent)
{
}

void SetupController::bind(cusdr_SetupWidget* view, Settings* model)
{
    m_view = view;
    m_model = model;

    if (!m_view || !m_model) {
        return;
    }

    // Load initial states
    bool hasPennyOrPenelope = m_model->getPenelopePresence() || m_model->getPennyLanePresence();
    m_view->setTabEnabled(2, (m_model->getHWInterface() == QSDR::Hermes) || hasPennyOrPenelope);
    m_view->setTabEnabled(3, m_model->getAlexPresence());

    // Connect Model -> View
    connect(m_model, &Settings::systemStateChanged, this, [this](QSDR::_Error, QSDR::_HWInterfaceMode hwmode, QSDR::_ServerMode, QSDR::_DataEngineState) {
        bool hasPennyOrPenelope = m_model->getPenelopePresence() || m_model->getPennyLanePresence();
        m_view->setTabEnabled(2, (hwmode == QSDR::Hermes) || hasPennyOrPenelope);
    });

    connect(m_model, &Settings::alexPresenceChanged, this, [this](bool val) {
        m_view->setTabEnabled(3, val);
    });

    connect(m_model, &Settings::penelopePresenceChanged, this, [this](bool val) {
        bool hasPennyOrPenelope = val || m_model->getPennyLanePresence();
        m_view->setTabEnabled(2, (m_model->getHWInterface() == QSDR::Hermes) || hasPennyOrPenelope);
    });

    connect(m_model, &Settings::pennyLanePresenceChanged, this, [this](bool val) {
        bool hasPennyOrPenelope = m_model->getPenelopePresence() || val;
        m_view->setTabEnabled(2, (m_model->getHWInterface() == QSDR::Hermes) || hasPennyOrPenelope);
    });
}
