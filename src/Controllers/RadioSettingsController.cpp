#include "RadioSettingsController.h"

#ifdef HAVE_SOAPYSDR

#include "cusdr_settings.h"
#include "UI/cusdr_radiosettingswidget.h"

RadioSettingsController::RadioSettingsController(QObject* parent)
    : QObject(parent)
{
}

void RadioSettingsController::bind(cusdr_radioSettingsWidget* view, Settings* model)
{
    m_view = view;
    m_model = model;

    if (!m_view || !m_model) {
        return;
    }

    // --- 1. Initialize View state from Model ---
    m_view->setTxFullDuplex(m_model->getTxFullDuplex());
    m_view->setSoapyIQBalance(m_model->getSoapyIQBalance());
    m_view->setSoapyAutoCalibrate(m_model->getSoapyAutoCalibrate());
    m_view->setSoapyLnaGain(m_model->getSoapyLnaGain());
    m_view->setSoapyTiaGain(m_model->getSoapyTiaGain());
    m_view->setSoapyPgaGain(m_model->getSoapyPgaGain());
    m_view->setSoapyOverallGain(m_model->getSoapyOverallGain());
    m_view->setAntennaList(m_model->getSoapyAntennaList(), m_model->getSoapyRxAntenna());
    m_view->setTxAntennaList(m_model->getSoapyTxAntennaList(), m_model->getSoapyTxAntenna());
    m_view->updateGainGroupVisibility(m_model->getSoapyHardwareKey());

    // --- 2. View -> Model (User interaction events) ---
    connect(m_view, &cusdr_radioSettingsWidget::txFullDuplexRequested, this, [this](bool enabled) {
        if (m_model->getTxFullDuplex() != enabled) {
            m_model->setTxFullDuplex(enabled);
        }
    });

    connect(m_view, &cusdr_radioSettingsWidget::soapyIQBalanceRequested, this, [this](bool enabled) {
        if (m_model->getSoapyIQBalance() != enabled) {
            m_model->setSoapyIQBalance(enabled);
        }
    });

    connect(m_view, &cusdr_radioSettingsWidget::soapyAutoCalibrateRequested, this, [this](bool enabled) {
        if (m_model->getSoapyAutoCalibrate() != enabled) {
            m_model->setSoapyAutoCalibrate(enabled);
        }
    });

    connect(m_view, &cusdr_radioSettingsWidget::soapyLnaGainRequested, this, [this](int val) {
        if (m_model->getSoapyLnaGain() != val) {
            m_model->setSoapyLnaGain(val);
        }
    });

    connect(m_view, &cusdr_radioSettingsWidget::soapyTiaGainRequested, this, [this](int val) {
        if (m_model->getSoapyTiaGain() != val) {
            m_model->setSoapyTiaGain(val);
        }
    });

    connect(m_view, &cusdr_radioSettingsWidget::soapyPgaGainRequested, this, [this](int val) {
        if (m_model->getSoapyPgaGain() != val) {
            m_model->setSoapyPgaGain(val);
        }
    });

    connect(m_view, &cusdr_radioSettingsWidget::soapyOverallGainRequested, this, [this](int val) {
        if (m_model->getSoapyOverallGain() != val) {
            m_model->setSoapyOverallGain(val);
        }
    });

    connect(m_view, &cusdr_radioSettingsWidget::soapyRxAntennaRequested, this, [this](const QString& ant) {
        if (m_model->getSoapyRxAntenna() != ant) {
            m_model->setSoapyRxAntenna(ant);
        }
    });

    connect(m_view, &cusdr_radioSettingsWidget::soapyTxAntennaRequested, this, [this](const QString& ant) {
        if (m_model->getSoapyTxAntenna() != ant) {
            m_model->setSoapyTxAntenna(ant);
        }
    });

    // --- 3. Model -> View (Model update notifications) ---
    connect(m_model, &Settings::txFullDuplexChanged, this, [this](bool enabled) {
        m_view->setTxFullDuplex(enabled);
    });

    connect(m_model, &Settings::soapyAutoCalibrateChanged, this, [this](bool enabled) {
        m_view->setSoapyAutoCalibrate(enabled);
    });

    connect(m_model, &Settings::soapyAntennaListChanged, this, [this](const QStringList& list) {
        m_view->setAntennaList(list, m_model->getSoapyRxAntenna());
    });

    connect(m_model, &Settings::soapyTxAntennaListChanged, this, [this](const QStringList& list) {
        m_view->setTxAntennaList(list, m_model->getSoapyTxAntenna());
    });

    connect(m_model, &Settings::soapyHardwareKeyChanged, this, [this](const QString& key) {
        m_view->updateGainGroupVisibility(key);
    });
}

#endif // HAVE_SOAPYSDR
