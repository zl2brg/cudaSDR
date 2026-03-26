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

#define LOG_GRAPHICS
//#define GRAPHICS_DEBUG

// use: GRAPHICS_DEBUG

#include "cusdr_oglReceiverPanel.h"

#include <QGuiApplication>
#include <QPainter>
#include <cstring>

//#include <QtGui>
//#include <QDebug>
//#include <QFileInfo>
//#include <QElapsedTimerr>
//#include <QImage>
//#include <QString>
//#include <QOpenGLFrameBufferObject>

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

#define	btn_height		14
#define	btn_width		60
#define	btn_widthb		70
#define	btn_widths		34

QGLReceiverPanel::QGLReceiverPanel(QWidget *parent, int rx)
	: QOpenGLWidget(parent)

	, set(Settings::instance())
	, m_serverMode(set->getCurrentServerMode())
	, m_hwInterface(set->getHWInterface())
	, m_dataEngineState(QSDR::DataEngineDown)
	, m_mousePos(QPoint(-1, -1))
	, m_mouseDownPos(QPoint(-1, -1))
	, m_panSpectrumBinsLength(0)
	, m_filterLeft(0)
	, m_filterRight(0)
	, m_filterTop(0)
	, m_filterBottom(0)
	, m_receiver(rx)
	//, m_frequencyRxOnRx(0)
	, m_spectrumSize(set->getSpectrumSize())
	, m_sampleSize(0)
	, m_oldSampleSize(0)
	, m_specAveragingCnt(set->getSpectrumAveragingCnt(m_receiver))
	, m_currentReceiver(set->getCurrentReceiver())
	, m_waterfallAlpha(255)
	, m_freqRulerDisplayWidth(0)
	, m_oldWaterfallWidth(0)
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
	, m_filterChanged(true)
	, m_showFilterLeftBoundary(false)
	, m_showFilterRightBoundary(false)
	, m_highlightFilter(false)
	, m_dragMouse(false)
	, m_panLocked(set->getPanLockedStatus(m_receiver))
	, m_clickVFO(set->getClickVFOStatus(m_receiver))
	, m_freqScaleZoomFactor(1.0f)
	, m_scaleMult(1.0f)
	, m_filterLowerFrequency(-3050.0)
	, m_filterUpperFrequency(-150.0)
	//, m_freqRulerPosition(0.5)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	const bool isWayland = QGuiApplication::platformName().contains("wayland", Qt::CaseInsensitive);
	setUpdateBehavior(isWayland ? QOpenGLWidget::NoPartialUpdate : QOpenGLWidget::PartialUpdate);
	//setAutoBufferSwap(true);
	setAutoFillBackground(false);

	setMouseTracking(true);
	setFocusPolicy(Qt::StrongFocus);

	//GRAPHICS_DEBUG << "set spectrum buffer size to: " << m_spectrumSize;

	m_bigHeight = 600;
	m_bigWidth = 1000;

	setupDisplayRegions(size());
	m_oldWidth = size().width();
	m_rxDataList = set->getReceiverDataList();
	m_freqRulerPosition = m_rxDataList.at(m_receiver).freqRulerPosition;
	m_centerFrequency = m_rxDataList.at(m_receiver).ctrFrequency;
	m_vfoFrequency = m_rxDataList.at(m_receiver).vfoFrequency;

	if (m_vfoFrequency > m_centerFrequency + m_sampleRate/2)
		m_vfoFrequency = m_centerFrequency + m_sampleRate/2;
	else if (m_vfoFrequency < m_centerFrequency - m_sampleRate/2)
		m_vfoFrequency = m_centerFrequency - m_sampleRate/2;
	else

	m_deltaFrequency = m_centerFrequency - m_vfoFrequency;
	m_deltaF = (qreal)(1.0*m_deltaFrequency/m_sampleRate);
	
	m_dBmScalePanadapterRenew = true;
	m_dBmScalePanadapterUpdate = true;
	m_freqScalePanadapterRenew = true;
	m_freqScalePanadapterUpdate = true;
	m_panGridRenew = true;
	m_panGridUpdate = true;
	m_waterfallUpdate = true;
	m_secScaleWaterfallUpdate = true;
	m_secScaleWaterfallRenew = true;
	m_waterfallDisplayUpdate = true;
	m_panMode = m_rxDataList.at(m_receiver).panMode;
	m_waterfallMode = m_rxDataList.at(m_receiver).waterfallMode;

	HamBand band = m_rxDataList.at(m_receiver).hamBand;

	m_dBmPanMin = m_rxDataList.at(m_receiver).dBmPanScaleMinList.at(band);
	m_dBmPanMax = m_rxDataList.at(m_receiver).dBmPanScaleMaxList.at(band);

	m_waterfallOffsetLo = m_rxDataList.at(m_receiver).waterfallOffsetLo;
	m_waterfallOffsetHi = m_rxDataList.at(m_receiver).waterfallOffsetHi;

	m_secWaterfallMin = 0.0;
	m_secWaterfallMax = 0.0;

	m_filterLowerFrequency = m_rxDataList.at(m_receiver).filterLo;
	m_filterUpperFrequency = m_rxDataList.at(m_receiver).filterHi;
	m_filterWidth = qAbs((int)(m_filterUpperFrequency - m_filterLowerFrequency));

	m_mouseWheelFreqStep = m_rxDataList.at(m_receiver).mouseWheelFreqStep;

	m_adcMode = m_rxDataList.at(m_receiver).adcMode;
	m_adcModeString = set->getADCModeString(m_receiver);

	m_agcMode = m_rxDataList.at(m_receiver).agcMode;
	m_agcModeString = set->getAGCModeString(m_receiver);
	m_agcFixedGain = m_rxDataList.at(m_receiver).agcFixedGain_dB;

	m_dspModeString = set->getDSPModeString(m_rxDataList.at(m_receiver).dspModeList.at(m_receiver));

	m_agcHangEnabled = m_rxDataList.at(m_receiver).hangEnabled;
	m_showAGCLines = m_rxDataList.at(m_receiver).agcLines;

	radioPopup = new RadioPopupWidget(this, m_receiver);

	fonts = new CFonts(this);
	m_fonts = fonts->getFonts();

	m_fonts.smallFont.setBold(true);
	m_fonts.bigFont1.setBold(false);
	m_fonts.bigFont2.setBold(false);

	timer = 0;
	m_waterfallTextureId = 0;

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
    m_textureFBO = nullptr;
    m_waterfallLineFBO = nullptr;
    m_waterfallFBO = nullptr;
    m_secScaleWaterfallFBO = nullptr;

	m_waterfallLineCnt = 0;

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
	
	if (m_frequencyScaleFBO) {

		delete m_frequencyScaleFBO;
        m_frequencyScaleFBO = nullptr;
	}

	if (m_waterfallLineFBO) {

		delete m_waterfallLineFBO;
        m_waterfallLineFBO = nullptr;
	}
		
	if (m_waterfallFBO) {

		delete m_waterfallFBO;
        m_waterfallFBO = nullptr;
	}

	if (m_textureFBO) {

		delete m_textureFBO;
        m_textureFBO = nullptr;
	}

	if (m_dBmScaleFBO) {

		delete m_dBmScaleFBO;
        m_dBmScaleFBO = nullptr;
	}


	if (m_secScaleWaterfallFBO) {

		delete m_secScaleWaterfallFBO;
        m_secScaleWaterfallFBO = nullptr;
	}

	if (m_waterfallTextureId != 0) {

		makeCurrent();
		glDeleteTextures(1, &m_waterfallTextureId);
		m_waterfallTextureId = 0;
		doneCurrent();
	}

    while (!specAv_queue.isEmpty())
        specAv_queue.dequeue();
    delete fonts;

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

	CHECKED_CONNECT(
		set,
		SIGNAL(systemStateChanged(
					QObject *,
					QSDR::_Error,
					QSDR::_HWInterfaceMode,
					QSDR::_ServerMode,
					QSDR::_DataEngineState)),
		this,
		SLOT(systemStateChanged(
					QObject *,
					QSDR::_Error,
					QSDR::_HWInterfaceMode,
					QSDR::_ServerMode,
					QSDR::_DataEngineState)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(graphicModeChanged(
					QObject *,
					int,
					PanGraphicsMode,
					WaterfallColorMode)),
		this, 
		SLOT(graphicModeChanged(
					QObject *,
					int,
					PanGraphicsMode,
					WaterfallColorMode)));

//	CHECKED_CONNECT(
//		set,
//		SIGNAL(spectrumSizeChanged(QObject *, int)),
//		this,
//		SLOT(setSpectrumSize(QObject *, int)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(freqRulerPositionChanged(QObject *, int, float)), 
		this, 
		SLOT(freqRulerPositionChanged(QObject *, int, float)));

	CHECKED_CONNECT(
		set,
		SIGNAL(ctrFrequencyChanged(QObject *, int, int, long)),
		this,
		SLOT(setCtrFrequency(QObject *, int, int, long)));

	CHECKED_CONNECT(
		set,
		SIGNAL(vfoFrequencyChanged(QObject *, int, int, long)),
		this,
		SLOT(setVFOFrequency(QObject *, int, int, long)));

	CHECKED_CONNECT(
		set,
		SIGNAL(hamBandChanged(QObject *, int, bool, HamBand)),
		this,
		SLOT(setHamBand(QObject *, int, bool, HamBand)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(currentReceiverChanged(QObject *, int)),
		this, 
		SLOT(setCurrentReceiver(QObject *, int)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(sampleRateChanged(QObject *, int)), 
		this, 
		SLOT(sampleRateChanged(QObject *, int)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(filterFrequenciesChanged(QObject *, int, qreal, qreal)), 
		this, 
		SLOT(setFilterFrequencies(QObject *, int, qreal, qreal)));

//	CHECKED_CONNECT(
//		set,
//		SIGNAL(waterfallTimeChanged(int, int)),
//		this,
//		SLOT(setWaterfallTime(int, int)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(waterfallOffesetLoChanged(int, int)),
		this,
		SLOT(setWaterfallOffesetLo(int, int)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(waterfallOffesetHiChanged(int, int)),
		this,
		SLOT(setWaterfallOffesetHi(int, int)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(spectrumAveragingChanged(QObject *, int, bool)), 
		this, 
		SLOT(setSpectrumAveraging(QObject *, int, bool)));

	/*CHECKED_CONNECT(
		set, 
		SIGNAL(spectrumAveragingCntChanged(int)), 
		this, 
		SLOT(setSpectrumAveragingCnt(int)));*/

	CHECKED_CONNECT(
		set,
		SIGNAL(spectrumBufferChanged(int, const qVectorFloat&)),
		this,
		SLOT(setSpectrumBuffer(int, const qVectorFloat&)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(panGridStatusChanged(bool, int)),
		this,
		SLOT(setPanGridStatus(bool, int)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(peakHoldStatusChanged(bool, int)),
		this,
		SLOT(setPeakHoldStatus(bool, int)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(panLockedStatusChanged(bool, int)),
		this,
		SLOT(setPanLockedStatus(bool, int)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(clickVFOStatusChanged(bool, int)),
		this,
		SLOT(setClickVFOStatus(bool, int)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(hairCrossStatusChanged(bool, int)),
		this,
		SLOT(setHairCrossStatus(bool, int)));

	/*CHECKED_CONNECT(
		set, 
		SIGNAL(panadapterColorChanged()),
		this,
		SLOT(setSimpleWaterfallColor()));*/

	CHECKED_CONNECT(
		set, 
		SIGNAL(panadapterColorChanged()),
		this,
		SLOT(setPanadapterColors()));

	CHECKED_CONNECT(
		set, 
		SIGNAL(mercuryAttenuatorChanged(QObject *, HamBand, int)),
		this, 
		SLOT(setMercuryAttenuator(QObject *, HamBand, int)));

	CHECKED_CONNECT(
		set,
		SIGNAL(adcOverflowChanged(int)),
		this,
		SLOT(setADCStatus(int)));

	CHECKED_CONNECT(
		set,
		SIGNAL(framesPerSecondChanged(QObject*, int, int)),
		this,
		SLOT(setFramesPerSecond(QObject*, int, int)));

//	CHECKED_CONNECT(
//		set,
//		SIGNAL(agcThresholdLine_dBmChanged(QObject *, int, qreal)),
//		this,
//		SLOT(setAGCThresholdLine_dBm(QObject *, int, qreal)));

	CHECKED_CONNECT(
		set,
		SIGNAL(agcFixedGainChanged_dB(QObject *, int, qreal)),
		this,
		SLOT(setAGCLineFixedLevel(QObject *, int, qreal)));

	CHECKED_CONNECT(
		set,
		SIGNAL(agcLineLevelsChanged(QObject *, int, qreal, qreal)),
		this,
		SLOT(setAGCLineLevels(QObject *, int, qreal, qreal)));

	CHECKED_CONNECT(
		set,
		SIGNAL(agcModeChanged(QObject *, int, AGCMode, bool)),
		this,
		SLOT(setAGCMode(QObject *, int, AGCMode, bool)));

	CHECKED_CONNECT(
		set,
		SIGNAL(showAGCLinesStatusChanged(QObject *, bool, int)),
		this,
		SLOT(setAGCLinesStatus(QObject *, bool, int)));

	CHECKED_CONNECT(
		set,
		SIGNAL(adcModeChanged(QObject *, int, ADCMode)),
		this,
		SLOT(setADCMode(QObject *, int, ADCMode)));

	CHECKED_CONNECT(
		set,
		SIGNAL(dspModeChanged(QObject *, int, DSPMode)),
		this,
		SLOT(setDSPMode(QObject *, int, DSPMode)));

//	CHECKED_CONNECT(
//		set,
//		SIGNAL(agcHangEnabledChanged(QObject *, int, bool)),
//		this,
//		SLOT(setAGCHangEnabled(QObject *, int, bool)));

	CHECKED_CONNECT(
		set,
		SIGNAL(mouseWheelFreqStepChanged(QObject *, int, qreal)),
		this,
		SLOT(setMouseWheelFreqStep(QObject *, int, qreal)));

	CHECKED_CONNECT(
		radioPopup,
		SIGNAL(vfoToMidBtnEvent()),
		this,
		SLOT(setVfoToMidFrequency()));

	CHECKED_CONNECT(
		radioPopup,
		SIGNAL(midToVfoBtnEvent()),
		this,
		SLOT(setMidToVfoFrequency()));
}

void QGLReceiverPanel::initializeGL() {

	if (!isValid()) return;
     initializeOpenGLFunctions();

	glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
    
	m_cnt = 0;

	// ── Core-profile shader for spectrum rendering ───────────────────────────
	static const char *vertSrc = R"(
		#version 130
		in vec3 a_pos;
		in vec3 a_color;
		uniform mat4 u_mvp;
		out vec3 v_color;
		void main() {
			gl_Position = u_mvp * vec4(a_pos, 1.0);
			v_color = a_color;
		}
	)";

	static const char *fragSrc = R"(
		#version 130
		in vec3 v_color;
		out vec4 fragColor;
		void main() {
			fragColor = vec4(v_color, 1.0);
		}
	)";

	m_panProgram = new QOpenGLShaderProgram(this);
	if (!m_panProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertSrc))
		qWarning() << "RX vertex shader:" << m_panProgram->log();
	if (!m_panProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc))
		qWarning() << "RX fragment shader:" << m_panProgram->log();
	if (!m_panProgram->link())
		qWarning() << "RX shader link:" << m_panProgram->log();

	m_attrPos    = m_panProgram->attributeLocation("a_pos");
	m_attrColor  = m_panProgram->attributeLocation("a_color");
	m_uniformMvp = m_panProgram->uniformLocation("u_mvp");

	// ── Persistent VAO + VBO (streaming, updated each draw) ─────────────────
	m_panVao.create();
	m_panVbo.create();
	m_panVbo.setUsagePattern(QOpenGLBuffer::StreamDraw);

	m_panVao.bind();
	m_panVbo.bind();
	// stride = 6 floats (xyz + rgb), attribute pointers set once in VAO
	glVertexAttribPointer(m_attrPos,   3, GL_FLOAT, GL_FALSE,
	                      6 * sizeof(float), reinterpret_cast<void*>(0));
	glVertexAttribPointer(m_attrColor, 3, GL_FLOAT, GL_FALSE,
	                      6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
	glEnableVertexAttribArray(m_attrPos);
	glEnableVertexAttribArray(m_attrColor);
	m_panVbo.release();
	m_panVao.release();
}

void QGLReceiverPanel::paintGL() {
	switch (m_serverMode) {

		case QSDR::NoServerMode:

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			break;

		case QSDR::SDRMode:

       //     if (freqChangeTimer.elapsed() > 50)  m_spectrumAveraging = m_spectrumAveragingOld;

			if (m_resizeTime.elapsed() > 200 || m_dataEngineState == QSDR::DataEngineDown)
				paintReceiverDisplay();
			
			break;
	}
}
 
void QGLReceiverPanel::paintReceiverDisplay() {
 
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

    drawPanadapter();
    drawPanHorizontalScale();
    drawPanVerticalScale();
    drawPanadapterGrid();
    drawCenterLine();
    drawPanFilter();

	if (m_dataEngineState == QSDR::DataEngineUp && m_showAGCLines && (m_receiver == m_currentReceiver))
		drawAGCControl();

	if (m_panRect.width() > 300 && m_panRect.height() > 80) {

        drawVFOControl();
        drawReceiverInfo();
	}

	if (m_waterfallDisplayUpdate && m_waterfallRect.height() > 10) {

        drawWaterfall();
    //    drawWaterfallVerticalScale();
		m_waterfallDisplayUpdate = false;
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
			m_crossHairCursor)
			drawCrossHair();
	}
 }

void QGLReceiverPanel::paint3DPanadapterMode() {
}

void QGLReceiverPanel::drawPanadapter() {

    float dpr = devicePixelRatio();
    GLint vertexArrayLength = (GLint)m_panadapterBins.size();
    GLint height = m_panRect.height();
    GLint x1 = m_panRect.left();
    GLint y1 = m_panRect.top();
    GLint x2 = x1 + m_panRect.width();
    GLint y2 = y1 + m_panRect.height();

    float yScale;
    float yTop;
    qreal dBmRange = qAbs(m_dBmPanMax - m_dBmPanMin);
    yScale = m_panRect.height() / dBmRange;
    yTop = (float)y2;

    if (m_dataEngineState == QSDR::DataEngineUp)
        glClear(GL_DEPTH_BUFFER_BIT);
    else
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    // Set up shader + VBO early (used for both background and spectrum)
    QMatrix4x4 mvp;
    mvp.setToIdentity();
    mvp.ortho(0.0f, (float)size().width(), (float)size().height(), 0.0f, -5.0f, 5.0f);
    m_panProgram->bind();
    m_panProgram->setUniformValue(m_uniformMvp, mvp);
    m_panVao.bind();
    m_panVbo.bind();

    // Draw background as a triangle strip with per-vertex colours [x,y,z,r,g,b]
    {
        float rT, gT, bT, rB, gB, bB;
        if (m_dataEngineState == QSDR::DataEngineUp) {
            if (m_receiver == m_currentReceiver) {
                rT = 0.7f * m_bkgRed;  gT = 0.7f * m_bkgGreen;  bT = 0.7f * m_bkgBlue;
                rB = 0.3f * m_bkgRed;  gB = 0.3f * m_bkgGreen;  bB = 0.3f * m_bkgBlue;
            } else {
                rT = rB = 0.4f * m_bkgRed;  gT = gB = 0.4f * m_bkgGreen;  bT = bB = 0.4f * m_bkgBlue;
            }
        } else {
            // pre-multiply QColor(30,30,50,155) onto cleared-black background
            rT = rB = (30.0f * 155.0f) / (255.0f * 255.0f);
            gT = gB = (30.0f * 155.0f) / (255.0f * 255.0f);
            bT = bB = (50.0f * 155.0f) / (255.0f * 255.0f);
        }
        const float bg[4*6] = {
            (float)x1, (float)y1, -4.0f,  rT, gT, bT,
            (float)x2, (float)y1, -4.0f,  rT, gT, bT,
            (float)x1, (float)y2, -4.0f,  rB, gB, bB,
            (float)x2, (float)y2, -4.0f,  rB, gB, bB,
        };
        m_panVbo.allocate(bg, sizeof(bg));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    if (vertexArrayLength < 2) {
        m_panVbo.release();
        m_panVao.release();
        m_panProgram->release();
        return;
    }

    // Set a DPR-aware scissor box
    glScissor(x1, (int)(size().height() * dpr - y2 * dpr), (int)((x2 - x1) * dpr), (int)(height * dpr));
    glEnable(GL_SCISSOR_TEST);

    spectrumBufferMutex.lock();

    switch (m_panMode) {

        case (PanGraphicsMode) FilledLine: {
            // Build interleaved [x,y,z,r,g,b] for filled strip (2 verts per x: top + bottom)
            QVector<float> bgData(2 * vertexArrayLength * 6);
            mutex.lock();
            float rF = m_redF, gF = m_greenF, bF = m_blueF;
            float rL = m_red,  gL = m_green,  bL = m_blue;
            mutex.unlock();

            for (int i = 0; i < vertexArrayLength; i++) {
                float px = (float)(i / m_scaleMult);
                float py = (float)(yTop - yScale * m_panadapterBins.at(i));
                // top vertex
                bgData[(2*i+0)*6+0] = px;  bgData[(2*i+0)*6+1] = py;    bgData[(2*i+0)*6+2] = -1.5f;
                bgData[(2*i+0)*6+3] = 0.7f*rF; bgData[(2*i+0)*6+4] = 0.7f*gF; bgData[(2*i+0)*6+5] = 0.7f*bF;
                // bottom vertex
                bgData[(2*i+1)*6+0] = px;  bgData[(2*i+1)*6+1] = yTop;  bgData[(2*i+1)*6+2] = -1.5f;
                bgData[(2*i+1)*6+3] = 0.3f*rF; bgData[(2*i+1)*6+4] = 0.3f*gF; bgData[(2*i+1)*6+5] = 0.3f*bF;
            }
            m_panVbo.allocate(bgData.constData(), bgData.size() * sizeof(float));
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 2 * vertexArrayLength);

            // Line strip — reuse top vertices from bgData, with line color
            QVector<float> lineData(vertexArrayLength * 6);
            for (int i = 0; i < vertexArrayLength; i++) {
                lineData[i*6+0] = bgData[(2*i)*6+0];
                lineData[i*6+1] = bgData[(2*i)*6+1];
                lineData[i*6+2] = bgData[(2*i)*6+2];
                lineData[i*6+3] = rL; lineData[i*6+4] = gL; lineData[i*6+5] = bL;
            }
            m_panVbo.allocate(lineData.constData(), lineData.size() * sizeof(float));
            glEnable(GL_LINE_SMOOTH);
            glDrawArrays(GL_LINE_STRIP, 0, vertexArrayLength);
            glDisable(GL_LINE_SMOOTH);
            break;
        }

        case (PanGraphicsMode) Line: {
            mutex.lock();
            float rL = m_red, gL = m_green, bL = m_blue;
            mutex.unlock();

            QVector<float> lineData(vertexArrayLength * 6);
            for (int i = 0; i < vertexArrayLength; i++) {
                lineData[i*6+0] = (float)(i / m_scaleMult);
                lineData[i*6+1] = (float)(yTop - yScale * m_panadapterBins.at(i));
                lineData[i*6+2] = -1.0f;
                lineData[i*6+3] = rL; lineData[i*6+4] = gL; lineData[i*6+5] = bL;
            }
            m_panVbo.allocate(lineData.constData(), lineData.size() * sizeof(float));
            glEnable(GL_LINE_SMOOTH);
            glDrawArrays(GL_LINE_STRIP, 0, vertexArrayLength);
            glDisable(GL_LINE_SMOOTH);
            break;
        }

        case (PanGraphicsMode) Solid: {
            mutex.lock();
            float rT = m_redST, gT = m_greenST, bT = m_blueST;
            float rB = m_redSB, gB = m_greenSB, bB = m_blueSB;
            mutex.unlock();

            QVector<float> solData(2 * vertexArrayLength * 6);
            for (int i = 0; i < vertexArrayLength; i++) {
                float px = (float)(i / m_scaleMult);
                float py = (float)(yTop - yScale * m_panadapterBins.at(i));
                // top vertex
                solData[(2*i+0)*6+0] = px; solData[(2*i+0)*6+1] = py;    solData[(2*i+0)*6+2] = -1.0f;
                solData[(2*i+0)*6+3] = rT; solData[(2*i+0)*6+4] = gT;    solData[(2*i+0)*6+5] = bT;
                // bottom vertex
                solData[(2*i+1)*6+0] = px; solData[(2*i+1)*6+1] = yTop;  solData[(2*i+1)*6+2] = -1.0f;
                solData[(2*i+1)*6+3] = rB; solData[(2*i+1)*6+4] = gB;    solData[(2*i+1)*6+5] = bB;
            }
            m_panVbo.allocate(solData.constData(), solData.size() * sizeof(float));
            glDrawArrays(GL_LINES, 0, 2 * vertexArrayLength);
            break;
        }
    }

    m_panVbo.release();
    m_panVao.release();
    m_panProgram->release();
    spectrumBufferMutex.unlock();

    glDisable(GL_SCISSOR_TEST);
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

    if (!m_dBmScaleFBO || m_dBmScalePanadapterUpdate || m_dBmScalePanadapterRenew)	{

        if (!m_dBmScaleFBO || m_dBmScalePanadapterRenew) {

            if (m_dBmScaleFBO) {
                delete m_dBmScaleFBO;
                m_dBmScaleFBO = 0;
            }
            // Create FBO with logical size - let Qt handle DPR automatically
            m_dBmScaleFBO = new QOpenGLFramebufferObject(width, height);
        }
        glPushAttrib(GL_VIEWPORT_BIT | GL_TEXTURE_BIT);
        glViewport(0, 0, width, height);
        setProjectionOrthographic(width, height);

        m_dBmScaleFBO->bind();
        // Make sure to use scaled width/height in renderPanVerticalScale
        renderPanVerticalScale();
        m_dBmScaleFBO->release();

        m_dBmScalePanadapterUpdate = false;
        m_dBmScalePanadapterRenew = false;
        glPopAttrib();
        setProjectionOrthographic(size().width(), size().height());
    }

    renderTexture(m_dBmScalePanRect, m_dBmScaleFBO->texture(), 0.0f);
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

    qreal unit = (qreal)(height / qAbs(m_dBmPanMax - m_dBmPanMin));

    m_dBmScale = getYRuler2(m_dBmScalePanRect, fontHeight, unit, m_dBmPanMin, m_dBmPanMax);

    QRect textRect(0, 0, fontMaxWidth, fontHeight);
    textRect.moveLeft(3);

    int len    = m_dBmScale.mainPointPositions.length();
    int sublen = m_dBmScale.subPointPositions.length();

    // draw the scale background
    painter.fillRect(0, 0, width, height, QColor(30, 30, 30, 180));

    // draw tick marks with QPainter
    if (len > 0) {
        painter.setPen(QPen(QColor(166, 194, 207), 1));
        for (int i = 0; i < len; i++) {
            int y = (int)m_dBmScale.mainPointPositions.at(i);
            painter.drawLine(width - 4, y, width, y);
        }
        if (sublen > 0) {
            painter.setPen(QPen(QColor(115, 143, 156), 1));
            for (int i = 0; i < sublen; i++) {
                int y = (int)m_dBmScale.subPointPositions.at(i);
                painter.drawLine(width - 2, y, width, y);
            }
        }
    }

    painter.setPen(QPen(QColor(191,219,232)));
    painter.setFont(m_fonts.normalFont);

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

    if (!m_frequencyScaleFBO || m_freqScalePanadapterUpdate || m_freqScalePanadapterRenew) {

        if (!m_frequencyScaleFBO || m_freqScalePanadapterRenew) {

            if (m_frequencyScaleFBO) {

                delete m_frequencyScaleFBO;
                m_frequencyScaleFBO = 0;
            }

            m_frequencyScaleFBO = new QOpenGLFramebufferObject(width, height);  // Use logical size
            //if (m_frequencyScaleFBO)
            //	GRAPHICS_DEBUG << "m_frequencyScaleFBO generated.";
        }

        glPushAttrib(GL_VIEWPORT_BIT | GL_TEXTURE_BIT);
        glViewport(0, 0, width, height);
        setProjectionOrthographic(width, height);

        m_frequencyScaleFBO->bind();
            renderPanHorizontalScale();
        m_frequencyScaleFBO->release();

        glPopAttrib();
        setProjectionOrthographic(size().width(), size().height());
		m_freqScalePanadapterUpdate = false;
		m_freqScalePanadapterRenew = false;
	}

	renderTexture(m_freqScalePanRect, m_frequencyScaleFBO->texture(), 0.0f);
}

void QGLReceiverPanel::drawPanadapterGrid() {

	if (!m_panGrid) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    int alpha = (m_receiver == m_currentReceiver) ? 255 : 204;
    QColor gc(int(m_redGrid * 255), int(m_greenGrid * 255), int(m_blueGrid * 255), alpha);
    painter.setPen(QPen(gc, 2, Qt::DotLine));

    // Vertical lines (frequency grid)
    int len = m_frequencyScale.mainPointPositions.length();
    for (int i = 0; i < len; i++) {
        int x = m_frequencyScale.mainPointPositions.at(i);
        painter.drawLine(x, m_panRect.top(), x, m_panRect.bottom());
    }

    // Horizontal lines (dBm grid)
    len = m_dBmScale.mainPointPositions.length();
    for (int i = 0; i < len; i++) {
        int y = m_dBmScale.mainPointPositions.at(i);
        painter.drawLine(m_panRect.left(), y, m_panRect.right(), y);
    }
}

void QGLReceiverPanel::drawCenterLine() {

	// draw a line for the center frequency
	GLint y1, y2;
    y1 = m_panRect.top();
    y2 = m_displayCenterlineHeight;
	
	if (y2 > y1 + 3) {

        GLint x = m_panRect.width()/2 ;
        GLint y = m_panRect.top() + m_panRect.height() - 1 ;
		QColor col = QColor(80, 180, 240, 180);

		QPainter painter(this);

		// center frequency line
		painter.setPen(QPen(col, 3));
		painter.drawLine(x, y1 + 1, x, y - 1);
			
		x = m_panRect.left() + qRound((qreal)(m_panRect.width()/2.0f)  - m_deltaF * m_panRect.width() / m_freqScaleZoomFactor);
		col = set->getPanadapterColors().panCenterLineColor;
		col.setAlpha(255);

		painter.setPen(QPen(col, 3));

		// VFO frequency line
		if (m_dragMouse && !m_panLocked) {
			painter.drawLine(x, m_freqScalePanRect.bottom() + 1, x, m_freqScalePanRect.bottom() + m_waterfallRect.height() - 1);
		}
		painter.drawLine(x, y1 + 1, x, y - 1);
	}

	// draw a frequency line from a different receiver
	/*if (m_frequencyRxOnRx != 0) {
		...
	}*/
}

void QGLReceiverPanel::drawPanFilter() {

	QColor color;

	if (m_highlightFilter)
		color = QColor(150, 150, 150, 160);
	else
		color = QColor(150, 150, 150, 100);

	m_filterLeft = m_panRect.left() + qRound((qreal)(m_panRect.width()/2.0f) + (m_filterLo - m_deltaF) * m_panRect.width() / m_freqScaleZoomFactor);
	m_filterRight = m_panRect.left() + qRound((qreal)(m_panRect.width()/2.0f) + (m_filterHi - m_deltaF) * m_panRect.width() / m_freqScaleZoomFactor);
	m_filterTop = m_panRect.top() + 1;
	m_filterBottom = m_panRect.top() + m_panRect.height() - 1;

	m_filterRect = QRect(m_filterLeft, m_filterTop, m_filterRight - m_filterLeft, m_filterBottom - m_filterTop);

	QPainter painter(this);
	painter.setFont(m_smallSize ? m_fonts.smallFont : m_fonts.bigFont1);

	if ((m_filterLeft >= m_panRect.left() && m_filterLeft <= m_panRect.right()) ||
		(m_filterRight >= m_panRect.left() && m_filterRight <= m_panRect.right()) ||
		(m_filterLeft < m_panRect.left() && m_filterRight > m_panRect.right()))
	{
		if (m_filterRect.height() > 5) painter.fillRect(m_filterRect, color);
	}

	// filter boundaries
	if (m_showFilterLeftBoundary) {

		painter.setPen(QPen(QColor(150, 150, 150, 230), 1));
		painter.drawLine(m_filterLeft, m_filterTop, m_filterLeft, m_filterBottom);

		QString str1 = QString("Filter Lo");
		QString str2 = frequencyString(m_filterLowerFrequency, true);

		// shadow
		painter.setPen(Qt::black);
		painter.drawText(m_filterLeft + 5, m_filterTop + 44, str1);
		painter.drawText(m_filterLeft + 5, m_filterTop + 64, str2);
		// foreground
		painter.setPen(Qt::white);
		painter.drawText(m_filterLeft + 3, m_filterTop + 42, str1);
		painter.drawText(m_filterLeft + 3, m_filterTop + 62, str2);
	}

	if (m_showFilterRightBoundary) {

		painter.setPen(QPen(QColor(150, 150, 150, 230), 1));
		painter.drawLine(m_filterRight, m_filterTop, m_filterRight, m_filterBottom);

		QString str1 = QString("Filter Hi");
		QString str2 = frequencyString(m_filterUpperFrequency, true);

		// shadow
		painter.setPen(Qt::black);
		painter.drawText(m_filterRight + 5, m_filterTop + 44, str1);
		painter.drawText(m_filterRight + 5, m_filterTop + 64, str2);
		// foreground
		painter.setPen(Qt::white);
		painter.drawText(m_filterRight + 3, m_filterTop + 42, str1);
		painter.drawText(m_filterRight + 3, m_filterTop + 62, str2);
	}
}

void QGLReceiverPanel::drawWaterfall() {
	if (m_waterfallRect.isEmpty()) return;

	int top = m_waterfallRect.top();
	int left = m_waterfallRect.left();
    int width = m_waterfallRect.width();
    int height = m_waterfallRect.height();
	//int height = this->size().height();


	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	// check for framebuffer objects
	if (set->getFBOPresence()) {
		const bool useDirectWaylandPath = QGuiApplication::platformName().contains("wayland", Qt::CaseInsensitive);

		if (useDirectWaylandPath) {

			if (m_dataEngineState == QSDR::DataEngineUp) {

				const int frameSize = width * height;
				if (width > 0 && height > 0 && (m_waterfallUpdate || m_waterfallFramePixel.size() != frameSize)) {

					m_waterfallFramePixel.clear();
					m_waterfallFramePixel.resize(frameSize);

					TGL_ubyteRGBA black;
					black.red = 0; black.green = 0; black.blue = 0; black.alpha = 255;

					for (int i = 0; i < frameSize; ++i)
						m_waterfallFramePixel[i] = black;

					m_waterfallLineCnt = 0;
					m_waterfallUpdate = false;
				}

				if (frameSize > 0 && m_waterfallFramePixel.size() == frameSize) {

					if (height > 1) {
						std::memmove(
							m_waterfallFramePixel.data() + width,
							m_waterfallFramePixel.data(),
							sizeof(TGL_ubyteRGBA) * width * (height - 1));
					}

					std::memcpy(
						m_waterfallFramePixel.data(),
						m_waterfallPixel.data(),
						sizeof(TGL_ubyteRGBA) * width);

					if (m_waterfallTextureId == 0)
						glGenTextures(1, &m_waterfallTextureId);

					GLint oldTex;
					glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTex);

					glBindTexture(GL_TEXTURE_2D, m_waterfallTextureId);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_waterfallFramePixel.data());

					renderTexture(QRect(left, top, width, height), m_waterfallTextureId, 0.0f);

					glBindTexture(GL_TEXTURE_2D, oldTex);

					m_waterfallLineCnt++;
					if (m_waterfallLineCnt > height) m_waterfallLineCnt = height;
				}
			}
			else {

				glScissor(left, size().height() - (top + height), width, height);
				glEnable(GL_SCISSOR_TEST);
				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);
				glDisable(GL_SCISSOR_TEST);
			}

			return;
		}
	
		// create the FBOs if not exist
		if (!m_textureFBO || !m_waterfallLineFBO || !m_waterfallFBO || m_waterfallUpdate) {
	
			if (m_waterfallLineFBO) {

				delete m_waterfallLineFBO;
				m_waterfallLineFBO = 0;
			}
		
			if (m_waterfallFBO) {

				delete m_waterfallFBO;
				m_waterfallFBO = 0;
			}

			if (m_textureFBO) {

				delete m_textureFBO;
				m_textureFBO = 0;
			}

			if (QOpenGLFramebufferObject::hasOpenGLFramebufferBlit()) {
			
				//QOpenGLFramebufferObjectFormat format;
				//format.setSamples(2);
				//format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);

				m_waterfallLineFBO = new QOpenGLFramebufferObject(width , 1);
				//if (m_waterfallLineFBO)
				//GRAPHICS_DEBUG << "m_waterfallLineFBO generated.";

				m_waterfallFBO = new QOpenGLFramebufferObject(width, height);
				//if (m_waterfallFBO)
				//GRAPHICS_DEBUG << "m_waterfallFBO generated.";

                m_textureFBO = new QOpenGLFramebufferObject(width, height);
				//if (m_textureFBO)
				//GRAPHICS_DEBUG << "m_textureFBO generated.";
				
				// Avoid linear filtering shimmer when shifting by single-pixel rows.
				glBindTexture(GL_TEXTURE_2D, m_waterfallLineFBO->texture());
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

				glBindTexture(GL_TEXTURE_2D, m_waterfallFBO->texture());
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

				glBindTexture(GL_TEXTURE_2D, m_textureFBO->texture());
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glBindTexture(GL_TEXTURE_2D, 0);

			}	
			else {
			
				GRAPHICS_DEBUG << "has no OpenGL FramebufferBlit";
			}

			m_waterfallUpdate = false;

			// Clear the FBOs to black
			for (auto *fbo : {m_waterfallFBO, m_textureFBO, m_waterfallLineFBO}) {
				if (fbo) { fbo->bind(); glClearColor(0,0,0,1); glClear(GL_COLOR_BUFFER_BIT); fbo->release(); }
			}

			m_waterfallLineCnt = 0;
						
			if (width > 0) {

				m_waterfallPixel.clear();
				m_waterfallPixel.resize(width);
		
				TGL_ubyteRGBA col;
				col.red = 0; col.green = 0;	col.blue = 0; col.alpha = 255;
		
				for (int i = 0; i < width; i++) 
					m_waterfallPixel[i] = col;
			}
		}

		if (m_dataEngineState == QSDR::DataEngineUp) {

			GLint oldTex;
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTex);

			m_waterfallLineFBO->bind();
			glBindTexture(GL_TEXTURE_2D, m_waterfallLineFBO->texture());
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, 1, GL_RGBA, GL_UNSIGNED_BYTE, m_waterfallPixel.data());
			glBindTexture(GL_TEXTURE_2D, oldTex);
			m_waterfallLineFBO->release();

			m_waterfallLineCnt++;
			if (m_waterfallLineCnt > height) m_waterfallLineCnt = height;

			// Scroll: shift old content DOWN toward screen bottom.
			// blitFramebuffer uses GL coords (y=0 at bottom, y=height-1 at top/screen-top).
			// Copy GL rows 1..height-1 → rows 0..height-2 (shift down by 1).
			QOpenGLFramebufferObject::blitFramebuffer(
				m_textureFBO, QRect(0, 0, width, height - 1),
				m_waterfallFBO, QRect(0, 1, width, height - 1),
				GL_COLOR_BUFFER_BIT, GL_NEAREST);

			// Place new line at GL row height-1 (FBO top = screen top).
			QOpenGLFramebufferObject::blitFramebuffer(
				m_textureFBO, QRect(0, height - 1, width, 1),
				m_waterfallLineFBO, QRect(0, 0, width, 1),
				GL_COLOR_BUFFER_BIT, GL_NEAREST);

			// Draw the completed waterfall texture to screen
			glBindTexture(GL_TEXTURE_2D, oldTex);
			renderTexture(QRect(left, top, width, height), m_textureFBO->texture(), 0.0f);

			// Paint black over any not-yet-filled rows at the bottom
			if (m_waterfallLineCnt < height) {
				int unfilledTop = top + m_waterfallLineCnt;
				int unfilledH   = height - m_waterfallLineCnt;
				glScissor(left, size().height() - (unfilledTop + unfilledH), width, unfilledH);
				glEnable(GL_SCISSOR_TEST);
				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);
				glDisable(GL_SCISSOR_TEST);
			}

			// Swap so m_waterfallFBO holds the completed frame for next cycle
			qSwap(m_waterfallFBO, m_textureFBO);


		}
		else {

			glScissor(left, size().height() - (top + height), width, height);
			glEnable(GL_SCISSOR_TEST);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			glDisable(GL_SCISSOR_TEST);
		}
	}
	else {
	}
}

