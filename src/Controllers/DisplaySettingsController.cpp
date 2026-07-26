#include "DisplaySettingsController.h"
#include "cusdr_settings.h"
#include "cusdr_displayTabWidget.h"
#include "cusdr_displayWidget.h"
#include "cusdr_colorsWidget.h"

DisplaySettingsController::DisplaySettingsController(QObject* parent)
    : QObject(parent)
{
}

void DisplaySettingsController::bind(DisplayTabWidget* container, Settings* model)
{
    m_container = container;
    m_model = model;

    if (!m_container || !m_model) {
        return;
    }

    m_displayView = m_container->findChild<DisplayOptionsWidget*>();
    m_colorView = m_container->findChild<ColorOptionsWidget*>();

    // --- Bind DisplayOptionsWidget ---
    if (m_displayView) {
        // Initial setup helper to load values for a receiver
        auto loadReceiverValues = [this](int rx) {
            m_displayView->setFramesPerSecond(m_model->getFramesPerSecond(rx));
            m_displayView->setSpectrumAveragingCnt(m_model->getSpectrumAveragingCnt(rx));
            m_displayView->setWaterfallTime(5); // default placeholder as Settings has no getter for this stub
            m_displayView->setWaterfallOffsetLo(m_model->getWaterfallOffsetLo(rx));
            m_displayView->setWaterfallOffsetHi(m_model->getWaterfallOffsetHi(rx));
            m_displayView->setPanadapterMode(m_model->getPanadapterMode(rx));
            m_displayView->setWaterfallColorMode(m_model->getWaterfallColorMode(rx));
            m_displayView->setPanAveragingMode(m_model->getPanAveragingMode(rx));
            m_displayView->setPanDetectorMode(m_model->getPanDetectorMode(rx));
            m_displayView->setfftSize(m_model->getfftSize(rx));
            m_displayView->setfmsqLevel(80); // default placeholder as Settings has no getter for this stub
        };

        // Load initial receiver values (rx = 0 by default)
        loadReceiverValues(0);

        // Load global/shared values
        m_displayView->setSMeterHoldTime(m_model->getSMeterHoldTime());
        m_displayView->setCallsign(m_model->getCallsign());
        m_displayView->setWidebandAveragingCnt(m_model->getSpectrumAveragingCnt(-1));

        connect(m_model, &Settings::spectrumAveragingCntChanged, this, [this](int rx, int val) {
            if (rx == -1) {
                m_displayView->setWidebandAveragingCnt(val);
            } else if (m_displayView->currentReceiver() == rx) {
                m_displayView->setSpectrumAveragingCnt(val);
            }
        });

        // View -> Model
        connect(m_displayView, &DisplayOptionsWidget::receiverChanged, this, [loadReceiverValues](int rx) {
            loadReceiverValues(rx);
        });

        connect(m_displayView, &DisplayOptionsWidget::framesPerSecondRequested, this, [this](int rx, int val) {
            m_model->setFramesPerSecond(rx, val);
        });

        connect(m_displayView, &DisplayOptionsWidget::spectrumAveragingCntRequested, this, [this](int rx, int val) {
            m_model->setSpectrumAveragingCnt(rx, val);
        });

        connect(m_displayView, &DisplayOptionsWidget::waterfallTimeRequested, this, [this](int rx, int val) {
            m_model->setWaterfallTime(rx, val);
        });

        connect(m_displayView, &DisplayOptionsWidget::waterfallOffsetLoRequested, this, [this](int rx, int val) {
            m_model->setWaterfallOffesetLo(rx, val);
        });

        connect(m_displayView, &DisplayOptionsWidget::waterfallOffsetHiRequested, this, [this](int rx, int val) {
            m_model->setWaterfallOffesetHi(rx, val);
        });

        connect(m_displayView, &DisplayOptionsWidget::sMeterHoldTimeRequested, this, [this](int val) {
            m_model->setSMeterHoldTime(val);
        });

        connect(m_displayView, &DisplayOptionsWidget::callsignRequested, this, [this](const QString& val) {
            m_model->setCallsign(val);
        });

        connect(m_displayView, &DisplayOptionsWidget::graphicsStateRequested, this, [this](int rx, int panadapterMode, int waterColorMode) {
            m_model->setGraphicsState(rx, static_cast<PanGraphicsMode>(panadapterMode), static_cast<WaterfallColorMode>(waterColorMode));
        });

        connect(m_displayView, &DisplayOptionsWidget::panAveragingModeRequested, this, [this](int rx, int mode) {
            m_model->setPanAveragingMode(rx, static_cast<PanAveragingMode>(mode));
        });

        connect(m_displayView, &DisplayOptionsWidget::panDetectorModeRequested, this, [this](int rx, int mode) {
            m_model->setPanDetectorMode(rx, static_cast<PanDetectorMode>(mode));
        });

        connect(m_displayView, &DisplayOptionsWidget::fftSizeRequested, this, [this](int rx, int size) {
            m_model->setfftSize(rx, size);
        });

        connect(m_displayView, &DisplayOptionsWidget::fmsqLevelRequested, this, [this](int rx, int val) {
            m_model->setfmsqLevel(rx, val);
        });
    }

    // --- Bind ColorOptionsWidget ---
    if (m_colorView) {
        // Initial setup
        m_colorView->setPanadapterColors(m_model->getPanadapterColors());

        // View -> Model
        connect(m_colorView, &ColorOptionsWidget::panadapterColorsRequested, this, [this](const TPanadapterColors& colors) {
            m_model->setPanadapterColors(colors);
        });
    }

    // --- Bind 3D Panel when created ---
    connect(m_container, &DisplayTabWidget::panel3DCreated, this, [this](QGL3DPanel* panel) {
        connect(m_model, &Settings::ctrFrequencyChanged, panel, &QGL3DPanel::setCtrFrequency);
        connect(m_model, &Settings::vfoFrequencyChanged, panel, &QGL3DPanel::setVFOFrequency);
    });
}
