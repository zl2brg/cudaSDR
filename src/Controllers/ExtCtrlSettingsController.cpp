#include "ExtCtrlSettingsController.h"
#include "cusdr_settings.h"
#include "cusdr_extCtrlWidget.h"

ExtCtrlSettingsController::ExtCtrlSettingsController(QObject* parent)
    : QObject(parent)
{
}

void ExtCtrlSettingsController::bind(ExtCtrlWidget* view, Settings* model)
{
    m_view = view;
    m_model = model;

    if (!m_view || !m_model) {
        return;
    }

    // --- 1. Populate initial View state from Model ---
    m_view->setPennyOCEnabled(m_model->getPennyOCEnabled());
    m_view->setRxPins(m_model->getRxJ6Pins());
    m_view->setTxPins(m_model->getTxJ6Pins());

    // --- 2. View -> Model (User interaction events) ---
    connect(m_view, &ExtCtrlWidget::pennyOCEnabledRequested, this, [this](bool enabled) {
        if (m_model->getPennyOCEnabled() != enabled) {
            m_model->setPennyOCEnabled(enabled);
        }
    });

    connect(m_view, &ExtCtrlWidget::rxPinsRequested, this, [this](const QList<int>& pins) {
        if (m_model->getRxJ6Pins() != pins) {
            m_model->setRxJ6Pins(pins);
        }
    });

    connect(m_view, &ExtCtrlWidget::txPinsRequested, this, [this](const QList<int>& pins) {
        if (m_model->getTxJ6Pins() != pins) {
            m_model->setTxJ6Pins(pins);
        }
    });

    // --- 3. Model -> View (Model update notifications) ---
    connect(m_model, &Settings::pennyOCEnabledChanged, this, [this](bool enabled) {
        m_view->setPennyOCEnabled(enabled);
    });

    connect(m_model, &Settings::rxJ6PinsChanged, this, [this](const QList<int>& pins) {
        m_view->setRxPins(pins);
    });

    connect(m_model, &Settings::txJ6PinsChanged, this, [this](const QList<int>& pins) {
        m_view->setTxPins(pins);
    });
}