void QGLReceiverPanel::drawWaterfallVerticalScale() {

	if (!m_secScaleWaterfallRect.isValid()) return;

    int width = m_secScaleWaterfallRect.width();
    int height = m_secScaleWaterfallRect.height();
    
    // Safety check for valid dimensions and reasonable size bounds
    if (width <= 0 || height <= 0 || width > 1000 || height > 10000) {
        return;
    }
    
    // Additional check: don't render if dimensions seem unreasonable during startup
    if (m_resizeTime.elapsed() < 500 && (width > size().width() || height > size().height())) {
        return;
    }

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	if (!m_secScaleWaterfallFBO || m_secScaleWaterfallUpdate || m_secScaleWaterfallRenew)	{

		if (!m_secScaleWaterfallFBO || m_secScaleWaterfallRenew) {

			if (m_secScaleWaterfallFBO) {

				delete m_secScaleWaterfallFBO;
				m_secScaleWaterfallFBO = 0;
			}
            // Create FBO with logical size - consistent with panadapter scales
            m_secScaleWaterfallFBO = new QOpenGLFramebufferObject(width, height);
			//if (m_secScaleWaterfallFBO)
			//	GRAPHICS_DEBUG << "m_secScaleWaterfallFBO generated.";
		}

        glPushAttrib(GL_VIEWPORT_BIT | GL_TEXTURE_BIT);
        glViewport(0, 0, width, height);
        setProjectionOrthographic(width, height);

				m_secScaleWaterfallFBO->bind();
				renderWaterfallVerticalScale();
			m_secScaleWaterfallFBO->release();

        glPopAttrib();

		m_secScaleWaterfallUpdate = false;
		m_secScaleWaterfallRenew = false;
	}

	renderTexture(m_secScaleWaterfallRect, m_secScaleWaterfallFBO->texture(), 4.0f);
    glViewport(0, 0, size().width(), size().height());
    setProjectionOrthographic(size().width(), size().height());
}

