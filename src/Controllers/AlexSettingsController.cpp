#include "AlexSettingsController.h"
#include "cusdr_settings.h"
#include "cusdr_alexTabWidget.h"
#include "cusdr_alexAntennaWidget.h"
#include "cusdr_alexFilterWidget.h"
#include "Models/RadioModel.h"

AlexSettingsController::AlexSettingsController(QObject* parent)
    : QObject(parent)
{
}

void AlexSettingsController::bind(AlexTabWidget* container, Settings* model)
{
    bind(container, nullptr, model);
}

void AlexSettingsController::bind(AlexTabWidget* container, RadioModel* radioModel, Settings* model)
{
    m_container = container;
    m_radioModel = radioModel;
    m_model = model;

    if (!m_container || !m_model) {
        return;
    }

    m_antennaView = m_container->findChild<AlexAntennaWidget*>();
    m_filterView = m_container->findChild<AlexFilterWidget*>();

    // --- Bind Antenna View ---
    if (m_antennaView) {
        // Settings is the source of truth at bind time. RadioModel.txParams().alexStates
        // starts empty and is only filled later by DataEngine — using it here would
        // wipe the view's band list and crash on the next antenna click.
        const quint16 alexConfig = m_model->getAlexConfig();
        const QList<int> alexStates = m_model->getAlexStates();
        if (m_radioModel) {
            m_radioModel->txParams().alexConfig = alexConfig;
            m_radioModel->txParams().alexStates = alexStates;
        }
        m_antennaView->setAlexConfig(alexConfig);
        m_antennaView->setAlexStates(alexStates);

        // View -> Model
        connect(m_antennaView, &AlexAntennaWidget::alexStateRequested, this, [this](int index, int state) {
            if (m_radioModel) {
                auto& states = m_radioModel->txParams().alexStates;
                if (index >= 0 && index < states.size())
                    states[index] = state;
            }
            m_model->setAlexState(index, state);
        });

        // Model -> View
        connect(m_model, &Settings::alexStatesChanged, this, [this](const QList<int>& states) {
            if (m_radioModel) {
                m_radioModel->txParams().alexStates = states;
            }
            m_antennaView->setAlexStates(states);
        });
        connect(m_model, &Settings::alexStateChanged, this, [this](HamBand, const QList<int>& states) {
            if (m_radioModel) {
                m_radioModel->txParams().alexStates = states;
            }
            m_antennaView->setAlexStates(states);
        });

        connect(m_model, &Settings::alexConfigurationChanged, this, [this](quint16 config) {
            if (m_radioModel) {
                m_radioModel->txParams().alexConfig = config;
            }
            m_antennaView->setAlexConfig(config);
        });
    }

    // --- Bind Filter View ---
    if (m_filterView) {
        // Initial setup — same Settings-first seeding as antenna view.
        const int currentRx = m_model->getCurrentReceiver();
        const quint16 alexConfig = m_model->getAlexConfig();
        const QList<int> alexStates = m_model->getAlexStates();
        if (m_radioModel) {
            m_radioModel->txParams().alexConfig = alexConfig;
            m_radioModel->txParams().alexStates = alexStates;
        }
        m_filterView->setAlexConfig(alexConfig);
        m_filterView->setAlexStates(alexStates);
        m_filterView->setAlexManualState((alexConfig & 0x01) != 0);
        m_filterView->setFrequencies(
            m_model->getHPFLoFrequencies(),
            m_model->getHPFHiFrequencies(),
            m_model->getLPFLoFrequencies(),
            m_model->getLPFHiFrequencies()
        );
        m_filterView->setCurrentReceiver(currentRx);
        m_filterView->setFrequency(0, currentRx, m_model->getVfoFrequency(currentRx));

        // View -> Model
        connect(m_filterView, &AlexFilterWidget::manualFilterRequested, this, [this](bool manual) {
            m_model->setAlexToManual(manual);
        });

        connect(m_filterView, &AlexFilterWidget::alexConfigurationRequested, this, [this](quint16 config) {
            if (m_radioModel) {
                m_radioModel->txParams().alexConfig = config;
            }
            m_model->setAlexConfiguration(config);
        });

        connect(m_filterView, &AlexFilterWidget::hpfLoFrequencyRequested, this, [this](int filter, long val) {
            m_model->setAlexHPFLoFrequencies(filter, val);
        });

        connect(m_filterView, &AlexFilterWidget::hpfHiFrequencyRequested, this, [this](int filter, long val) {
            m_model->setAlexHPFHiFrequencies(filter, val);
        });

        connect(m_filterView, &AlexFilterWidget::lpfLoFrequencyRequested, this, [this](int filter, long val) {
            m_model->setAlexLPFLoFrequencies(filter, val);
        });

        connect(m_filterView, &AlexFilterWidget::lpfHiFrequencyRequested, this, [this](int filter, long val) {
            m_model->setAlexLPFHiFrequencies(filter, val);
        });

        // Model -> View
        connect(m_model, &Settings::alexManualStateChanged, this, [this](bool manual) {
            m_filterView->setAlexManualState(manual);
        });

        connect(m_model, &Settings::alexConfigurationChanged, this, [this](quint16 config) {
            if (m_radioModel) {
                m_radioModel->txParams().alexConfig = config;
            }
            m_filterView->setAlexConfig(config);
        });

        connect(m_model, &Settings::alexStatesChanged, this, [this](const QList<int>& states) {
            if (m_radioModel) {
                m_radioModel->txParams().alexStates = states;
            }
            m_filterView->setAlexStates(states);
        });

        connect(m_model, &Settings::currentReceiverChanged, this, [this](int rx) {
            m_filterView->setCurrentReceiver(rx);
            m_filterView->setFrequency(0, rx, m_model->getVfoFrequency(rx));
        });

        connect(m_model, &Settings::vfoFrequencyChanged, this, [this](int mode, int rx, qint64 freq) {
            m_filterView->setFrequency(mode, rx, freq);
        });
    }
}
