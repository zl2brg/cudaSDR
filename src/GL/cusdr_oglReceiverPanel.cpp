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
#include "cusdr_glShaders.h"
#include "cusdr_glDraw.h"
#include "Controllers/RadioPopupController.h"

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
	, m_mousePos(QPoint(-1, -1))
	, m_mouseDownPos(QPoint(-1, -1))
	, m_panSpectrumBinsLength(0)
	, m_waterfallRenderer(nullptr)
	, m_panadapterRenderer(nullptr)
	, m_overlayRenderer(nullptr)
	, m_bigHeight(0)
	, m_glTextColor(Qt::white)
	, m_panelDpr(devicePixelRatioF())

	, m_filterRight(0)
	, m_filterTop(0)
	, m_filterBottom(0)
	, m_receiver(model ? model->id() : 0)
	//, m_frequencyRxOnRx(0)
	, m_spectrumSize(set->getSpectrumSize())
	, m_sampleSize(0)
	, m_oldSampleSize(0)
	, m_specAveragingCnt(set->getSpectrumAveragingCnt(m_receiver))
	, m_currentReceiver(set->getCurrentReceiver())
	, m_waterfallAlpha(255)
	, m_freqRulerDisplayWidth(0)
	, m_displayTop(0)
	, m_panSpectrumMinimumHeight(0)
	, m_snapMouse(3)
	, m_sampleRate(set->getSampleRate())
	, m_adcStatus(0)
	, m_fftMult(1)
	, m_smallSize(true)
	, m_spectrumVertexColorUpdate(false)
	, m_spectrumColorsChanged(true)
	, m_spectrumAveraging(set->getSpectrumAveraging(m_receiver))
	//, m_spectrumAveragingOld(m_spectrumAveraging)
	, m_crossHair(set->getHairCrossStatus(m_receiver))
    , m_crossHairCursor(false)
	, m_panGrid(set->getPanGridStatus(m_receiver))
	, m_peakHold(set->getPeakHoldStatus(m_receiver))
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
	//setAutoBufferSwap(true);
	setAutoFillBackground(false);

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
	m_adcMode = set->getADCMode(m_receiver);
	m_dspModeString = set->getDSPModeString(set->getDSPMode(m_receiver));
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

	m_displayTime.start();
	m_resizeTime.start();
	//freqChangeTimer.start();
	
	m_fps = set->getFramesPerSecond(m_receiver);
	m_secWaterfallMin = -(1.0/m_fps) * m_secScaleWaterfallRect.height();

	
	m_dBmPanLogGain = 0;//49;//69 // allow user to calibrate this value

	m_cameraDistance = 0;
	m_cameraAngle = QPoint(0, 0);

	m_mousePos = QPoint(-100, -100);
	
	m_gridColor = set->getPanadapterColors().gridLineColor;
	m_darkColor = QColor(150, 150, 150, 100);

	m_redGrid   = (GLfloat)(m_gridColor.red()/256.0);
	m_greenGrid = (GLfloat)(m_gridColor.green()/256.0);
	m_blueGrid  = (GLfloat)(m_gridColor.blue()/256.0);

	m_bkgRed   = (GLfloat)(set->getPanadapterColors().panBackgroundColor.red() / 256.0);
	m_bkgGreen = (GLfloat)(set->getPanadapterColors().panBackgroundColor.green() / 256.0);
	m_bkgBlue  = (GLfloat)(set->getPanadapterColors().panBackgroundColor.blue() / 256.0);

	m_red	= (GLfloat)(set->getPanadapterColors().panLineColor.red() / 256.0);
	m_green = (GLfloat)(set->getPanadapterColors().panLineColor.green() / 256.0);
	m_blue	= (GLfloat)(set->getPanadapterColors().panLineColor.blue() / 256.0);

	m_redF	 = (GLfloat)(set->getPanadapterColors().panLineFilledColor.red() / 256.0);
	m_greenF = (GLfloat)(set->getPanadapterColors().panLineFilledColor.green() / 256.0);
	m_blueF  = (GLfloat)(set->getPanadapterColors().panLineFilledColor.blue() / 256.0);

	m_redST	  = (GLfloat)(set->getPanadapterColors().panSolidTopColor.red() / 256.0);
	m_greenST = (GLfloat)(set->getPanadapterColors().panSolidTopColor.green() / 256.0);
	m_blueST  = (GLfloat)(set->getPanadapterColors().panSolidTopColor.blue() / 256.0);

	m_redSB   = (GLfloat)(set->getPanadapterColors().panSolidBottomColor.red() / 256.0);
	m_greenSB = (GLfloat)(set->getPanadapterColors().panSolidBottomColor.green() / 256.0);
	m_blueSB  = (GLfloat)(set->getPanadapterColors().panSolidBottomColor.blue() / 256.0);

	m_waterfallLoColor = QColor(0, 0, 0, m_waterfallAlpha);
	m_waterfallHiColor = QColor(192, 124, 255, m_waterfallAlpha);
	m_waterfallMidColor = set->getPanadapterColors().waterfallColor.toRgb();
	m_waterfallColorRange = (int)(m_dBmPanMax - m_dBmPanMin);

    m_frequencyScaleFBO = nullptr;
    m_dBmScaleFBO = nullptr;
    m_secScaleWaterfallFBO = nullptr;


	m_haircrossOffsetRight = 30;
	m_haircrossOffsetLeft = 116;
	m_haircrossMaxRight = 110;
	m_haircrossMinTop = 40;

	if (m_specAveragingCnt > 0)
		m_scale = 1.0f / m_specAveragingCnt;
	else
		m_scale = 1.0f;

}

QGLReceiverPanel::~QGLReceiverPanel() {

    qDebug() << "rx panel destructor" << m_receiver;
    disconnect(set, 0, this, 0);

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
	
	if (m_frequencyScaleFBO) {

		delete m_frequencyScaleFBO;
        m_frequencyScaleFBO = nullptr;
	}

	if (m_dBmScaleFBO) {

		delete m_dBmScaleFBO;
        m_dBmScaleFBO = nullptr;
	}


	if (m_secScaleWaterfallFBO) {

		delete m_secScaleWaterfallFBO;
	m_secScaleWaterfallFBO = nullptr;
	}

	if (m_waterfallRenderer) {
	    delete m_waterfallRenderer;
	    m_waterfallRenderer = nullptr;
	}

    if (m_panadapterRenderer) {
        delete m_panadapterRenderer;
        m_panadapterRenderer = nullptr;
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
    connect(m_sliceModel, &SliceModel::cwDecodeEnabledChanged, this, qOverload<>(&QGLReceiverPanel::update));
    connect(m_sliceModel, &SliceModel::cwTrackedPitchChanged, this, qOverload<>(&QGLReceiverPanel::update));

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
    if (!m_textureProgram->link())
        qCritical() << "Receiver panel texture shader link failed:" << m_textureProgram->log();

	//*****************************************************************
	// default initialization

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
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

	QRect mouse_rect(0, 0, 100, 100);
	mouse_rect.moveCenter(m_mousePos);

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
	//m_displayTime.restart();

    if (m_freqScalePanRect.isValid())
        updateFrequencyRuler();
    if (m_dBmScalePanRect.isValid())
        updateDBmRuler();

    drawPanadapter();
    drawBandPlanStrip();

    glDisable(GL_DEPTH_TEST);
    drawPanHorizontalScale();
    drawPanVerticalScale();
    drawPanadapterGrid();

	if (m_dataEngineState == QSDR::DataEngineUp && m_showAGCLines && (m_receiver == m_currentReceiver)) {
        ensurePanelViewport();
		drawAGCControl();
    }

	if (m_panRect.width() > 300 && m_panRect.height() > 80) {

        ensurePanelViewport();
        drawVFOControl();
        drawReceiverInfo();
        drawCwDecoderHUD();
	}

	if (m_waterfallRect.height() > 10) {
        ensurePanelViewport();
        drawWaterfall();
        glDisable(GL_DEPTH_TEST);
        drawCenterLine();
        drawPanFilter();
        glEnable(GL_DEPTH_TEST);
        drawWaterfallVerticalScale();
    } else {
        glDisable(GL_DEPTH_TEST);
        drawCenterLine();
        drawPanFilter();
        glEnable(GL_DEPTH_TEST);
    }
    if (m_waterfallDisplayUpdate)
        m_waterfallDisplayUpdate = false;

	if (m_crossHair) {

		if (m_mouseRegion != freqScalePanadapterRegion && 
			m_mouseRegion != dBmScalePanadapterRegion && 
			m_mouseRegion != filterRegion &&
			m_mouseRegion != filterRegionLow &&
			m_mouseRegion != filterRegionHigh &&
			m_mouseRegion != agcThresholdLine &&
			m_mouseRegion != agcHangLine &&
			m_mouseRegion != agcFixedGainLine &&
			m_crossHairCursor)
			drawCrossHair();
	}
}

void QGLReceiverPanel::paint3DPanadapterMode() {
}

void QGLReceiverPanel::drawBandPlanStrip()
{
	RadioModel *radioModel = qobject_cast<RadioModel *>(m_sliceModel ? m_sliceModel->parent() : nullptr);
	BandPlanManager *plan = radioModel ? radioModel->bandPlan() : nullptr;
	if (!plan || plan->isEmpty())
		return;

	const qreal span = displayedFrequencySpanHz();
	if (span <= 0.0 || !m_panRect.isValid())
		return;

	ensurePanelViewport();

	const qint64 loHz = qint64(qreal(m_centerFrequency) - span / 2.0);
	const qint64 hiHz = qint64(qreal(m_centerFrequency) + span / 2.0);

	const QDateTime nowUtc = QDateTime::currentDateTimeUtc();
	const int utcMinOfDay = nowUtc.time().hour() * 60 + nowUtc.time().minute();
	const int dayOfWeek = nowUtc.date().dayOfWeek();

	QVector<BandSpot> spots = plan->spotsInSpan(loHz, hiHz, utcMinOfDay, dayOfWeek);
	if (spots.isEmpty())
		return;

	// When zoomed way out, thin labels to digimode / time / beacon markers.
	if (span > 1.5e6 && spots.size() > 16) {
		QVector<BandSpot> priority;
		priority.reserve(spots.size());
		for (const BandSpot &s : spots) {
			const QString &l = s.label;
			if (l.contains(QLatin1String("FT8"))
			    || l.contains(QLatin1String("FT4"))
			    || l.contains(QLatin1String("WSPR"))
			    || l.contains(QLatin1String("JS8"))
			    || l.contains(QLatin1String("PSK"))
			    || l.contains(QLatin1String("IBP"))
			    || l.contains(QLatin1String("WWV"))
			    || l.contains(QLatin1String("WWVH"))
			    || l.contains(QLatin1String("CHU"))
			    || l.contains(QLatin1String("beacon"), Qt::CaseInsensitive))
				priority.append(s);
		}
		if (!priority.isEmpty())
			spots = priority;
	}

	OGLText *spotText = m_oglTextNormal ? m_oglTextNormal
	                                    : (m_oglTextSmall ? m_oglTextSmall : m_oglTextTiny);
	if (!spotText)
		return;

	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	const int left = m_panRect.left();
	const int right = m_panRect.right();
	const int baseY = m_panRect.bottom(); // sit just above the frequency ruler
	const qreal pxPerHz = qreal(m_panRect.width()) / span;

	auto freqToX = [&](qint64 freqHz) -> int {
		return left + qRound((qreal(freqHz) - qreal(m_centerFrequency) + span / 2.0) * pxPerHz);
	};

	auto spotColor = [](const QString &label) -> QColor {
		if (label.contains(QLatin1String("FT8")))
			return QColor(90, 200, 130);
		if (label.contains(QLatin1String("FT4")))
			return QColor(130, 190, 230);
		if (label.contains(QLatin1String("WSPR")))
			return QColor(230, 170, 70);
		if (label.contains(QLatin1String("JS8")))
			return QColor(190, 150, 230);
		if (label.contains(QLatin1String("[CW"), Qt::CaseInsensitive) || label.endsWith(QLatin1String("CW")))
			return QColor(255, 215, 0); // Gold for CW RBN spots
		if (label.contains(QLatin1String("[SSB"), Qt::CaseInsensitive) || label.contains(QLatin1String("[USB"), Qt::CaseInsensitive) || label.contains(QLatin1String("[LSB"), Qt::CaseInsensitive))
			return QColor(80, 190, 245); // Sky blue for SSB spots
		if (label.contains(QLatin1String("[RTTY"), Qt::CaseInsensitive) || label.contains(QLatin1String("RTTY")))
			return QColor(245, 90, 180); // Magenta for RTTY spots
		if (label.contains(QLatin1String("IBP")) || label.contains(QLatin1String("beacon"), Qt::CaseInsensitive))
			return QColor(110, 210, 210);
		return QColor(200, 205, 215);
	};

	const int textH = spotText->fontMetrics().height();
	const int lanePitch = textH + 3;
	int lane = 0;
	int lastLabelRight[3] = { left - 1000, left - 1000, left - 1000 };

	for (const BandSpot &spot : spots) {
		const int x = freqToX(spot.freqHz);
		if (x < left || x > right)
			continue;

		const QColor col = spotColor(spot.label);
		const int textW = spotText->fontMetrics().horizontalAdvance(spot.label);

		// Pick the first staggered lane that doesn't overlap the previous label.
		int chosen = 0;
		for (; chosen < 3; ++chosen) {
			if (x - 2 >= lastLabelRight[chosen] + 6)
				break;
		}
		if (chosen >= 3)
			chosen = lane % 3;
		lane = chosen + 1;
		lastLabelRight[chosen] = x - 2 + textW + 8;

		// Labels grow upward from the frequency ruler.
		const int labelY = baseY - (chosen + 1) * lanePitch;
		if (labelY < m_panRect.top() + 2)
			continue;

		const int labelX = qBound(left, x - 2, right - textW - 6);
		drawPanelRect(QRect(x, labelY, 1, baseY - labelY), QColor(col.red(), col.green(), col.blue(), 140), -1.25f);
		drawPanelRect(QRect(x - 2, baseY - 3, 5, 5), col, -1.2f);
		drawPanelRect(QRect(labelX, labelY, textW + 6, textH + 1), QColor(8, 10, 14, 190), -1.15f);

		m_glTextColor = QColor(0, 0, 0, 160);
		renderPanelText(spotText, float(labelX + 4), float(labelY + 1), -1.1f, spot.label);
		m_glTextColor = col;
		renderPanelText(spotText, float(labelX + 3), float(labelY), -1.05f, spot.label);
	}
}

void QGLReceiverPanel::drawPanadapter() {

    const bool showSpectrum = (m_dataEngineState == QSDR::DataEngineUp && !m_panadapterBins.isEmpty());

    if (!showSpectrum) {
        if (m_dataEngineState != QSDR::DataEngineUp)
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (m_panadapterRenderer && m_panRect.isValid()) {
            float dpr = (float)devicePixelRatio();
            QMatrix4x4 projection;
            projection.ortho(0, size().width(), size().height(), 0, -10, 10);
            PanadapterRenderer::Colors colors = { m_red, m_green, m_blue, m_redF, m_greenF, m_blueF, m_redST, m_greenST, m_blueST, m_redSB, m_greenSB, m_blueSB, m_bkgRed, m_bkgGreen, m_bkgBlue };
            m_panadapterRenderer->renderIdleBackground(this, projection, m_panRect, dpr, size().height(), colors, m_dataEngineState, (m_receiver == m_currentReceiver));
        }
        return;
    }

    if (m_dataEngineState == QSDR::DataEngineUp)
        glClear(GL_DEPTH_BUFFER_BIT);
    else
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glLineWidth(1);

    if (m_panadapterRenderer) {
        float dpr = (float)devicePixelRatio();
        QMatrix4x4 projection;
        projection.ortho(0, size().width(), size().height(), 0, -10, 10);
        PanadapterRenderer::Colors colors = { m_red, m_green, m_blue, m_redF, m_greenF, m_blueF, m_redST, m_greenST, m_blueST, m_redSB, m_greenSB, m_blueSB, m_bkgRed, m_bkgGreen, m_bkgBlue };
        const QVector<qreal> emptyPeakHold;
        const QVector<qreal>& peakHoldBins =
            (m_peakHold && m_panPeakHoldBins.size() == m_panadapterBins.size())
                ? m_panPeakHoldBins
                : emptyPeakHold;
        m_panadapterRenderer->render(this, projection, m_panRect, m_panadapterBins, m_dBmPanMax, m_dBmPanMin,
                                   m_panMode, m_scaleMult, dpr, size().height(), colors, m_dataEngineState,
                                   (m_receiver == m_currentReceiver), peakHoldBins);
        if (m_panadapterRenderer->usesCompositePass())
            m_panadapterRenderer->compositeToDefaultFramebuffer(this, projection, m_panRect, dpr, size().height());
    }

    glEnable(GL_DEPTH_TEST);
}

void QGLReceiverPanel::drawPanVerticalScale() {

    if (!m_dBmScalePanRect.isValid()) return;

    // Use logical dimensions for consistent rendering
    int width = m_dBmScalePanRect.width();
    int height = m_dBmScalePanRect.height();
    
    // Safety check for valid dimensions
    if (width <= 0 || height <= 0) return;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    const bool regenDBmScale = !m_dBmScaleFBO || m_dBmScalePanadapterRenew
                               || (m_dBmScalePanadapterUpdate && !m_dragDBmScale);

    if (regenDBmScale) {

        if (!m_dBmScaleFBO || m_dBmScalePanadapterRenew) {

            if (m_dBmScaleFBO) {
                delete m_dBmScaleFBO;
                m_dBmScaleFBO = 0;
            }
            m_dBmScaleFBO = new QOpenGLFramebufferObject(width, height);
        }
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glViewport(0, 0, width, height);

        m_dBmScaleFBO->bind();
        renderPanVerticalScale();
        m_dBmScaleFBO->release();

        if (!m_dragDBmScale)
            m_dBmScalePanadapterUpdate = false;
        m_dBmScalePanadapterRenew = false;
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    }

    drawCachedTexture(m_dBmScalePanRect, m_dBmScaleFBO->texture(), 0.0f);

    if (m_dragDBmScale && m_overlayRenderer && m_dBmScaleFBO) {
        QMatrix4x4 projection;
        projection.ortho(0, size().width(), size().height(), 0, -10, 10);
        const float alpha = (m_receiver == m_currentReceiver) ? 1.0f : 0.8f;
        m_overlayRenderer->drawDBmScaleTicks(projection, m_dBmScalePanRect, m_dBmScale,
                                            m_redGrid, m_greenGrid, m_blueGrid, alpha);
    }
}


void QGLReceiverPanel::renderPanVerticalScale() {

    QString str;
    int spacing = 7;
    int fontHeight;
    int fontMaxWidth;
    QOpenGLPaintDevice paintDevice(m_dBmScaleFBO->size());

    painter.begin(&paintDevice);

    if (m_smallSize) {
        fontHeight = m_fonts.smallFontMetrics->tightBoundingRect(".0dBm").height() + spacing;
        fontMaxWidth = m_fonts.smallFontMetrics->boundingRect("-000.0").width();
    } else {
        fontHeight = m_fonts.bigFont1Metrics->tightBoundingRect(".0dBm").height() + spacing;
        fontMaxWidth = m_fonts.bigFont1Metrics->boundingRect("-000.0").width();
    }

    int width = m_dBmScalePanRect.width();
    int height = m_dBmScalePanRect.height();

    QRect textRect(0, 0, fontMaxWidth, fontHeight);
    textRect.moveLeft(3);

    int len    = m_dBmScale.mainPointPositions.length();
    int sublen = m_dBmScale.subPointPositions.length();

    // draw the scale background
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(0, 0, width, height, QColor(30, 30, 30, 180));
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    painter.setPen(QPen(QColor(166, 194, 206), 1, Qt::SolidLine, Qt::FlatCap));
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            const int y = m_dBmScale.mainPointPositions.at(i);
            painter.drawLine(width, y, width - 4, y);
        }
        painter.setPen(QPen(QColor(115, 143, 155), 1, Qt::SolidLine, Qt::FlatCap));
        for (int i = 0; i < sublen; i++) {
            const int y = m_dBmScale.subPointPositions.at(i);
            painter.drawLine(width, y, width - 2, y);
        }
    }

    painter.setPen(QPen(QColor(191,219,232)));
    painter.setFont(m_oglTextNormal->font());

    if (len > 0) {
        for (int i = 0; i < len; i++) {
            textRect.moveBottom(m_dBmScale.mainPointPositions.at(i) + textRect.height()/2);
            if (textRect.y() > textRect.height() && textRect.bottom() <= (height - textRect.height()/2)) {
                str = QString::number(m_dBmScale.mainPoints.at(i), 'f', 1);
                painter.drawText(textRect.x(), textRect.y() + fontHeight, str);
            }
        }
    }

    textRect.moveTop(height - textRect.height());
    painter.setPen(QPen(QColor(239, 56, 109)));
    str = QString("dBm");
    painter.drawText(textRect.x() + 18, textRect.y() + fontHeight, str);
    painter.end();

}