void QGLReceiverPanel::drawCrossHair() {

	QRect rect(0, m_panRect.top(), width(), height() - m_panRect.top());

	int x = m_mousePos.x();
	int y = m_mousePos.y();

	// Scale values by DPR for high-DPI displays
	qreal dpr = devicePixelRatio();
	int textOffset = 20 * dpr;
	int spacing = 6 * dpr;
	int crossHairSize = 20 * dpr;

	QPainter painter(this);
	painter.setClipRect(rect);

	// horizontal line
	painter.setPen(QPen(QColor(95, 95, 95, 255), 1));
	painter.drawLine(m_dBmScalePanRect.right() - 2, y, rect.right() - 1, y);

	// vertical line
	painter.drawLine(x, rect.top() + 1, x, rect.bottom() - 1);

	// cross hair
	painter.setPen(QPen(QColor(180, 180, 180, 255), 1));
	painter.drawLine(x,               y - crossHairSize, x,               y + crossHairSize);
	painter.drawLine(x - crossHairSize, y,               x + crossHairSize, y);

	// text
	QString dFstr;
	QString fstr;
	QString dBstr;

	int dx = m_panRect.width()/2 - x;
	qreal unit = (qreal)((m_sampleRate * m_freqScaleZoomFactor) / m_panRect.width());
	qreal df = unit * dx;
	qreal frequency = m_centerFrequency - df;
	
	dFstr = frequencyString(m_deltaFrequency - df, true);
	fstr = frequencyString(frequency);

	qreal dBm = glPixelTodBm(m_panRect, m_dBmPanMax, m_dBmPanMin, y);
	dBstr = QString::number(dBm, 'f', 1) + " dBm";

	int rectWidth;
	int fontHeight;
	if (m_smallSize) {

		rectWidth = m_fonts.smallFontMetrics->boundingRect(fstr).width();
		fontHeight = m_fonts.smallFontMetrics->tightBoundingRect("0").height() + spacing;
		painter.setFont(m_fonts.smallFont);
	}
	else {

        rectWidth = m_fonts.bigFont1Metrics->horizontalAdvance(fstr);
		fontHeight = m_fonts.bigFont1Metrics->tightBoundingRect("0").height() + spacing;
		painter.setFont(m_fonts.bigFont1);
	}

	m_haircrossMaxRight = rectWidth + textOffset;
	m_smallSize ? m_haircrossMinTop = 40 * dpr : m_haircrossMinTop = 60 * dpr;


	int tx, ty;
	if (x > m_panRect.width() - m_haircrossMaxRight) {

		tx = x - m_haircrossMaxRight;
		if (y > m_haircrossMinTop)
			m_smallSize ? ty = y - (42 * dpr) : ty = y - (62 * dpr);
		else
			m_smallSize ? ty = y + (10 * dpr) : ty = y + (30 * dpr);
	}
	else {

		tx = x + textOffset;
		if (y > m_haircrossMinTop)
			m_smallSize ? ty = y - (42 * dpr) : ty = y - (62 * dpr);
		else
			m_smallSize ? ty = y + (10 * dpr) : ty = y + (30 * dpr);
	}

	// delta frequency and frequency
	painter.setPen(QColor(200, 200, 200, 255));
	painter.drawText(tx, ty, dFstr);
	painter.drawText(tx, ty + fontHeight, fstr);

	// dBm value
	if (m_mouseRegion == panadapterRegion)
		painter.drawText(tx, ty + 2*fontHeight, dBstr);

	// Ham band text
	if (m_oldMousePosX != m_mousePos.x()) {

		m_bandText = getHamBandTextString(set->getHamBandTextList(), true, frequency);
		m_oldMousePosX = m_mousePos.x();
	}

	painter.setPen(QColor(240, 209, 110, 255));
	painter.drawText(tx, ty + (m_smallSize ? 4 : 5)*fontHeight, m_bandText);
}

