#include "Models/RadioModel.h"
#include "Models/BandPlanManager.h"
#include "Models/RadioTelemetry.h"
#include "Models/SliceModel.h"
/**
* @file  cusdr_oglReceiverPanel.cpp
* @brief Receiver panel class for cuSDR
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2011-03-14
*/

/*
 *   Copyright 2011 Hermann von Hasseln, DL3HVH
 *	 Copyright 2018 Simon Eatough ZL2BRG
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

// use: GRAPHICS_DEBUG

#include "cusdr_oglReceiverPanel.h"
#include "WaterfallRenderer.h"
#include "GridRenderer.h"
#include "TraceRenderer.h"
#include "HudRenderer.h"
#include "PanadapterInputController.h"
#include "cusdr_glShaders.h"
#include "cusdr_glDraw.h"
#include "Controllers/RadioPopupController.h"
#include "UI/FrequencyEntryDialog.h"

#include <QGuiApplication>
#include <QMatrix4x4>
#include <algorithm>
#include <cmath>
#include <cstring>

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif



QGLReceiverPanel::QGLReceiverPanel(SliceModel *model, QWidget *parent)
	: QOpenGLWidget(parent)
        , m_sliceModel(model)

	, set(Settings::instance())
	, m_serverMode(set->getCurrentServerMode())
	, m_hwInterface(set->getHWInterface())
	, m_dataEngineState(QSDR::DataEngineDown)
	, m_glTextColor(Qt::white)
	, m_panelDpr(devicePixelRatioF())
	, m_mousePos(QPoint(-1, -1))
	, m_mouseDownPos(QPoint(-1, -1))
	, m_panSpectrumBinsLength(0)
	, m_filterRight(0)
	, m_filterTop(0)
	, m_filterBottom(0)
	, m_waterfallRenderer(nullptr)
	, m_panadapterRenderer(nullptr)
	, m_overlayRenderer(nullptr)
	, m_gridRenderer(nullptr)
	, m_traceRenderer(nullptr)
	, m_hudRenderer(nullptr)
	, m_inputController(nullptr)
	, m_bigHeight(0)
	, m_receiver(model ? model->id() : 0)
	//, m_frequencyRxOnRx(0)
	, m_spectrumSize(set->getSpectrumSize())
	, m_sampleSize(0)
	, m_oldSampleSize(0)
	, m_specAveragingCnt(model ? model->spectrumAveragingCnt() : set->getSpectrumAveragingCnt(m_receiver))
	, m_currentReceiver(set->getCurrentReceiver())
	, m_waterfallAlpha(255)
	, m_freqRulerDisplayWidth(0)
	, m_displayTop(0)
	, m_panSpectrumMinimumHeight(0)
	, m_snapMouse(3)
	, m_sampleRate(set->getSampleRate())
	, m_mercuryAttenuator(0)
	, m_adcStatus(0)
	, m_fftMult(1)
	, m_smallSize(true)
	, m_spectrumVertexColorUpdate(false)
	, m_spectrumColorsChanged(true)
	, m_spectrumAveraging(model ? model->spectrumAveraging() : set->getSpectrumAveraging(m_receiver))
	//, m_spectrumAveragingOld(m_spectrumAveraging)
	, m_crossHair(set->getHairCrossStatus(m_receiver))
    , m_crossHairCursor(false)
	, m_panGrid(model ? model->panGrid() : set->getPanGridStatus(m_receiver))
	, m_peakHold(model ? model->peakHold() : set->getPeakHoldStatus(m_receiver))
	, m_peakHoldBufferResize(true)
	, m_filterChanged(true)
	, m_showFilterLeftBoundary(false)
	, m_showFilterRightBoundary(false)
	, m_highlightFilter(false)
	, m_dragMouse(false)
	, m_dragDBmScale(false)
	, m_dragFreqScale(false)
	, m_dragFreqScaleZoom(false)
	, m_panLocked(set->getPanLockedStatus(m_receiver))
	, m_clickVFO(set->getClickVFOStatus(m_receiver))
	, m_freqScaleZoomFactor(1.0f)
	, m_scaleMult(1.0f)
	, m_filterLowerFrequency(-3050.0)
	, m_filterUpperFrequency(-150.0)
	, m_freqRulerPosition(set->getFreqRulerPosition(model ? model->id() : 0))
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	// Full repaint each frame — PartialUpdate corrupts pan/waterfall on desktop GLX.
	setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
	setAutoFillBackground(false);
	setAttribute(Qt::WA_OpaquePaintEvent);
	setAttribute(Qt::WA_NoSystemBackground);
	disableVSyncOnNativeWayland(this);

	setMouseTracking(true);
	setFocusPolicy(Qt::StrongFocus);

	//GRAPHICS_DEBUG << "set spectrum buffer size to: " << m_spectrumSize;

	m_bigHeight = 600;
	m_bigWidth = 1000;

	setupDisplayRegions(size());
	m_oldWidth = size().width();
	if (m_sliceModel) {
		m_centerFrequency = m_sliceModel->centerFrequency();
		m_vfoFrequency = m_sliceModel->frequency();
		m_panMode = m_sliceModel->panMode();
		m_waterfallMode = m_sliceModel->waterfallMode();
		m_waterfallOffsetLo = m_sliceModel->waterfallOffsetLo();
		m_waterfallOffsetHi = m_sliceModel->waterfallOffsetHi();
		m_filterLowerFrequency = m_sliceModel->filterLow();
		m_filterUpperFrequency = m_sliceModel->filterHigh();
		m_agcMode = m_sliceModel->agcMode();
		m_agcFixedGain = m_sliceModel->agcFixedGain();
		m_dBmPanMin = m_sliceModel->dBmPanScaleMin();
		m_dBmPanMax = m_sliceModel->dBmPanScaleMax();
	} else {
		m_centerFrequency = set->getCtrFrequency(m_receiver);
		m_vfoFrequency = set->getVfoFrequency(m_receiver);
		m_panMode = set->getPanadapterMode(m_receiver);
		m_waterfallMode = set->getWaterfallColorMode(m_receiver);
		m_waterfallOffsetLo = set->getWaterfallOffsetLo(m_receiver);
		m_waterfallOffsetHi = set->getWaterfallOffsetHi(m_receiver);
		m_filterLowerFrequency = set->getFilterLo(m_receiver);
		m_filterUpperFrequency = set->getFilterHi(m_receiver);
		m_agcMode = set->getAGCMode(m_receiver);
		m_agcFixedGain = set->getAGCFixedGain_dB(m_receiver);
		const HamBand band = set->getCurrentHamBand(m_receiver);
		m_dBmPanMin = set->getdBmPanScaleMin(m_receiver, band);
		m_dBmPanMax = set->getdBmPanScaleMax(m_receiver, band);
	}
	m_mouseWheelFreqStep = set->getMouseWheelFreqStep(m_receiver);
	{
		const HamBand band = set->getCurrentHamBand(m_receiver);
		const QList<int> attns = set->getMercuryAttenuators(m_receiver);
		const int bandIndex = int(band);
		m_mercuryAttenuator = (bandIndex >= 0 && bandIndex < attns.size()) ? attns.at(bandIndex) : 0;
	}
	m_adcMode = set->getADCMode(m_receiver);
	m_dspModeString = set->getDSPModeString(m_sliceModel ? m_sliceModel->dspMode() : set->getDSPMode(m_receiver));
	m_agcHangEnabled = set->getHangEnabled(m_receiver);
	m_showAGCLines = set->getAgcLines(m_receiver);

	if (m_vfoFrequency > m_centerFrequency + m_sampleRate/2)
		m_vfoFrequency = m_centerFrequency + m_sampleRate/2;
	else if (m_vfoFrequency < m_centerFrequency - m_sampleRate/2)
		m_vfoFrequency = m_centerFrequency - m_sampleRate/2;

	m_deltaFrequency = m_centerFrequency - m_vfoFrequency;
	m_deltaF = (qreal)(1.0*m_deltaFrequency/m_sampleRate);
	
	m_dBmScalePanadapterRenew = true;
	m_dBmScalePanadapterUpdate = true;
	m_freqScalePanadapterRenew = true;
	m_freqScalePanadapterUpdate = true;
	m_panGridRenew = true;
	m_panGridUpdate = true;
	if (m_waterfallRenderer) m_waterfallRenderer->reset();
	m_secScaleWaterfallUpdate = true;
	m_secScaleWaterfallRenew = true;
	m_waterfallDisplayUpdate = true;
	m_spectrumDirty = false;
	m_filterWidth = qAbs((int)(m_filterUpperFrequency - m_filterLowerFrequency));
	m_adcModeString = set->getADCModeString(m_receiver);
	m_agcModeString = set->getAGCModeString(m_receiver);

	m_secWaterfallMin = 0.0;
	m_secWaterfallMax = 0.0;

	radioPopup = new RadioPopupWidget(m_sliceModel, this);
	radioPopupController = new RadioPopupController(this);
	radioPopupController->bind(radioPopup, m_sliceModel, set);

	fonts = new CFonts(this);
	m_fonts = fonts->getFonts();

	m_fonts.smallFont.setBold(true);
	m_fonts.bigFont1.setBold(false);
	m_fonts.bigFont2.setBold(false);

	m_oglTextTiny = new OGLText(m_fonts.tinyFont, m_panelDpr);
    m_oglTextSmall = new OGLText(m_fonts.smallFont, m_panelDpr);
	m_oglTextNormal = new OGLText(m_fonts.normalFont, m_panelDpr);
	m_oglTextFreq1 = new OGLText(m_fonts.freqFont1, m_panelDpr);
	m_oglTextFreq2 = new OGLText(m_fonts.freqFont2, m_panelDpr);
	m_oglTextBig1 = new OGLText(m_fonts.bigFont1, m_panelDpr);
	m_oglTextBig2 = new OGLText(m_fonts.bigFont2, m_panelDpr);
	m_oglTextHuge = new OGLText(m_fonts.hugeFont, m_panelDpr);


    m_shaderProgram = nullptr;
    m_textureProgram = nullptr;
    m_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);

	setupConnections();

	m_spectrumBinWorker = new SpectrumBinWorker(this);
	CHECKED_CONNECT(m_spectrumBinWorker, &SpectrumBinWorker::binsReady,
	                this, &QGLReceiverPanel::onSpectrumBinsReady);
	m_spectrumBinWorker->start(QThread::LowPriority);

	m_displayTime.start();
	m_resizeTime.start();
	//freqChangeTimer.start();
	
	m_fps = set->getFramesPerSecond(m_receiver);
	m_secWaterfallMin = -(1.0/m_fps) * m_secScaleWaterfallRect.height();

	
	m_dBmPanLogGain = 0;//49;//69 // allow user to calibrate this value

	m_cameraDistance = 0;
	m_cameraAngle = QPoint(0, 0);

	m_mousePos = QPoint(-100, -100);
	
	RadioModel* radioModel = qobject_cast<RadioModel*>(m_sliceModel ? m_sliceModel->parent() : nullptr);
	const TPanadapterColors colors = radioModel ? radioModel->panadapterColors() : set->getPanadapterColors();

	m_gridColor = colors.gridLineColor;
	m_darkColor = QColor(150, 150, 150, 100);

	m_redGrid   = (GLfloat)(m_gridColor.red()/256.0);
	m_greenGrid = (GLfloat)(m_gridColor.green()/256.0);
	m_blueGrid  = (GLfloat)(m_gridColor.blue()/256.0);

	m_bkgRed   = (GLfloat)(colors.panBackgroundColor.red() / 256.0);
	m_bkgGreen = (GLfloat)(colors.panBackgroundColor.green() / 256.0);
	m_bkgBlue  = (GLfloat)(colors.panBackgroundColor.blue() / 256.0);

	m_red	= (GLfloat)(colors.panLineColor.red() / 256.0);
	m_green = (GLfloat)(colors.panLineColor.green() / 256.0);
	m_blue	= (GLfloat)(colors.panLineColor.blue() / 256.0);

	m_redF	 = (GLfloat)(colors.panLineFilledColor.red() / 256.0);
	m_greenF = (GLfloat)(colors.panLineFilledColor.green() / 256.0);
	m_blueF  = (GLfloat)(colors.panLineFilledColor.blue() / 256.0);

	m_redST	  = (GLfloat)(colors.panSolidTopColor.red() / 256.0);
	m_greenST = (GLfloat)(colors.panSolidTopColor.green() / 256.0);
	m_blueST  = (GLfloat)(colors.panSolidTopColor.blue() / 256.0);

	m_redSB   = (GLfloat)(colors.panSolidBottomColor.red() / 256.0);
	m_greenSB = (GLfloat)(colors.panSolidBottomColor.green() / 256.0);
	m_blueSB  = (GLfloat)(colors.panSolidBottomColor.blue() / 256.0);

	m_waterfallLoColor = QColor(0, 0, 0, m_waterfallAlpha);
	m_waterfallHiColor = QColor(192, 124, 255, m_waterfallAlpha);
	m_waterfallMidColor = colors.waterfallColor.toRgb();

	m_haircrossOffsetRight = 30;
	m_haircrossOffsetLeft = 116;
	m_haircrossMaxRight = 110;
	m_haircrossMinTop = 40;

	if (m_specAveragingCnt > 0)
		m_scale = 1.0f / m_specAveragingCnt;
	else
		m_scale = 1.0f;

	m_gridRenderer = new GridRenderer(this);
	m_traceRenderer = new TraceRenderer(this);
	m_hudRenderer = new HudRenderer(this);
	m_inputController = new PanadapterInputController(this);

	m_sMeterHoldTimer.start();
	m_sMeterDisplayTimer.start();
}

QGLReceiverPanel::~QGLReceiverPanel() {

    qDebug() << "rx panel destructor" << m_receiver;
    disconnect(set, 0, this, 0);

    if (m_spectrumBinWorker) {
        disconnect(m_spectrumBinWorker, nullptr, this, nullptr);
        m_spectrumBinWorker->stop();
        m_spectrumBinWorker->wait(1000);
        m_spectrumBinWorker = nullptr;
    }

    if (m_shaderProgram) {
        delete m_shaderProgram;
        m_shaderProgram = nullptr;
    }

    if (m_textureProgram) {
        delete m_textureProgram;
        m_textureProgram = nullptr;
    }

    if (m_vao.isCreated()) {
        m_vao.destroy();
    }

    if (m_vbo.isCreated()) {
        m_vbo.destroy();
    }
	
	if (m_gridRenderer) {
		delete m_gridRenderer;
		m_gridRenderer = nullptr;
	}
	if (m_traceRenderer) {
		delete m_traceRenderer;
		m_traceRenderer = nullptr;
	}
	if (m_hudRenderer) {
		delete m_hudRenderer;
		m_hudRenderer = nullptr;
	}
	if (m_inputController) {
		delete m_inputController;
		m_inputController = nullptr;
	}

	if (m_waterfallRenderer) {
	    delete m_waterfallRenderer;
	    m_waterfallRenderer = nullptr;
	}

    if (m_panadapterRenderer) {
        delete m_panadapterRenderer;
        m_panadapterRenderer = nullptr;
    }

    if (m_overlayRenderer) {
        delete m_overlayRenderer;
        m_overlayRenderer = nullptr;
    }

    while (!specAv_queue.isEmpty())
        specAv_queue.dequeue();

    if (radioPopup) {
        radioPopup->close();
        delete radioPopup;
        radioPopup = nullptr;
    }

    delete fonts;
    delete m_oglTextTiny;
    delete m_oglTextSmall;
    delete m_oglTextNormal;
    delete m_oglTextFreq1;
    delete m_oglTextFreq2;
    delete m_oglTextBig1;
    delete m_oglTextBig2;
    delete m_oglTextHuge;

}

QSize QGLReceiverPanel::minimumSizeHint() const {
	
	if (m_receiver == 0)
		return QSize(width(), 250);
	else
		return QSize(250, 120);
	//return QSize(width(), height());
}

QSize QGLReceiverPanel::sizeHint() const {
	
	return QSize(width(), height());
}

void QGLReceiverPanel::setupConnections() {

    connect(set, &Settings::systemStateChanged,          this, &QGLReceiverPanel::systemStateChanged);
    connect(set, &Settings::freqRulerPositionChanged,    this, &QGLReceiverPanel::freqRulerPositionChanged);
    connect(set, &Settings::hamBandChanged,              this, &QGLReceiverPanel::setHamBand);
    connect(set, &Settings::currentReceiverChanged,      this, &QGLReceiverPanel::setCurrentReceiver);
    connect(set, &Settings::sampleRateChanged,           this, &QGLReceiverPanel::sampleRateChanged);
    CHECKED_CONNECT(set, &Settings::spectrumBufferChanged, this, &QGLReceiverPanel::setSpectrumBuffer);
    if (RadioModel* radioModel = qobject_cast<RadioModel*>(m_sliceModel ? m_sliceModel->parent() : nullptr)) {
        if (RadioTelemetry* tel = radioModel->telemetry()) {
            connect(tel, &RadioTelemetry::adcOverflowChanged, this, &QGLReceiverPanel::setADCStatus);
        }
        if (BandPlanManager* plan = radioModel->bandPlan()) {
            connect(plan, &BandPlanManager::planChanged, this, qOverload<>(&QGLReceiverPanel::update));
        }
        connect(radioModel, &RadioModel::colorsChanged, this, [this]() {
            setPanadapterColors();
        });
        connect(radioModel, &RadioModel::sampleRateChanged, this, &QGLReceiverPanel::sampleRateChanged);
    }
    connect(set, &Settings::panLockedStatusChanged,      this, &QGLReceiverPanel::setPanLockedStatus);
    connect(set, &Settings::clickVFOStatusChanged,       this, &QGLReceiverPanel::setClickVFOStatus);
    connect(set, &Settings::hairCrossStatusChanged,      this, &QGLReceiverPanel::setHairCrossStatus);
    connect(set, &Settings::panadapterColorChanged,      this, &QGLReceiverPanel::setPanadapterColors);
    connect(set, &Settings::mercuryAttenuatorChanged,    this, &QGLReceiverPanel::setMercuryAttenuator);
    connect(set, &Settings::framesPerSecondChanged,      this, &QGLReceiverPanel::setFramesPerSecond);
    connect(set, &Settings::agcLineLevelsChanged,        this, &QGLReceiverPanel::setAGCLineLevels);
    connect(set, &Settings::showAGCLinesStatusChanged,   this, &QGLReceiverPanel::setAGCLinesStatus);
    connect(set, &Settings::adcModeChanged,              this, &QGLReceiverPanel::setADCMode);
    connect(set, &Settings::mouseWheelFreqStepChanged,   this, &QGLReceiverPanel::setMouseWheelFreqStep);

    connect(m_sliceModel, &SliceModel::frequencyChanged, this, [this](qint64 freq){ this->setVFOFrequency(0, m_sliceModel->id(), freq); });
    connect(m_sliceModel, &SliceModel::centerFrequencyChanged, this, [this](qint64 freq){ setCtrFrequency(0, m_sliceModel->id(), freq); });
    connect(m_sliceModel, &SliceModel::dspModeChanged, this, [this](DSPMode mode){ setDSPMode(m_sliceModel->id(), mode); });
    connect(m_sliceModel, &SliceModel::filterChanged, this, [this](){ setFilterFrequencies(m_sliceModel->id(), (qreal)m_sliceModel->filterLow(), (qreal)m_sliceModel->filterHigh()); });
    connect(m_sliceModel, &SliceModel::spectrumAveragingChanged, [this](bool enabled){ setSpectrumAveraging(m_sliceModel->id(), enabled); });
    connect(m_sliceModel, &SliceModel::spectrumAveragingCntChanged, [this](int count){ setSpectrumAveragingCnt(count); });
    connect(m_sliceModel, &SliceModel::panModeChanged, [this](PanGraphicsMode mode){ graphicModeChanged(m_sliceModel->id(), mode, m_sliceModel->waterfallMode()); });
    connect(m_sliceModel, &SliceModel::waterfallModeChanged, [this](WaterfallColorMode mode){ graphicModeChanged(m_sliceModel->id(), m_sliceModel->panMode(), mode); });
    connect(m_sliceModel, &SliceModel::panGridChanged, this, [this](bool enabled){ setPanGridStatus(enabled, m_sliceModel->id()); });
    connect(m_sliceModel, &SliceModel::peakHoldChanged, this, [this](bool enabled){ setPeakHoldStatus(enabled, m_sliceModel->id()); });
    connect(m_sliceModel, &SliceModel::waterfallOffsetChanged, [this](){ setWaterfallOffesetLo(m_sliceModel->id(), m_sliceModel->waterfallOffsetLo()); setWaterfallOffesetHi(m_sliceModel->id(), m_sliceModel->waterfallOffsetHi()); });
    connect(m_sliceModel, &SliceModel::agcModeChanged, this,
            [this](AGCMode mode) {
                const bool hang = (mode != (AGCMode)agcOFF && mode != (AGCMode)agcMED && mode != (AGCMode)agcFAST);
                setAGCMode(m_sliceModel->id(), mode, hang);
            });

    connect(m_sliceModel, &SliceModel::panScaleChanged, this, [this]() {
        m_dBmPanMin = m_sliceModel->dBmPanScaleMin();
        m_dBmPanMax = m_sliceModel->dBmPanScaleMax();
        m_dBmScalePanadapterUpdate = true;
        m_peakHoldBufferResize = true;
        update();
    });

    connect(m_sliceModel, &SliceModel::cwDecodedTextChanged, this, qOverload<>(&QGLReceiverPanel::update));
    connect(m_sliceModel, &SliceModel::cwToneActiveChanged, this, qOverload<>(&QGLReceiverPanel::update));
    connect(m_sliceModel, &SliceModel::sMeterValueChanged, this, &QGLReceiverPanel::updateSMeterValue);
    connect(m_sliceModel, &SliceModel::sMeterPeakValueChanged, this, &QGLReceiverPanel::updateSMeterPeakValue);
    connect(m_sliceModel, &SliceModel::cwDecodeEnabledChanged, this, qOverload<>(&QGLReceiverPanel::update));
    connect(m_sliceModel, &SliceModel::cwTrackedPitchChanged, this, qOverload<>(&QGLReceiverPanel::update));
    connect(m_sliceModel, &SliceModel::activeVfoChanged, this, [this](SliceModel::ActiveVfo){ update(); });

    if (set) {
        connect(set, &Settings::showPanadapterSMeterChanged, this, qOverload<>(&QGLReceiverPanel::update));
        connect(set, &Settings::showPanadapterFreqChanged, this, qOverload<>(&QGLReceiverPanel::update));
        connect(set, &Settings::radioStateChanged, this, qOverload<>(&QGLReceiverPanel::update));
    }

	connect(radioPopup, &RadioPopupWidget::vfoToMidBtnEvent, this, &QGLReceiverPanel::setVfoToMidFrequency);
	connect(radioPopup, &RadioPopupWidget::midToVfoBtnEvent, this, &QGLReceiverPanel::setMidToVfoFrequency);
}

void QGLReceiverPanel::initializeGL() {

	if (!isValid()) return;
     initializeOpenGLFunctions();

    // --- Modern OpenGL Setup ---
    m_shaderProgram = new QOpenGLShaderProgram(this);

    if (!m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, GlShaders::coloredVertexSource())) {
        qCritical() << "Vertex shader compilation failed:" << m_shaderProgram->log();
    }

    if (!m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, GlShaders::coloredFragmentSource())) {
        qCritical() << "Fragment shader compilation failed:" << m_shaderProgram->log();
    }

    m_shaderProgram->bindAttributeLocation("position", 0);
    m_shaderProgram->bindAttributeLocation("color", 1);

    if (!m_shaderProgram->link()) {
        qCritical() << "Shader program linking failed:" << m_shaderProgram->log();
    }

    m_vao.create();
    m_vao.bind();

    m_vbo.create();
    m_vbo.bind();
    m_vbo.setUsagePattern(QOpenGLBuffer::StreamDraw); // Frequently updated

    m_shaderProgram->enableAttributeArray(0);
    m_shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 7);

    m_shaderProgram->enableAttributeArray(1);
    m_shaderProgram->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 4, sizeof(float) * 7);

    m_vao.release();
    m_vbo.release();

    m_textureProgram = new QOpenGLShaderProgram(this);
    m_textureProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, GlShaders::texturedQuadVertexSource());
    m_textureProgram->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                              GlShaders::texturedFragmentSource("tex"));
    m_textureProgram->bindAttributeLocation("position", 0);
    m_textureProgram->bindAttributeLocation("texCoord", 1);
    if (!m_textureProgram->link()) {
        qCritical() << "Receiver panel texture shader link failed:" << m_textureProgram->log();
    }

	//*****************************************************************
	// default initialization

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // 4-byte pixel alignment
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

	glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

    m_waterfallRenderer = new WaterfallRenderer();
    m_waterfallRenderer->initialize();

    m_panadapterRenderer = new PanadapterRenderer();
    if (!m_panadapterRenderer->initialize(context(), m_shaderProgram)) {
        qWarning() << "PanadapterRenderer init failed for rx" << m_receiver;
    }

    m_overlayRenderer = new OverlayRenderer();
    m_overlayRenderer->initialize(m_shaderProgram);

    if (!m_gridRenderer)
        m_gridRenderer = new GridRenderer(this);
    if (!m_traceRenderer)
        m_traceRenderer = new TraceRenderer(this);
    if (!m_hudRenderer)
        m_hudRenderer = new HudRenderer(this);
}

QMatrix4x4 QGLReceiverPanel::panelProjection() const
{
    QMatrix4x4 projection;
    projection.ortho(0, width(), height(), 0, -10, 10);
    return projection;
}

qreal QGLReceiverPanel::displayedFrequencySpanHz() const
{
    if (m_spectrumSize <= 0 || m_sampleRate <= 0)
        return 0;
    const int visibleBins = qMax(1, int(std::floor(m_fftMult * m_spectrumSize * m_freqScaleZoomFactor)));
    return qreal(m_sampleRate) * qreal(visibleBins) / qreal(m_spectrumSize);
}

float QGLReceiverPanel::displayedZoomFactor() const
{
    if (m_sampleRate <= 0)
        return float(m_freqScaleZoomFactor);
    return float(displayedFrequencySpanHz() / qreal(m_sampleRate));
}

void QGLReceiverPanel::ensurePanelViewport()
{
    QOpenGLFramebufferObject::bindDefault();
    const qreal ratio = devicePixelRatioF();
    glViewport(0, 0, GLsizei(qRound(width() * ratio)), GLsizei(qRound(height() * ratio)));
}

void QGLReceiverPanel::syncTextDevicePixelRatio()
{
    const qreal ratio = devicePixelRatioF();
    if (qFuzzyCompare(ratio, m_panelDpr))
        return;
    m_panelDpr = ratio;
    m_oglTextTiny->setDevicePixelRatio(m_panelDpr);
    m_oglTextSmall->setDevicePixelRatio(m_panelDpr);
    m_oglTextNormal->setDevicePixelRatio(m_panelDpr);
    m_oglTextFreq1->setDevicePixelRatio(m_panelDpr);
    m_oglTextFreq2->setDevicePixelRatio(m_panelDpr);
    m_oglTextBig1->setDevicePixelRatio(m_panelDpr);
    m_oglTextBig2->setDevicePixelRatio(m_panelDpr);
    m_oglTextHuge->setDevicePixelRatio(m_panelDpr);
}

void QGLReceiverPanel::drawPanelRect(const QRect &rect, const QColor &color, float z)
{
    if (rect.isEmpty())
        return;
    m_vao.bind();
    if (m_shaderProgram && m_shaderProgram->isLinked())
        GlDraw::drawSolidRect(this, m_shaderProgram, m_vbo, panelProjection(), rect, color, z);
}

void QGLReceiverPanel::renderPanelText(OGLText *text, float x, float y, const QString &str)
{
    renderPanelText(text, x, y, 0.0f, str);
}

void QGLReceiverPanel::renderPanelText(OGLText *text, float x, float y, float z, const QString &str)
{
    if (!text)
        return;
    if (GlShaders::isOpenGLES())
        text->renderText(panelProjection(), x, y, z, str, m_glTextColor);
    else
        text->renderText(x, y, z, str, m_glTextColor);
}

void QGLReceiverPanel::drawCachedTexture(const QRect &rect, GLuint texId, float z)
{
    if (rect.isEmpty() || !texId)
        return;
    m_vao.bind();
    if (m_textureProgram && m_textureProgram->isLinked())
        GlDraw::renderTexturedQuad(this, m_textureProgram, m_vbo, panelProjection(), rect, texId, z);
}

void QGLReceiverPanel::paintGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    syncTextDevicePixelRatio();
    ensurePanelViewport();

	switch (m_serverMode) {

		case QSDR::NoServerMode:

			drawPanelRect(QRect(0, 0, width(), height()), QColor(0, 0, 0), -5.0f);
			break;

		case QSDR::SDRMode:

       //     if (freqChangeTimer.elapsed() > 50)  m_spectrumAveraging = m_spectrumAveragingOld;

			if (m_resizeTime.elapsed() > 200 || m_dataEngineState == QSDR::DataEngineDown)
				paintReceiverDisplay();
			
			break;
	}
}
 
void QGLReceiverPanel::paintReceiverDisplay() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (m_filterChanged) {
		m_filterLo = m_filterLowerFrequency / m_sampleRate;
		m_filterHi = m_filterUpperFrequency / m_sampleRate;
		m_filterWidth = qAbs((int)(m_filterUpperFrequency - m_filterLowerFrequency));

		if (m_filterWidth < 1000) {
			QString str = "%1";
			m_filterWidthString = str.arg(m_filterWidth);
		}
		else {
			QString str = "%1k%2";
			m_filterWidthString = str.arg((int)(m_filterWidth/1000)).arg((int)((m_filterWidth%1000)/100));
		}
		m_filterChanged = false;
	}

    // Spectrum binning runs on SpectrumBinWorker; paint only draws the latest snapshot.
    scheduleSpectrumBinning();

    // 4 Discrete Modular Render Passes:
    renderTracePass();
    renderGridPass();
    renderFilterBandwidthPass();
    renderMarkerHudPass();
}

void QGLReceiverPanel::renderTracePass() {
    drawPanadapter();
    drawBandPlanStrip();

    if (m_waterfallRect.height() > 10) {
        ensurePanelViewport();
        drawWaterfall();
    }
    if (m_waterfallDisplayUpdate)
        m_waterfallDisplayUpdate = false;
}

void QGLReceiverPanel::renderGridPass() {
    if (m_freqScalePanRect.isValid())
        updateFrequencyRuler();
    if (m_dBmScalePanRect.isValid())
        updateDBmRuler();

    glDisable(GL_DEPTH_TEST);
    drawPanHorizontalScale();
    drawPanVerticalScale();
    drawPanadapterGrid();
    if (m_waterfallRect.height() > 10) {
        glEnable(GL_DEPTH_TEST);
        drawWaterfallVerticalScale();
    }
    glEnable(GL_DEPTH_TEST);
}

void QGLReceiverPanel::renderFilterBandwidthPass() {
    glDisable(GL_DEPTH_TEST);
    drawCenterLine();
    drawPanFilter();
    glEnable(GL_DEPTH_TEST);

    if (m_dataEngineState == QSDR::DataEngineUp && m_showAGCLines && (m_receiver == m_currentReceiver)) {
        ensurePanelViewport();
        drawAGCControl();
    }
}

void QGLReceiverPanel::renderMarkerHudPass() {
    if (m_panRect.width() > 300 && m_panRect.height() > 80) {
        ensurePanelViewport();
        drawVFOControl();
        drawReceiverInfo();
        drawCwDecoderHUD();
        drawPanadapterSMeter();
        drawPanadapterFreq();
    }

    if (m_crossHair) {
        if (m_mouseRegion != freqScalePanadapterRegion && 
            m_mouseRegion != dBmScalePanadapterRegion && 
            m_mouseRegion != filterRegion &&
            m_mouseRegion != filterRegionLow &&
            m_mouseRegion != filterRegionHigh &&
            m_mouseRegion != agcThresholdLine &&
            m_mouseRegion != agcHangLine &&
            m_mouseRegion != agcFixedGainLine &&
            m_crossHairCursor) {
            drawCrossHair();
        }
    }
}

void QGLReceiverPanel::paint3DPanadapterMode() {
}

void QGLReceiverPanel::drawBandPlanStrip()
{
	if (m_traceRenderer)
		m_traceRenderer->drawBandPlanStrip();
}

void QGLReceiverPanel::drawPanadapter() {
    if (m_traceRenderer)
        m_traceRenderer->drawPanadapter();
}

void QGLReceiverPanel::drawPanVerticalScale() {
    if (m_gridRenderer)
        m_gridRenderer->drawPanVerticalScale();
}




void QGLReceiverPanel::drawPanHorizontalScale() {
    if (m_gridRenderer)
        m_gridRenderer->drawPanHorizontalScale();
}

void QGLReceiverPanel::updateFrequencyRuler()
{
    if (m_gridRenderer)
        m_gridRenderer->updateFrequencyRuler();
}

void QGLReceiverPanel::updateDBmRuler()
{
    if (m_gridRenderer)
        m_gridRenderer->updateDBmRuler();
}

void QGLReceiverPanel::drawPanadapterGrid() {
    if (m_gridRenderer)
        m_gridRenderer->drawPanadapterGrid();
}

void QGLReceiverPanel::drawCenterLine() {
    if (m_overlayRenderer) {
        QMatrix4x4 projection;
        projection.ortho(0, size().width(), size().height(), 0, -10, 10);
        m_overlayRenderer->drawCenterLine(projection, m_panRect, m_freqScalePanRect, m_waterfallRect, m_displayCenterlineHeight, (float)m_deltaF, displayedZoomFactor(),
                                          set->getPanadapterColors().distanceLineColor,
                                          set->getPanadapterColors().panCenterLineColor,
                                          m_dragMouse, m_panLocked);
    }
}

void QGLReceiverPanel::drawPanFilter() {
    if (m_overlayRenderer) {
        QMatrix4x4 projection;
        projection.ortho(0, size().width(), size().height(), 0, -10, 10);
        m_overlayRenderer->drawFilter(projection, m_panRect, m_waterfallRect, m_filterLo, m_filterHi, (float)m_deltaF, displayedZoomFactor(),
                                      set->getPanadapterColors().panFilterColor,
                                      m_highlightFilter, m_dragMouse, m_showFilterLeftBoundary, m_showFilterRightBoundary,
                                      m_filterLeft, m_filterRight, m_filterTop, m_filterBottom);
        // Update m_filterRect so getRegion() mouse hit-testing uses current pixel positions
        m_filterRect = QRect(m_filterLeft, m_filterTop, m_filterRight - m_filterLeft, m_filterBottom - m_filterTop);
    }

    if (m_hudRenderer)
        m_hudRenderer->drawFilterLabels();
}

void QGLReceiverPanel::drawWaterfall() {
    if (m_traceRenderer)
        m_traceRenderer->drawWaterfall();
}

void QGLReceiverPanel::drawWaterfallVerticalScale() {
    if (m_gridRenderer)
        m_gridRenderer->drawWaterfallVerticalScale();
}

void QGLReceiverPanel::drawCrossHair() {
    if (m_hudRenderer)
        m_hudRenderer->drawCrossHair();
}

void QGLReceiverPanel::drawVFOControl() {
    if (m_hudRenderer)
        m_hudRenderer->drawVFOControl();
}

void QGLReceiverPanel::drawReceiverInfo() {
    if (m_hudRenderer)
        m_hudRenderer->drawReceiverInfo();
}

void QGLReceiverPanel::drawCwDecoderHUD() {
    if (m_hudRenderer)
        m_hudRenderer->drawCwDecoderHUD();
}

void QGLReceiverPanel::drawPanadapterSMeter() {
    if (m_hudRenderer)
        m_hudRenderer->drawPanadapterSMeter();
}

void QGLReceiverPanel::drawPanadapterFreq() {
    if (m_hudRenderer)
        m_hudRenderer->drawPanadapterFreq();
}

void QGLReceiverPanel::updateSMeterValue(double value) {
    const float offset = (set->getHWInterface() == QSDR::SoapySDR) ? 90.0f : 140.0f;
    const float tmp = static_cast<float>(value) + offset;

    // Fast-attack (0.80), smooth-decay (0.15) analog meter ballistics
    if (tmp > m_sMeterAvgVal) {
        m_sMeterAvgVal = tmp * 0.80f + m_sMeterAvgVal * 0.20f;
    } else {
        m_sMeterAvgVal = tmp * 0.15f + m_sMeterAvgVal * 0.85f;
    }

    if (m_sMeterDisplayTimer.elapsed() > 60) {
        m_sMeterOrgValue = static_cast<float>(value);
        m_sMeterDisplayTimer.restart();
    }
    update();
}

void QGLReceiverPanel::updateSMeterPeakValue(double value) {
    const float offset = (set->getHWInterface() == QSDR::SoapySDR) ? 90.0f : 140.0f;
    const float tmp = static_cast<float>(value) + offset;
    m_sMeterPeakVal = tmp;

    if (tmp > m_sMeterHoldMax) {
        m_sMeterHoldMax = tmp;
        m_sMeterHoldTimer.restart();
    } else {
        const int holdTime = m_sliceModel ? m_sliceModel->sMeterHoldTime() : set->getSMeterHoldTime();
        if (m_sMeterHoldTimer.elapsed() > holdTime) {
            m_sMeterHoldMax = m_sMeterHoldMax * 0.90f + m_sMeterAvgVal * 0.10f;
        }
    }
    update();
}

void QGLReceiverPanel::drawAGCControl() {
    if (m_overlayRenderer) {
        QMatrix4x4 projection;
        projection.ortho(0, size().width(), size().height(), 0, -10, 10);
        m_overlayRenderer->drawAGCControl(projection, m_panRect, m_dBmScalePanRect, m_agcMode, m_agcHangEnabled, m_agcThresholdOld, m_agcHangLevelOld, m_agcFixedGain, m_dBmPanMax, m_dBmPanMin, (float)devicePixelRatio(), size().height(), m_agcThresholdPixel, m_agcHangLevelPixel, m_agcFixedGainLevelPixel);
        if (m_hudRenderer)
            m_hudRenderer->drawAGCLabels();
    }
}
 





 


//********************************************************************

void QGLReceiverPanel::getRegion(QPoint p) {
	if (m_inputController)
		m_inputController->getRegion(p);
}

void QGLReceiverPanel::resizeGL(int iWidth, int iHeight) {

    if (m_gridRenderer)
        m_gridRenderer->invalidateScaleFBOs();

    // Update all sizes and rectangles
    setupDisplayRegions(QSize(iWidth, iHeight));
    
    syncTextDevicePixelRatio();
    ensurePanelViewport();
    
    // Mark all for renewal
    m_panGridRenew = true;
    m_panFrequencyScale = true;
    m_panGridRenew = true;
    m_spectrumVertexColorUpdate = true;
    if (m_waterfallRenderer) m_waterfallRenderer->reset();
    m_peakHoldBufferResize = true;

    update();

}

void QGLReceiverPanel::setupDisplayRegions(QSize size) {

	m_displayTop = 0;
	//m_displayTop = m_fonts.fontHeightSmallFont + 2;
	int freqScaleRectHeight = 20;
	//int dBmScaleWidth = 45;
	
	m_freqScalePanRect = QRect(
			0, 
			m_displayTop + qRound((size.height() - m_displayTop - freqScaleRectHeight) * m_freqRulerPosition), 
			size.width(), 
			freqScaleRectHeight);

	m_panRect = QRect(
			0,
			m_displayTop, 
			size.width(),
			m_freqScalePanRect.top() - m_displayTop);

	// Frequency markers (DX spots) are drawn above the ruler inside m_panRect.
	m_bandPlanRect = QRect();


	if (m_panRect.height() != m_oldPanRectHeight) {

		m_dBmScalePanadapterRenew = true;
		m_panGridRenew = true;
		m_secScaleWaterfallRenew = true;
	}

	m_oldPanRectHeight = m_panRect.height();
	m_panRectWidth = (GLint)m_panRect.width();
	m_displayCenterlineHeight = m_panRect.top() + (m_panRect.height() - 3);
	
	m_waterfallRect = QRect(
			m_freqScalePanRect.left(), 
			m_freqScalePanRect.top() + m_freqScalePanRect.height(),
			m_freqScalePanRect.width(),
			size.height() - m_freqScalePanRect.top() - m_freqScalePanRect.height());
			
	if (m_waterfallRenderer) m_waterfallRenderer->reset();

	if ((m_panRect.height() + m_waterfallRect.height()) > m_bigHeight && m_panRect.width() > m_bigWidth)
		m_smallSize = false;
	else
		m_smallSize = true;
	
	m_dBmScalePanRect = QRect(
						m_panRect.left(), 
						m_panRect.top(), 
						45, 
						m_panRect.height());

	m_secScaleWaterfallRect = QRect(
								m_waterfallRect.left(),
								m_freqScalePanRect.top() + m_freqScalePanRect.height(),
								45,
								m_waterfallRect.height());

	m_secWaterfallMin = -(1.0/m_fps) * m_secScaleWaterfallRect.height();

	m_freqScalePanadapterUpdate = true;
	m_dBmScalePanadapterUpdate = true;
	m_secScaleWaterfallUpdate = true;
	m_panGridUpdate = true;
	
//	GRAPHICS_DEBUG << "***************************************************************************";
//	GRAPHICS_DEBUG << "receiver:" << m_receiver;
//	GRAPHICS_DEBUG << "total size" << size.height();
//	GRAPHICS_DEBUG << "sizes (top, bottom, height):";
//	GRAPHICS_DEBUG << "panRect" << m_panRect.top() << m_panRect.bottom() << m_panRect.height();
//	//GRAPHICS_DEBUG << "panRect (H/W): " << m_panRect.height() << ": " << m_panRect.width();
//	GRAPHICS_DEBUG << "waterfallRect" << m_waterfallRect.top() << m_waterfallRect.bottom() << m_waterfallRect.height();
//	GRAPHICS_DEBUG << "freqScalePanRect" << m_freqScalePanRect.top() << m_freqScalePanRect.bottom() << m_freqScalePanRect.height();
//	GRAPHICS_DEBUG << "dBmScalePanRect" << m_dBmScalePanRect.top() << m_dBmScalePanRect.bottom() << m_dBmScalePanRect.height();
//	GRAPHICS_DEBUG << "";
	
}

void QGLReceiverPanel::saveGLState()
{
}

void QGLReceiverPanel::restoreGLState()
{
}
 
void QGLReceiverPanel::showText(float x, float y, float z = 0.0f, const QString &text = "", bool smallText = true) {

	if (smallText)
		renderPanelText(m_oglTextSmall, x, y, z, text);
	else
		renderPanelText(m_oglTextBig1, x, y, z, text);
}

//********************************************************************
// HMI control
 
void QGLReceiverPanel::enterEvent(QEnterEvent *event) {
	if (m_inputController)
		m_inputController->handleEnter(event);
	QOpenGLWidget::enterEvent(event);
}

void QGLReceiverPanel::leaveEvent(QEvent *event) {
	if (m_inputController)
		m_inputController->handleLeave(event);
	QOpenGLWidget::leaveEvent(event);
}

void QGLReceiverPanel::wheelEvent(QWheelEvent* event) {
	if (m_inputController)
		m_inputController->handleWheel(event);
}

void QGLReceiverPanel::mousePressEvent(QMouseEvent* event) {
	if (m_inputController)
		m_inputController->handleMousePress(event);
}

void QGLReceiverPanel::mouseReleaseEvent(QMouseEvent *event) {
	if (m_inputController)
		m_inputController->handleMouseRelease(event);
}

void QGLReceiverPanel::mouseDoubleClickEvent(QMouseEvent *event) {
	if (m_inputController)
		m_inputController->handleMouseDoubleClick(event);
}

void QGLReceiverPanel::mouseMoveEvent(QMouseEvent* event) {
	if (m_inputController)
		m_inputController->handleMouseMove(event);
}

void QGLReceiverPanel::keyPressEvent(QKeyEvent* event) {
	
	//GRAPHICS_DEBUG << "keyPressEvent";
	if (event->key() == Qt::Key_Control) {
		
		//m_keyCTRLpressed = true;
		//GRAPHICS_DEBUG << "m_keyCTRLpressed =" << m_keyCTRLpressed;
		//printf("Ry %f\n",ry);
		//ry+=10.0f;
 	}
	else if (event->key() == Qt::Key_T) {
			
			//printf("Ry %f\n",ry);
			//ry-=10.0f;
	}
 	else if (event->key() == Qt::Key_P) {
		
		//drawTeapot = !drawTeapot;
	}
 	else if (event->key() == Qt::Key_W) {
		
		//approach -= 0.1f;
	}
 	else if (event->key() == Qt::Key_S) {
		
		//approach += 0.1f;
	}
	else {
		
		//m_keyCTRLpressed = false;
		//GRAPHICS_DEBUG << "m_keyCTRLpressed =" << m_keyCTRLpressed;
		//event->ignore();
 	}

	QWidget::keyPressEvent(event);
 	//updateGL();
//	update();
}

//void QGLReceiverPanel::timerEvent(QElapsedTimerrEvent *) {
//
//	update();
//}
 
//********************************************************************

void QGLReceiverPanel::setSpectrumSize(int value) {

	if (m_dataEngineState == QSDR::DataEngineDown) {

		GRAPHICS_DEBUG << "set spectrum size to: " << value;
		m_spectrumSize = value;
		m_cachedSpectrumBuffer.clear();
//		update();
	}
}

void QGLReceiverPanel::setCtrFrequency(int mode, int rx, qint64 freq) {
    Q_UNUSED(mode)
    if (m_receiver != rx) return;

    const bool freqChanged = (m_centerFrequency != freq);
    m_centerFrequency = freq;

    // Settings emits the VFO change before the centre moves, so a retune that
    // leaves the old span (VFO A/B switch, CAT band hop) arrives here clamped to
    // the span it just left. Only that case is recoverable from the dial: re-read
    // it against the new centre, else the VFO cursor and filter sit off-panel.
    const qint64 spanLow  = m_centerFrequency - m_sampleRate/2;
    const qint64 spanHigh = m_centerFrequency + m_sampleRate/2;
    if (m_vfoFrequency < spanLow || m_vfoFrequency > spanHigh)
        m_vfoFrequency = qBound(spanLow, set->getVfoFrequency(m_receiver), spanHigh);

    m_deltaFrequency = m_centerFrequency - m_vfoFrequency;
    m_deltaF = (qreal)(1.0 * m_deltaFrequency / m_sampleRate);

    if (freqChanged) {
        m_freqScalePanadapterUpdate = true;
        if (m_peakHold)
            resetPeakHoldBins();
    }

    // Digit-wheel retunes fire ctr+vfo every notch. Extra NoPartialUpdate clears here
    // flash the window under Core 3.3. While the engine is running, spectrum frames
    // already repaint; only force a paint when idle (or throttled if somehow needed).
    if (!m_dragMouse && m_dataEngineState != QSDR::DataEngineUp
        && m_displayTime.elapsed() >= 33) {
        m_displayTime.restart();
        update();
    }
}

void QGLReceiverPanel::setVFOFrequency(int mode, int rx, qint64 freq) {

	Q_UNUSED(mode)
	
	if (m_receiver != rx) return;

	qint64 newFreq = freq;
	if (newFreq > m_centerFrequency + m_sampleRate/2)
		newFreq = m_centerFrequency + m_sampleRate/2;
	else if (newFreq < m_centerFrequency - m_sampleRate/2)
		newFreq = m_centerFrequency - m_sampleRate/2;

	const bool unchanged = (m_vfoFrequency == newFreq);
	m_vfoFrequency = newFreq;
	// Always resync NCO offset display — callers may have pre-assigned
	// m_vfoFrequency (click-to-tune) before Settings emits frequencyChanged.
	m_deltaFrequency = m_centerFrequency - m_vfoFrequency;
	m_deltaF = (qreal)(1.0*m_deltaFrequency/m_sampleRate);

	if (unchanged) return;

	// Refresh the frequency-scale FBO so any previously baked VFO panOffset
	// (removed above) cannot leave stale labels under the spectrum.
	m_freqScalePanadapterUpdate = true;

    // Spectrum frames redraw the VFO/filter; skip NoPartialUpdate clears while live.
    if (m_dataEngineState != QSDR::DataEngineUp && m_displayTime.elapsed() >= 33) {
	    m_displayTime.restart();
	    update();
    }
}

void QGLReceiverPanel::setVfoToMidFrequency() {

	m_vfoFrequency = m_centerFrequency;
	m_deltaFrequency = 0;
	m_deltaF = 0;

	set->setVFOFrequency(0, m_receiver, m_vfoFrequency);
	set->setNCOFrequency(false, m_receiver, 0);
}

void QGLReceiverPanel::setMidToVfoFrequency() {

	m_centerFrequency = m_vfoFrequency;
	m_deltaFrequency = 0;
	m_deltaF = 0;

	set->setCtrFrequency(0, m_receiver, m_centerFrequency);
	set->setNCOFrequency(false, m_receiver, 0);
	update();
}

void QGLReceiverPanel::setFilterFrequencies(int rx, qreal lo, qreal hi) {

	if (m_receiver != rx) return;
		
	m_filterLowerFrequency = lo;
	m_filterUpperFrequency = hi;
	m_filterChanged = true;
	update();
}

void QGLReceiverPanel::setCurrentReceiver(int value) {

	if (m_currentReceiver == value) return;

	const bool wasCurrent = (m_receiver == m_currentReceiver);
	const bool isCurrent = (m_receiver == value);
	m_currentReceiver = value;
	// Only panels whose active/inactive styling changes need a redraw.
	if (wasCurrent || isCurrent) {
		m_panGridUpdate = true;
		update();
	}
}

void QGLReceiverPanel::freqRulerPositionChanged(int rx, float pos) {

	if (rx == m_receiver) {
		
		m_freqRulerPosition = pos;
		
		setupDisplayRegions(size());
		update();
	}
}

void QGLReceiverPanel::setSpectrumBuffer(int rx, const qVectorFloat& buffer) {

	if (m_receiver != rx) return;

	if (buffer.size() >= m_spectrumSize)
		m_cachedSpectrumBuffer = buffer;
	else
		m_cachedSpectrumBuffer.clear();

	if (m_dataEngineState != QSDR::DataEngineUp || buffer.size() < m_spectrumSize)
		return;

	if (m_spectrumAveraging)
		spectrumBufferMutex.lock();
	coalesceIncomingSpectrum(buffer);
	if (m_spectrumAveraging)
		spectrumBufferMutex.unlock();

	// Do not schedule the worker here — that drains coalesce every FFT and
	// reintroduces TX/RX flicker. Bin once per display frame (paint / onReady).
	requestThrottledUpdate();
}

void QGLReceiverPanel::coalesceIncomingSpectrum(const QVector<float>& buffer)
{
	const int n = m_spectrumSize;
	if (buffer.size() < n)
		return;

	if (!m_spectrumDirty || m_coalescedSpectrum.size() != n) {
		m_coalescedSpectrum.resize(n);
		for (int i = 0; i < n; ++i)
			m_coalescedSpectrum[i] = buffer.at(i);
	} else {
		for (int i = 0; i < n; ++i) {
			const float v = buffer.at(i);
			if (v > m_coalescedSpectrum[i])
				m_coalescedSpectrum[i] = v;
		}
	}
	m_spectrumDirty = true;
}

void QGLReceiverPanel::resetCoalescedSpectrum()
{
	m_coalescedSpectrum.clear();
	m_spectrumDirty = false;
}

void QGLReceiverPanel::scheduleSpectrumBinning()
{
	if (!m_spectrumBinWorker || !m_spectrumDirty || m_dataEngineState != QSDR::DataEngineUp)
		return;
	if (m_coalescedSpectrum.size() < m_spectrumSize)
		return;
	if (m_panRectWidth <= 0 || !m_panRect.isValid())
		return;

	SpectrumBinWorker::Request req;
	if (m_spectrumAveraging)
		spectrumBufferMutex.lock();
	req.spectrum = m_coalescedSpectrum;
	resetCoalescedSpectrum();
	if (m_spectrumAveraging)
		spectrumBufferMutex.unlock();

	req.params.spectrumSize = m_spectrumSize;
	req.params.panPixelCount = static_cast<int>(m_panRectWidth);
	req.params.fftMult = m_fftMult;
	req.params.freqScaleZoomFactor = m_freqScaleZoomFactor;
	req.params.dBmPanMin = m_dBmPanMin;
	req.params.dBmPanLogGain = m_dBmPanLogGain;
	req.params.mercuryAttenuator = (m_mercuryAttenuator != 0);
	req.params.peakHold = m_peakHold;
	if (m_peakHold) {
		req.peakHoldBins.resize(m_panPeakHoldBins.size());
		for (int i = 0; i < m_panPeakHoldBins.size(); ++i)
			req.peakHoldBins[i] = static_cast<float>(m_panPeakHoldBins.at(i));
	}
	req.generation = ++m_spectrumBinGeneration;
	m_spectrumBinWorker->submit(req);
}

void QGLReceiverPanel::onSpectrumBinsReady(SpectrumBinWorker::Result result)
{
	if (result.generation < m_spectrumBinAppliedGeneration)
		return;
	m_spectrumBinAppliedGeneration = result.generation;
	applyPanBinResult(result.bins);

	// Pick up frames that coalesced while the worker was busy.
	if (m_spectrumDirty)
		scheduleSpectrumBinning();

	requestThrottledUpdate();
}

void QGLReceiverPanel::applyPanBinResult(const DisplayUtils::PanBinResult& bins)
{
	if (bins.panBins.isEmpty())
		return;

	const qreal scaleMultOld = m_scaleMult;
	m_sampleSize = bins.sampleSize;
	m_panScale = bins.panScale;
	m_scaleMult = bins.scaleMult;
	m_panSpectrumBinsLength = bins.panSpectrumBinsLength;
	m_panadapterBins = bins.panBins;

	if (scaleMultOld != m_scaleMult && m_waterfallRenderer)
		m_waterfallRenderer->reset();

	m_waterfallPixel.clear();
	m_waterfallPixel.resize(bins.waterfallPixels.size());
	for (int i = 0; i < bins.waterfallPixels.size(); ++i)
		m_waterfallPixel[i] = bins.waterfallPixels.at(i);

	if (bins.peakHoldBins.size() == bins.panSpectrumBinsLength) {
		m_panPeakHoldBins.resize(bins.peakHoldBins.size());
		for (int i = 0; i < bins.peakHoldBins.size(); ++i)
			m_panPeakHoldBins[i] = bins.peakHoldBins.at(i);
		m_peakHoldBufferResize = false;
	}

	if (m_sampleSize != m_oldSampleSize) {
		GRAPHICS_DEBUG << "m_panSpectrumBinsLength = " << m_panSpectrumBinsLength;
		GRAPHICS_DEBUG << "m_sampleSize =            " << m_sampleSize;
		m_oldSampleSize = m_sampleSize;
	}

	m_waterfallDisplayUpdate = true;
}

void QGLReceiverPanel::requestThrottledUpdate()
{
	int frameIntervalMs = (m_fps > 0) ? (1000 / m_fps) : 33;
	frameIntervalMs = qMax(frameIntervalMs, 33); // ≤ ~30 FPS
	if (m_displayTime.elapsed() >= frameIntervalMs) {
		m_displayTime.restart();
		update();
	}
}

void QGLReceiverPanel::recomputeDisplayBinsFromCache()
{
	if (m_dataEngineState != QSDR::DataEngineUp || m_cachedSpectrumBuffer.size() < m_spectrumSize)
		return;

	resetCoalescedSpectrum();

	QVector<float> specBuf = m_cachedSpectrumBuffer;
	QVector<float> waterBuf = m_cachedSpectrumBuffer;

	if (m_spectrumAveraging) {
		spectrumBufferMutex.lock();
		computeDisplayBins(specBuf, waterBuf);
		spectrumBufferMutex.unlock();
	} else {
		computeDisplayBins(specBuf, waterBuf);
	}
}

qint64 QGLReceiverPanel::findPeakFrequencyNear(qint64 targetFreq, int searchRadiusHz, bool *found) const
{
	if (found) *found = false;
	if (m_cachedSpectrumBuffer.size() < 256 || m_sampleRate <= 0)
		return targetFreq;

	const int N = m_cachedSpectrumBuffer.size();
	const double hzPerBin = static_cast<double>(m_sampleRate) / static_cast<double>(N);
	const double targetBin = (static_cast<double>(N) / 2.0) + (static_cast<double>(targetFreq - m_centerFrequency) / hzPerBin);
	const int radiusBins = qBound(3, static_cast<int>(std::ceil(searchRadiusHz / hzPerBin)), N / 4);

	const int minBin = qBound(2, static_cast<int>(std::floor(targetBin - radiusBins)), N - 3);
	const int maxBin = qBound(2, static_cast<int>(std::ceil(targetBin + radiusBins)), N - 3);

	if (minBin >= maxBin)
		return targetFreq;

	int bestBin = -1;
	float maxVal = -999.0f;
	float meanVal = 0.0f;

	for (int k = minBin; k <= maxBin; ++k) {
		const float val = m_cachedSpectrumBuffer.at(k);
		meanVal += val;
		if (val > maxVal) {
			maxVal = val;
			bestBin = k;
		}
	}
	meanVal /= static_cast<float>(maxBin - minBin + 1);

	// Ensure it is a distinct local peak (at least 2.5 dB above local neighborhood mean)
	if (bestBin > minBin && bestBin < maxBin && (maxVal - meanVal) >= 2.5f) {
		const float y1 = m_cachedSpectrumBuffer.at(bestBin - 1);
		const float y2 = m_cachedSpectrumBuffer.at(bestBin);
		const float y3 = m_cachedSpectrumBuffer.at(bestBin + 1);

		if (y2 >= y1 && y2 >= y3) {
			const float denom = 2.0f * (y1 - 2.0f * y2 + y3);
			float delta = 0.0f;
			if (std::abs(denom) > 1e-6f) {
				delta = (y1 - y3) / denom;
				delta = qBound(-0.5f, delta, 0.5f);
			}

			const double refinedBin = static_cast<double>(bestBin) + delta;
			const qint64 peakHz = m_centerFrequency + qRound64((refinedBin - (static_cast<double>(N) / 2.0)) * hzPerBin);

			if (found) *found = true;
			return peakHz;
		}
	}

	return targetFreq;
}

void QGLReceiverPanel::computeDisplayBins(QVector<float>& buffer, QVector<float>& waterfallBuffer) {

	if (buffer.size() < m_spectrumSize || waterfallBuffer.size() < m_spectrumSize)
		return;

	m_sampleSize = (int)floor(m_fftMult * m_spectrumSize * m_freqScaleZoomFactor);

	// fftMult escalation only works when the engine is down (setSampleSize/setSpectrumSize
	// are no-ops while the engine is running). Skip escalation while running to avoid
	// cascading that corrupts m_fftMult and m_freqScaleZoomFactor.
	if (m_dataEngineState != QSDR::DataEngineUp) {

		if (m_sampleSize < 2048) {

			if (m_fftMult == 1) {

				GRAPHICS_DEBUG << "set sample size to 8192";
				set->setSampleSize(m_receiver, 8192);
				m_dBmPanLogGain += 6;
				m_fftMult = 2;
				return;
			}
			else if (m_fftMult == 2) {

				GRAPHICS_DEBUG << "set sample size to 16384";
				set->setSampleSize(m_receiver, 16384);
				m_dBmPanLogGain += 6;
				m_fftMult = 4;
				return;
			}
			else if (m_fftMult == 4) {

				GRAPHICS_DEBUG << "set sample size to 32768";
				set->setSampleSize(m_receiver, 32768);
				m_dBmPanLogGain += 6;
				m_fftMult = 8;
				return;
			}
			else if (m_fftMult == 8) {

				GRAPHICS_DEBUG << "set sample size to 65536";
				set->setSampleSize(m_receiver, 65536);
				m_dBmPanLogGain += 6;
				m_fftMult = 16;
				return;
			}
		}
		else if (m_sampleSize > 4096) {

			if (m_fftMult == 2) {

				GRAPHICS_DEBUG << "set sample size to 4096";
				set->setSampleSize(m_receiver, 4096);
				m_dBmPanLogGain -= 6;
				m_fftMult = 1;
				return;
			}
			else if (m_fftMult == 4) {

				GRAPHICS_DEBUG << "set sample size to 8192";
				set->setSampleSize(m_receiver, 8192);
				m_dBmPanLogGain -= 6;
				m_fftMult = 2;
				return;
			}
			else if (m_fftMult == 8) {

				GRAPHICS_DEBUG << "set sample size to 16384";
				set->setSampleSize(m_receiver, 16384);
				m_dBmPanLogGain -= 6;
				m_fftMult = 4;
				return;
			}
			else if (m_fftMult == 16) {

				GRAPHICS_DEBUG << "set sample size to 32768";
				set->setSampleSize(m_receiver, 32768);
				m_dBmPanLogGain -= 6;
				m_fftMult = 8;
				return;
			}
		}

	} // end engine-down-only escalation

	if (m_panRectWidth <= 0 || !m_panRect.isValid())
		return;

	DisplayUtils::PanBinParams params;
	params.spectrumSize = m_spectrumSize;
	params.panPixelCount = static_cast<int>(m_panRectWidth);
	params.fftMult = m_fftMult;
	params.freqScaleZoomFactor = m_freqScaleZoomFactor;
	params.dBmPanMin = m_dBmPanMin;
	params.dBmPanLogGain = m_dBmPanLogGain;
	params.mercuryAttenuator = (m_mercuryAttenuator != 0);
	params.peakHold = m_peakHold;

	QVector<float> peakHoldIn;
	if (m_peakHold) {
		peakHoldIn.resize(m_panPeakHoldBins.size());
		for (int i = 0; i < m_panPeakHoldBins.size(); ++i)
			peakHoldIn[i] = static_cast<float>(m_panPeakHoldBins.at(i));
	}

	applyPanBinResult(DisplayUtils::binPanadapterSpectrum(
		buffer, waterfallBuffer, params, peakHoldIn));
}

void QGLReceiverPanel::setFramesPerSecond(int rx, int value) {

	if (m_receiver != rx) return;

	m_fps = value;

	m_secWaterfallMin = -(1.0/m_fps) * m_secScaleWaterfallRect.height();
	m_secScaleWaterfallRenew = true;
	m_secScaleWaterfallUpdate = true;
}

void QGLReceiverPanel::systemStateChanged(
	QSDR::_Error err, 
	QSDR::_HWInterfaceMode hwmode, 
	QSDR::_ServerMode mode, 
	QSDR::_DataEngineState state)
{
	Q_UNUSED (err)
	Q_UNUSED (hwmode)
	Q_UNUSED (state)

	if (m_dataEngineState != state)
		m_dataEngineState = state;

	if (state == QSDR::DataEngineDown) {
		m_fftMult = 1;
		m_cachedSpectrumBuffer.clear();
		m_panadapterBins.clear();
		m_waterfallPixel.clear();
		resetCoalescedSpectrum();
		// Drop in-flight worker results that would otherwise re-apply after teardown.
		m_spectrumBinAppliedGeneration = ++m_spectrumBinGeneration;
		m_waterfallDisplayUpdate = true;
		update();
	}

	if (m_serverMode != mode)
		m_serverMode = mode;

//	if (m_serverMode == mode)
//		return;
//	else
//		m_serverMode = mode;

	//resizeGL(width(), height());
	m_displayTime.restart();
}

void QGLReceiverPanel::graphicModeChanged(
	int rx,
	PanGraphicsMode panMode,
	WaterfallColorMode waterfallColorMode)
{
	if (m_receiver != rx) return;
	
	
	if (m_panMode != panMode)
		m_panMode = panMode;

	if (m_waterfallMode != waterfallColorMode)
		m_waterfallMode = waterfallColorMode;

	update();
}

void QGLReceiverPanel::setSpectrumAveraging(int rx, bool value) {

	if (m_receiver != rx) return;

	QMutexLocker locker(&spectrumBufferMutex);
	if (m_spectrumAveraging != value)
		m_spectrumAveraging = value;
	while (!specAv_queue.isEmpty())
		specAv_queue.dequeue();
}

void QGLReceiverPanel::setSpectrumAveragingCnt(int value) {

	QMutexLocker locker(&spectrumBufferMutex);

	while (!specAv_queue.isEmpty())
		specAv_queue.dequeue();

	m_specAveragingCnt = value;
	m_scale = (m_specAveragingCnt > 0) ? 1.0f / m_specAveragingCnt : 1.0f;
}

void QGLReceiverPanel::setPanGridStatus(bool value, int rx) {

	if (m_receiver != rx) return;

	QMutexLocker locker(&spectrumBufferMutex);
	if (m_panGrid != value)
		m_panGrid = value;
}

void QGLReceiverPanel::setPeakHoldStatus(bool value, int rx) {

	if (m_receiver != rx) return;

	QMutexLocker locker(&spectrumBufferMutex);
	if (m_peakHold == value)
		return;

	m_peakHold = value;
	resetPeakHoldBins();
}

void QGLReceiverPanel::resetPeakHoldBins() {
	m_panPeakHoldBins.clear();
	if (m_panSpectrumBinsLength > 0) {
		m_panPeakHoldBins.resize(m_panSpectrumBinsLength);
		m_panPeakHoldBins.fill(-500.0);
	}
	m_peakHoldBufferResize = false;
}

void QGLReceiverPanel::setPanLockedStatus(bool value, int rx) {

	if (m_receiver != rx) return;
	
	if (m_panLocked == value)
		return;
	else
		m_panLocked = value;
}

void QGLReceiverPanel::setClickVFOStatus(bool value, int rx) {

	if (m_receiver != rx) return;
	
	if (m_clickVFO == value)
		return;
	else
		m_clickVFO = value;
}

void QGLReceiverPanel::setHairCrossStatus(bool value, int rx) {

	if (m_receiver != rx) return;
    m_crossHair = value;
}

void QGLReceiverPanel::sampleRateChanged(int value) {

	m_sampleRate = value;
	m_deltaF = (qreal)(1.0*m_deltaFrequency/m_sampleRate);

	m_freqScalePanadapterUpdate = true;
	m_panGridUpdate = true;
	m_filterChanged = true;
	m_peakHoldBufferResize = true;
}

void QGLReceiverPanel::setMercuryAttenuator(HamBand band, int value) {

	Q_UNUSED(band)

	m_mercuryAttenuator = value;
}

void QGLReceiverPanel::setPanadapterColors() {

	m_spectrumColorsChanged = true;

	RadioModel* radioModel = qobject_cast<RadioModel*>(m_sliceModel ? m_sliceModel->parent() : nullptr);
	const TPanadapterColors colors = radioModel ? radioModel->panadapterColors() : set->getPanadapterColors();

	mutex.lock();
	m_bkgRed   = (GLfloat)(colors.panBackgroundColor.red() / 256.0);
	m_bkgGreen = (GLfloat)(colors.panBackgroundColor.green() / 256.0);
	m_bkgBlue  = (GLfloat)(colors.panBackgroundColor.blue() / 256.0);

	m_red	= (GLfloat)(colors.panLineColor.red() / 256.0);
	m_green = (GLfloat)(colors.panLineColor.green() / 256.0);
	m_blue	= (GLfloat)(colors.panLineColor.blue() / 256.0);

	m_redF	 = (GLfloat)(colors.panLineFilledColor.red() / 256.0);
	m_greenF = (GLfloat)(colors.panLineFilledColor.green() / 256.0);
	m_blueF  = (GLfloat)(colors.panLineFilledColor.blue() / 256.0);

	m_redST	  = (GLfloat)(colors.panSolidTopColor.red() / 256.0);
	m_greenST = (GLfloat)(colors.panSolidTopColor.green() / 256.0);
	m_blueST  = (GLfloat)(colors.panSolidTopColor.blue() / 256.0);

	m_redSB   = (GLfloat)(colors.panSolidBottomColor.red() / 256.0);
	m_greenSB = (GLfloat)(colors.panSolidBottomColor.green() / 256.0);
	m_blueSB  = (GLfloat)(colors.panSolidBottomColor.blue() / 256.0);

	m_waterfallMidColor = colors.waterfallColor.toRgb() ;

	QColor gridColor = m_gridColor;
	m_gridColor = colors.gridLineColor;

	if (gridColor != m_gridColor) {

		m_redGrid   = (GLfloat)(m_gridColor.red()/256.0);
		m_greenGrid = (GLfloat)(m_gridColor.green()/256.0);
		m_blueGrid  = (GLfloat)(m_gridColor.blue()/256.0);

		m_panGridUpdate = true;
	}
	mutex.unlock();
	update();
}

void QGLReceiverPanel::setWaterfallOffesetLo(int rx, int value) {

	if (m_receiver != rx) return;

	m_waterfallOffsetLo = value;
	update();
}

void QGLReceiverPanel::setWaterfallOffesetHi(int rx, int value) {

	if (m_receiver != rx) return;

	m_waterfallOffsetHi = value;
	update();
}

void QGLReceiverPanel::setdBmScaleMin(int rx, qreal value) {

	if (m_receiver != rx) return;

	m_dBmPanMin = value;

	m_dBmScalePanadapterUpdate = true;
	m_panGridUpdate = true;
	m_peakHoldBufferResize = true;
	update();
}

void QGLReceiverPanel::setdBmScaleMax(int rx, qreal value) {

	if (m_receiver != rx) return;

	m_dBmPanMax = value;

	m_dBmScalePanadapterUpdate = true;
	m_panGridUpdate = true;
	m_peakHoldBufferResize = true;
	update();
}

void QGLReceiverPanel::setMouseWheelFreqStep(int rx, qreal step) {

	if (m_receiver != rx) return;
	m_mouseWheelFreqStep = step;
	update();
}

void QGLReceiverPanel::setHamBand(int rx, bool byButton, HamBand band) {

	Q_UNUSED(byButton)
	if (m_receiver != rx) return;

	m_dBmPanMin = set->getdBmPanScaleMin(m_receiver, band);
	m_dBmPanMax = set->getdBmPanScaleMax(m_receiver, band);
	if (m_sliceModel) {
		m_sliceModel->setDBmPanScaleMin(m_dBmPanMin);
		m_sliceModel->setDBmPanScaleMax(m_dBmPanMax);
	}

	m_dBmScalePanadapterUpdate = true;
	m_panGridUpdate = true;
	m_peakHoldBufferResize = true;
}

void QGLReceiverPanel::setADCStatus(int value) {

	m_adcStatus = value;
	QTimer::singleShot(50, this, &QGLReceiverPanel::updateADCStatus);
}
void QGLReceiverPanel::updateADCStatus() {

	if (m_dataEngineState == QSDR::DataEngineUp)
		m_adcStatus = 1;
	else
		m_adcStatus = 0;
}

void QGLReceiverPanel::setAGCLineLevels(int rx, qreal thresh, qreal hang) {

	if (m_receiver != rx) return;
	if (m_agcThresholdOld == thresh && m_agcHangLevelOld == hang) return;

	m_agcThresholdOld = thresh;
	m_agcHangLevelOld = hang;
}

void QGLReceiverPanel::setAGCLineFixedLevel(int rx, qreal value) {

	if (m_receiver != rx) return;
	if (m_agcFixedGain == value) return;

	m_agcFixedGain = value;
}

void QGLReceiverPanel::setADCMode(int rx, ADCMode mode) {

	if (m_receiver != rx) return;

	m_adcMode = mode;
	m_adcModeString = set->getADCModeString(m_receiver);
}

void QGLReceiverPanel::setAGCMode(int rx, AGCMode mode, bool hangEnabled) {

	if (m_receiver != rx) return;

	if (m_agcHangEnabled == hangEnabled && m_agcMode == mode) return;

	m_agcMode = mode;
	m_agcModeString = set->getAGCModeString(m_receiver);
	m_agcHangEnabled = hangEnabled;
	GRAPHICS_DEBUG << "m_agcHangEnabled = " << m_agcHangEnabled;
}

void QGLReceiverPanel::setAGCLinesStatus(bool value, int rx) {

	if (m_receiver != rx) return;

	m_showAGCLines = value;
}

void QGLReceiverPanel::setDSPMode(int rx, DSPMode mode) {
        update();

	if (m_receiver != rx) return;

	m_dspMode = mode;
	m_dspModeString = set->getDSPModeString(m_dspMode);
}

void QGLReceiverPanel::showRadioPopup(bool value) {

	Q_UNUSED (value)

	radioPopup->showPopupWidget(QCursor::pos());
}

void QGLReceiverPanel::qglColor(QColor color)
{
    m_glTextColor = color;
}