void QGLReceiverPanel::drawPanHorizontalScale() {

	if (!m_freqScalePanRect.isValid()) return;

    int width = m_freqScalePanRect.width();
    int height = m_freqScalePanRect.height();
    
    // Safety check for valid dimensions
    if (width <= 0 || height <= 0) return;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

    const bool regenFreqScale = !m_frequencyScaleFBO || m_freqScalePanadapterRenew
                                || m_freqScalePanadapterUpdate
                                || m_dragMouse
                                || (m_dragFreqScale && !m_dragFreqScaleZoom);

    if (regenFreqScale) {

        if (!m_frequencyScaleFBO || m_freqScalePanadapterRenew) {

            if (m_frequencyScaleFBO) {

                delete m_frequencyScaleFBO;
                m_frequencyScaleFBO = 0;
            }

            m_frequencyScaleFBO = new QOpenGLFramebufferObject(width, height);  // Use logical size
        }

        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glViewport(0, 0, width, height);  // Use logical coordinates

        m_frequencyScaleFBO->bind();
            renderPanHorizontalScale();
        m_frequencyScaleFBO->release();

        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        m_freqScalePanadapterUpdate = false;
		m_freqScalePanadapterRenew = false;
	}

	drawCachedTexture(m_freqScalePanRect, m_frequencyScaleFBO->texture(), 0.0f);
}

void QGLReceiverPanel::updateFrequencyRuler()
{
    if (!m_freqScalePanRect.isValid())
        return;

    const qreal freqSpan = displayedFrequencySpanHz();
    if (freqSpan <= 0)
        return;

    const int plotLeft = m_dBmScalePanRect.isValid() ? m_dBmScalePanRect.width() : 0;
    const int plotWidth = m_freqScalePanRect.width() - plotLeft;
    if (plotWidth <= 0)
        return;

    const int fullWidth = m_freqScalePanRect.width();

    const qreal lowerFreq = qreal(m_centerFrequency) - freqSpan / 2;
    const qreal upperFreq = qreal(m_centerFrequency) + freqSpan / 2;

    // Ruler must stay locked to the panadapter centre (hardware LO). Do not
    // offset by m_deltaF / VFO-NCO: the spectrum is LO-relative, and shifting
    // the scale with click-VFO made a signal at centre read as centre−Δf
    // (e.g. click +4 kHz → peak labelled centre−4 kHz).
    const qreal plotLo = lowerFreq + qreal(plotLeft) * freqSpan / qreal(fullWidth);
    const qreal plotFreqSpan = upperFreq - plotLo;
    const qreal visibleLo = plotLo;
    const qreal visibleHi = visibleLo + plotFreqSpan;
    const qreal unit = qreal(plotWidth) / plotFreqSpan;
    const int fontMaxWidth = m_fonts.smallFontMetrics->boundingRect(QStringLiteral("000.000.0")).width();

    const QRect plotRect(0, 0, plotWidth, m_freqScalePanRect.height());
    m_frequencyScale = getXRuler(plotRect, fontMaxWidth, unit, visibleLo, visibleHi);
}

void QGLReceiverPanel::updateDBmRuler()
{
    if (!m_dBmScalePanRect.isValid())
        return;

    const qreal dBmRange = qAbs(m_dBmPanMax - m_dBmPanMin);
    if (dBmRange <= 0)
        return;

    int spacing = 7;
    int fontHeight;
    if (m_smallSize)
        fontHeight = m_fonts.smallFontMetrics->tightBoundingRect(QStringLiteral(".0dBm")).height() + spacing;
    else
        fontHeight = m_fonts.bigFont1Metrics->tightBoundingRect(QStringLiteral(".0dBm")).height() + spacing;

    const qreal unit = qreal(m_dBmScalePanRect.height()) / dBmRange;
    m_dBmScale = getYRuler2(m_dBmScalePanRect, fontHeight, unit, m_dBmPanMin, m_dBmPanMax);
}

void QGLReceiverPanel::drawPanadapterGrid() {
    if (m_overlayRenderer) {
        QMatrix4x4 projection;
        projection.ortho(0, size().width(), size().height(), 0, -10, 10);
        float alpha = (m_receiver == m_currentReceiver) ? 1.0f : 0.8f;
        m_overlayRenderer->drawGrid(projection, m_panRect, m_freqScalePanRect, m_frequencyScale, m_dBmScale,
                                  m_dBmScalePanRect.width(),
                                  m_redGrid, m_greenGrid, m_blueGrid, alpha, m_panGrid);
    }
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

    ensurePanelViewport();
    // Re-render text using the original logic which is already texture-based
    if (m_showFilterLeftBoundary) {
		QString str1 = QString("Filter Lo");
		QString str2 = frequencyString(m_filterLowerFrequency, true);
		m_glTextColor = QColor(0, 0, 0, 255);
		if (m_smallSize) {
			renderPanelText(m_oglTextSmall, m_filterLeft + 5, m_filterTop + 44, 4.0f, str1);
			renderPanelText(m_oglTextSmall, m_filterLeft + 5, m_filterTop + 64, 4.0f, str2);
		} else {
			renderPanelText(m_oglTextBig1, m_filterLeft + 5, m_filterTop + 44, 4.0f, str1);
			renderPanelText(m_oglTextBig1, m_filterLeft + 5, m_filterTop + 64, 4.0f, str2);
		}
		m_glTextColor = QColor(255, 255, 255, 255);
		if (m_smallSize) {
			renderPanelText(m_oglTextSmall, m_filterLeft + 3, m_filterTop + 42, 5.0f, str1);
			renderPanelText(m_oglTextSmall, m_filterLeft + 3, m_filterTop + 62, 5.0f, str2);
		} else {
			renderPanelText(m_oglTextBig1, m_filterLeft + 3, m_filterTop + 42, 5.0f, str1);
			renderPanelText(m_oglTextBig1, m_filterLeft + 3, m_filterTop + 62, 5.0f, str2);
		}
    }
    if (m_showFilterRightBoundary) {
		QString str1 = QString("Filter Hi");
		QString str2 = frequencyString(m_filterUpperFrequency, true);
		m_glTextColor = QColor(0, 0, 0, 255);
		if (m_smallSize) {
			renderPanelText(m_oglTextSmall, m_filterRight + 5, m_filterTop + 44, 4.0f, str1);
			renderPanelText(m_oglTextSmall, m_filterRight + 5, m_filterTop + 64, 4.0f, str2);
		} else {
			renderPanelText(m_oglTextBig1, m_filterRight + 5, m_filterTop + 44, 4.0f, str1);
			renderPanelText(m_oglTextBig1, m_filterRight + 5, m_filterTop + 64, 4.0f, str2);
		}
		m_glTextColor = QColor(255, 255, 255, 255);
		if (m_smallSize) {
			renderPanelText(m_oglTextSmall, m_filterRight + 3, m_filterTop + 42, 5.0f, str1);
			renderPanelText(m_oglTextSmall, m_filterRight + 3, m_filterTop + 62, 5.0f, str2);
		} else {
			renderPanelText(m_oglTextBig1, m_filterRight + 3, m_filterTop + 42, 5.0f, str1);
			renderPanelText(m_oglTextBig1, m_filterRight + 3, m_filterTop + 62, 5.0f, str2);
		}
    }
}

void QGLReceiverPanel::drawWaterfall() {
    if (!m_waterfallRenderer || m_waterfallRect.isEmpty())
        return;

    glDisable(GL_DEPTH_TEST);
    m_waterfallRenderer->render(panelProjection(), m_waterfallRect, m_waterfallPixel, m_dataEngineState);
}