void QGLReceiverPanel::drawVFOControl() {

	QPainter painter(this);
	painter.setFont(m_fonts.smallFont);

	// lock Panadapter
	QString str = "PAN LOCKED";
	int x1 = m_dBmScalePanRect.right() + 5;
	int y1 = 3;

	if (m_panLocked) {
		
		QColor fgColor = (m_dataEngineState == QSDR::DataEngineUp)
		                 ? QColor(255, 170, 90, 200)
		                 : QColor(150, 150, 150, 100);
		painter.setPen(Qt::black);
		painter.drawText(x1+3, y1, str);
		painter.setPen(fgColor);
		painter.drawText(x1+1, y1-2, str);
	}
	
	// click VFO
    x1 += m_fonts.smallFontMetrics->horizontalAdvance(str) + 12;
	str = "CLICK VFO";

	if (m_clickVFO) {

		QColor fgColor = (m_dataEngineState == QSDR::DataEngineUp)
		                 ? QColor(255, 170, 90, 200)
		                 : QColor(150, 150, 150, 100);
		painter.setPen(Qt::black);
		painter.drawText(x1+3, y1, str);
		painter.setPen(fgColor);
		painter.drawText(x1+1, y1-2, str);
	}

	// FFT size
	str = "sample size: %1";
    x1 = m_panRect.right() - m_fonts.smallFontMetrics->horizontalAdvance(str) - 65;

	if (m_dataEngineState == QSDR::DataEngineUp) {
		painter.setPen(Qt::black);
		painter.drawText(x1+3, y1, str.arg(m_sampleSize));
		painter.setPen(QColor(255, 170, 90, 200));
		painter.drawText(x1+1, y1-2, str.arg(m_sampleSize));
	}
		
	str = "FFT: %1";
	QString s;

	switch (m_fftMult) {

		case 1:  s = "4k";  break;
		case 2:  s = "8k";  break;
		case 4:  s = "16k"; break;
		case 8:  s = "32k"; break;
		case 16: s = "64k"; break;
	}
    x1 = m_panRect.right() - m_fonts.smallFontMetrics->horizontalAdvance(str) - 5;

	if (m_dataEngineState == QSDR::DataEngineUp) {
		painter.setPen(Qt::black);
		painter.drawText(x1+3, y1, str.arg(s));
		painter.setPen(QColor(255, 170, 90, 200));
		painter.drawText(x1+1, y1-2, str.arg(s));
	}


	int delta = qRound((m_deltaF * m_panRect.width())/m_freqScaleZoomFactor);

	if (delta > m_panRect.width()/2) {
	
		QColor col = QColor(255, 40, 40, 255);
		str = "<< VFO %1";
		str = str.arg(frequencyString(m_vfoFrequency, false));

		int x = m_dBmScalePanRect.right();
		int y = 25;

        QRect rect = QRect(x, y, m_fonts.smallFontMetrics->horizontalAdvance(str) + 4, m_fonts.fontHeightSmallFont + 2);
		painter.fillRect(rect, col);
		painter.setPen(Qt::white);
		painter.drawText(x+1, y + m_fonts.fontHeightSmallFont - 2, str);
	}

	if (delta < -m_panRect.width()/2) {
		
		QColor col = QColor(255, 40, 40, 255);
		str = "%1 VFO >>";
		str = str.arg(frequencyString(m_vfoFrequency, false));

        int x = m_panRect.right() - m_fonts.smallFontMetrics->horizontalAdvance(str);
		int y = 25;

        QRect rect = QRect(x, y, m_fonts.smallFontMetrics->horizontalAdvance(str) + 4, m_fonts.fontHeightSmallFont + 2);
		painter.fillRect(rect, col);
		painter.setPen(Qt::white);
		painter.drawText(x+1, y + m_fonts.fontHeightSmallFont - 2, str);
	}
}