void QGLReceiverPanel::drawWaterfallVerticalScale() {

    if (!m_secScaleWaterfallRect.isValid())
        return;

    const int width = m_secScaleWaterfallRect.width();
    const int height = m_secScaleWaterfallRect.height();

    if (width <= 0 || height <= 0 || width > 1000 || height > 10000)
        return;

    if (m_resizeTime.elapsed() < 500 && (width > size().width() || height > size().height()))
        return;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    const bool regenSecScale = !m_secScaleWaterfallFBO || m_secScaleWaterfallRenew || m_secScaleWaterfallUpdate;

    if (regenSecScale) {

        if (!m_secScaleWaterfallFBO || m_secScaleWaterfallRenew) {
            if (m_secScaleWaterfallFBO) {
                delete m_secScaleWaterfallFBO;
                m_secScaleWaterfallFBO = nullptr;
            }
            m_secScaleWaterfallFBO = new QOpenGLFramebufferObject(width, height);
        }

        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glViewport(0, 0, width, height);

        m_secScaleWaterfallFBO->bind();
        renderWaterfallVerticalScale();
        m_secScaleWaterfallFBO->release();

        m_secScaleWaterfallUpdate = false;
        m_secScaleWaterfallRenew = false;
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    }

    if (m_secScaleWaterfallFBO)
        drawCachedTexture(m_secScaleWaterfallRect, m_secScaleWaterfallFBO->texture(), 4.0f);
}

void QGLReceiverPanel::drawCrossHair() {
    if (!m_overlayRenderer) return;

    ensurePanelViewport();

    int mouseX = m_mousePos.x();
    int mouseY = m_mousePos.y();
    const int textOffset = 20;
    const int spacing = 6;

    QMatrix4x4 projection;
    projection.ortho(0, size().width(), size().height(), 0, -10, 10);
    m_overlayRenderer->drawCrossHair(projection, m_panRect, m_dBmScalePanRect, m_mousePos,
                                     (float)devicePixelRatioF(), size().height());

    // text	
    QString dFstr;
    QString fstr;
    QString dBstr;

    int dx = m_panRect.width()/2 - mouseX;
    qreal unit = displayedFrequencySpanHz() / m_panRect.width();
    qreal df = unit * dx;
    qreal frequency = m_centerFrequency - df;
    
    dFstr = frequencyString(m_deltaFrequency - df, true);
    fstr = frequencyString(frequency);

    qreal dBm = glPixelTodBm(m_panRect, m_dBmPanMax, m_dBmPanMin, mouseY);
    dBstr = QString::number(dBm, 'f', 1) + " dBm";

    int rectWidth;
    int fontHeight;
    if (m_smallSize) {
        rectWidth = m_fonts.smallFontMetrics->boundingRect(fstr).width();
        fontHeight = m_fonts.smallFontMetrics->tightBoundingRect("0").height() + spacing;
    } else {
        rectWidth = m_fonts.bigFont1Metrics->horizontalAdvance(fstr);
        fontHeight = m_fonts.bigFont1Metrics->tightBoundingRect("0").height() + spacing;
    }

    m_haircrossMaxRight = rectWidth + textOffset;
    m_smallSize ? m_haircrossMinTop = 40 : m_haircrossMinTop = 60;

    int tx, ty;
    if (mouseX > m_panRect.width() - m_haircrossMaxRight) {
        tx = mouseX - m_haircrossMaxRight;
        if (mouseY > m_haircrossMinTop)
            ty = m_smallSize ? mouseY - 42 : mouseY - 62;
        else
            ty = m_smallSize ? mouseY + 10 : mouseY + 30;
    } else {
        tx = mouseX + textOffset;
        if (mouseY > m_haircrossMinTop)
            ty = m_smallSize ? mouseY - 42 : mouseY - 62;
        else
            ty = m_smallSize ? mouseY + 10 : mouseY + 30;
    }

    m_glTextColor = QColor(200, 200, 200, 255);
    if (m_smallSize) {
        renderPanelText(m_oglTextSmall, tx, ty, 5.0f, dFstr);
        renderPanelText(m_oglTextSmall, tx, ty + fontHeight, 5.0f, fstr);
    } else {
        renderPanelText(m_oglTextBig1, tx, ty, 5.0f, dFstr);
        renderPanelText(m_oglTextBig1, tx, ty + fontHeight, 5.0f, fstr);
    }

    if (m_mouseRegion == panadapterRegion) {
        if (m_smallSize)
            renderPanelText(m_oglTextSmall, tx, ty + 2 * fontHeight, 5.0f, dBstr);
        else
            renderPanelText(m_oglTextBig1, tx, ty + 2 * fontHeight, 5.0f, dBstr);
    }

    if (m_oldMousePosX != mouseX) {
        m_bandText = getHamBandTextString(set->getHamBandTextList(), true, frequency);
        m_oldMousePosX = mouseX;
    }

    m_glTextColor = QColor(239, 209, 110, 255);
    if (m_smallSize)
        renderPanelText(m_oglTextSmall, tx, ty + 4 * fontHeight, 5.0f, m_bandText);
    else
        renderPanelText(m_oglTextBig1, tx, ty + 5 * fontHeight, 5.0f, m_bandText);
}

void QGLReceiverPanel::drawVFOControl() {

	// lock Panadapter
	QString str = "PAN LOCKED";
	int x1 = m_dBmScalePanRect.right() + 5;
	int y1 = 3;

	if (m_panLocked) {
		
		if (m_dataEngineState == QSDR::DataEngineUp) {
				
			m_glTextColor = QColor(0, 0, 0, 255);
			renderPanelText(m_oglTextSmall, x1 + 3, y1, 0.0f, str);
			m_glTextColor = QColor(255, 170, 90, 200);
			renderPanelText(m_oglTextSmall, x1 + 1, y1 - 2, 1.0f, str);
		}
		else {

			m_glTextColor = QColor(0, 0, 0, 255);
			renderPanelText(m_oglTextSmall, x1 + 3, y1, 0.0f, str);
			m_glTextColor = QColor(150, 150, 150, 100);
			renderPanelText(m_oglTextSmall, x1 + 1, y1 - 2, 1.0f, str);
		}
	}
	
	// click VFO
    x1 += m_fonts.smallFontMetrics->horizontalAdvance(str) + 12;
	str = "CLICK VFO";

	if (m_clickVFO) {

		if (m_dataEngineState == QSDR::DataEngineUp) {
				
			m_glTextColor = QColor(0, 0, 0, 255);
			renderPanelText(m_oglTextSmall, x1 + 3, y1, 0.0f, str);
			m_glTextColor = QColor(255, 170, 90, 200);
			renderPanelText(m_oglTextSmall, x1 + 1, y1 - 2, 1.0f, str);
		}
		else {

			m_glTextColor = QColor(0, 0, 0, 255);
			renderPanelText(m_oglTextSmall, x1 + 3, y1, 0.0f, str);
			m_glTextColor = QColor(150, 150, 150, 100);
			renderPanelText(m_oglTextSmall, x1 + 1, y1 - 2, 1.0f, str);
		}
	}

	// FFT size
		str = "sample size: %1";
        x1 = m_panRect.right() - m_fonts.smallFontMetrics->horizontalAdvance(str) - 65;

		if (m_dataEngineState == QSDR::DataEngineUp) {
				
			m_glTextColor = QColor(0, 0, 0, 255);
			renderPanelText(m_oglTextSmall, x1 + 3, y1, 0.0f, str.arg(m_sampleSize));
			m_glTextColor = QColor(255, 170, 90, 200);
			renderPanelText(m_oglTextSmall, x1 + 1, y1 - 2, 1.0f, str.arg(m_sampleSize));
		}
		
		str = "FFT: %1";
		//float res;
		QString s;

		switch (m_fftMult) {

			case 1:
				s = "4k";
				break;

			case 2:
				s = "8k";
				break;

			case 4:
				s = "16k";
				break;

			case 8:
				s = "32k";
				break;

			case 16:
				s = "64k";
				break;
		}
        x1 = m_panRect.right() - m_fonts.smallFontMetrics->horizontalAdvance(str) - 5;

		if (m_dataEngineState == QSDR::DataEngineUp) {
				
			m_glTextColor = QColor(0, 0, 0, 255);
			renderPanelText(m_oglTextSmall, x1 + 3, y1, 0.0f, str.arg(s));
			m_glTextColor = QColor(255, 170, 90, 200);
			renderPanelText(m_oglTextSmall, x1 + 1, y1 - 2, 1.0f, str.arg(s));
		}


	int delta = qRound((m_deltaF * m_panRect.width()) / displayedZoomFactor());
	//GRAPHICS_DEBUG << "delta = " << delta;

	if (delta > m_panRect.width()/2) {
	
		QColor col = QColor(255, 40, 40, 255);
		str = "<< VFO %1";
		str = str.arg(frequencyString(m_vfoFrequency, false));

		int x = m_dBmScalePanRect.right();
		int y = 25;

        QRect rect = QRect(x, y, m_fonts.smallFontMetrics->horizontalAdvance(str) + 4, m_fonts.fontHeightSmallFont + 2);
		drawPanelRect(rect, col, 2.0f);
		m_glTextColor = QColor(255, 255, 255, 255);
		renderPanelText(m_oglTextSmall, x + 1, y - 2, 3.0f, str);
	}

	if (delta < -m_panRect.width()/2) {
		
		QColor col = QColor(255, 40, 40, 255);
		str = "%1 VFO >>";
		str = str.arg(frequencyString(m_vfoFrequency, false));

        int x = m_panRect.right() - m_fonts.smallFontMetrics->horizontalAdvance(str);
		int y = 25;

        QRect rect = QRect(x, y, m_fonts.smallFontMetrics->horizontalAdvance(str) + 4, m_fonts.fontHeightSmallFont + 2);
		drawPanelRect(rect, col, 2.0f);
		m_glTextColor = QColor(255, 255, 255, 255);
		renderPanelText(m_oglTextSmall, x + 1, y - 2, 3.0f, str);
	}

	//qglColor(QColor(0, 0, 0));
	//m_oglTextSmall->renderFreqText(x1+1, y1-2, 3.0f, str);

	// set Center = VFO frequency button
	/*QColor col;
    x1 += m_fonts.smallFontMetrics->horizontalAdvance(str) + 7;
	str = "mid = vfo";

	if (m_dataEngineState == QSDR::DataEngineUp) {

		if (m_receiver == m_currentReceiver) {
		
			if (m_panLocked)
				col = QColor(1, 150, 140, 140);
			else
				col = QColor(1, 230, 220, 140);
		}
		else
			col = QColor(90, 100, 90, 140);
	}
	else
		col = m_darkColor;
	
    m_midToVfoButtonRect = QRect(x1, y1, m_fonts.smallFontMetrics->horizontalAdvance(str) + 5, m_fonts.fontHeightSmallFont + 2);
	drawPanelRect(m_midToVfoButtonRect, col, 2.0f);
	qglColor(QColor(0, 0, 0));
	m_oglTextSmall->renderFreqText(x1+1, y1-2, 3.0f, str);*/


	// set VFO = Center frequency button
    /*x1 += m_fonts.smallFontMetrics->horizontalAdvance(str) + 7;
	str = "vfo = mid";
	
	if (m_dataEngineState == QSDR::DataEngineUp) {

		if (m_receiver == m_currentReceiver)
			col = QColor(1, 230, 220, 140);
		else
			col = QColor(90, 100, 90, 140);
	}
	else
		col = m_darkColor;

    m_vfoToMidButtonRect = QRect(x1, y1, m_fonts.smallFontMetrics->horizontalAdvance(str) + 5, m_fonts.fontHeightSmallFont + 2);
	drawPanelRect(m_vfoToMidButtonRect, col, 2.0f);
	qglColor(QColor(0, 0, 0));
	m_oglTextSmall->renderFreqText(x1+1, y1-2, 3.0f, str);*/
}

void QGLReceiverPanel::drawReceiverInfo() {

	QString str;
    const int badgeH = m_fonts.fontHeightSmallFont + 2;

    auto drawRxBadge = [&](int &x, int y, const QString &label, const QColor &bg) {
        if (label.isEmpty())
            return;
        const int w = m_oglTextSmall->fontMetrics().horizontalAdvance(label) + 3;
        drawPanelRect(QRect(x, y, w, badgeH), bg, 2.0f);
        m_glTextColor = QColor(0, 0, 0, 255);
        renderPanelText(m_oglTextSmall, x + 1, y + 1, 2.0f, label);
        x += w + 4;
    };
	// mouse wheel freq step size
	/*if (m_dataEngineState == QSDR::DataEngineUp) {

		if (m_receiver == m_currentReceiver)
			col = QColor(1, 190, 180, 180);
		else
			col = QColor(1, 100, 90, 180);
	}
	else
		col = m_darkColor;

	str = "%1";
	str = str.arg(set->getValue1000(m_mouseWheelFreqStep, 0, "Hz"));

	int x1 = m_panRect.width() - (m_fonts.smallFontMetrics->tightBoundingRect(str).width() + 9);
	int y1 = 3;

	rect = QRect(x1+2, y1, m_fonts.smallFontMetrics->tightBoundingRect(str).width() + 5, m_fonts.fontHeightSmallFont + 2);
	drawPanelRect(rect, col, 2.0f);
	qglColor(QColor(0, 0, 0));
	m_oglTextSmall->renderFreqText(x1+3, y1-2, 3.0f, str);*/


	// AGC mode
	//if (m_dataEngineState == QSDR::DataEngineUp) {

	//	if (m_receiver == m_currentReceiver) {

	//		if (m_showAGCLines)
	//			col = QColor(255, 170, 90, 180);
	//		else
	//			col = QColor(215, 130, 50, 180);
	//	}
	//	else
	//		col = QColor(165, 80, 1);
	//}
	//else
	//	col = m_darkColor;

	//str = "%1";
	////str = str.arg(set->getAGCModeString(m_receiver));
	//str = str.arg(m_agcModeString);

	//x1 -= m_fonts.smallFontMetrics->tightBoundingRect(str).width() + 7;
	//y1 = 3;

	//m_agcButtonRect = QRect(x1+2, y1, m_fonts.smallFontMetrics->tightBoundingRect(str).width() + 5, m_fonts.fontHeightSmallFont + 2);
	//drawPanelRect(m_agcButtonRect, col, 2.0f);
	//qglColor(QColor(0, 0, 0));
	//m_oglTextSmall->renderFreqText(x1+3, y1-2, 3.0f, str);


	// main frequency display
	glDisable(GL_MULTISAMPLE);
	if (m_panRect.height() > 15) {

        const int fLength = m_fonts.bigFont1Metrics->horizontalAdvance(QStringLiteral("55.555.555")) + 30;
		int x = m_panRect.left() + qRound((qreal)(m_panRect.width() / 2.0f) - m_deltaF * m_panRect.width() / displayedZoomFactor()) + 10;
		if (x > m_panRect.right() - fLength)
            x -= fLength + 20;

		QColor colFlt;
		QColor colADC;
		QColor colAGC;
		QColor colDSP;

		if (m_dataEngineState == QSDR::DataEngineUp) {

			if (m_receiver == set->getCurrentReceiver()) {

				colDSP = QColor(1, 190, 180, 180);
				colFlt = QColor(200, 190, 50, 180);
				colADC = QColor(215, 130, 50, 180);
				if (m_showAGCLines)
					colAGC = QColor(255, 170, 90, 180);
				else
					colAGC = QColor(215, 130, 50, 180);
			}
			else {

				colFlt = QColor(110, 100, 1, 180);
				colDSP = QColor(1, 100, 90, 180);
				colAGC = QColor(165, 80, 1, 180);
				colADC = QColor(165, 80, 1, 180);
			}
		}
		else {

			colFlt = m_darkColor;
			colADC = m_darkColor;
			colAGC = m_darkColor;
			colDSP = m_darkColor;
		}

		int x1 = x;
		const int y1 = 3;
        drawRxBadge(x1, y1, m_filterWidthString, colFlt);
        drawRxBadge(x1, y1, m_dspModeString, colDSP);
        drawRxBadge(x1, y1, m_agcModeString, colAGC);
        drawRxBadge(x1, y1, m_adcModeString, colADC);

		TFrequency f;
		f.freqMHz = (int)(m_vfoFrequency / 1000);
		f.freqkHz = (int)(m_vfoFrequency % 1000);
        const int freqY = y1 + 20;
		str = "%1.%2";
		const int f1 = f.freqMHz;
		const int f2 = f.freqkHz;

		const QString fstr = str.arg(f1 / 1000).arg(f1 - 1000 * (int)(f1 / 1000), 3, 10, QLatin1Char('0'));
        const int fstrWidth = m_oglTextBig2->fontMetrics().horizontalAdvance(fstr);
        Q_UNUSED(f2)

        m_glTextColor = QColor(255, 255, 255, 255);
        renderPanelText(m_oglTextBig2, x + 2, freqY, fstr);
        renderPanelText(m_oglTextBig2, x + fstrWidth + 2, freqY, QStringLiteral("Mhz"));

	if (m_panRect.height() > 15 && m_deltaFrequency != 0) {

		f.freqMHz = (int)(m_centerFrequency / 1000);
		f.freqkHz = (int)(m_centerFrequency % 1000);

        const int centerY = freqY + 25;
        const int cf1 = f.freqMHz;
        const int cf2 = f.freqkHz;
        Q_UNUSED(cf2)

        const QString centerStr = str.arg(cf1 / 1000).arg(cf1 - 1000 * (int)(cf1 / 1000), 3, 10, QLatin1Char('0'));
        const int centerWidth = m_oglTextBig2->fontMetrics().horizontalAdvance(centerStr);

        m_glTextColor = QColor(255, 255, 255, 255);
        renderPanelText(m_oglTextBig2, x + 2, centerY, centerStr);
        renderPanelText(m_oglTextBig2, x + centerWidth + 2, centerY, QStringLiteral("Mhz"));
	}

    }
}

void QGLReceiverPanel::drawCwDecoderHUD() {
    const DSPMode mode = m_sliceModel ? m_sliceModel->dspMode() : m_dspMode;
    const DSPMode setMode = static_cast<DSPMode>(set->getDSPMode(m_receiver));
    const bool isCw = (mode == DSPMode::CWL || mode == DSPMode::CWU ||
                       m_dspMode == DSPMode::CWL || m_dspMode == DSPMode::CWU ||
                       setMode == DSPMode::CWL || setMode == DSPMode::CWU);
    if (!isCw || (m_sliceModel && !m_sliceModel->cwDecodeEnabled())) {
        m_cwTextRect = QRect();
        return;
    }

    const QString text = m_sliceModel ? m_sliceModel->cwDecodedText() : QString();
    const int wpm = m_sliceModel ? m_sliceModel->cwWpm() : 20;
    const bool toneOn = m_sliceModel ? m_sliceModel->cwToneActive() : false;

    ensurePanelViewport();

    // 1. Calculate the exact target CW tone frequency (+pitch for CWU, -pitch for CWL)
    const int cwPitch = set->getCwSidetoneFreq();
    const int trackedPitch = m_sliceModel ? m_sliceModel->cwTrackedPitch() : cwPitch;
    const qint64 vfoFreq = m_sliceModel ? m_sliceModel->frequency() : m_vfoFrequency;
    const qint64 centerFreq = m_sliceModel ? m_sliceModel->centerFrequency() : m_centerFrequency;

    // Tracked tone frequency for the dynamic green marker line
    const qint64 targetToneFreq = (mode == DSPMode::CWL) ? (vfoFreq - trackedPitch) : (vfoFreq + trackedPitch);
    // Nominal center frequency for steady, non-jittering text box placement
    const qint64 nominalToneFreq = (mode == DSPMode::CWL) ? (vfoFreq - cwPitch) : (vfoFreq + cwPitch);

    // 2. Compute screen X positions
    const float zoomFactor = displayedZoomFactor();
    const float sampleRate = (m_sampleRate > 0) ? (float)m_sampleRate : 48000.0f;
    const float targetDeltaF = (float)(targetToneFreq - centerFreq) / sampleRate;
    const float nominalDeltaF = (float)(nominalToneFreq - centerFreq) / sampleRate;

    const float cwX = (float)m_panRect.left() + ((float)m_panRect.width() / 2.0f) + (targetDeltaF * (float)m_panRect.width() / zoomFactor);
    const float nominalX = (float)m_panRect.left() + ((float)m_panRect.width() / 2.0f) + (nominalDeltaF * (float)m_panRect.width() / zoomFactor);

    const int panLeft = m_panRect.left();
    const int panRight = m_panRect.right();
    const int panTop = m_panRect.top();
    const int panBottom = m_panRect.bottom();

    const bool lineVisible = (cwX >= (float)panLeft && cwX <= (float)panRight);

    const int badgeH = m_fonts.fontHeightNormalFont + 6;
    const QString badgeText = (toneOn && std::abs(trackedPitch - cwPitch) > 6)
        ? QStringLiteral("CW %1W %2H").arg(wpm > 0 ? wpm : 20).arg(trackedPitch)
        : QStringLiteral("CW %1 WPM").arg(wpm > 0 ? wpm : 20);
    // Fixed constant badge width to prevent horizontal jitter
    const int badgeW = m_oglTextSmall->fontMetrics().horizontalAdvance(QStringLiteral("CW 99W 9999H")) + 16;

    QString displayStr = text;
    if (displayStr.isEmpty()) {
        displayStr = (toneOn && std::abs(trackedPitch - cwPitch) > 6)
            ? QStringLiteral("<%1Hz CW>").arg(trackedPitch)
            : QStringLiteral("<%1Hz CW>").arg(cwPitch);
    }

    const int maxChars = 40;
    const int maxTextW = m_oglTextNormal 
        ? qMax(280, m_oglTextNormal->fontMetrics().averageCharWidth() * maxChars + 24)
        : 320;
    const int defaultBoxW = badgeW + maxTextW + 14;

    // Determine CW Box coordinates: custom user-dragged or default anchored position
    int textX = qRound(nominalX) + 6;
    int textY = panTop + (m_panRect.height() / 2) - (badgeH / 2);

    if (m_hasCustomCwBoxPos) {
        textX = m_cwBoxPos.x();
        textY = m_cwBoxPos.y();
    } else if (textX + defaultBoxW > panRight) {
        // Shift to left of nominal line if near right panadapter border
        textX = qRound(nominalX) - 6 - defaultBoxW;
    }

    textX = qBound(panLeft + 4, textX, panRight - 80);
    textY = qBound(panTop + 4, textY, panBottom - badgeH - 4);

    const int availableW = panRight - textX - 8;
    if (availableW > 60) {
        const int textAvailableW = qBound(20, availableW - badgeW - 10, maxTextW);
        QString trimmedText = displayStr;
        if (trimmedText.length() > maxChars) {
            trimmedText = trimmedText.right(maxChars);
        }
        while (!trimmedText.isEmpty() && m_oglTextNormal->fontMetrics().horizontalAdvance(trimmedText) > textAvailableW) {
            trimmedText.remove(0, 1);
        }

        const int totalW = badgeW + (trimmedText.isEmpty() ? 0 : m_oglTextNormal->fontMetrics().horizontalAdvance(trimmedText) + 8) + 6;
        m_cwTextRect = QRect(textX, textY, totalW, badgeH);

        const int boxMidY = textY + (badgeH / 2);
        const int boxLeftX = textX;
        const int boxRightX = textX + totalW;

        // Dynamic pitch marker color (glowing green on tone, soft emerald when idle)
        const QColor lineColor = toneOn ? QColor(50, 240, 130, 230) : QColor(35, 170, 110, 140);

        if (lineVisible) {
            const int lineX = qRound(cwX);

            // Find the spectrum curve/peak Y coordinate at lineX
            int signalPeakY = panBottom - 12;
            const int relX = lineX - panLeft;
            if (!m_panadapterBins.isEmpty() && relX >= 0 && relX < m_panadapterBins.size()) {
                const qreal dBmRange = qMax(10.0, m_dBmPanMax - m_dBmPanMin);
                const qreal yScale = (qreal)m_panRect.height() / dBmRange;

                // Sample a 3-bin window around the line to find the local peak Y
                qreal maxBinVal = m_panadapterBins.at(relX);
                if (relX > 0) maxBinVal = qMax(maxBinVal, m_panadapterBins.at(relX - 1));
                if (relX < m_panadapterBins.size() - 1) maxBinVal = qMax(maxBinVal, m_panadapterBins.at(relX + 1));

                const int curveY = panBottom - qRound(yScale * maxBinVal);
                signalPeakY = qBound(panTop + 10, curveY, panBottom - 4);
            }

            // Downward arrow tip stops 8px above the spectrum peak
            const int arrowTipY = qBound(panTop + 16, signalPeakY - 8, panBottom - 6);

            // 1. Vertical Arrow Shaft extending from box level down to arrow tip
            if (boxMidY < arrowTipY - 6) {
                drawPanelRect(QRect(lineX - 1, boxMidY, 2, arrowTipY - 6 - boxMidY), lineColor, 3.5f);
            } else if (boxMidY > arrowTipY + 6) {
                drawPanelRect(QRect(lineX - 1, arrowTipY + 6, 2, boxMidY - (arrowTipY + 6)), lineColor, 3.5f);
            }

            // 2. Downward Arrowhead pointing directly at the CW RF peak (at lineX, arrowTipY)
            drawPanelRect(QRect(lineX - 4, arrowTipY - 6, 9, 2), lineColor, 3.6f);
            drawPanelRect(QRect(lineX - 3, arrowTipY - 4, 7, 2), lineColor, 3.6f);
            drawPanelRect(QRect(lineX - 2, arrowTipY - 2, 5, 2), lineColor, 3.6f);
            drawPanelRect(QRect(lineX - 1, arrowTipY,     3, 2), lineColor, 3.6f);

            // 3. Horizontal Green Guide / Leader Line extending to the LHS of the CW display box
            if (boxLeftX > lineX) {
                // Box is to the right of the signal: leader line extends from lineX to boxLeftX
                drawPanelRect(QRect(lineX, boxMidY - 1, qMax(1, boxLeftX - lineX), 2), lineColor, 3.5f);
            } else if (boxRightX < lineX) {
                // Box is to the left of the signal: leader line extends from boxRightX to lineX
                drawPanelRect(QRect(boxRightX, boxMidY - 1, qMax(1, lineX - boxRightX), 2), lineColor, 3.5f);
            }
            // Small guide pip at the attachment point on the LHS of the box
            drawPanelRect(QRect(boxLeftX - 3, boxMidY - 2, 4, 4), lineColor, 3.6f);
        }

        // Background container (stable dark slate)
        drawPanelRect(m_cwTextRect, QColor(10, 14, 20, 215), 3.4f);

        // WPM Badge (stable dark badge)
        drawPanelRect(QRect(textX + 2, textY + 2, badgeW, badgeH - 4), QColor(28, 38, 48, 230), 3.5f);

        // Tone indicator dot (lights green on active mark tone)
        drawPanelRect(QRect(textX + 5, textY + (badgeH / 2) - 2, 5, 5),
                      toneOn ? QColor(60, 240, 130) : QColor(90, 110, 130), 3.6f);

        m_glTextColor = Qt::white;
        renderPanelText(m_oglTextSmall, float(textX + 14), float(textY + 3), 3.6f, badgeText);

        // Decoded text in high-contrast gold
        if (!trimmedText.isEmpty()) {
            m_glTextColor = text.isEmpty() ? QColor(130, 160, 180, 190) : QColor(255, 235, 130, 255);
            renderPanelText(m_oglTextNormal, float(textX + badgeW + 6), float(textY + 2), 3.6f, trimmedText);
        }
    } else {
        m_cwTextRect = QRect();
    }
}

void QGLReceiverPanel::drawAGCControl() {
    if (m_overlayRenderer) {
        QMatrix4x4 projection;
        projection.ortho(0, size().width(), size().height(), 0, -10, 10);
        m_overlayRenderer->drawAGCControl(projection, m_panRect, m_dBmScalePanRect, m_agcMode, m_agcHangEnabled, m_agcThresholdOld, m_agcHangLevelOld, m_agcFixedGain, m_dBmPanMax, m_dBmPanMin, (float)devicePixelRatio(), size().height(), m_agcThresholdPixel, m_agcHangLevelPixel, m_agcFixedGainLevelPixel);

        // Text rendering remains here for now
        if (m_agcMode == (AGCMode) agcOFF) {
            QString str = "AGC-F";
            m_glTextColor = QColor(0, 0, 0, 255);
            renderPanelText(m_oglTextSmall, m_panRect.right() - 32, m_agcFixedGainLevelPixel - 13, 4.0f, str);
            m_glTextColor = QColor(225, 125, 225, 255);
            renderPanelText(m_oglTextSmall, m_panRect.right() - 34, m_agcFixedGainLevelPixel - 15, 5.0f, str);
        } else {
            QString str = "AGC-T";
            m_glTextColor = QColor(0, 0, 0, 255);
            renderPanelText(m_oglTextSmall, m_panRect.right() - 32, m_agcThresholdPixel - 13, 4.0f, str);
            m_glTextColor = QColor(225, 125, 125, 255);
            renderPanelText(m_oglTextSmall, m_panRect.right() - 34, m_agcThresholdPixel - 15, 5.0f, str);
            if (m_agcHangEnabled) {
                str = "AGC-H";
                m_glTextColor = QColor(0, 0, 0, 255);
                renderPanelText(m_oglTextSmall, m_panRect.right() - 32, m_agcHangLevelPixel - 13, 4.0f, str);
                m_glTextColor = QColor(125, 225, 125, 255);
                renderPanelText(m_oglTextSmall, m_panRect.right() - 34, m_agcHangLevelPixel - 15, 5.0f, str);
            }
        }
    }
}
 