void QGLReceiverPanel::drawReceiverInfo() {

	QString str;
    QPainter painter(this);
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
	drawGLRect(rect, col, 2.0f);
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
	//drawGLRect(m_agcButtonRect, col, 2.0f);
	//qglColor(QColor(0, 0, 0));
	//m_oglTextSmall->renderFreqText(x1+3, y1-2, 3.0f, str);


	// main frequency display
	if (m_panRect.height() > 15) {

        int fLength = m_fonts.bigFont1Metrics->horizontalAdvance("55.555.555") + 30;
		//GLint x = m_panRect.width()/2 - 65;
		GLint x = m_panRect.left() + qRound((qreal)(m_panRect.width()/2.0f)  - m_deltaF * m_panRect.width() / m_freqScaleZoomFactor) + 10;
		if (x > m_panRect.right() - fLength) x -= fLength + 20;

		int alpha = 0; (void)alpha; // retained for potential future use
		QColor colFlt;
		QColor colADC;
		QColor colAGC;
		QColor colDSP;
		QRect rect;

		if (m_dataEngineState == QSDR::DataEngineUp) {

			if (m_receiver == set->getCurrentReceiver()) {

				colDSP = QColor(1, 190, 180, 180);
				colFlt = QColor(200, 190, 50, 180);
				colADC = QColor(215, 130, 50, 180);
				if (m_showAGCLines)
					colAGC = QColor(255, 170, 90, 180);
				else
					colAGC = QColor(215, 130, 50, 180);
							
				alpha = 255;
			}
			else {

				alpha = 155;
				colFlt = QColor(110, 100, 1, 180);
				colDSP = QColor(1, 100, 90, 180);
				colAGC = QColor(165, 80, 1);
				colADC = QColor(165, 80, 1);
			}
		}
		else {

			alpha = 100;
			colFlt = m_darkColor;
			colADC = m_darkColor;
			colAGC = m_darkColor;
			colDSP = m_darkColor;
		}


		str = "%1";
		str = str.arg(m_filterWidthString);

		int x1 = x;
		int y1 = 3;
        painter.setFont(m_fonts.smallFont);
        painter.setPen(Qt::black);
        // Filter width
        rect = QRect(x1, y1, m_fonts.smallFontMetrics->horizontalAdvance(str) + 3, m_fonts.fontHeightSmallFont + 2);
        painter.fillRect(rect,colFlt);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawText(rect,Qt::AlignCenter,str);
		// DSP mode
        x1 += m_fonts.smallFontMetrics->horizontalAdvance(str) + 5;
		str = "%1";
		str = str.arg(m_dspModeString);
		
        rect = QRect(x1, y1, m_fonts.smallFontMetrics->horizontalAdvance(str) + 3, m_fonts.fontHeightSmallFont + 2);
        painter.fillRect(rect,colDSP);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawText(rect,Qt::AlignCenter,str);

		// AGC mode
        x1 += m_fonts.smallFontMetrics->horizontalAdvance(str) + 4;

		str = "%1";
		str = str.arg(m_agcModeString);
        rect = QRect(x1, y1, m_fonts.smallFontMetrics->horizontalAdvance(str) + 4, m_fonts.fontHeightSmallFont + 2);
        m_agcButtonRect = rect;

         painter.fillRect(rect,colAGC);
         painter.drawText(rect,Qt::AlignCenter,str);

		// ADC mode
        x1 += m_fonts.smallFontMetrics->horizontalAdvance(str) + 4;
		str = "%1";
		str = str.arg(m_adcModeString);

        rect = QRect(x1, y1, m_fonts.smallFontMetrics->horizontalAdvance(str) + 4, m_fonts.fontHeightSmallFont + 2);
        painter.fillRect(rect,colADC);
        painter.drawText(rect,Qt::AlignCenter,str);

		// VFO frequency
		TFrequency f;
		f.freqMHz = (int)(m_vfoFrequency / 1000);
		f.freqkHz = (int)(m_vfoFrequency % 1000);
        y1 += 20;
		str = "%1.%2";
		int f1 = f.freqMHz;
		int f2 = f.freqkHz;

		QString fstr = str.arg(f1/1000).arg(f1 - 1000 * (int)(f1/1000), 3, 10, QLatin1Char('0'));
        
        // Calculate string widths using font metrics (logical coordinates)
        int fstrWidth = m_fonts.bigFont2Metrics->horizontalAdvance(fstr);
        QString kHzStr = str.arg(f2, 3, 10, QLatin1Char('0'));

        painter.setPen(Qt::white);
        painter.setFont(m_fonts.bigFont2);
        painter.drawText(x+2,y1,fstr);
        painter.drawText(x + fstrWidth + 2,y1,"Mhz");




	if (m_panRect.height() > 15 && m_deltaFrequency != 0) {

		// center frequency
		TFrequency f;
		f.freqMHz = (int)(m_centerFrequency / 1000);
		f.freqkHz = (int)(m_centerFrequency % 1000);

        y1 += 25;
        str = "%1.%2";
        int f1 = f.freqMHz;
        int f2 = f.freqkHz;

        QString fstr = str.arg(f1/1000).arg(f1 - 1000 * (int)(f1/1000), 3, 10, QLatin1Char('0'));

        // Calculate string widths using font metrics (logical coordinates)
        int fstrWidth = m_fonts.bigFont2Metrics->horizontalAdvance(fstr);
        QString kHzStr = str.arg(f2, 3, 10, QLatin1Char('0'));

        painter.setPen(Qt::white);
        painter.setFont(m_fonts.bigFont2);
        painter.drawText(x+2,y1,fstr);
        painter.drawText(x + fstrWidth + 2,y1,"Mhz");
	}

    }
}

void QGLReceiverPanel::drawAGCControl() {

	QPainter painter(this);
	painter.setFont(m_fonts.smallFont);
	painter.setClipRect(m_panRect);

	if (m_agcMode == (AGCMode) agcOFF) {

		m_agcFixedGainLevelPixel = dBmToGLPixel(m_panRect, m_dBmPanMax, m_dBmPanMin, -m_agcFixedGain);

		QString str = "AGC-F";
		// shadow text
		painter.setPen(QColor(0, 0, 0, 255));
		painter.drawText(m_panRect.right() - 32, m_agcFixedGainLevelPixel - 13, str);
		// main text
		painter.setPen(QColor(225, 125, 225, 255));
		painter.drawText(m_panRect.right() - 34, m_agcFixedGainLevelPixel - 15, str);

		// AGC fixed gain line (shadow)
		painter.setPen(QPen(QColor(0, 0, 0, 255), 1, Qt::DashLine));
		painter.drawLine(m_dBmScalePanRect.right() - 1, m_agcFixedGainLevelPixel + 2, m_panRect.right() - 1, m_agcFixedGainLevelPixel);
		// AGC fixed gain line (main)
		painter.setPen(QPen(QColor(225, 125, 225, 255), 1, Qt::DashLine));
		painter.drawLine(m_dBmScalePanRect.right() - 3, m_agcFixedGainLevelPixel, m_panRect.right() - 1, m_agcFixedGainLevelPixel);

	}
	else {

		m_agcThresholdPixel = dBmToGLPixel(m_panRect, m_dBmPanMax, m_dBmPanMin, m_agcThresholdOld);

		if (m_agcHangEnabled)
			m_agcHangLevelPixel = dBmToGLPixel(m_panRect, m_dBmPanMax, m_dBmPanMin, m_agcHangLevelOld);

		QString str = "AGC-T";
		painter.setPen(QColor(0, 0, 0, 255));
		painter.drawText(m_panRect.right() - 32, m_agcThresholdPixel - 13, str);
		painter.setPen(QColor(225, 125, 125, 255));
		painter.drawText(m_panRect.right() - 34, m_agcThresholdPixel - 15, str);

		// AGC threshold line
		painter.setPen(QPen(QColor(0, 0, 0, 255), 1, Qt::DashLine));
		painter.drawLine(m_dBmScalePanRect.right() - 1, m_agcThresholdPixel + 2, m_panRect.right() - 1, m_agcThresholdPixel);
		painter.setPen(QPen(QColor(225, 125, 125, 255), 1, Qt::DashLine));
		painter.drawLine(m_dBmScalePanRect.right() - 3, m_agcThresholdPixel, m_panRect.right() - 1, m_agcThresholdPixel);

		// AGC hang threshold line
		if (m_agcHangEnabled) {

			str = "AGC-H";
			painter.setPen(QColor(0, 0, 0, 255));
			painter.drawText(m_panRect.right() - 32, m_agcHangLevelPixel - 13, str);
			painter.setPen(QColor(125, 225, 125, 255));
			painter.drawText(m_panRect.right() - 34, m_agcHangLevelPixel - 15, str);

			painter.setPen(QPen(QColor(0, 0, 0, 255), 1, Qt::DashLine));
			painter.drawLine(m_dBmScalePanRect.right() - 1, m_agcHangLevelPixel + 2, m_panRect.right() - 1, m_agcHangLevelPixel);
			painter.setPen(QPen(QColor(125, 225, 125, 255), 1, Qt::DashLine));
			painter.drawLine(m_dBmScalePanRect.right() - 3, m_agcHangLevelPixel, m_panRect.right() - 1, m_agcHangLevelPixel);
		}
	}
}
 