void QGLReceiverPanel::renderPanHorizontalScale() {

	//GRAPHICS_DEBUG << "render frequency scale";
	int fontHeight;
    QColor textColor = QColor(140, 180, 200);
    const qreal freqSpan = displayedFrequencySpanHz();
    qreal upperFreq = (qreal)m_centerFrequency + freqSpan / 2;

    const int plotLeft = m_dBmScalePanRect.isValid() ? m_dBmScalePanRect.width() : 0;
    const int fullWidth = m_freqScalePanRect.width();

    QOpenGLPaintDevice paintDevice(m_frequencyScaleFBO->size());
    painter.begin(&paintDevice);

    fontHeight = m_fonts.smallFontMetrics->tightBoundingRect(".0kMGHz").height();

	// draw the frequency scale
	int		offset_X		= -1;
	int		textOffset_y	= 5;
	double	freqScale		= 1;

	QString fstr = QString(" Hz ");
	if (upperFreq >= 1e6) { freqScale = 1e6; fstr = QString("  MHz "); }
	else
	if (upperFreq >= 1e3) { freqScale = 1e3; fstr = QString("  kHz "); }

    auto formatFreqLabel = [&](qreal freqHz, bool allowExtraDigit) -> QString {
        const int decimals = (allowExtraDigit && freqScale >= 1e3) ? 4 : 3;
        QString str = QString::number(freqHz / freqScale, 'f', decimals);
        if (freqScale == 1e3 && decimals == 3)
            while (str.endsWith('0')) str.remove(str.size() - 1, 1);
        if (str.endsWith('.')) str.remove(str.size() - 1, 1);
        return str;
    };

	// Full-width background; ruler positions are plot-relative (see updateFrequencyRuler).
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(0, 0, fullWidth, m_freqScalePanRect.height(), QColor(0, 0, 0, 255));
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    QRect scaledTextRect(0, textOffset_y, 1, m_freqScalePanRect.height());
    scaledTextRect.setWidth(m_fonts.smallFontMetrics->horizontalAdvance(fstr));
    scaledTextRect.moveLeft(fullWidth - scaledTextRect.width());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setFont(m_oglTextSmall->font());
    painter.setPen(QPen(textColor,3, Qt::SolidLine, Qt::FlatCap));

	int len = m_frequencyScale.mainPointPositions.length();
	int lastLabelRight = plotLeft;

	if (len > 0) {

		for (int i = 0; i < len; i++) {
            const int screenX = plotLeft + m_frequencyScale.mainPointPositions.at(i);
            if (screenX < plotLeft || screenX > fullWidth)
                continue;
            painter.drawLine(screenX, 1, screenX, 4);
		}

		for (int i = 0; i < len; i++) {
            const int tickX = m_frequencyScale.mainPointPositions.at(i);
            const int screenX = plotLeft + tickX;
            if (screenX < plotLeft || screenX > fullWidth)
                continue;

			QString str = formatFreqLabel(m_frequencyScale.mainPoints.at(i), tickX == 0);

            int textWidth = m_fonts.smallFontMetrics->horizontalAdvance(str);

            int textX = (tickX == 0) ? (screenX + 2) : (screenX + offset_X - (textWidth / 2));

            if (textX < plotLeft + 2)
                textX = plotLeft + 2;

            QRect textRect(textX, textOffset_y, textWidth, fontHeight);

			if (textRect.right() >= scaledTextRect.left()) continue;
            if (textRect.left() < lastLabelRight + 2) continue;

             painter.drawText(textRect.x(), textRect.y() + m_oglTextSmall->fontMetrics().height(), str);
             lastLabelRight = textRect.right();

		}
	}

    len = m_frequencyScale.subPointPositions.length();
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            const int screenX = plotLeft + m_frequencyScale.subPointPositions.at(i);
            if (screenX < plotLeft || screenX > fullWidth)
                continue;
            painter.drawLine(screenX, 1, screenX, 3);
        }
    }

	painter.setPen(QPen(QColor(239, 56, 109)));
     painter.drawText(fullWidth - 30, textOffset_y + 10, fstr);
    painter.end();
}

void QGLReceiverPanel::renderPanadapterGrid()
{
    // Clear to transparent so only grid lines are visible
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(2.0f);

    QVector<GlDraw::Vec3Rgb> verts;
    verts.reserve((m_frequencyScale.mainPointPositions.length()
                   + m_dBmScale.mainPointPositions.length()) * 2);

    const float r = m_redGrid;
    const float g = m_greenGrid;
    const float b = m_blueGrid;

    int len = m_frequencyScale.mainPointPositions.length();
    if (len > 0) {
        const float y1 = 0.0f;
        const float y2 = float(m_panRect.height() - 1);

        for (int i = 0; i < len; i++) {
            const float x = float(m_frequencyScale.mainPointPositions.at(i) - m_panRect.left());
            if (x >= 0.0f && x < float(m_panRect.width())) {
                verts.append({ x, y1, 0.0f, r, g, b });
                verts.append({ x, y2, 0.0f, r, g, b });
            }
        }
    }

    len = m_dBmScale.mainPointPositions.length();
    if (len > 0) {
        const float x1 = 0.0f;
        const float x2 = float(m_panRect.width() - 1);

        for (int i = 0; i < len; i++) {
            const float y = float(m_dBmScale.mainPointPositions.at(i) - m_panRect.top());
            if (y >= 0.0f && y < float(m_panRect.height())) {
                verts.append({ x1, y, 0.0f, r, g, b });
                verts.append({ x2, y, 0.0f, r, g, b });
            }
        }
    }

    if (!verts.isEmpty() && m_shaderProgram && m_shaderProgram->isLinked()) {
        QMatrix4x4 projection;
        projection.ortho(0, m_panRect.width(), m_panRect.height(), 0, -10, 10);
        m_vao.bind();
        GlDraw::drawColoredLines(this, m_shaderProgram, m_vbo, projection,
                                 verts.constData(), verts.size());
    }

    glDisable(GL_BLEND);
}
 
void QGLReceiverPanel::renderWaterfallVerticalScale() {

    if (!m_secScaleWaterfallFBO)
        return;

    QString str;
    const int spacing = 7;
    int fontHeight;
    int fontMaxWidth;
    const QFontMetrics *labelMetrics;

    if (m_smallSize) {
        fontHeight = m_fonts.smallFontMetrics->tightBoundingRect(QStringLiteral(".0s")).height() + spacing;
        fontMaxWidth = m_fonts.smallFontMetrics->boundingRect(QStringLiteral("000.0")).width();
        labelMetrics = m_fonts.smallFontMetrics;
    } else {
        fontHeight = m_fonts.bigFont1Metrics->tightBoundingRect(QStringLiteral(".0s")).height() + spacing;
        fontMaxWidth = m_fonts.bigFont1Metrics->boundingRect(QStringLiteral("000.0")).width();
        labelMetrics = m_fonts.bigFont1Metrics;
    }

    const int width = m_secScaleWaterfallRect.width();
    const int height = m_secScaleWaterfallRect.height();
    const qreal secRange = qAbs(m_secWaterfallMax - m_secWaterfallMin);
    if (secRange <= 0)
        return;

    const qreal unit = qreal(height) / secRange;
    m_secScale = getYRuler2(m_secScaleWaterfallRect, fontHeight, unit, m_secWaterfallMin, m_secWaterfallMax);

    QOpenGLPaintDevice paintDevice(m_secScaleWaterfallFBO->size());
    painter.begin(&paintDevice);

    QRect textRect(0, 0, fontMaxWidth, fontHeight);
    textRect.moveLeft(m_smallSize ? 3 : -1);

    const int len = m_secScale.mainPointPositions.length();
    const int sublen = m_secScale.subPointPositions.length();

    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(0, 0, width, height, QColor(40, 40, 40, 180));
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    painter.setPen(QPen(QColor(166, 194, 206), 1, Qt::SolidLine, Qt::FlatCap));
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            const int y = m_secScale.mainPointPositions.at(i);
            painter.drawLine(width, y, width - 4, y);
        }
        painter.setPen(QPen(QColor(115, 143, 155), 1, Qt::SolidLine, Qt::FlatCap));
        for (int i = 0; i < sublen; i++) {
            const int y = m_secScale.subPointPositions.at(i);
            painter.drawLine(width, y, width - 2, y);
        }
    }

    painter.setPen(QPen(QColor(242, 245, 232)));
    painter.setFont(m_smallSize ? m_fonts.smallFont : m_fonts.bigFont1);

    int yOld = -textRect.height();
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            textRect.moveBottom(m_secScale.mainPointPositions.at(i) + textRect.height() / 2);
            if (textRect.y() >= yOld && textRect.bottom() <= (height - textRect.height())) {
                str = QString::number(m_secScale.mainPoints.at(i), 'f', 1);
                const int textX = textRect.x() + fontMaxWidth - labelMetrics->horizontalAdvance(str);
                painter.drawText(textX, textRect.y() + fontHeight, str);
                yOld = textRect.bottom();
            }
        }
    }

    textRect.moveTop(height - textRect.height());
    painter.setPen(QPen(QColor(239, 56, 109)));
    str = QStringLiteral("sec");
    const int secLabelX = m_smallSize ? textRect.x() : textRect.x() + 10;
    painter.drawText(secLabelX, textRect.y() + fontHeight, str);
    painter.end();
}

//********************************************************************

void QGLReceiverPanel::getRegion(QPoint p) {

	//QRect mouse_rect(0, 0, 100, 100);
	//mouse_rect.moveCenter(p);
	/*if (m_lockedPanButtonRect.contains(p)) {

		m_mouseRegion = lockedPanButtonRegion;
	}
	else if (m_vfoToMidButtonRect.contains(p)) {

		m_mouseRegion = vfoToMidButtonRegion;
	}
	else if (m_midToVfoButtonRect.contains(p)) {

		m_mouseRegion = midToVfoButtonRegion;
	}*/
	if (m_agcButtonRect.contains(p)) {

		m_mouseRegion = agcButtonRegion;
	}

	else if (m_freqScalePanRect.contains(p)) {

		m_mouseRegion = freqScalePanadapterRegion;
		
	}
	else if (m_dBmScalePanRect.contains(p)) {

		m_mouseRegion = dBmScalePanadapterRegion;

	}
	else if (m_filterRect.width() > 0
			 && qAbs(p.x() - m_filterRect.left()) < m_snapMouse
			 && ((m_panRect.contains(p) && !m_dBmScalePanRect.contains(p))
			     || (m_waterfallRect.contains(p) && !m_secScaleWaterfallRect.contains(p))))
	{
		m_mouseRegion = filterRegionLow;
		m_mouseDownFilterFrequencyLo = m_filterLowerFrequency;
	}
	else if (m_filterRect.width() > 0
			 && qAbs(p.x() - m_filterRect.right()) < m_snapMouse
			 && ((m_panRect.contains(p) && !m_dBmScalePanRect.contains(p))
			     || (m_waterfallRect.contains(p) && !m_secScaleWaterfallRect.contains(p))))
	{
		m_mouseRegion = filterRegionHigh;
		m_mouseDownFilterFrequencyHi = m_filterUpperFrequency;
	}
	else if (m_filterRect.contains(p)) {

		m_mouseRegion = filterRegion;

	}
	else if (m_filterRect.width() > 0
			 && p.x() >= m_filterRect.left() && p.x() <= m_filterRect.right()
			 && m_waterfallRect.contains(p) && !m_secScaleWaterfallRect.contains(p)) {

		m_mouseRegion = filterRegion;

	}
	//else if (qAbs(p.y() - m_agcThresholdPixel) < m_snapMouse && !m_crossHairCursor) {
	else if (qAbs(p.y() - m_agcThresholdPixel) < m_snapMouse) {

		m_mouseRegion = agcThresholdLine;
		m_mouseDownAGCThreshold = m_agcThresholdOld;
	}
	//else if (qAbs(p.y() - m_agcHangLevelPixel) < m_snapMouse && !m_crossHairCursor) {
	else if (qAbs(p.y() - m_agcHangLevelPixel) < m_snapMouse) {

		m_mouseRegion = agcHangLine;
		m_mouseDownAGCHangLevel = m_agcHangLevelOld;
	}
	//else if (qAbs(p.y() - m_agcFixedGainLevelPixel) < m_snapMouse && !m_crossHairCursor) {
	else if (qAbs(p.y() - m_agcFixedGainLevelPixel) < m_snapMouse) {

		m_mouseRegion = agcFixedGainLine;
		m_mouseDownFixedGainLevel = -m_agcFixedGain;
	}
	else if (m_panRect.contains(p)) {
		m_mouseRegion = panadapterRegion;
	}
	else if (m_waterfallRect.contains(p)) {

		m_mouseRegion = waterfallRegion;

	}
	else
		m_mouseRegion = elsewhere;
	//GRAPHICS_DEBUG << "region" << m_mouseRegion;
}

void QGLReceiverPanel::resizeGL(int iWidth, int iHeight) {
    
    if (m_dBmScaleFBO) { delete m_dBmScaleFBO; m_dBmScaleFBO = nullptr; }
    if (m_frequencyScaleFBO) { delete m_frequencyScaleFBO; m_frequencyScaleFBO = nullptr; }
    if (m_secScaleWaterfallFBO) { delete m_secScaleWaterfallFBO; m_secScaleWaterfallFBO = nullptr; }
    
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

	setFocus(Qt::MouseFocusReason);

	m_mousePos = QPoint(-1, -1);
	m_mouseRegion = elsewhere;

	setCursor(Qt::BlankCursor);

	QOpenGLWidget::enterEvent(event);
}

void QGLReceiverPanel::leaveEvent(QEnterEvent *event) {

	m_mousePos = QPoint(-100, -100);
	m_mouseRegion = elsewhere;
	QOpenGLWidget::leaveEvent(event);
}

void QGLReceiverPanel::wheelEvent(QWheelEvent* event) {

    getRegion(event->position().toPoint());  // mouse pos set by mouseMoveEvent
	double freqStep = set->getMouseWheelFreqStep(m_currentReceiver);

	switch (m_mouseRegion) {

		case panadapterRegion:
		case waterfallRegion:
		case filterRegion:
		case filterRegionLow:
		case filterRegionHigh:

			double delta = 0;
            if (event->angleDelta().y() < 0)
				delta = -freqStep;
            else if (event->angleDelta().y() > 0)
				delta =  freqStep;

			if (!m_panLocked) {
			
				if (m_centerFrequency + delta > set->getMaxFrequency())
					m_centerFrequency = set->getMaxFrequency();
				else if (m_centerFrequency + delta < set->getMinFrequency())
					m_centerFrequency = set->getMinFrequency();
				else
					// snap to the frequency step
					m_centerFrequency = (long)(qRound((m_centerFrequency + delta) / qAbs(freqStep)) * qAbs(freqStep));

				m_vfoFrequency = m_centerFrequency - m_deltaFrequency;
			}
			else {

				if (m_vfoFrequency + delta > m_centerFrequency + m_sampleRate/2)
					m_vfoFrequency = m_centerFrequency + m_sampleRate/2;
				else if (m_vfoFrequency + delta < m_centerFrequency - m_sampleRate/2)
					m_vfoFrequency = m_centerFrequency - m_sampleRate/2;
				else
					// snap to the frequency step
					m_vfoFrequency = (long)(qRound((m_vfoFrequency + delta) / qAbs(freqStep)) * qAbs(freqStep));

				m_deltaFrequency = m_centerFrequency - m_vfoFrequency;
				m_deltaF = (qreal)(1.0*m_deltaFrequency/m_sampleRate);
				//GRAPHICS_DEBUG << "m_deltaFrequency: " << m_deltaFrequency;
			}
			
			set->setCtrFrequency(0, m_receiver, m_centerFrequency);
			set->setVFOFrequency(0, m_receiver, m_vfoFrequency);
			break;
	}

 	//updateGL();
}

void QGLReceiverPanel::mousePressEvent(QMouseEvent* event) {
	
	// Right-click on decoded CW text box erases the text
	if (event->button() == Qt::RightButton) {
		if (m_cwTextRect.isValid() && m_cwTextRect.contains(event->pos())) {
			if (m_sliceModel) {
				m_sliceModel->setCwDecodedText(QString());
			}
			update();
			event->accept();
			return;
		}
	}

	// Left-click on decoded CW text box starts movable dragging
	if (event->button() == Qt::LeftButton && m_cwTextRect.isValid() && m_cwTextRect.contains(event->pos())) {
		m_dragCwText = true;
		m_cwDragStartMouse = event->pos();
		if (!m_hasCustomCwBoxPos) {
			m_cwBoxPos = m_cwTextRect.topLeft();
			m_hasCustomCwBoxPos = true;
		}
		setCursor(Qt::ClosedHandCursor);
		event->accept();
		return;
	}

	//GRAPHICS_DEBUG << "mousePressEvent";
	m_mousePos = event->pos();
	m_mouseDownPos = m_mousePos;

	getRegion(m_mousePos);

	// Click-to-tune (Click-VFO or Shift+Click) on panadapter or waterfall
	if (event->button() == Qt::LeftButton && (m_clickVFO || (event->modifiers() & Qt::ShiftModifier))) {
		if (m_panRect.contains(m_mousePos) || m_waterfallRect.contains(m_mousePos)) {
			m_dragMouse = false;
			m_highlightFilter = false;

			if (m_receiver != set->getCurrentReceiver()) {
				set->setCurrentReceiver(m_receiver);
			}

			const int dx = m_panRect.width()/2 - m_mousePos.x();
			const qreal unit = displayedFrequencySpanHz() / m_panRect.width();
			qint64 clickedFreq = (qint64)(qRound(m_centerFrequency - (unit * dx)));

			const DSPMode mode = m_sliceModel ? m_sliceModel->dspMode() : m_dspMode;
			const bool isCw = (mode == DSPMode::CWL || mode == DSPMode::CWU);
			const int cwPitch = set->getCwSidetoneFreq();

			// Auto-snap to nearest peak within ~25 pixels if clicking near a CW carrier or with Shift key
			bool peakFound = false;
			const qint64 peakRf = findPeakFrequencyNear(clickedFreq, qMax(600, qRound(unit * 25.0)), &peakFound);

			qint64 newVfo = clickedFreq;
			if (peakFound && isCw) {
				// Zero-beat snap for CW
				newVfo = (mode == DSPMode::CWL) ? (peakRf + cwPitch) : (peakRf - cwPitch);
			} else if (peakFound && (event->modifiers() & Qt::ShiftModifier)) {
				newVfo = peakRf;
			}

			newVfo = qBound(m_centerFrequency - m_sampleRate/2, newVfo, m_centerFrequency + m_sampleRate/2);

			m_vfoFrequency = newVfo;
			m_deltaFrequency = m_centerFrequency - m_vfoFrequency;
			m_deltaF = (qreal)(1.0 * m_deltaFrequency / m_sampleRate);

			set->setVFOFrequency(0, m_receiver, m_vfoFrequency);
			update();
			event->accept();
			return;
		}
	}

	if (m_mouseRegion == agcButtonRegion) {

		if (event->buttons() == Qt::LeftButton) {

			if (m_showAGCLines) {

				m_showAGCLines = false;
				set->setAGCShowLines(m_receiver, false);
			}
			else {

				m_showAGCLines = true;
				set->setAGCShowLines(m_receiver, true);
			}
		}
	}
	else if (m_mouseRegion == panadapterRegion || m_mouseRegion == waterfallRegion) {

		if (event->buttons() == Qt::LeftButton && m_receiver != set->getCurrentReceiver()) {

			set->setCurrentReceiver(m_receiver);
		}
		else if (event->buttons() == Qt::LeftButton) {

			m_crossHairCursor = false;
			if (cursor().shape() != Qt::OpenHandCursor)
				setCursor(Qt::OpenHandCursor);
			m_dragMouse = true;
		}
		else if (event->buttons() == Qt::RightButton) {

			showRadioPopup(true);
		}
	}
	else if (m_mouseRegion == filterRegion) {

		if (event->buttons() == Qt::LeftButton) {
			m_highlightFilter = true;
			m_mouseDownFilterFrequencyLo = m_filterLowerFrequency;
			m_mouseDownFilterFrequencyHi = m_filterUpperFrequency;
		}
	}
	else if (m_mouseRegion == filterRegionLow || m_mouseRegion == filterRegionHigh) {

		if (event->buttons() == Qt::LeftButton) {
			m_mouseDownFilterFrequencyLo = m_filterLowerFrequency;
			m_mouseDownFilterFrequencyHi = m_filterUpperFrequency;
		}
	}
	else if (m_mouseRegion == freqScalePanadapterRegion) {

		m_rulerMouseDownPos = m_freqScalePanRect.topLeft();

		if (event->buttons() == Qt::LeftButton || event->buttons() == Qt::RightButton)
			m_dragFreqScale = true;
		m_dragFreqScaleZoom = (event->button() == Qt::RightButton);
		if (event->buttons() == Qt::RightButton)
			setCursor(Qt::SplitHCursor);

		return;
	}
	else if (m_mouseRegion == dBmScalePanadapterRegion) {

		m_rulerMouseDownPos = m_dBmScalePanRect.topLeft();

		if (event->buttons() == Qt::LeftButton)
			m_dragDBmScale = true;
		else if (event->buttons() == Qt::RightButton)
			setCursor(Qt::SplitVCursor);
		return;
	}
	
}

void QGLReceiverPanel::mouseReleaseEvent(QMouseEvent *event) {

	if (m_dragCwText) {
		m_dragCwText = false;
		if (cursor().shape() != Qt::ArrowCursor)
			setCursor(Qt::ArrowCursor);
		update();
		event->accept();
		return;
	}

	//GRAPHICS_DEBUG << "mouseReleaseEvent";
	m_mousePos = event->pos();
	m_mouseDownPos = m_mousePos;

	getRegion(m_mousePos);

	if (m_mouseRegion == freqScalePanadapterRegion) {

		m_dragFreqScale = false;
		m_dragFreqScaleZoom = false;
		m_freqScalePanadapterUpdate = true;
		if (m_crossHair) {
			if (cursor().shape() != Qt::BlankCursor)
				setCursor(Qt::BlankCursor);
		} else if (cursor().shape() != Qt::ArrowCursor) {
			setCursor(Qt::ArrowCursor);
		}
		update();
		return;
	}
	//else if (m_mouseRegion == panadapterRegion || m_mouseRegion == waterfallRegion) {
	//}
	const bool wasDragging = m_dragMouse || m_highlightFilter
	                         || m_showFilterLeftBoundary || m_showFilterRightBoundary;
	m_dragMouse = false;
	m_dragDBmScale = false;
	m_dragFreqScale = false;
	m_dragFreqScaleZoom = false;
	m_showFilterLeftBoundary = false;
	m_showFilterRightBoundary = false;
	m_highlightFilter = false;
	m_freqScalePanadapterUpdate = true;
	m_dBmScalePanadapterUpdate = true;
	m_crossHairCursor = true;
	if (m_crossHair) {
		if (cursor().shape() != Qt::BlankCursor)
			setCursor(Qt::BlankCursor);
	} else if (cursor().shape() != Qt::ArrowCursor) {
		setCursor(Qt::ArrowCursor);
	}
	// Spectrum frames already refresh the pan; only force a paint when ending a drag overlay.
	if (wasDragging)
		update();
}

void QGLReceiverPanel::mouseDoubleClickEvent(QMouseEvent *event) {

	m_mousePos = event->pos();
	m_mouseDownPos = m_mousePos;

	getRegion(m_mousePos);

	if (m_mouseRegion == panadapterRegion || m_mouseRegion == waterfallRegion) {

		if (event->button() == Qt::LeftButton) {

			const int dx = m_panRect.width()/2 - m_mousePos.x();
			const qreal unit = displayedFrequencySpanHz() / m_panRect.width();
			qint64 clickedFreq = (qint64)(qRound(m_centerFrequency - (unit * dx)));

			const DSPMode mode = m_sliceModel ? m_sliceModel->dspMode() : m_dspMode;
			const bool isCw = (mode == DSPMode::CWL || mode == DSPMode::CWU);
			const int cwPitch = set->getCwSidetoneFreq();

			// Auto-snap to nearest spectral peak within ±35 pixels
			bool peakFound = false;
			const qint64 peakRf = findPeakFrequencyNear(clickedFreq, qMax(800, qRound(unit * 35.0)), &peakFound);

			qint64 newVfo = clickedFreq;
			if (peakFound && isCw) {
				// Zero-beat snap for CW
				newVfo = (mode == DSPMode::CWL) ? (peakRf + cwPitch) : (peakRf - cwPitch);
			} else if (peakFound) {
				newVfo = peakRf;
			}

			newVfo = qBound(m_centerFrequency - m_sampleRate/2, newVfo, m_centerFrequency + m_sampleRate/2);

			m_vfoFrequency = newVfo;
			m_deltaFrequency = m_centerFrequency - m_vfoFrequency;
			m_deltaF = (qreal)(1.0 * m_deltaFrequency / m_sampleRate);

			set->setVFOFrequency(0, m_receiver, m_vfoFrequency);
			update();
			event->accept();
			return;
		}
	}
}