void QGLReceiverPanel::renderPanHorizontalScale() {

	//GRAPHICS_DEBUG << "render frequency scale";
	int fontHeight;
	int fontMaxWidth;
    QColor textColor = QColor(140, 180, 200);
    qreal freqSpan = (qreal)(m_sampleRate * m_freqScaleZoomFactor);
    qreal lowerFreq = (qreal)m_centerFrequency - freqSpan / 2;
    qreal upperFreq = (qreal)m_centerFrequency + freqSpan / 2;
    qreal unit = (qreal)(m_freqScalePanRect.width() / freqSpan);

    QOpenGLPaintDevice paintDevice(m_frequencyScaleFBO->size());
    painter.begin(&paintDevice);

    fontHeight = m_fonts.smallFontMetrics->tightBoundingRect(".0kMGHz").height();
    fontMaxWidth = m_fonts.smallFontMetrics->boundingRect("000.000.0").width();

	m_frequencyScale = getXRuler(m_freqScalePanRect, fontMaxWidth, unit, lowerFreq, upperFreq);

	// draw the frequency scale
	int		offset_X		= -1;
	int		textOffset_y	= 5;
	double	freqScale		= 1;

	QString fstr = QString(" Hz ");
	if (upperFreq >= 1e6) { freqScale = 1e6; fstr = QString("  MHz "); }
	else
	if (upperFreq >= 1e3) { freqScale = 1e3; fstr = QString("  kHz "); }

	// draw the scale background
    painter.fillRect(0, 0, m_freqScalePanRect.width(), m_freqScalePanRect.height(), QColor(0, 0, 0, 255));

    QRect scaledTextRect(0, textOffset_y, 1, m_freqScalePanRect.height());
    scaledTextRect.setWidth(m_fonts.smallFontMetrics->horizontalAdvance(fstr));
    scaledTextRect.moveLeft(m_freqScalePanRect.width() - scaledTextRect.width());// - menu_pull_right_rect.width());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setFont(m_fonts.smallFont);  // Set consistent font for horizontal scale
    // Remove double scaling - framebuffer is already DPR-scaled
    // painter.scale(dpr,dpr);
    painter.setPen(QPen(textColor,3, Qt::SolidLine, Qt::FlatCap));
	int len = m_frequencyScale.mainPointPositions.length();
	if (len > 0) {

		for (int i = 0; i < len; i++) {
            painter.drawLine(m_frequencyScale.mainPointPositions.at(i), 1, m_frequencyScale.mainPointPositions.at(i), 4);
		}
		
		for (int i = 0; i < len; i++) {
		
			QString str = QString::number(m_frequencyScale.mainPoints.at(i) / freqScale, 'f', 3);
			
			if (i > 0) {

				double delta = m_frequencyScale.mainPoints.at(i) - m_frequencyScale.mainPoints.at(i-1);
				if (delta < 1000.0)
					str = QString::number(m_frequencyScale.mainPoints.at(i) / freqScale, 'f', 4);
				else
				if (freqScale == 1e3)
					while (str.endsWith('0')) str.remove(str.size() - 1, 1);
			}
			if (str.endsWith('.')) str.remove(str.size() - 1, 1);

            int textWidth = m_fonts.smallFontMetrics->horizontalAdvance(str);

            QRect textRect(m_frequencyScale.mainPointPositions.at(i) + offset_X - (textWidth / 2), textOffset_y, textWidth, fontHeight);

			if (textRect.left() < 0 || textRect.right() >= scaledTextRect.left()) continue;
             painter.drawText(textRect.x(), textRect.y() + m_fonts.smallFontMetrics->height(), str);

		}
	}

    len = m_frequencyScale.subPointPositions.length();
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            painter.drawLine(m_frequencyScale.subPointPositions.at(i), 1, m_frequencyScale.subPointPositions.at(i), 3);
        }
    }

	painter.setPen(QPen(QColor(239, 56, 109)));
     painter.drawText(m_freqScalePanRect.width() - 30, textOffset_y + 10, fstr);
    painter.end();
}

void QGLReceiverPanel::renderPanadapterGrid() {
    // Superseded by drawPanadapterGrid() which uses QPainter directly.
}
 
void QGLReceiverPanel::renderWaterfallVerticalScale() {

	QString str;

	int spacing = 7;
	int fontHeight;
	int fontMaxWidth;
	if (m_smallSize) {

		fontHeight = m_fonts.smallFontMetrics->tightBoundingRect(".0s").height() + spacing;
		fontMaxWidth = m_fonts.smallFontMetrics->boundingRect("000.0").width();
	}
	else {

		fontHeight = m_fonts.bigFont1Metrics->tightBoundingRect(".0s").height() + spacing;
		fontMaxWidth = m_fonts.bigFont1Metrics->boundingRect("000.0").width();
	}

    GLint width = m_secScaleWaterfallRect.width();
    GLint height = m_secScaleWaterfallRect.height();

	qreal unit = (qreal)(height / qAbs(m_secWaterfallMax - m_secWaterfallMin));

	m_secScale = getYRuler2(m_secScaleWaterfallRect, fontHeight, unit, m_secWaterfallMin, m_secWaterfallMax);
	//m_secScale = getYRuler3(m_secScaleWaterfallRect, fontHeight, unit, m_secWaterfallMin, m_secWaterfallMax, 10.0f);

	//glClear(GL_COLOR_BUFFER_BIT);

	QRect textRect(0, 0, fontMaxWidth, fontHeight);
	if (m_smallSize)
		textRect.moveLeft(3);
	else
		textRect.moveLeft(-1);

	int yOld = -textRect.height();

	int len		= m_secScale.mainPointPositions.length();
	int sublen	= m_secScale.subPointPositions.length();

	// draw tick marks and labels with QPainter
	QOpenGLPaintDevice paintDevice(QSize(width, height));
	QPainter painter(&paintDevice);
	painter.setRenderHint(QPainter::Antialiasing, false);
	painter.fillRect(0, 0, width, height, QColor(40, 40, 40, 180));

	if (len > 0) {

		painter.setPen(QPen(QColor(166, 194, 207), 1));
		for (int i = 0; i < len; i++) {
			int y = (int)m_secScale.mainPointPositions.at(i);
			painter.drawLine(width - 4, y, width, y);
		}

		if (sublen > 0) {
			painter.setPen(QPen(QColor(115, 143, 156), 1));
			for (int i = 0; i < sublen; i++) {
				int y = (int)m_secScale.subPointPositions.at(i);
				painter.drawLine(width - 2, y, width, y);
			}
		}

		painter.setPen(QColor(242, 245, 232));
		painter.setFont(m_smallSize ? m_fonts.smallFont : m_fonts.bigFont1);

		for (int i = 0; i < len; i++) {

			textRect.moveBottom((int)m_secScale.mainPointPositions.at(i) + textRect.height()/2);

			if (textRect.y() >= yOld && textRect.bottom() <= (height - textRect.height())) {

				str = QString::number(m_secScale.mainPoints.at(i), 'f', 1);
				int strWidth = m_smallSize
				    ? m_fonts.smallFontMetrics->tightBoundingRect(str).width()
				    : m_fonts.bigFont1Metrics->tightBoundingRect(str).width();
				painter.drawText(textRect.x() + fontMaxWidth - strWidth, textRect.y() + fontHeight, str);
				yOld = textRect.bottom();
			}
		}
	}

	textRect.moveTop(height - textRect.height());
	painter.setPen(QColor(240, 56, 110));
	str = QString("sec");
	painter.drawText(m_smallSize ? textRect.x() : textRect.x() + 10, textRect.y() + fontHeight, str);
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
	else if (qAbs(p.x() - m_filterRect.left()) < m_snapMouse &&
			 m_panRect.contains(p)
	) {
		m_mouseRegion = filterRegionLow;
		m_mouseDownFilterFrequencyLo = m_filterLowerFrequency;
	}
	else if (qAbs(p.x() - m_filterRect.right()) < m_snapMouse &&
			 m_panRect.contains(p)
	) {
		m_mouseRegion = filterRegionHigh;
		m_mouseDownFilterFrequencyHi = m_filterUpperFrequency;
	}
	else if (m_filterRect.contains(p)) {

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
    
    // Set up viewport and projection
  //  glViewport(0, 0, iWidth, iHeight);
  //  setProjectionOrthographic(iWidth, iHeight);
    
    // Mark all for renewal
    m_panGridRenew = true;
    m_panFrequencyScale = true;
    m_panGridRenew = true;
    m_spectrumVertexColorUpdate = true;
    m_waterfallUpdate = true;

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
			//m_freqScalePanRect.top(),
			m_freqScalePanRect.width(),
			//size.height() - m_displayTop - m_freqScalePanRect.top() - m_freqScalePanRect.height());
			size.height() - m_freqScalePanRect.top() - m_freqScalePanRect.height());
			//size.height() - m_freqScalePanRect.top());	
			
	m_waterfallUpdate = true;

	if ((m_panRect.height() + m_waterfallRect.height()) > m_bigHeight && m_panRect.width() > m_bigWidth)
		m_smallSize = false;
	else
		m_smallSize = true;
	
	m_dBmScalePanRect = QRect(
						m_panRect.left(), 
						m_displayTop, 
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
			
				if (m_centerFrequency + delta > MAXFREQUENCY)
					m_centerFrequency = MAXFREQUENCY;
				else if (m_centerFrequency + delta < 0)
					m_centerFrequency = 0;
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
			
			set->setCtrFrequency(this, 0, m_receiver, m_centerFrequency);
			set->setVFOFrequency(this, 0, m_receiver, m_vfoFrequency);
			break;
	}

 	//updateGL();
}

void QGLReceiverPanel::mousePressEvent(QMouseEvent* event) {
	
	//GRAPHICS_DEBUG << "mousePressEvent";
	m_mousePos = event->pos();
	m_mouseDownPos = m_mousePos;

	getRegion(m_mousePos);

	if (m_mouseRegion == agcButtonRegion) {

		if (event->buttons() == Qt::LeftButton) {

			if (m_showAGCLines) {

				m_showAGCLines = false;
				set->setAGCShowLines(this, m_receiver, false);
			}
			else {

				m_showAGCLines = true;
				set->setAGCShowLines(this, m_receiver, true);
			}
		}
	}
	else if (m_mouseRegion == panadapterRegion || m_mouseRegion == waterfallRegion) {

		if (event->buttons() == Qt::LeftButton && m_receiver != set->getCurrentReceiver()) {

			set->setCurrentReceiver(this, m_receiver);
		}
		else if (event->buttons() == Qt::LeftButton && m_clickVFO) {

			m_crossHairCursor = false;
			setCursor(Qt::OpenHandCursor);
			m_dragMouse = true;



            int dx = m_panRect.width()/2 - m_mousePos.x();
			qreal unit = (qreal)((m_sampleRate * m_freqScaleZoomFactor) / m_panRect.width());
            m_vfoFrequency = (long)(qRound((m_centerFrequency - (unit * dx))));
			set->setVFOFrequency(this, 0, m_receiver, m_vfoFrequency);		
		}
		else if (event->buttons() == Qt::LeftButton) {

			m_crossHairCursor = false;
			setCursor(Qt::OpenHandCursor);
			m_dragMouse = true;
		}
		else if (event->buttons() == Qt::RightButton) {

			showRadioPopup(true);
		}
	}
	else if (m_mouseRegion == filterRegion) {

		//setCursor(Qt::ArrowCursor);
		if (event->buttons() == Qt::LeftButton)
			m_highlightFilter = true;
	}
	else if (m_mouseRegion == freqScalePanadapterRegion) {

		m_rulerMouseDownPos = m_freqScalePanRect.topLeft();
		
		if (event->buttons() == Qt::RightButton) setCursor(Qt::SplitHCursor);

		return;
	}
	else if (m_mouseRegion == dBmScalePanadapterRegion) {

		m_rulerMouseDownPos = m_dBmScalePanRect.topLeft();

		if (event->buttons() == Qt::RightButton) 
			setCursor(Qt::SplitVCursor);
		return;
	}
	
}