void QGLReceiverPanel::mouseMoveEvent(QMouseEvent* event) {
	m_mousePos = event->pos();

	if (m_dragCwText && (event->buttons() & Qt::LeftButton)) {
		const QPoint delta = event->pos() - m_cwDragStartMouse;
		m_cwDragStartMouse = event->pos();
		m_cwBoxPos += delta;
		const int w = m_cwTextRect.width() > 0 ? m_cwTextRect.width() : 200;
		const int h = m_cwTextRect.height() > 0 ? m_cwTextRect.height() : 24;
		m_cwBoxPos.setX(qBound(m_panRect.left() + 4, m_cwBoxPos.x(), m_panRect.right() - w - 4));
		m_cwBoxPos.setY(qBound(m_panRect.top() + 4, m_cwBoxPos.y(), m_panRect.bottom() - h - 4));
		update();
		event->accept();
		return;
	}

	if (event->buttons() == Qt::NoButton) {
		getRegion(m_mousePos);
		if (m_cwTextRect.isValid() && m_cwTextRect.contains(m_mousePos)) {
			if (cursor().shape() != Qt::OpenHandCursor)
				setCursor(Qt::OpenHandCursor);
		}
	}
	
	switch (m_mouseRegion) {

		//case lockedPanButtonRegion:
		//case vfoToMidButtonRegion:
		//case midToVfoButtonRegion:

		//if (m_crossHairCursor) setCursor(Qt::ArrowCursor);

		//	break;

		case agcThresholdLine:

			//GRAPHICS_DEBUG << "agcThresholdLine Rx:" << m_receiver;
			if (!m_showAGCLines || (m_agcMode == (AGCMode) agcOFF))
				break;

			m_crossHairCursor = false;
			if (cursor().shape() != Qt::SizeVerCursor)
				setCursor(Qt::SizeVerCursor);

			if (event->buttons() == Qt::LeftButton) {

                QPoint dPos = m_mouseDownPos - m_mousePos;

				qreal unit = qAbs(m_dBmPanMax - m_dBmPanMin) / m_panRect.height();
				qreal dAGCThreshold =  dPos.y() * unit;

				m_agcThresholdNew = m_mouseDownAGCThreshold + dAGCThreshold;
				if (m_agcThresholdNew > m_dBmPanMax-2)
					m_agcThresholdNew = m_dBmPanMax-2;

				if (m_agcThresholdNew < m_dBmPanMin+2)
					m_agcThresholdNew = m_dBmPanMin+2;

				set->setAGCThreshold_dB(m_receiver, m_agcThresholdNew);
		//		set->setAGCMaximumGain_dB(m_receiver, m_agcMaximumGain_dB);
			}
			break;

		case agcHangLine:

			//GRAPHICS_DEBUG << "agcHangLine Rx:" << m_receiver;
			if (!m_showAGCLines || (m_agcMode == (AGCMode) agcOFF) || !m_agcHangEnabled)
				break;

			m_crossHairCursor = false;
			if (cursor().shape() != Qt::SizeVerCursor)
				setCursor(Qt::SizeVerCursor);

			if (event->buttons() == Qt::LeftButton) {

                QPoint dPos = m_mouseDownPos - m_mousePos;

				qreal unit = qAbs(m_dBmPanMax - m_dBmPanMin) / m_panRect.height();
				qreal dAGCThreshold =  dPos.y() * unit;

				m_agcHangLevelNew = m_mouseDownAGCHangLevel + dAGCThreshold;

				if (m_agcHangLevelNew > m_dBmPanMax-2)
					m_agcHangLevelNew = m_dBmPanMax-2;

				if (m_agcHangLevelNew < m_dBmPanMin+2)
					m_agcHangLevelNew = m_dBmPanMin+2;
				set->setAGCHangThreshold(m_receiver, m_agcHangLevelNew);
			}
			break;

		case agcFixedGainLine:

			//GRAPHICS_DEBUG << "agcFixedGainLine Rx:" << m_receiver;
			if (!m_showAGCLines || (m_agcMode != (AGCMode) agcOFF))
				break;

			m_crossHairCursor = false;
			if (cursor().shape() != Qt::SizeVerCursor)
				setCursor(Qt::SizeVerCursor);

			if (event->buttons() == Qt::LeftButton) {

                QPoint dPos = m_mouseDownPos - m_mousePos;

				qreal unit = qAbs(m_dBmPanMax - m_dBmPanMin) / m_panRect.height();
				qreal dAGCFixedGain =  dPos.y() * unit;

				qreal agcFixedGain = m_mouseDownFixedGainLevel + dAGCFixedGain;

				if (agcFixedGain > m_dBmPanMax-2)
					agcFixedGain = m_dBmPanMax-2;

				if (agcFixedGain < m_dBmPanMin+2)
					agcFixedGain = m_dBmPanMin+2;

				set->setAGCFixedGain_dB(m_receiver, -agcFixedGain);
			}
			break;

		case panadapterRegion:
		case waterfallRegion:
			
			//GRAPHICS_DEBUG << "panadapterRegion Rx:" << m_receiver;
			if (!m_dragMouse) {

				m_crossHairCursor = true;
				if (m_crossHair) {
					if (cursor().shape() != Qt::BlankCursor)
						setCursor(Qt::BlankCursor);
				} else if (cursor().shape() != Qt::ArrowCursor) {
					setCursor(Qt::ArrowCursor);
				}
			}
			
			// Click-VFO sets m_dragMouse=false and returns from press; ignore
			// LeftButton moves until a real pan drag starts, or the centre
			// retunes from sub-pixel jitter between click and release.
			if (m_dragMouse && (event->buttons() == Qt::LeftButton)) {

                QPoint dPos = m_mouseDownPos - m_mousePos;
				
				qreal unit = displayedFrequencySpanHz() / m_freqScalePanRect.width();
				qreal deltaFreq = unit * dPos.x();
				
				long newFrequency = m_centerFrequency + deltaFreq;
				if (newFrequency > set->getMaxFrequency())
					newFrequency = set->getMaxFrequency();
				else if (newFrequency < set->getMinFrequency())
					newFrequency = set->getMinFrequency();
				else if (newFrequency + deltaFreq < 0)
					newFrequency = 0;
				else {

					if (m_panLocked) {

						if (m_vfoFrequency > m_centerFrequency + m_sampleRate/2)
							m_vfoFrequency = m_centerFrequency + m_sampleRate/2;
						else if (m_vfoFrequency < m_centerFrequency - m_sampleRate/2)
							m_vfoFrequency = m_centerFrequency - m_sampleRate/2;

						m_vfoFrequency -= deltaFreq;
					}
					else
						m_centerFrequency += deltaFreq;
				}

				if (m_panLocked) {
				
					set->setVFOFrequency(0, m_receiver, m_vfoFrequency);

					m_deltaFrequency = m_centerFrequency - m_vfoFrequency;
					m_deltaF = (qreal)(1.0*m_deltaFrequency/m_sampleRate);
					m_freqScalePanadapterUpdate = true;
					m_panGridUpdate = true;
				}
				else {

					m_vfoFrequency = m_centerFrequency - m_deltaFrequency;
                    GRAPHICS_DEBUG << "vfo freq " << m_vfoFrequency << m_centerFrequency;
					m_freqScalePanadapterUpdate = true;
					m_panGridUpdate = true;
					set->setVFOFrequency(0, m_receiver, m_vfoFrequency);
					set->setCtrFrequency(0, m_receiver, m_centerFrequency);
				}

                m_mouseDownPos = m_mousePos;

				m_displayCenterlineHeight = m_panRect.top() + (m_panRect.height() - 3);

				m_showFilterLeftBoundary = false;
				m_showFilterRightBoundary = false;
				m_highlightFilter = false;

				update();
			}
			m_displayCenterlineHeight = m_panRect.top() + (m_panRect.height() - 3);

			m_showFilterLeftBoundary = false;
			m_showFilterRightBoundary = false;
			m_highlightFilter = false;
			break;

		//case waterfallRegion:

			//GRAPHICS_DEBUG << "waterfallRegion Rx:" << m_receiver;
			//m_crossHairCursor = true;
			//setCursor(Qt::BlankCursor);
			
			/*if (event->buttons() == Qt::LeftButton) {

				m_cameraAngle += (pos - m_mouseDownPos);
				m_mouseDownPos = pos;
			}
			else
			if (event->buttons() == Qt::RightButton) {

				m_cameraDistance += (pos.y() - m_mouseDownPos.y()) * 0.2f;
				m_mouseDownPos = pos;
			}
			setCursor(Qt::ArrowCursor);*/
			//update();
			//break;

		case dBmScalePanadapterRegion:
			//GRAPHICS_DEBUG << "dBmScalePanadapterRegion";
			if (event->buttons() == Qt::LeftButton) {

				m_dragDBmScale = true;
                QPoint dPos = m_mouseDownPos - m_mousePos;
                qreal unit = (qreal)(qAbs(m_dBmPanMax - m_dBmPanMin) / m_panRect.height()) * 1.5;
				
                qreal newMin = m_dBmPanMin - unit * dPos.y();
                qreal newMax = m_dBmPanMax - unit * dPos.y();

				if (newMin > MINDBM && newMax < MAXDBM) {

					m_dBmPanMin = newMin;
					m_dBmPanMax = newMax;

					set->setdBmPanScaleMin(m_receiver, m_dBmPanMin);
					set->setdBmPanScaleMax(m_receiver, m_dBmPanMax);
				}
				
                m_mouseDownPos = m_mousePos;
				m_dBmScalePanadapterUpdate = true;

//				if (m_displayTime.elapsed() >= 50) {
//
//					m_displayTime.restart();
//					update();
//				}
			}
			else
			if (event->buttons() == Qt::RightButton &&
				event->modifiers() == Qt::ControlModifier) {

				m_dragDBmScale = true;

                QPoint dPos = m_mouseDownPos - m_mousePos;
				if (dPos.y() > 0)
					m_dBmPanDelta = 0.5f;
				else if (dPos.y() < 0)
					m_dBmPanDelta = -0.5f;
					
				m_dBmPanMin += m_dBmPanDelta;
				m_dBmPanMax -= m_dBmPanDelta;

				if (qAbs(m_dBmPanMax - m_dBmPanMin) < 10) {

					m_dBmPanMin -= m_dBmPanDelta;
					m_dBmPanMax += m_dBmPanDelta;
				}
				if (m_dBmPanMin < MINDBM) m_dBmPanMin = MINDBM;
				if (m_dBmPanMax > MAXDBM) m_dBmPanMax = MAXDBM;

				set->setdBmPanScaleMin(m_receiver, m_dBmPanMin);
				set->setdBmPanScaleMax(m_receiver, m_dBmPanMax);

                m_mouseDownPos = m_mousePos;
				m_dBmScalePanadapterUpdate = true;

//				if (m_displayTime.elapsed() >= 50) {
//
//					m_displayTime.restart();
//					update();
//				}
			}
			if (event->buttons() == Qt::RightButton) {

				m_dragDBmScale = true;
                QPoint dPos = m_mouseDownPos - m_mousePos;
				if (dPos.y() > 0)
					m_dBmPanDelta = 0.5f;
				else if (dPos.y() < 0)
					m_dBmPanDelta = -0.5f;
					
				m_dBmPanMax -= m_dBmPanDelta;

				if (qAbs(m_dBmPanMax - m_dBmPanMin) < 10) {

					m_dBmPanMin -= m_dBmPanDelta;
					m_dBmPanMax += m_dBmPanDelta;
				}
				if (m_dBmPanMin < MINDBM) m_dBmPanMin = MINDBM;
				if (m_dBmPanMax > MAXDBM) m_dBmPanMax = MAXDBM;

				set->setdBmPanScaleMin(m_receiver, m_dBmPanMin);
				set->setdBmPanScaleMax(m_receiver, m_dBmPanMax);

                m_mouseDownPos = m_mousePos;
				m_dBmScalePanadapterUpdate = true;

//				if (m_displayTime.elapsed() >= 50) {
//
//					m_displayTime.restart();
//					update();
//				}
			}
			else {

				setCursor(Qt::ArrowCursor);

//				if (m_displayTime.elapsed() >= 50) {
//
//					m_displayTime.restart();
//					update();
//				}
			}
			break;

		case freqScalePanadapterRegion:
			//GRAPHICS_DEBUG << "freqScalePanadapterRegion Rx" << m_receiver;
			if (event->buttons() != Qt::NoButton)
				m_dragFreqScale = true;
			if (event->buttons() == Qt::RightButton)
				m_dragFreqScaleZoom = true;
			else if (event->buttons() == Qt::LeftButton)
				m_dragFreqScaleZoom = false;

			if (event->buttons() == Qt::LeftButton &&
				event->modifiers() == Qt::ShiftModifier) {
				
                QPoint dPos = m_mouseDownPos - m_mousePos;
				int bottom_y = height() - m_freqScalePanRect.height();
				int new_y = m_rulerMouseDownPos.y() - dPos.y();
				
				if (new_y < m_panRect.top() + m_panSpectrumMinimumHeight) 
					new_y = m_panRect.top() + m_panSpectrumMinimumHeight;
				if (new_y > bottom_y) 
					new_y = bottom_y;
				
				m_freqRulerPosition = (float)(new_y - m_panRect.top()) / (bottom_y - m_panRect.top());
				set->setFreqRulerPosition(m_receiver, m_freqRulerPosition);

//				if (m_displayTime.elapsed() >= 50) {
//
//					m_displayTime.restart();
//					update();
//				}
			}
			else if (event->buttons() == Qt::LeftButton) {

                QPoint dPos = m_mouseDownPos - m_mousePos;
				
				qreal unit = displayedFrequencySpanHz() / m_freqScalePanRect.width();
				qreal deltaFreq = unit * dPos.x();
				
				/*if (m_freqScaleZoomFactor < 1.0) {

				}
				else {*/

					long newFrequency = m_centerFrequency + deltaFreq;
					if (newFrequency > set->getMaxFrequency())
						newFrequency = set->getMaxFrequency();
					else if (newFrequency < set->getMinFrequency())
						newFrequency = set->getMinFrequency();
					else if (newFrequency + deltaFreq < 0)
						newFrequency = 0;
					else {
					
						m_centerFrequency += deltaFreq;
					}

					if (!m_panLocked) {

						m_vfoFrequency = m_centerFrequency - m_deltaFrequency;
						set->setVFOFrequency(0, m_receiver, m_vfoFrequency);
						set->setCtrFrequency(0, m_receiver, m_centerFrequency);
					}

					//set->setVFOFrequency(0, m_receiver, m_vfoFrequency);
					//set->setCtrFrequency(0, m_receiver, m_centerFrequency);

					else {

						m_deltaFrequency =  m_centerFrequency - m_vfoFrequency;
						m_deltaF = (qreal)(1.0*m_deltaFrequency/m_sampleRate);

						qreal vol = set->getMainVolume(m_receiver);
						set->setMainVolume(m_receiver, 0.0f);
						set->setCtrFrequency(0, m_receiver, m_centerFrequency);
						set->setNCOFrequency(true, m_receiver, -m_deltaFrequency);
						set->setMainVolume(m_receiver, vol);
					}

					//set->setCtrFrequency(0, m_receiver, m_centerFrequency);
				//}
                m_mouseDownPos = m_mousePos;

				m_displayCenterlineHeight = m_panRect.top() + (m_panRect.height() - 3);

				m_showFilterLeftBoundary = false;
				m_showFilterRightBoundary = false;
				m_highlightFilter = false;
			}
			else
			if (event->buttons() == Qt::RightButton) {

                QPoint dPos = m_mouseDownPos - m_mousePos;
				if (dPos.x() > 0)
					m_freqScaleZoomFactor += 0.01;
				else if (dPos.x() < 0)
					m_freqScaleZoomFactor -= 0.01;

				if (m_freqScaleZoomFactor > 1.0) m_freqScaleZoomFactor = 1.0;
				if (m_freqScaleZoomFactor < 0.05) m_freqScaleZoomFactor = 0.05;
				//if (m_freqScaleZoomFactor < 0.15) m_freqScaleZoomFactor = 0.15;

                m_mouseDownPos = m_mousePos;
				m_freqScalePanadapterUpdate = true;
				m_panGridUpdate = true;
				recomputeDisplayBinsFromCache();
				update();
			}
			else
				setCursor(Qt::ArrowCursor);

			m_showFilterLeftBoundary = false;
			m_showFilterRightBoundary = false;
			m_highlightFilter = false;
			break;

		case filterRegionLow:

			setCursor(Qt::SizeHorCursor);
			m_showFilterLeftBoundary = true;
			if (event->buttons() == Qt::LeftButton) {

                QPoint dPos = m_mouseDownPos - m_mousePos;
				qreal dFreq = dPos.x() * displayedFrequencySpanHz() / m_panRect.width();

				m_filterLowerFrequency = qRound(m_mouseDownFilterFrequencyLo - dFreq);
				set->setRXFilter(m_receiver, m_filterLowerFrequency, m_filterUpperFrequency);

//				if (m_displayTime.elapsed() >= 50) {
//
//					m_displayTime.restart();
//					update();
//				}
			}

			m_highlightFilter = false;
			break;

		case filterRegionHigh:

			setCursor(Qt::SizeHorCursor);
			m_showFilterRightBoundary = true;
			if (event->buttons() == Qt::LeftButton) {

                QPoint dPos = m_mouseDownPos - m_mousePos;
				qreal dFreq = dPos.x() * displayedFrequencySpanHz() / m_panRect.width();

				m_filterUpperFrequency = qRound(m_mouseDownFilterFrequencyHi - dFreq);
				set->setRXFilter(m_receiver, m_filterLowerFrequency, m_filterUpperFrequency);

//				if (m_displayTime.elapsed() >= 50) {
//
//					m_displayTime.restart();
//					update();
//				}
			}

			m_highlightFilter = false;
			break;

		case filterRegion:

			setCursor(Qt::ArrowCursor);
			m_displayCenterlineHeight = m_panRect.top() + (size().height() - 3);
			
			if (event->buttons() == Qt::LeftButton) {

				m_highlightFilter = true;
                QPoint dPos = m_mouseDownPos - m_mousePos;
				qreal dFreq = dPos.x() * displayedFrequencySpanHz() / m_panRect.width();

				m_filterUpperFrequency = qRound(m_mouseDownFilterFrequencyHi - dFreq);
				m_filterLowerFrequency = qRound(m_mouseDownFilterFrequencyLo - dFreq);
				set->setRXFilter(m_receiver, m_filterLowerFrequency, m_filterUpperFrequency);

//				if (m_displayTime.elapsed() >= 50) {
//
//					m_displayTime.restart();
//					update();
//				}
			}
			m_showFilterLeftBoundary = false;
			m_showFilterRightBoundary = false;
			break;

		case elsewhere:
			//GRAPHICS_DEBUG << "elsewhere";
			break;
	}

	// Do not call update() from idle mouse moves. Crosshair/overlays are redrawn on
	// the next spectrum frame (setSpectrumBuffer). Extra NoPartialUpdate clears here
	// flash the whole multi-QOpenGLWidget window.
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

	QVector<float> specBuf(m_spectrumSize);
	QVector<float> waterBuf(m_spectrumSize);
	waterBuf = buffer;


	if (m_dataEngineState == QSDR::DataEngineUp) {

		if (m_spectrumAveraging) {
	
			spectrumBufferMutex.lock();
			specBuf = buffer;
			computeDisplayBins(specBuf, waterBuf);
			spectrumBufferMutex.unlock();
		}
		else {

			specBuf = buffer;
			if (m_dataEngineState == QSDR::DataEngineUp)
				computeDisplayBins(specBuf, waterBuf);
		}
	}
}