void QGLReceiverPanel::mouseReleaseEvent(QMouseEvent *event) {

	//GRAPHICS_DEBUG << "mouseReleaseEvent";
	m_mousePos = event->pos();
	m_mouseDownPos = m_mousePos;

	getRegion(m_mousePos);

	if (m_mouseRegion == freqScalePanadapterRegion) {

		return;
	}
	//else if (m_mouseRegion == panadapterRegion || m_mouseRegion == waterfallRegion) {
	//}
	m_dragMouse = false;
	m_crossHairCursor = true;
	if (m_crossHair)
		setCursor(Qt::BlankCursor);
	else
		setCursor(Qt::ArrowCursor);
}

void QGLReceiverPanel::mouseDoubleClickEvent(QMouseEvent *event) {

	//GRAPHICS_DEBUG << "mouseDoubleClickEvent";
	m_mousePos = event->pos();
	m_mouseDownPos = m_mousePos;

	getRegion(m_mousePos);

	if (m_mouseRegion == panadapterRegion) {

		if (event->buttons() == Qt::LeftButton) {

			//set->showRadioPopupWidget();
//			if (!band160mBtn->isVisible())
//				band160mBtn->show();
//			else
//				band160mBtn->hide();
		}
	}
}

void QGLReceiverPanel::mouseMoveEvent(QMouseEvent* event) {
	m_mousePos = event->pos();

    if (event->buttons() == Qt::NoButton) getRegion(m_mousePos);
	
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

				set->setAGCThreshold_dB(this, m_receiver, m_agcThresholdNew);
		//		set->setAGCMaximumGain_dB(this, m_receiver, m_agcThresholdNew);
			}
			break;

		case agcHangLine:

			//GRAPHICS_DEBUG << "agcHangLine Rx:" << m_receiver;
			if (!m_showAGCLines || (m_agcMode == (AGCMode) agcOFF) || !m_agcHangEnabled)
				break;

			m_crossHairCursor = false;
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
				set->setAGCHangThreshold(this, m_receiver, m_agcHangLevelNew);
			}
			break;

		case agcFixedGainLine:

			//GRAPHICS_DEBUG << "agcFixedGainLine Rx:" << m_receiver;
			if (!m_showAGCLines || (m_agcMode != (AGCMode) agcOFF))
				break;

			m_crossHairCursor = false;
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

				set->setAGCFixedGain_dB(this, m_receiver, -agcFixedGain);
			}
			break;

		case panadapterRegion:
		case waterfallRegion:
			
			//GRAPHICS_DEBUG << "panadapterRegion Rx:" << m_receiver;
			if (!m_dragMouse) {

				m_crossHairCursor = true;
				if (m_crossHair)
					setCursor(Qt::BlankCursor);
				else
					setCursor(Qt::ArrowCursor);
			}
			
			if (event->buttons() == Qt::LeftButton) {

                QPoint dPos = m_mouseDownPos - m_mousePos;
				
				qreal unit = (qreal)((m_sampleRate * m_freqScaleZoomFactor) / m_freqScalePanRect.width());
				qreal deltaFreq = unit * dPos.x();
				
				long newFrequency = m_centerFrequency + deltaFreq;
				if (newFrequency > MAXFREQUENCY)
					newFrequency = MAXFREQUENCY;
				else
				if (newFrequency + deltaFreq < 0)
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
				
					set->setVFOFrequency(this, 0, m_receiver, m_vfoFrequency);

					m_deltaFrequency = m_centerFrequency - m_vfoFrequency;
					m_deltaF = (qreal)(1.0*m_deltaFrequency/m_sampleRate);
				}
				else {

					m_vfoFrequency = m_centerFrequency - m_deltaFrequency;
                    GRAPHICS_DEBUG << "vfo freq " << m_vfoFrequency << m_centerFrequency;
					set->setVFOFrequency(this, 0, m_receiver, m_vfoFrequency);
					set->setCtrFrequency(this, 0, m_receiver, m_centerFrequency);
				}

                m_mouseDownPos = m_mousePos;

				m_displayCenterlineHeight = m_panRect.top() + (m_panRect.height() - 3);

				m_showFilterLeftBoundary = false;
				m_showFilterRightBoundary = false;
				m_highlightFilter = false;

//				if (m_displayTime.elapsed() >= 50) {
//
//					m_displayTime.restart();
//					update();
//				}
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
				m_panGridUpdate = true;

//				if (m_displayTime.elapsed() >= 50) {
//
//					m_displayTime.restart();
//					update();
//				}
			}
			else
			if (event->buttons() == Qt::RightButton &&
				event->modifiers() == Qt::ControlModifier) {

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
				m_panGridUpdate = true;

//				if (m_displayTime.elapsed() >= 50) {
//
//					m_displayTime.restart();
//					update();
//				}
			}
			if (event->buttons() == Qt::RightButton) {

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
				m_panGridUpdate = true;

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
				set->setFreqRulerPosition(this, m_receiver, m_freqRulerPosition);

//				if (m_displayTime.elapsed() >= 50) {
//
//					m_displayTime.restart();
//					update();
//				}
			}
			else if (event->buttons() == Qt::LeftButton) {

                QPoint dPos = m_mouseDownPos - m_mousePos;
				
				qreal unit = (qreal)((m_sampleRate * m_freqScaleZoomFactor) / m_freqScalePanRect.width());
				qreal deltaFreq = unit * dPos.x();
				
				/*if (m_freqScaleZoomFactor < 1.0) {

				}
				else {*/

					long newFrequency = m_centerFrequency + deltaFreq;
					if (newFrequency > MAXFREQUENCY)
						newFrequency = MAXFREQUENCY;
					else
					if (newFrequency + deltaFreq < 0)
						newFrequency = 0;
					else {
					
						m_centerFrequency += deltaFreq;
					}

					if (!m_panLocked) {

						m_vfoFrequency = m_centerFrequency - m_deltaFrequency;
						set->setVFOFrequency(this, 0, m_receiver, m_vfoFrequency);
						set->setCtrFrequency(this, 0, m_receiver, m_centerFrequency);
					}

					//set->setVFOFrequency(this, 0, m_receiver, m_vfoFrequency);
					//set->setCtrFrequency(this, 0, m_receiver, m_centerFrequency);

					else {

						m_deltaFrequency =  m_centerFrequency - m_vfoFrequency;
						m_deltaF = (qreal)(1.0*m_deltaFrequency/m_sampleRate);

						qreal vol = set->getMainVolume(m_receiver);
						set->setMainVolume(this, m_receiver, 0.0f);
						set->setCtrFrequency(this, 0, m_receiver, m_centerFrequency);
						set->setNCOFrequency(this, true, m_receiver, -m_deltaFrequency);
						set->setMainVolume(this, m_receiver, vol);
					}

					//set->setCtrFrequency(this, 0, m_receiver, m_centerFrequency);
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

//				if (m_displayTime.elapsed() >= 50) {
//
//					m_displayTime.restart();
//					update();
//				}
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
				qreal dFreq = (qreal)(dPos.x() * m_sampleRate * m_freqScaleZoomFactor) / m_panRect.width();

				m_filterLowerFrequency = qRound(m_mouseDownFilterFrequencyLo - dFreq);
				set->setRXFilter(this, m_receiver, m_filterLowerFrequency, m_filterUpperFrequency);

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
				qreal dFreq = (qreal)(dPos.x() * m_sampleRate * m_freqScaleZoomFactor) / m_panRect.width();

				m_filterUpperFrequency = qRound(m_mouseDownFilterFrequencyHi - dFreq);
				set->setRXFilter(this, m_receiver, m_filterLowerFrequency, m_filterUpperFrequency);

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
				qreal dFreq = (qreal)(dPos.x() * m_sampleRate * m_freqScaleZoomFactor) / m_panRect.width();

				m_filterUpperFrequency = qRound(m_mouseDownFilterFrequencyHi - dFreq);
				m_filterLowerFrequency = qRound(m_mouseDownFilterFrequencyLo - dFreq);
				set->setRXFilter(this, m_receiver, m_filterLowerFrequency, m_filterUpperFrequency);

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

	if (m_displayTime.elapsed() >= 100) {

		m_displayTime.restart();
		update();
	}
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

void QGLReceiverPanel::setSpectrumSize(QObject *sender, int value) {

	Q_UNUSED(sender)

	if (m_dataEngineState == QSDR::DataEngineDown) {

		GRAPHICS_DEBUG << "set spectrum size to: " << value;
		m_spectrumSize = value;
//		update();
	}
}

void QGLReceiverPanel::setCtrFrequency(QObject *sender, int mode, int rx, long freq) {

	Q_UNUSED(sender)
	Q_UNUSED(mode)
	
	//if (sender == this) return;
	if (m_receiver != rx) return;

	m_rxDataList[rx].ctrFrequency = freq;

	//GRAPHICS_DEBUG << "set center F = " << freq;
	m_centerFrequency = freq;
	m_freqScalePanadapterUpdate = true;
	m_panGridUpdate = true;
//	update();
}

void QGLReceiverPanel::setVFOFrequency(QObject *sender, int mode, int rx, long freq) {

	Q_UNUSED(sender)
	Q_UNUSED(mode)
	
	//if (sender == this) return;
	if (m_receiver != rx) return;

	m_rxDataList[rx].vfoFrequency = freq;

    //GRAPHICS_DEBUG << "set vfo F = " << freq;
	m_vfoFrequency = freq;
	if (m_vfoFrequency > m_centerFrequency + m_sampleRate/2)
		m_vfoFrequency = m_centerFrequency + m_sampleRate/2;
	else if (m_vfoFrequency < m_centerFrequency - m_sampleRate/2)
		m_vfoFrequency = m_centerFrequency - m_sampleRate/2;

	m_deltaFrequency = m_centerFrequency - m_vfoFrequency;
	m_deltaF = (qreal)(1.0*m_deltaFrequency/m_sampleRate);
	
    // Note: Neither grid nor frequency scale need to update for VFO changes
    // The scale shows center frequency range, not VFO position
    // m_freqScalePanadapterUpdate = true;
    // m_panGridUpdate = true;
    update();
}

void QGLReceiverPanel::setVfoToMidFrequency() {

	m_vfoFrequency = m_centerFrequency;
	m_deltaFrequency = 0;
	m_deltaF = 0;

	set->setVFOFrequency(this, 0, m_receiver, m_vfoFrequency);
	set->setNCOFrequency(this, false, m_receiver, 0);
}

void QGLReceiverPanel::setMidToVfoFrequency() {

	m_centerFrequency = m_vfoFrequency;
	m_deltaFrequency = 0;
	m_deltaF = 0;

	set->setCtrFrequency(this, 0, m_receiver, m_centerFrequency);
	set->setNCOFrequency(this, false, m_receiver, 0);
	update();
}

void QGLReceiverPanel::setFilterFrequencies(QObject *sender, int rx, qreal lo, qreal hi) {

	Q_UNUSED(sender)

	if (m_receiver != rx) return;
		
	m_filterLowerFrequency = lo;
	m_filterUpperFrequency = hi;
	m_filterChanged = true;
	update();
}

void QGLReceiverPanel::setCurrentReceiver(QObject *sender, int value) {

	Q_UNUSED(sender)

	m_currentReceiver = value;
	m_panGridUpdate = true;
	update();
}

void QGLReceiverPanel::freqRulerPositionChanged(QObject *sender, int rx, float pos) {

	Q_UNUSED (sender)

	if (rx == m_receiver) {
		
		m_freqRulerPosition = pos;
		
		setupDisplayRegions(size());
		update();
	}
}

void QGLReceiverPanel::setSpectrumBuffer(int rx, const qVectorFloat& buffer) {

	if (m_receiver != rx) return;

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

void QGLReceiverPanel::computeDisplayBins(QVector<float>& buffer, QVector<float>& waterfallBuffer) {

	//int m_sampleSize = 0;
	int deltaSampleSize = 0;
	int idx = 0;
	int lIdx = 0;
	int rIdx = 0;
	qreal localMax;

		m_sampleSize = (int)floor(m_fftMult * m_spectrumSize * m_freqScaleZoomFactor);
		deltaSampleSize = m_spectrumSize - m_sampleSize;
	//	qDebug() << "m_ssamplesdize" << m_sampleSize << deltaSampleSize << m_fftMult;
			

		if (m_sampleSize < 2048) {

			if (m_fftMult == 1) {

				GRAPHICS_DEBUG << "set sample size to 8192";
				set->setSampleSize(this, m_receiver, 8192);
				//m_dBmPanLogGain += 3.0103;
				m_dBmPanLogGain += 6;
				m_fftMult = 2;

				return;
			}
			else if (m_fftMult == 2) {

				GRAPHICS_DEBUG << "set sample size to 16384";
				set->setSampleSize(this, m_receiver, 16384);
				//m_dBmPanLogGain += 3.0103;
				m_dBmPanLogGain += 6;
				m_fftMult = 4;

				return;
			}
			else if (m_fftMult == 4) {

				GRAPHICS_DEBUG << "set sample size to 32768";
				set->setSampleSize(this, m_receiver, 32768);
				//m_dBmPanLogGain += 3.0103;
				m_dBmPanLogGain += 6;
				m_fftMult = 8;

				return;
			}
			else if (m_fftMult == 8) {

				GRAPHICS_DEBUG << "set sample size to 65536";
				set->setSampleSize(this, m_receiver, 65536);
				//m_dBmPanLogGain += 3.0103;
				m_dBmPanLogGain += 6;
				m_fftMult = 16;

				return;
			}
		}
		else if (m_sampleSize > 4096) {

			if (m_fftMult == 2) {

				GRAPHICS_DEBUG << "set sample size to 4096";
				set->setSampleSize(this, m_receiver, 4096);
				//m_dBmPanLogGain -= 3.0103;
				m_dBmPanLogGain -= 6;
				m_fftMult = 1;

				return;
			}
			else if (m_fftMult == 4) {

				GRAPHICS_DEBUG << "set sample size to 8192";
				set->setSampleSize(this, m_receiver, 8192);
				//m_dBmPanLogGain -= 3.0103;
				m_dBmPanLogGain -= 6;
				m_fftMult = 2;

				return;
			}
			else if (m_fftMult == 8) {

				GRAPHICS_DEBUG << "set sample size to 16384";
				set->setSampleSize(this, m_receiver, 16384);
				//m_dBmPanLogGain -= 3.0103;
				m_dBmPanLogGain -= 6;
				m_fftMult = 4;

				return;
			}
			else if (m_fftMult == 16) {

				GRAPHICS_DEBUG << "set sample size to 32768";
				set->setSampleSize(this, m_receiver, 32768);
				//m_dBmPanLogGain -= 3.0103;
				m_dBmPanLogGain -= 6;
				m_fftMult = 8;

				return;
			}

	}

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

	m_panSpectrumBinsLength = (GLint)(m_scaleMult * m_panRectWidth);
//	qDebug() << "m_panSpectrumBinsLength =" << m_panSpectrumBinsLength;
	if (m_sampleSize != m_oldSampleSize) {
	
		GRAPHICS_DEBUG << "m_panSpectrumBinsLength = " << m_panSpectrumBinsLength;
		GRAPHICS_DEBUG << "m_sampleSize =            " << m_sampleSize;
		GRAPHICS_DEBUG << "deltaSampleSize =         " << deltaSampleSize;
		GRAPHICS_DEBUG << "";

		m_oldSampleSize = m_sampleSize;
	}

	if (m_scaleMultOld != m_scaleMult) {

		m_waterfallUpdate = true;
	}

	m_waterfallPixel.clear();
	m_waterfallPixel.resize(4 * m_panRectWidth);

	m_panadapterBins.clear();

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


		TGL_ubyteRGBA color;
		color.red   = (uchar)(pColor.red());
		color.green = (uchar)(pColor.green());
		color.blue  = (uchar)(pColor.blue());
		color.alpha = 255;
		
		for (int j = 0; j < (int)(1/m_scaleMult); j++) {

			m_waterfallPixel[(int)(i/m_scaleMult) + j] = color;
		}
	}

	m_waterfallDisplayUpdate = true;
	update();
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

void QGLReceiverPanel::setFramesPerSecond(QObject* sender, int rx, int value) {

	Q_UNUSED(sender)

	if (m_receiver != rx) return;

	m_fps = value;

	m_secWaterfallMin = -(1.0/m_fps) * m_secScaleWaterfallRect.height();
	m_secScaleWaterfallRenew = true;
	m_secScaleWaterfallUpdate = true;
}

void QGLReceiverPanel::systemStateChanged(
	QObject *sender, 
	QSDR::_Error err, 
	QSDR::_HWInterfaceMode hwmode, 
	QSDR::_ServerMode mode, 
	QSDR::_DataEngineState state)
{
	Q_UNUSED (sender)
	Q_UNUSED (err)
	Q_UNUSED (hwmode)
	Q_UNUSED (state)

	if (m_dataEngineState != state)
		m_dataEngineState = state;

	if (state == QSDR::DataEngineDown)
		m_fftMult = 1;
	//	m_panadapterBins.clear();

	if (state == QSDR::DataEngineDown)

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
	QObject *sender,
	int rx,
	PanGraphicsMode panMode,
	WaterfallColorMode waterfallColorMode)
{
	Q_UNUSED (sender)

	if (m_receiver != rx) return;
	
	
	if (m_panMode != panMode)
		m_panMode = panMode;

	if (m_waterfallMode != waterfallColorMode)
		m_waterfallMode = waterfallColorMode;
}

 void QGLReceiverPanel::setSpectrumAveraging(QObject* sender, int rx, bool value) {

	 Q_UNUSED (sender)

	 if (m_receiver != rx) return;

	 spectrumBufferMutex.lock();

	 if (m_spectrumAveraging == value) 
		 return;
	 else
		 m_spectrumAveraging = value;

	 spectrumBufferMutex.unlock();
 }

void QGLReceiverPanel::setSpectrumAveragingCnt(int value) {

	spectrumBufferMutex.lock();

		//m_tmp.clear();

		while (!specAv_queue.isEmpty())
			specAv_queue.dequeue();

		m_specAveragingCnt = value;

		if (m_specAveragingCnt > 0)
			m_scale = 1.0f / m_specAveragingCnt;
		else
			m_scale = 1.0f;

	spectrumBufferMutex.unlock();
}

void QGLReceiverPanel::setPanGridStatus(bool value, int rx) {

	if (m_receiver != rx) return;

	spectrumBufferMutex.lock();

	 if (m_panGrid == value) 
		 return;
	 else
		 m_panGrid = value;

	 spectrumBufferMutex.unlock();
}

void QGLReceiverPanel::setPeakHoldStatus(bool value, int rx) {

	if (m_receiver != rx) return;
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

void QGLReceiverPanel::sampleRateChanged(QObject *sender, int value) {

	Q_UNUSED(sender)
	
	m_sampleRate = value;
	m_deltaF = (qreal)(1.0*m_deltaFrequency/m_sampleRate);

	m_freqScalePanadapterUpdate = true;
	m_panGridUpdate = true;
	m_filterChanged = true;
}

void QGLReceiverPanel::setMercuryAttenuator(QObject* sender, HamBand band, int value) {

	Q_UNUSED(sender)
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
}

void QGLReceiverPanel::setdBmScaleMax(int rx, qreal value) {

	if (m_receiver != rx) return;

	m_dBmPanMax = value;

	m_dBmScalePanadapterUpdate = true;
	m_panGridUpdate = true;
}

void QGLReceiverPanel::setMouseWheelFreqStep(QObject *sender, int rx, qreal step) {

	Q_UNUSED(sender)

	if (m_receiver != rx) return;
	m_mouseWheelFreqStep = step;
}

void QGLReceiverPanel::setHamBand(QObject *sender, int rx, bool byButton, HamBand band) {

	Q_UNUSED(sender)

	if (m_receiver != rx || !byButton) return;
	// we change the dBm-scale only, if the frequency is changed by changing the band by button.
	// That is, if we change band by changing the frequency, the dBm scale stays constant.

	//m_panLocked = false;
	//m_deltaFrequency = 0;
	//m_deltaF = 0;

	m_rxDataList[m_receiver].hamBand = band;
	//m_dspModeString = set->getDSPModeString(m_rxDataList[m_receiver].dspMode);

	m_dBmPanMin = m_rxDataList.at(m_receiver).dBmPanScaleMinList.at(band);
	m_dBmPanMax = m_rxDataList.at(m_receiver).dBmPanScaleMaxList.at(band);

	m_dBmScalePanadapterUpdate = true;
	m_panGridUpdate = true;
}

void QGLReceiverPanel::setADCStatus(int value) {

	m_adcStatus = value;
    QTimer::singleShot(50, this, SLOT(updateADCStatus()));
}

void QGLReceiverPanel::updateADCStatus() {

	if (m_dataEngineState == QSDR::DataEngineUp)
		m_adcStatus = 1;
	else
		m_adcStatus = 0;
}

void QGLReceiverPanel::setAGCLineLevels(QObject *sender, int rx, qreal thresh, qreal hang) {

	Q_UNUSED(sender)

	if (m_receiver != rx) return;
	if (m_agcThresholdOld == thresh && m_agcHangLevelOld == hang) return;

	m_agcThresholdOld = thresh;
	m_agcHangLevelOld = hang;
}

void QGLReceiverPanel::setAGCLineFixedLevel(QObject *sender, int rx, qreal value) {

	Q_UNUSED(sender)

	if (m_receiver != rx) return;
	if (m_agcFixedGain == value) return;

	m_agcFixedGain = value;
}

void QGLReceiverPanel::setADCMode(QObject *sender, int rx, ADCMode mode) {

	Q_UNUSED(sender)

	if (m_receiver != rx) return;

	m_adcMode = mode;
	m_adcModeString = set->getADCModeString(m_receiver);
}

void QGLReceiverPanel::setAGCMode(QObject *sender, int rx, AGCMode mode, bool hangEnabled) {

	Q_UNUSED(sender)

	if (m_receiver != rx) return;

	if (m_agcHangEnabled == hangEnabled && m_agcMode == mode) return;

	m_agcMode = mode;
	m_agcModeString = set->getAGCModeString(m_receiver);
	m_agcHangEnabled = hangEnabled;
	GRAPHICS_DEBUG << "m_agcHangEnabled = " << m_agcHangEnabled;
}

void QGLReceiverPanel::setAGCLinesStatus(QObject* sender, bool value, int rx) {

	Q_UNUSED (sender)

	if (m_receiver != rx) return;

	m_showAGCLines = value;
}

void QGLReceiverPanel::setDSPMode(QObject* sender, int rx, DSPMode mode) {

	Q_UNUSED(sender)

	if (m_receiver != rx) return;

	m_dspMode = mode;
	m_dspModeString = set->getDSPModeString(m_dspMode);
}

void QGLReceiverPanel::showRadioPopup(bool value) {

	Q_UNUSED (value)

	radioPopup->showPopupWidget(this, QCursor::pos());
}

//void QGLReceiverPanel::setAGCHangEnabled(QObject *sender, int rx, bool hangEnabled) {
//
//	Q_UNUSED(sender)
//
//	if (m_receiver != rx) return;
//
//	if (m_agcHangEnabled == hangEnabled) return;
//	m_agcHangEnabled = hangEnabled;
//	GRAPHICS_DEBUG << "m_agcHangEnabled = " << m_agcHangEnabled;
//
//	update();
//}