void QGLReceiverPanel::recomputeDisplayBinsFromCache()
{
	if (m_dataEngineState != QSDR::DataEngineUp || m_cachedSpectrumBuffer.size() < m_spectrumSize)
		return;

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

	//int m_sampleSize = 0;
	int deltaSampleSize = 0;
	int idx = 0;
	int lIdx = 0;
	int rIdx = 0;
	qreal localMax;

		m_sampleSize = (int)floor(m_fftMult * m_spectrumSize * m_freqScaleZoomFactor);
		deltaSampleSize = m_spectrumSize - m_sampleSize;
	//	qDebug() << "m_ssamplesdize" << m_sampleSize << deltaSampleSize << m_fftMult;
			

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

	m_panScale = (qreal)(1.0 * m_sampleSize / m_panRectWidth);
	m_scaleMultOld = m_scaleMult;

	if (m_panScale < 0.125) {
		m_scaleMult = 0.0625;
	}
	else if (m_panScale < 0.25) {
		m_scaleMult = 0.125;
	}
	else if (m_panScale < 0.5) {
		m_scaleMult = 0.25;
	}
	else if (m_panScale < 1.0) {
		m_scaleMult = 0.5;
	}
	else {
		m_scaleMult = 1.0;
	}

	const int panPixelCount = qBound(0, static_cast<int>(m_panRectWidth), 16384);
	m_panSpectrumBinsLength = qBound(0, static_cast<GLint>(m_scaleMult * m_panRectWidth), panPixelCount);
//	qDebug() << "m_panSpectrumBinsLength =" << m_panSpectrumBinsLength;
	if (m_sampleSize != m_oldSampleSize) {
	
		GRAPHICS_DEBUG << "m_panSpectrumBinsLength = " << m_panSpectrumBinsLength;
		GRAPHICS_DEBUG << "m_sampleSize =            " << m_sampleSize;
		GRAPHICS_DEBUG << "deltaSampleSize =         " << deltaSampleSize;
		GRAPHICS_DEBUG << "";

		m_oldSampleSize = m_sampleSize;
	}

	if (m_scaleMultOld != m_scaleMult) {

		if (m_waterfallRenderer) m_waterfallRenderer->reset();
	}

	m_waterfallPixel.clear();
	if (panPixelCount > 0)
		m_waterfallPixel.resize(panPixelCount);

	m_panadapterBins.clear();

	if (m_peakHold && (m_peakHoldBufferResize || m_panPeakHoldBins.size() != m_panSpectrumBinsLength)) {
		resetPeakHoldBins();
		m_peakHoldBufferResize = false;
	}

	for (int i = 0; i < m_panSpectrumBinsLength; i++) {
		
		//qreal max;
		idx = 0;
		lIdx = (int)qFloor((qreal)(i * m_panScale / m_scaleMult));
		rIdx = (int)qFloor((qreal)(i * m_panScale / m_scaleMult) + m_panScale / m_scaleMult);
					
		// max value; later we try mean value also!
		localMax = -10000.0F;

		for (int j = lIdx; j < rIdx; j++) {

			if (buffer.at(j) > localMax) {
				idx = j;
			}
		}


		// shift the beginning of the bins by half of the difference between
		// full spectrum size and reduced spectrum size due to zooming
		idx += deltaSampleSize/2;

		QColor pColor;

//		if (buffer.at(idx) < -120)
//		{
//			val = -120 - buffer.at(idx);
//			qDebug() << "calc " << buffer.at(idx) << "val  " << val;
//		}
		if (m_mercuryAttenuator) {
			m_panadapterBins << buffer.at(idx) - m_dBmPanMin - m_dBmPanLogGain - 20.0f;
			pColor = getWaterfallColorAtPixel(waterfallBuffer.at(idx) - m_dBmPanLogGain - 20.0f);
		}
		else {
			m_panadapterBins << buffer.at(idx) - m_dBmPanMin - m_dBmPanLogGain;
			pColor = getWaterfallColorAtPixel(waterfallBuffer.at(idx) - m_dBmPanLogGain);
		}

		if (m_peakHold && i < m_panPeakHoldBins.size()
			&& (m_panadapterBins.at(i) > m_panPeakHoldBins.at(i))) {
			m_panPeakHoldBins[i] = m_panadapterBins.at(i);
		}

		TGL_ubyteRGBA color;
		color.red   = (uchar)(pColor.red());
		color.green = (uchar)(pColor.green());
		color.blue  = (uchar)(pColor.blue());
		color.alpha = 255;
		
		const int span = qMax(1, static_cast<int>(1.0 / m_scaleMult));
		for (int j = 0; j < span; j++) {
			const int wfIndex = static_cast<int>(i / m_scaleMult) + j;
			if (wfIndex >= 0 && wfIndex < m_waterfallPixel.size())
				m_waterfallPixel[wfIndex] = color;
		}
	}

	m_waterfallDisplayUpdate = true;
	// Cap multi-QOpenGLWidget recomposition: uncapped / high FPS makes the whole
	// window flash as sibling FBOs clear out of sync.
	int frameIntervalMs = (m_fps > 0) ? (1000 / m_fps) : 33;
	frameIntervalMs = qMax(frameIntervalMs, 33); // ≤ ~30 FPS
	if (m_displayTime.elapsed() >= frameIntervalMs) {
		m_displayTime.restart();
		update();
	}
}

// get waterfall colors - taken from PowerSDR/KISS Konsole
QColor QGLReceiverPanel::getWaterfallColorAtPixel(qreal value) {

	QColor color;
	//int r = 0; int g = 0; int b = 0;
	int r, g, b;
	int lowerThreshold = (int)m_dBmPanMin - m_waterfallOffsetLo;
	int upperThreshold = (int)m_dBmPanMax + m_waterfallOffsetHi;

	float offset;
	float globalRange;
	float localRange;
	float percent;
	
	switch (m_waterfallMode) {

		case (WaterfallColorMode) Simple:

			if (value <= lowerThreshold)
				color = m_waterfallLoColor;
			else 
			if (value >= upperThreshold)
					color = QColor(255, 255, 255);//m_waterfallHiColor;
			else {

				percent = (value - lowerThreshold) / (upperThreshold - lowerThreshold);
				if (percent <= 0.5)	{ // use a gradient between low and mid colors
				
					percent *= 2;

					r = (int)((1 - percent) * m_waterfallLoColor.red()   + percent * m_waterfallMidColor.red());
					g = (int)((1 - percent) * m_waterfallLoColor.green() + percent * m_waterfallMidColor.green());
					b = (int)((1 - percent) * m_waterfallLoColor.blue()  + percent * m_waterfallMidColor.blue());
				}
				else {	// use a gradient between mid and high colors

					percent = (float)(percent - 0.5) * 2;

					r = (int)((1 - percent) * m_waterfallMidColor.red()   + percent * 255);//m_waterfallHiColor.red());
					g = (int)((1 - percent) * m_waterfallMidColor.green() + percent * 255);//m_waterfallHiColor.green());
					b = (int)((1 - percent) * m_waterfallMidColor.blue()  + percent * 255);//m_waterfallHiColor.blue());
				}

				if (r > 255) r = 255;
				if (g > 255) g = 255;
				if (b > 255) b = 255;
				color = QColor(r, g, b, m_waterfallAlpha);
			}

			break;

		case (WaterfallColorMode) Enhanced:

			if (value <= lowerThreshold)
				color = m_waterfallLoColor;
			else 
			if (value >= upperThreshold)
					color = m_waterfallHiColor;
			else {

				offset = value - lowerThreshold;
				globalRange = offset / m_waterfallColorRange; // value from 0.0 to 1.0 where 1.0 is high and 0.0 is low.
                if (globalRange < (float)2/9) { // background to blue

					localRange = globalRange / ((float)2/9);
					r = (int)((1.0 - localRange) * m_waterfallLoColor.red());
					g = (int)((1.0 - localRange) * m_waterfallLoColor.green());
					b = (int)(m_waterfallLoColor.blue() + localRange * (255 - m_waterfallLoColor.blue()));
				}
				else 
				if (globalRange < (float)3/9) { // blue to blue-green

					localRange = (globalRange - (float)2/9) / ((float)1/9);
					r = 0;
					g = (int)(localRange * 255);
					b = 255;
				}
				else 
				if (globalRange < (float)4/9) { // blue-green to green

					localRange = (globalRange - (float)3/9) / ((float)1/9);
					r = 0;
					g = 255;
					b = (int)((1.0 - localRange) * 255);
				}
				else 
				if (globalRange < (float)5/9) { // green to red-green

					localRange = (globalRange - (float)4/9) / ((float)1/9);
					r = (int)(localRange * 255);
					g = 255;
					b = 0;
				}
				else 
				if (globalRange < (float)7/9) { // red-green to red

					localRange = (globalRange - (float)5/9) / ((float)2/9);
					r = 255;
					g = (int)((1.0 - localRange) * 255);
					b = 0;
				}
				else 
				if (globalRange < (float)8/9) { // red to red-blue

					localRange = (globalRange - (float)7/9) / ((float)1/9);
					r = 255;
					g = 0;
					b = (int)(localRange * 255);
				}
				else { // red-blue to purple end

					localRange = (globalRange - (float)8/9) / ((float)1/9);
					r = (int)((0.75 + 0.25 * (1.0 - localRange)) * 255);
					g = (int)(localRange * 255 * 0.5);
					b = 255;
				}

                if (r > 255) r = 255;
				if (g > 255) g = 255;
				if (b > 255) b = 255;
				if (r < 0) r = 0;
				if (g < 0) g = 0;
				if (b < 0) b = 0;
				color = QColor(r, g, b, m_waterfallAlpha);
			}

			break;
	}
	
	return color;
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
}

void QGLReceiverPanel::setSpectrumAveraging(int rx, bool value) {

	if (m_receiver != rx) return;

	QMutexLocker locker(&spectrumBufferMutex);
	if (m_spectrumAveraging != value)
		m_spectrumAveraging = value;
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

	mutex.lock();
	m_bkgRed   = (GLfloat)(set->getPanadapterColors().panBackgroundColor.red() / 256.0);
	m_bkgGreen = (GLfloat)(set->getPanadapterColors().panBackgroundColor.green() / 256.0);
	m_bkgBlue  = (GLfloat)(set->getPanadapterColors().panBackgroundColor.blue() / 256.0);

	m_red	= (GLfloat)(set->getPanadapterColors().panLineColor.red() / 256.0);
	m_green = (GLfloat)(set->getPanadapterColors().panLineColor.green() / 256.0);
	m_blue	= (GLfloat)(set->getPanadapterColors().panLineColor.blue() / 256.0);

	m_redF	 = (GLfloat)(set->getPanadapterColors().panLineFilledColor.red() / 256.0);
	m_greenF = (GLfloat)(set->getPanadapterColors().panLineFilledColor.green() / 256.0);
	m_blueF  = (GLfloat)(set->getPanadapterColors().panLineFilledColor.blue() / 256.0);

	m_redST	  = (GLfloat)(set->getPanadapterColors().panSolidTopColor.red() / 256.0);
	m_greenST = (GLfloat)(set->getPanadapterColors().panSolidTopColor.green() / 256.0);
	m_blueST  = (GLfloat)(set->getPanadapterColors().panSolidTopColor.blue() / 256.0);

	m_redSB   = (GLfloat)(set->getPanadapterColors().panSolidBottomColor.red() / 256.0);
	m_greenSB = (GLfloat)(set->getPanadapterColors().panSolidBottomColor.green() / 256.0);
	m_blueSB  = (GLfloat)(set->getPanadapterColors().panSolidBottomColor.blue() / 256.0);

	m_waterfallMidColor = set->getPanadapterColors().waterfallColor.toRgb() ;

	QColor gridColor = m_gridColor;
	m_gridColor = set->getPanadapterColors().gridLineColor;

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
}

void QGLReceiverPanel::setWaterfallOffesetHi(int rx, int value) {

	if (m_receiver != rx) return;

	m_waterfallOffsetHi = value;
}

void QGLReceiverPanel::setdBmScaleMin(int rx, qreal value) {

	if (m_receiver != rx) return;

	m_dBmPanMin = value;

	m_dBmScalePanadapterUpdate = true;
	m_panGridUpdate = true;
	m_peakHoldBufferResize = true;
}

void QGLReceiverPanel::setdBmScaleMax(int rx, qreal value) {

	if (m_receiver != rx) return;

	m_dBmPanMax = value;

	m_dBmScalePanadapterUpdate = true;
	m_panGridUpdate = true;
	m_peakHoldBufferResize = true;
}

void QGLReceiverPanel::setMouseWheelFreqStep(int rx, qreal step) {

	if (m_receiver != rx) return;
	m_mouseWheelFreqStep = step;
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

