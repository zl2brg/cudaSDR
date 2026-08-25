/**
* @file  cusdr_oglWidebandPanel.h
* @brief wide band spectrum panel class for cuSDR
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2012-02-11
*/

/*
 *   Copyright 2012 Hermann von Hasseln, DL3HVH
 *   Copyright 2025 Simon Eatough, ZL2BRG
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

#define LOG_WBGRAPHICS

// use: WBGRAPHICS_DEBUG

#include "cusdr_oglWidebandPanel.h"
#include "cusdr_glShaders.h"
#include "cusdr_glDraw.h"
#include "Models/RadioTelemetry.h"
#include <QGuiApplication>
#include <QScreen>
#include <QWindow>

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

QGLWidebandPanel::QGLWidebandPanel(QWidget *parent)
		: QOpenGLWidget(parent)
		, set(Settings::instance())
		, m_serverMode(set->getCurrentServerMode())
		, m_hwInterface(set->getHWInterface())
		, m_dataEngineState(set->getDataEngineState())
		, m_panMode(set->getPanadapterMode(0))
		//, m_specAveragingCnt(set->getSpectrumAveragingCnt())
		, m_mousePos(QPoint(-1, -1))
		, m_mouseDownPos(QPoint(-1, -1))
		, m_spectrumUpdate(false)
		, m_freqScaleUpdate(true)
		, m_freqScaleRenew(true)
		, m_dBmScaleUpdate(true)
		, m_dBmScaleRenew(true)
		, m_dragFreqScale(false)
		, m_dragFreqScaleZoom(false)
		, m_dragDBmScale(false)
		, m_panGridRenew(true)
		, m_spectrumColorsChanged(true)
		, m_crossHairCursor(false)
		//, m_panGrid(set->getPanGridStatus(0))
		, m_panGrid(true)
		, m_calibrate(false)
		, m_mercuryAttenuator(0)
		, m_panSpectrumBinsLength(0)
		, m_snapMouse(3)
		, m_currentReceiver(set->getCurrentReceiver())
		, m_sampleRate(set->getSampleRate())
		, m_freqScaleZoomFactor(1.0)
		, m_dBmScaleOffset(0.0)
		, m_widebandMaxFrequency((qreal)set->getMaxFrequency())
		, m_widebandMinFrequency(0.0)
{
//	QGL::setPreferredPaintEngine(QPaintEngine::OpenGL);
    dpr = devicePixelRatioF();
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setAutoFillBackground(false);
	setAttribute(Qt::WA_OpaquePaintEvent);
	setAttribute(Qt::WA_NoSystemBackground);
	// Full repaint each frame — PartialUpdate corrupts the pan background under Core GL.
	setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
	disableVSyncOnNativeWayland(this);
	
	setMouseTracking(true);
	//setFocusPolicy(Qt::StrongFocus);
	setupDisplayRegions(size());
	m_oldWidth = size().width();
	m_oldHeight = size().height();

	m_widebandOptions = set->getWidebandOptions();
	m_panMode = m_widebandOptions.panMode;
	m_specAveragingCnt = qMax(1, m_widebandOptions.averagingCnt);
	m_frequency = set->getVfoFrequency(0);

	m_lowerFrequency = 0.0;
	m_upperFrequency = m_widebandMaxFrequency;

	m_dBmPanMin = set->getWidebanddBmScaleMin();
	m_dBmPanMax = set->getWidebanddBmScaleMax();

	fonts = new CFonts(this);
	m_fonts = fonts->getFonts();
	
	m_fonts.smallFont.setBold(true);

	m_oglTextTiny = new OGLText(m_fonts.tinyFont, dpr);
	m_oglTextSmall = new OGLText(m_fonts.smallFont, dpr);
	m_oglTextNormal = new OGLText(m_fonts.normalFont, dpr);
     m_dprPollTimerId = startTimer(1000);

	timer = 0;

	setupConnections();

	m_panTimer.start();
	m_displayTime.start();
	m_resizeTime.start();

	m_wbSpectrumBuffer.resize(BIGWIDEBANDSIZE / 2);
	m_wbSpectrumBuffer.fill(-1000.0f);
	m_wbSpectrumBufferLength = m_wbSpectrumBuffer.size();

	m_dBmPanLogGain = 0;//69; // allow user to calibrate this value
	
	if (m_specAveragingCnt > 0)
		m_scale = 1.0f / m_specAveragingCnt;
	else
		m_scale = 1.0f;


	m_gridColor = set->getPanadapterColors().gridLineColor;

	m_redGrid   = (GLfloat)(m_gridColor.red()/256.0);
	m_greenGrid = (GLfloat)(m_gridColor.green()/256.0);
	m_blueGrid  = (GLfloat)(m_gridColor.blue()/256.0);

	m_r = (GLfloat)(set->getPanadapterColors().wideBandLineColor.red() / 256.0);
	m_g = (GLfloat)(set->getPanadapterColors().wideBandLineColor.green() / 256.0);
	m_b = (GLfloat)(set->getPanadapterColors().wideBandLineColor.blue() / 256.0);

	m_rf = (GLfloat)(set->getPanadapterColors().wideBandFilledColor.red() / 256.0);
	m_gf = (GLfloat)(set->getPanadapterColors().wideBandFilledColor.green() / 256.0);
	m_bf = (GLfloat)(set->getPanadapterColors().wideBandFilledColor.blue() / 256.0);

	m_redST	  = (GLfloat)(set->getPanadapterColors().panSolidTopColor.red() / 256.0);
	m_greenST = (GLfloat)(set->getPanadapterColors().panSolidTopColor.green() / 256.0);
	m_blueST  = (GLfloat)(set->getPanadapterColors().panSolidTopColor.blue() / 256.0);

	m_redSB   = (GLfloat)(set->getPanadapterColors().panSolidBottomColor.red() / 256.0);
	m_greenSB = (GLfloat)(set->getPanadapterColors().panSolidBottomColor.green() / 256.0);
	m_blueSB  = (GLfloat)(set->getPanadapterColors().panSolidBottomColor.blue() / 256.0);

	m_bkgRed   = (GLfloat)(set->getPanadapterColors().panBackgroundColor.red() / 256.0);
	m_bkgGreen = (GLfloat)(set->getPanadapterColors().panBackgroundColor.green() / 256.0);
	m_bkgBlue  = (GLfloat)(set->getPanadapterColors().panBackgroundColor.blue() / 256.0);
}

QGLWidebandPanel::~QGLWidebandPanel() {

	disconnect(set, 0, this, 0);

	while (!specAv_queue.isEmpty())
		specAv_queue.dequeue();

	if (m_panadapterRenderer) {
		m_panadapterRenderer->release();
		delete m_panadapterRenderer;
		m_panadapterRenderer = nullptr;
	}

    delete m_oglTextTiny;
    delete m_oglTextSmall;
    delete m_oglTextNormal;
    delete fonts;
    delete m_overlayRenderer;
}

QSize QGLWidebandPanel::minimumSizeHint() const {
	
	return QSize(width(), 50);
	//return QSize(width(), height());
}

QSize QGLWidebandPanel::sizeHint() const {
	
	//return QSize(width(), height());
	return QSize(width(), 120);
}

void QGLWidebandPanel::setupConnections() {
    connect(set, &Settings::systemStateChanged,
            this, &QGLWidebandPanel::systemStateChanged);

    connect(set, &Settings::graphicModeChanged,
            this, &QGLWidebandPanel::graphicModeChanged);

    if (RadioTelemetry* tel = telemetryFromSettings()) {
        connect(tel, &RadioTelemetry::widebandSpectrumBufferChanged,
                this, &QGLWidebandPanel::setWidebandSpectrumBuffer);
        connect(tel, &RadioTelemetry::widebandSpectrumBufferReset,
                this, &QGLWidebandPanel::resetWidebandSpectrumBuffer);
        connect(tel, &RadioTelemetry::widebandFrequencyRangeChanged,
                this, &QGLWidebandPanel::setWidebandFrequencyRange);
    }

    connect(set, &Settings::vfoFrequencyChanged,
            this, &QGLWidebandPanel::setFrequency);

    connect(set, &Settings::currentReceiverChanged,
            this, &QGLWidebandPanel::setCurrentReceiver);

    connect(set, &Settings::sampleRateChanged,
            this, &QGLWidebandPanel::sampleRateChanged);

    connect(set, &Settings::panadapterColorChanged,
            this, &QGLWidebandPanel::setPanadapterColors);

    // Uncomment if needed
    // connect(set, &Settings::panGridStatusChanged,
    //         this, &QGLWidebandPanel::setPanGridStatus);

    connect(set, &Settings::mercuryAttenuatorChanged,
            this, &QGLWidebandPanel::setMercuryAttenuator);

    // Uncomment if needed
    // connect(set, &Settings::spectrumAveragingCntChanged,
    //         this, &QGLWidebandPanel::setSpectrumAvera
}

void QGLWidebandPanel::initializeGL() {

	if (!isValid()) return;
	initializeOpenGLFunctions();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	m_cnt = 0;

	// ── Shader program (GLES2 or desktop GLSL) ─────────────────────────────
	m_program = new QOpenGLShaderProgram(this);
	if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, GlShaders::widebandVertexSource()))
		qWarning() << "WB vertex shader:" << m_program->log();
	if (!m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, GlShaders::widebandFragmentSource()))
		qWarning() << "WB fragment shader:" << m_program->log();
	if (!m_program->link())
		qWarning() << "WB shader link:" << m_program->log();

	m_attrPos    = m_program->attributeLocation("a_pos");
	m_attrColor  = m_program->attributeLocation("a_color");
	m_uniformMvp = m_program->uniformLocation("u_mvp");

	// ── Persistent VAO + VBO (streaming, updated each draw) ─────────────────
	m_vao.create();
	m_vbo.create();
	m_vbo.setUsagePattern(QOpenGLBuffer::StreamDraw);

	m_vao.bind();
	m_vbo.bind();
	// stride = 6 floats (xyz + rgb), pointers set once in the VAO
	glVertexAttribPointer(m_attrPos,   3, GL_FLOAT, GL_FALSE,
	                      6 * sizeof(float), reinterpret_cast<void*>(0));
	glVertexAttribPointer(m_attrColor, 3, GL_FLOAT, GL_FALSE,
	                      6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
	glEnableVertexAttribArray(m_attrPos);
	glEnableVertexAttribArray(m_attrColor);
	m_vbo.release();
	m_vao.release();

	m_textureProgram = new QOpenGLShaderProgram(this);
	m_textureProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, GlShaders::texturedQuadVertexSource());
	m_textureProgram->addShaderFromSourceCode(QOpenGLShader::Fragment,
	                                          GlShaders::texturedFragmentSource("tex"));
	m_textureProgram->bindAttributeLocation("position", 0);
	m_textureProgram->bindAttributeLocation("texCoord", 1);
	if (!m_textureProgram->link())
		qWarning() << "Wideband panel texture shader link failed:" << m_textureProgram->log();

	m_overlayRenderer = new OverlayRenderer();
	m_overlayRenderer->initialize(nullptr);

	// Own shader (WB uses a_pos/a_color); reuse vertex cache / RHI path like RX pan.
	m_panadapterRenderer = new PanadapterRenderer();
	if (!m_panadapterRenderer->initialize(context(), nullptr))
		qWarning() << "PanadapterRenderer init failed for wideband panel";
}

QMatrix4x4 QGLWidebandPanel::panelProjection() const
{
	QMatrix4x4 projection;
	projection.ortho(0, width(), height(), 0, -10, 10);
	return projection;
}

void QGLWidebandPanel::drawCachedTexture(const QRect &rect, GLuint texId, float z)
{
	if (rect.isEmpty() || !texId)
		return;
	m_vao.bind();
	if (m_textureProgram && m_textureProgram->isLinked())
		GlDraw::renderTexturedQuad(this, m_textureProgram, m_vbo, panelProjection(), rect, texId, z);
}

void QGLWidebandPanel::drawPanelRect(const QRect &rect, const QColor &color, float z)
{
	if (rect.isEmpty())
		return;
	m_vao.bind();
	if (m_program && m_program->isLinked())
		GlDraw::drawSolidRect(this, m_program, m_vbo, panelProjection(), rect, color, z);
}

void QGLWidebandPanel::paintGL() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	switch (m_serverMode) {

		case QSDR::NoServerMode:

            drawPanelRect(QRect(0, 0, width()* dpr, height() * dpr), QColor(65, 54, 54), -5.0f);
			break;

		case QSDR::SDRMode:
			
			if (m_resizeTime.elapsed() > 200 || m_dataEngineState == QSDR::DataEngineDown) {
			
                drawSpectrum();

                if (m_freqScaleRect.isValid())
                    updateFrequencyRuler();
                if (m_dBmScaleRect.isValid())
                    updateDBmRuler();

                drawHorizontalScale();
                drawVerticalScale();

            if (m_panGrid)
                    drawGrid();
//#todo use ham database
				// Ham band information
				drawHamBand(1810000, 2000000, "160m");
				drawHamBand(3500000, 3800000, "80m");
				drawHamBand(5258500, 5403500, "60m");
				drawHamBand(7000000, 7300000, "40m");
				drawHamBand(10100000, 10150000, "30m");
				drawHamBand(14000000, 14350000, "20m");
				drawHamBand(18068000, 18168000, "17m");
				drawHamBand(21000000, 21450000, "15m");
				drawHamBand(24890000, 24990000, "12m");
				drawHamBand(28000000, 29700000, "10m");
				drawHamBand(50000000, 54000000, "6m");

                //glColor4f(QColor(255, 255, 255, 130));
				//m_oglTextSmall->renderFreqText(m_panRect.right() - 100, m_panRect.top(), 5.0f, "Region 1");

				if (m_mouseRegion == panRegion && m_crossHairCursor)
					drawCrossHair();
			}

			break;
	}
}
 
//************************************************************************
// Core-profile helpers

void QGLWidebandPanel::setMvpOrtho(int w, int h)
{
	QMatrix4x4 mvp;
	mvp.setToIdentity();
	mvp.ortho(0.0f, (float)w, (float)h, 0.0f, -5.0f, 5.0f);
	m_program->setUniformValue(m_uniformMvp, mvp);
}

// Draws N vertices from interleaved float data [x,y,z, r,g,b] × N.
void QGLWidebandPanel::drawVertexColorArray(GLenum mode, const QVector<float> &data, int vertexCount)
{
	if (vertexCount <= 0 || data.size() < vertexCount * 6) return;
	m_vao.bind();
	m_vbo.bind();
	const int bytes = data.size() * (int)sizeof(float);
	if (bytes > m_vboCapacityBytes) {
		m_vbo.allocate(data.constData(), bytes);
		m_vboCapacityBytes = bytes;
	} else {
		m_vbo.write(0, data.constData(), bytes);
	}

	if (m_attrPos >= 0) {
		glEnableVertexAttribArray(m_attrPos);
		glVertexAttribPointer(m_attrPos, 3, GL_FLOAT, GL_FALSE,
		                      6 * sizeof(float), reinterpret_cast<void*>(0));
	}
	if (m_attrColor >= 0) {
		glEnableVertexAttribArray(m_attrColor);
		glVertexAttribPointer(m_attrColor, 3, GL_FLOAT, GL_FALSE,
		                      6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
	}

	glDrawArrays(mode, 0, vertexCount);

	if (m_attrPos >= 0)
		glDisableVertexAttribArray(m_attrPos);
	if (m_attrColor >= 0)
		glDisableVertexAttribArray(m_attrColor);

	m_vbo.release();
	m_vao.release();
}

//************************************************************************
void QGLWidebandPanel::rebuildWidebandPanBins(const QVector<float>& spectrum)
{
	m_widebandPanSpectrumBins.clear();
	m_wbScaleMult = 1.0;

	const GLint width = m_panRect.width();
	if (width <= 0 || spectrum.isEmpty() || m_wbSpectrumBufferLength <= 0)
		return;

	m_scaledBufferSize = qFloor(m_wbSpectrumBufferLength * m_freqScaleZoomFactor);
	if (m_scaledBufferSize <= 0)
		return;

	const qreal wbRange = qMax<qreal>(1.0, m_widebandMaxFrequency - m_widebandMinFrequency);
	const int deltaIdx = qFloor((qreal)(m_wbSpectrumBufferLength
	                                    * ((m_lowerFrequency - m_widebandMinFrequency) / wbRange)));

	qreal frequencyScale = (qreal)(1.0f * m_scaledBufferSize / width);
	qreal scaleMult = 1.0;
	if (frequencyScale < 0.125)
		scaleMult = 0.0625;
	else if (frequencyScale < 0.25)
		scaleMult = 0.125;
	else if (frequencyScale < 0.5)
		scaleMult = 0.25;
	else if (frequencyScale < 1.0)
		scaleMult = 0.5;

	const qreal scale = frequencyScale / scaleMult;
	const int vertexArrayLength = (GLint)(scaleMult * width);
	if (vertexArrayLength <= 0)
		return;

	const qreal dBmMin = m_calibrate ? m_dBmPanMinOld : m_dBmPanMin;
	const int n = spectrum.size();

	m_wbScaleMult = scaleMult;
	m_widebandPanSpectrumBins.resize(vertexArrayLength);

	for (int i = 0; i < vertexArrayLength; ++i) {
		int lIdx = (int)floor((qreal)(i * scale));
		int rIdx = (int)floor((qreal)(i * scale) + scale);
		if (rIdx <= lIdx)
			rIdx = lIdx + 1;

		float localMax = -10000.0f;
		int idx = lIdx;
		for (int j = lIdx; j < rIdx; ++j) {
			if (j < 0 || j >= n)
				continue;
			if (spectrum.at(j) > localMax) {
				localMax = spectrum.at(j);
				idx = j;
			}
		}
		idx += deltaIdx;

		qreal yvalue = 0.0;
		if (idx >= 0 && idx < n && idx < m_wbSpectrumBufferLength)
			yvalue = spectrum.at(idx) - dBmMin - m_dBmPanLogGain;
		if (m_mercuryAttenuator)
			yvalue -= 20.0;

		m_widebandPanSpectrumBins[i] = yvalue;
	}
}

void QGLWidebandPanel::drawSpectrum() {

	const GLint width  = m_panRect.width();
	const GLint height = m_panRect.height();
	if (width <= 0 || height <= 0)
		return;

	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_LINE_SMOOTH);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glLineWidth(1);

	const float dprF = float(devicePixelRatio());
	QMatrix4x4 projection;
	projection.ortho(0, size().width(), size().height(), 0, -10, 10);
	PanadapterRenderer::Colors colors = {
		m_r, m_g, m_b,
		m_rf, m_gf, m_bf,
		m_redST, m_greenST, m_blueST,
		m_redSB, m_greenSB, m_blueSB,
		m_bkgRed, m_bkgGreen, m_bkgBlue
	};

	const bool showSpectrum = (m_dataEngineState == QSDR::DataEngineUp
	                           && m_wbSpectrumBufferLength > 0);

	if (!showSpectrum) {
		if (m_panadapterRenderer)
			m_panadapterRenderer->renderIdleBackground(this, projection, m_panRect, dprF,
			                                           size().height(), colors, m_dataEngineState, true);
		glDisable(GL_BLEND);
		glDisable(GL_LINE_SMOOTH);
		return;
	}

	// One lock: copy latest averaged spectrum, then bin/draw lock-free.
	{
		QMutexLocker lock(&mutex);
		m_wbSpectrumSnapshot = m_wbSpectrumBuffer;
	}

	rebuildWidebandPanBins(m_wbSpectrumSnapshot);

	const qreal dBmMin = m_calibrate ? m_dBmPanMinOld : m_dBmPanMin;
	const qreal dBmMax = m_calibrate ? m_dBmPanMaxOld : m_dBmPanMax;

	if (m_panadapterRenderer && !m_widebandPanSpectrumBins.isEmpty()) {
		m_panadapterRenderer->render(this, projection, m_panRect, m_widebandPanSpectrumBins,
		                             dBmMax, dBmMin, m_panMode, float(m_wbScaleMult), dprF,
		                             size().height(), colors, m_dataEngineState, true,
		                             {}, true /* scaleColorByLevel: match legacy WB brightness */);
		if (m_panadapterRenderer->usesCompositePass())
			m_panadapterRenderer->compositeToDefaultFramebuffer(this, projection, m_panRect,
			                                                    dprF, size().height());
	}

	// Sample-rate window overlay (RX span marker on the wideband display).
	if (!m_calibrate) {
		const int y1 = m_panRect.top() + 15;
		const int y2 = m_panRect.height() - 15;
		const int centerFreq = (int)(m_frequencyUnit * (m_frequency - m_lowerFrequency));
		const int deltaF = (int)(m_frequencyUnit * (float)m_sampleRate / 2);
		const int x1 = centerFreq - deltaF;
		const int x2 = centerFreq + deltaF;

		const QRect rect = QRect(x1, y1, x2 - x1, y2 - y1);
		if (m_overlayRenderer && rect.isValid()) {
			m_overlayRenderer->drawFilledRect(panelProjection(), rect,
			                                 QColor(160, 235, 255, 80),
			                                 QColor(160, 235, 255, 40),
			                                 0.0f);
		}
		glLineWidth(1);
	}

	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
}

void QGLWidebandPanel::updateFrequencyRuler()
{
	if (!m_freqScaleRect.isValid())
		return;

	m_frequencySpan = (m_widebandMaxFrequency - m_widebandMinFrequency) * m_freqScaleZoomFactor;
	if (m_frequencySpan <= 0)
		return;

	m_frequencyUnit = qreal(m_freqScaleRect.width()) / m_frequencySpan;
	const int fontMaxWidth = m_fonts.smallFontMetrics->boundingRect(QStringLiteral("000.000")).width();
	m_frequencyScale = getXRuler(m_freqScaleRect, fontMaxWidth, m_frequencyUnit,
	                             m_lowerFrequency, m_upperFrequency);
}

void QGLWidebandPanel::updateDBmRuler()
{
	if (!m_dBmScaleRect.isValid())
		return;

	const qreal dBmRange = qAbs(m_dBmPanMax - m_dBmPanMin);
	if (dBmRange <= 0)
		return;

	const int spacing = 6;
	const int fontHeight = m_fonts.smallFontMetrics->tightBoundingRect(QStringLiteral(".0dBm")).height() + spacing;
	const qreal unit = qreal(m_dBmScaleRect.height()) / dBmRange;
	m_dBmScale = getYRuler2(m_dBmScaleRect, fontHeight, unit, m_dBmPanMin, m_dBmPanMax);
}

void QGLWidebandPanel::drawVerticalScale() {

	if (!m_dBmScaleRect.isValid()) return;

    int width = m_dBmScaleRect.width();
    int height = m_dBmScaleRect.height();
	if (width <= 0 || height <= 0) return;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	const bool regenDBmScale = !m_dBmScaleFBO || m_dBmScaleRenew
	                           || (m_dBmScaleUpdate && !m_dragDBmScale);

	if (regenDBmScale) {

		if (!m_dBmScaleFBO || m_dBmScaleRenew) {

			if (m_dBmScaleFBO) {
				delete m_dBmScaleFBO;
                m_dBmScaleFBO = nullptr;
			}

            m_dBmScaleFBO = new QOpenGLFramebufferObject(width, height);
		}

        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glViewport(0, 0, width, height);

		m_dBmScaleFBO->bind();
			renderVerticalScale();
		m_dBmScaleFBO->release();
		QOpenGLFramebufferObject::bindDefault();

        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

		if (!m_dragDBmScale)
			m_dBmScaleUpdate = false;
		m_dBmScaleRenew = false;
	}

	drawCachedTexture(m_dBmScaleRect, m_dBmScaleFBO ? m_dBmScaleFBO->texture() : 0, 0.0f);

	if (m_dragDBmScale && m_overlayRenderer && m_dBmScaleFBO) {
		QMatrix4x4 projection;
		projection.ortho(0, size().width(), size().height(), 0, -10, 10);
		m_overlayRenderer->drawDBmScaleTicks(projection, m_dBmScaleRect, m_dBmScale,
		                                     m_redGrid, m_greenGrid, m_blueGrid, 1.0f);
	}
}

void QGLWidebandPanel::drawHorizontalScale() {

	if (!m_freqScaleRect.isValid()) return;

    int width = m_freqScaleRect.width();
    int height = m_freqScaleRect.height();
	if (width <= 0 || height <= 0) return;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	const bool regenFreqScale = !m_frequencyScaleFBO || m_freqScaleRenew
	                            || m_freqScaleUpdate
	                            || (m_dragFreqScale && !m_dragFreqScaleZoom);

    if (regenFreqScale) {

		if (!m_frequencyScaleFBO || m_freqScaleRenew) {
			if (m_frequencyScaleFBO) {
				delete m_frequencyScaleFBO;
                m_frequencyScaleFBO = nullptr;
			}

            m_frequencyScaleFBO = new QOpenGLFramebufferObject(width, height);
        }

        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glViewport(0, 0, width, height);

		m_frequencyScaleFBO->bind();
        renderHorizontalScale();
        m_frequencyScaleFBO->release();
        QOpenGLFramebufferObject::bindDefault();

        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

		m_freqScaleUpdate = false;
		m_freqScaleRenew = false;
	}

	drawCachedTexture(m_freqScaleRect, m_frequencyScaleFBO ? m_frequencyScaleFBO->texture() : 0, 0.0f);
}

void QGLWidebandPanel::drawGrid() {
    if (!m_panRect.isValid() || !m_overlayRenderer)
		return;

	// Match receiver panadapter grid colour/alpha (OverlayRenderer draws dotted segments).
	m_overlayRenderer->drawGrid(panelProjection(),
	                            m_panRect,
	                            m_freqScaleRect.isValid() ? m_freqScaleRect : m_panRect,
	                            m_frequencyScale,
	                            m_dBmScale,
	                            0,
	                            m_redGrid,
	                            m_greenGrid,
	                            m_blueGrid,
	                            1.0f,
	                            true);
}

void QGLWidebandPanel::drawCrossHair() {

	if (!m_overlayRenderer || !m_panRect.isValid())
		return;

	const QMatrix4x4 projection = panelProjection();
	m_overlayRenderer->drawCrossHair(projection, m_panRect, m_dBmScaleRect, m_mousePos,
	                                 float(dpr), height());

	if (m_mouseRegion != panRegion || !m_oglTextSmall)
		return;

	const int x = m_mousePos.x();
	const int y = m_mousePos.y();
	const qreal wbRange = qMax<qreal>(1.0, m_widebandMaxFrequency - m_widebandMinFrequency);
	const qreal unit = (wbRange * m_freqScaleZoomFactor) / qMax(1, m_panRect.width());
	const qreal frequency = (unit * x) + m_lowerFrequency;
	const QString fstr = frequencyString(frequency);

	const qreal dBm = m_dBmPanMax
	                  - ((m_dBmPanMax - m_dBmPanMin)
	                     * ((qreal)(y - m_panRect.top()) / qMax(1, m_panRect.height())));
	const QString dBstr = QString::number(dBm, 'f', 1) + QStringLiteral(" dBm");

	const float tx = (x > m_panRect.width() - 85) ? float(x - 90) : float(x + 4);
	m_oglTextSmall->renderText(projection, tx, float(y - 8), fstr, QColor(255, 255, 255, 200));
	m_oglTextSmall->renderText(projection, tx, float(y + 16), dBstr, QColor(255, 255, 255, 200));
}

void QGLWidebandPanel::drawHamBand(
		int lo,
		int hi,
		const QString &band
) {
	if (!m_overlayRenderer || m_frequencyUnit <= 0.0 || !m_panRect.isValid())
		return;

	const int x1 = int(m_frequencyUnit * (lo - m_lowerFrequency));
	const int x2 = int(m_frequencyUnit * (hi - m_lowerFrequency));
	if (x2 <= x1)
		return;

	QRect rect(x1, m_panRect.top(), x2 - x1, m_panRect.height());
	rect = rect.intersected(m_panRect);
	if (!rect.isValid() || rect.width() < 1)
		return;

	// Match panadapter filter opacity: panFilterColor alpha × 0.4 (see OverlayRenderer::drawFilter).
	QColor fill = set->getPanadapterColors().panFilterColor;
	fill.setAlphaF(fill.alphaF() * 0.4f);
	m_overlayRenderer->drawFilledRect(panelProjection(), rect, fill, fill, 1.0f);

	if (m_oglTextNormal) {
		const QFontMetrics fm = m_oglTextNormal->fontMetrics();
		const int fontWidth = fm.horizontalAdvance(band);
		const float tx = float((x1 + x2 - fontWidth) / 2);
		const float ty = float(m_panRect.top() + fm.height());
		m_oglTextNormal->renderText(panelProjection(), tx, ty, band, QColor(255, 255, 255, 220));
	}
}

//************************************************************************
// The algorithms of the scale functions are taken from SDRMAXIII 
// (c) Catherine Moss, with permission.

void QGLWidebandPanel::renderVerticalScale() {

	QString str;
    QOpenGLPaintDevice paintDevice(m_dBmScaleFBO->size());

    painter.begin(&paintDevice);

	//QFontMetrics d_fm(m_smallFont);
	int spacing = 6;
	int fontHeight = m_fonts.smallFontMetrics->tightBoundingRect(".0dBm").height() + spacing;

    int width = m_dBmScaleRect.width();
    int height = m_dBmScaleRect.height();

	QRect textRect(0, 0, m_fonts.smallFontMetrics->boundingRect(QStringLiteral("-000.0")).width(), fontHeight);
	textRect.moveLeft(3);
	m_dBmScaleTextPos = -textRect.height();

	int len		= m_dBmScale.mainPointPositions.length();
	int sublen	= m_dBmScale.subPointPositions.length();
	
	// draw the scale background
	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.fillRect(0, 0, width, height, QColor(30, 30, 30, 180));
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QColor(165,193,206),1, Qt::SolidLine, Qt::FlatCap));
	if (len > 0) {
		for (int i = 0; i < len; i++) {
            painter.drawLine(0, m_dBmScale.mainPointPositions.at(i),4,m_dBmScale.mainPointPositions.at(i));
  		}
    painter.setPen(QPen(QColor(114,142,232),1, Qt::SolidLine, Qt::FlatCap));
		if (sublen > 0) {

			for (int i = 1; i < sublen; i++) {
               painter.drawLine(0,m_dBmScale.subPointPositions.at(i),2,m_dBmScale.subPointPositions.at(i));
			}
		}

		//glColor3f(0.75f, 0.86f, 0.91f);
    painter.setPen(QPen(QColor(191,219,232)));
    painter.setFont(m_oglTextNormal->font());

        for (int i = 0; i < len; i++) {

			textRect.moveBottom((int)m_dBmScale.mainPointPositions.at(i) + textRect.height()/2);
			
			//if (textRect.y() >= yOld && textRect.bottom() <= (m_dBmScaleRect.height() - textRect.height())) {
			if (textRect.y() > m_dBmScaleRect.top() + textRect.height() && textRect.bottom() <= (m_dBmScaleRect.height() - textRect.height()/2)) {
			
				str = QString::number((qreal)m_dBmScale.mainPoints.at(i), 'f', 1);
                painter.drawText(textRect.x() + 10, textRect.y() +  fontHeight, str);
				m_dBmScaleTextPos = textRect.bottom();
			}
		}
	}

	textRect.moveTop(m_dBmScaleRect.top());
    painter.setPen(QPen(QColor(239,56,109)));
	str = QString("dBm");
    painter.drawText(textRect.x() + 18  , textRect.y() +  fontHeight , str);
    painter.end();
}

void QGLWidebandPanel::renderHorizontalScale() {


    if (m_freqScaleRect.isEmpty()) return;
	QColor freqlabelColor = QColor(239,56,109);
    QColor textColor = QColor(140, 180, 200);
    QOpenGLPaintDevice paintDevice(m_frequencyScaleFBO->size());
    painter.begin(&paintDevice);
    //QFontMetrics d_fm(m_smallFont);
	int fontHeight = m_fonts.smallFontMetrics->tightBoundingRect(".0kMGHz").height();

	// draw the frequency scale
	int		offset_X		= -1;
	int		textOffset_y	= 5;
	double	freqScale		= 1;

	QString fstr = QString(" Hz ");
	if (m_upperFrequency >= 1e6) { freqScale = 1e6; fstr = QString("  MHz "); }
	else
	if (m_upperFrequency >= 1e3) { freqScale = 1e3; fstr = QString("  kHz "); }

	// draw the wide band scale background
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(0, 0, m_freqScaleRect.width(), m_freqScaleRect.height(), QColor(0, 0, 0, 255));
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
	QRect scaledTextRect(0, textOffset_y, 1, fontHeight);
    scaledTextRect.setWidth(m_fonts.smallFontMetrics->horizontalAdvance(fstr));
    scaledTextRect.moveLeft(m_freqScaleRect.width() - scaledTextRect.width());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(textColor,3, Qt::SolidLine, Qt::FlatCap));
    int len = m_frequencyScale.mainPointPositions.length();
    if (len > 0) {
		for (int i = 0; i < len; i++) {
            painter.drawLine(m_frequencyScale.mainPointPositions.at(i),1,m_frequencyScale.mainPointPositions.at(i),4);
		}
        painter.setFont( m_oglTextSmall->font());

        for (int i = 0; i < len; i++) {
		
			QString str = QString::number(m_frequencyScale.mainPoints.at(i) / freqScale, 'f', 3);
			if (freqScale == 1e3)
				while (str.endsWith('0')) str.remove(str.size() - 1, 1);

			if (str.endsWith('.')) str.remove(str.size() - 1, 1);

            int text_width = m_fonts.smallFontMetrics->horizontalAdvance(str);
			QRect textRect(m_frequencyScale.mainPointPositions.at(i) + offset_X - (text_width / 2), textOffset_y, text_width, fontHeight);

			if (textRect.left() < 0 || textRect.right() >= scaledTextRect.left()) continue;
            painter.drawText(textRect.x() , textRect.y() +  m_oglTextSmall->fontMetrics().height() , str);
        }
	}

    len = m_frequencyScale.subPointPositions.length();
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            painter.drawLine(m_frequencyScale.subPointPositions.at(i),1,m_frequencyScale.subPointPositions.at(i),3);
        }
    }

    painter.setPen(QPen(freqlabelColor));
    painter.drawText(m_freqScaleRect.width() - 30,  textOffset_y + 10 , fstr);
    painter.end();
}

void QGLWidebandPanel::renderGrid() {
	// Draw grid lines using QPainter (avoids deprecated glLineStipple / glBegin)
	QPen dotPen(m_gridColor, 1, Qt::DotLine, Qt::FlatCap);

	painter.begin(this);
	painter.setRenderHint(QPainter::Antialiasing, false);
	painter.setPen(dotPen);

	// Vertical lines (frequency grid)
	int len = m_frequencyScale.mainPointPositions.length();
	if (len > 0) {
		int y1 = m_panRect.top();
		int y2 = m_panRect.bottom();
		for (int i = 0; i < len; i++) {
			int x = m_frequencyScale.mainPointPositions.at(i);
			painter.drawLine(x, y1, x, y2);
		}
	}

	// Horizontal lines (dBm grid)
	len = m_dBmScale.mainPointPositions.length();
	if (len > 0) {
		int x1 = m_panRect.left();
		int x2 = m_panRect.right();
		for (int i = 0; i < len; i++) {
			int y = m_dBmScale.mainPointPositions.at(i);
			painter.drawLine(x1, y, x2, y);
		}
	}

	painter.end();
}
 
//********************************************************************

void QGLWidebandPanel::getRegion(QPoint p) {

	if (m_freqScaleRect.contains(p)) {

		m_mouseRegion = freqScaleRegion;
		
		if (m_displayTime.elapsed() >= 50) {
			
			m_displayTime.restart();
       //     update();
		}
	}
	else if (m_dBmScaleRect.contains(p)) {

		m_mouseRegion = dBmScaleRegion;

		if (m_displayTime.elapsed() >= 50) {
			
            m_displayTime.restart();
         //   update();
		}
	}
	else if (m_panRect.contains(p)) {

		m_mouseRegion = panRegion;

		// Spectrum/crosshair frames already refresh; avoid an extra update storm here.
	}
	else
		m_mouseRegion = elsewhere;

	/*if (m_displayTime.elapsed() >= 50) {

		m_displayTime.restart();
		updateGL();
		update();
	}*/
	//GRAPHICS_DEBUG << "region" << m_mouseRegion;
}

void QGLWidebandPanel::resizeGL(int iWidth, int iHeight) {

    if (m_dBmScaleFBO) { delete m_dBmScaleFBO; m_dBmScaleFBO = nullptr; }
    if (m_frequencyScaleFBO) { delete m_frequencyScaleFBO; m_frequencyScaleFBO = nullptr; }
    if (m_gridFBO) { delete m_gridFBO; m_gridFBO = nullptr; }

    // Update all sizes and rectangles
    setupDisplayRegions(QSize(iWidth, iHeight));

    // Mark all for renewal
    m_dBmScaleRenew = true;
    m_freqScaleRenew = true;
    m_panGridRenew = true;

    // Now, and only now, trigger update
    update();
}

void QGLWidebandPanel::setupDisplayRegions(QSize size) {

	m_displayTop = 0;
	int freqScaleRectHeight = 20;
	int dBmScaleWidth = 45;

	//WBGRAPHICS_DEBUG << "WB display width:" << size.width();
	//WBGRAPHICS_DEBUG << "WB display height:" << size.height()/6.0;
	
	m_freqScaleRect = QRect(
				0, 
				size.height() - m_displayTop - freqScaleRectHeight,
				size.width(), 
				freqScaleRectHeight);

	m_panRect = QRect(
			0,
			m_displayTop, 
			size.width(),
			m_freqScaleRect.top() - m_displayTop);

	m_dBmScaleRect = QRect(
				m_panRect.right() - dBmScaleWidth,
				m_displayTop, 
				dBmScaleWidth, 
				m_panRect.height());

	m_panRectWidth = (GLint)m_panRect.width();
	m_freqScaleUpdate = true;
	m_dBmScaleUpdate = true;
	m_panGridUpdate = true;
	
	//***************************************************************************
	/*WBGRAPHICS_DEBUG << "***************************************************************************";
	WBGRAPHICS_DEBUG << "total size" << size.height();
	WBGRAPHICS_DEBUG << "sizes (top, bottom, height):";
	WBGRAPHICS_DEBUG << "m_wideBandPanRect" << m_panRect.top() << m_panRect.bottom() << m_panRect.height();
	WBGRAPHICS_DEBUG << "m_freqScaleWideBandRect" << m_freqScaleRect.top() << m_freqScaleRect.bottom() << m_freqScaleRect.height();
	WBGRAPHICS_DEBUG << "m_dBmScaleWideBandRect" << m_dBmScaleRect.top() << m_dBmScaleRect.bottom() << m_dBmScaleRect.height();
	WBGRAPHICS_DEBUG << "";*/
}


 
//********************************************************************
// HMI control
 
void QGLWidebandPanel::enterEvent(QEnterEvent *event) {
	setFocus(Qt::MouseFocusReason);

	m_mousePos = QPoint(-1, -1);
	m_mouseRegion = elsewhere;
    update();

	QOpenGLWidget::enterEvent(event);
}

void QGLWidebandPanel::leaveEvent(QEvent *event) {
	m_mousePos = QPoint(-1, -1);
	m_mouseRegion = elsewhere;
    update();

	QOpenGLWidget::leaveEvent(event);
}

void QGLWidebandPanel::wheelEvent(QWheelEvent* event) {
	Q_UNUSED(event)
	
	//GRAPHICS_DEBUG << "wheelEvent";

	//if (event->buttons() == Qt::NoButton) getRegion(pos);

	//double freqStep = set->getMouseWheelFreqStep(m_currentReceiver);

	//switch (m_mouseRegion) {

	//	case panRegion:

	//		double delta = 0;
	//		if (event->delta() < 0) delta = -freqStep;
	//		else
	//		if (event->delta() > 0) delta =  freqStep;

	//		if (m_frequency + delta > set->getMaxFrequency())
	//			m_frequency = set->getMaxFrequency();
	//		else
	//		if (m_frequency + delta < 0)
	//			m_frequency = 0;
	//		else
	//			// snap to the frequency step
	//			m_frequency = (long)(qRound((m_frequency + delta) / qAbs(freqStep)) * qAbs(freqStep));

	//		set->setFrequency(true, m_currentReceiver, m_frequency);
	//		break;
	//}

 //	//updateGL();
	//update();
}

void QGLWidebandPanel::mousePressEvent(QMouseEvent* event) {
	//GRAPHICS_DEBUG << "mousePressEvent";
	m_mousePos = event->pos();
	m_mouseDownPos = m_mousePos;

	getRegion(m_mousePos);

	if (m_mouseRegion == freqScaleRegion) {

		m_yScaleMouseDownPos = m_freqScaleRect.topLeft();

		if (event->buttons() == Qt::LeftButton || event->buttons() == Qt::RightButton)
			m_dragFreqScale = true;
		m_dragFreqScaleZoom = (event->button() == Qt::RightButton);
		
		if (event->buttons() == Qt::RightButton) {
			if (cursor().shape() != Qt::SplitHCursor)
				setCursor(Qt::SplitHCursor);
		}
		// Scale drag feedback only — avoid stacking updates with other GL panels.
		update();
		return;
	}
	else if (m_mouseRegion == dBmScaleRegion) {

		if (event->buttons() == Qt::LeftButton)
			m_dragDBmScale = true;

		if (event->buttons() == Qt::RightButton) {

			if (cursor().shape() != Qt::SplitVCursor)
				setCursor(Qt::SplitVCursor);
			m_yScaleMouseDownPos = m_dBmScaleRect.topLeft();
		}

		if (event->buttons() == Qt::LeftButton &&
			event->modifiers() == Qt::ControlModifier) {

			if (cursor().shape() != Qt::SplitVCursor)
				setCursor(Qt::SplitVCursor);

			m_dBmPanMinOld = m_dBmPanMin;
			m_dBmPanMaxOld = m_dBmPanMax;

			m_dBmScaleOffset = 0.0;
			m_calibrate = true;
		}
        setupDisplayRegions(size());
        m_dBmScaleUpdate = true;
        m_freqScaleUpdate = true;

        update();
		return;
	}
	else if (m_mouseRegion == panRegion) {

		if (event->buttons() == Qt::RightButton) {

			if (m_crossHairCursor) {

				m_crossHairCursor = false;
				if (cursor().shape() != Qt::ArrowCursor)
					setCursor(Qt::ArrowCursor);
			}
			else {

				m_crossHairCursor = true;
				if (cursor().shape() != Qt::BlankCursor)
					setCursor(Qt::BlankCursor);
			}
			update();
		}
		else if (event->buttons() == Qt::LeftButton) {
			
			const qreal wbRange = qMax<qreal>(1.0, m_widebandMaxFrequency - m_widebandMinFrequency);
			float unit = (float)(m_panRect.width() / (wbRange * m_freqScaleZoomFactor));

			
			m_frequency = (long)(1000 * (int)(qRound(m_mousePos.x()/unit + m_lowerFrequency)/1000));
			// Frequency signals already refresh display/receiver/wideband — do not
			// add another full NoPartialUpdate clear here (causes click flicker).
			set->setCtrFrequency(0, m_currentReceiver, m_frequency);
			set->setVFOFrequency(0, m_currentReceiver, m_frequency);
		}
		return;
	}
}

void QGLWidebandPanel::mouseReleaseEvent(QMouseEvent *event) {
	//GRAPHICS_DEBUG << "mouseReleaseEvent";
	m_mousePos = event->pos();
	m_mouseDownPos = m_mousePos;

	getRegion(m_mousePos);

	if (m_mouseRegion == freqScaleRegion) {
		m_dragFreqScale = false;
		m_dragFreqScaleZoom = false;
		m_freqScaleUpdate = true;
		if (cursor().shape() != Qt::ArrowCursor)
			setCursor(Qt::ArrowCursor);
		update();
		return;
	}
	else if (m_mouseRegion == dBmScaleRegion) {
		m_dragDBmScale = false;
		m_dBmScaleUpdate = true;
		if (cursor().shape() != Qt::ArrowCursor)
			setCursor(Qt::ArrowCursor);
		update();
		return;
	}
	else if (m_mouseRegion == panRegion) {
		return;
	}
	
	if (m_calibrate) m_calibrate = false;
}

void QGLWidebandPanel::mouseMoveEvent(QMouseEvent* event) {
	QPoint pos = event->pos();
	m_mousePos = event->pos();

	if (event->buttons() == Qt::NoButton) getRegion(pos);

	bool needUpdate = false;

	switch (m_mouseRegion) {

		case panRegion:
			
			if (m_crossHairCursor) {
				if (cursor().shape() != Qt::BlankCursor)
					setCursor(Qt::BlankCursor);
			} else if (cursor().shape() != Qt::ArrowCursor) {
				setCursor(Qt::ArrowCursor);
			}
			// Crosshair tracks via spectrum paints (m_panTimer); no mouse-driven update().
			break;

		case dBmScaleRegion:
			
			if (event->buttons() == Qt::LeftButton &&
				event->modifiers() == Qt::ControlModifier) {

				QPoint dPos = m_mouseDownPos - pos;
				qreal unit = (qreal)(qAbs(m_dBmPanMax - m_dBmPanMin) / m_panRect.height());
				qreal delta =  - unit * dPos.y();

				qreal newMin = m_dBmPanMin + delta;
				qreal newMax = m_dBmPanMax + delta;

				m_dBmScaleOffset -= delta / unit;
				WBGRAPHICS_DEBUG << "m_dBmScaleOffset: " << m_dBmScaleOffset;

				if (newMin > MINDBM && newMax < MAXDBM) {

					m_dBmPanMin = newMin;
					m_dBmPanMax = newMax;
				}

				m_mouseDownPos = pos;
				m_dBmScaleUpdate = true;
				m_panGridUpdate = true;
				needUpdate = true;
			}
			else if (event->buttons() == Qt::LeftButton) {

				QPoint dPos = m_mouseDownPos - pos;
				qreal unit = (qreal)(qAbs(m_dBmPanMax - m_dBmPanMin) / m_panRect.height());
				
				qreal newMin = m_dBmPanMin - unit * dPos.y();
				qreal newMax = m_dBmPanMax - unit * dPos.y();

				if (newMin > MINDBM && newMax < MAXDBM) {
				
					m_dBmPanMin = newMin;
					m_dBmPanMax = newMax;

					set->setWidebanddBmScaleMin(m_dBmPanMin);
					set->setWidebanddBmScaleMax(m_dBmPanMax);
				}
				
				m_mouseDownPos = pos;
				m_dBmScaleUpdate = true;
				m_panGridUpdate = true;
				needUpdate = true;
			}
			else if (event->buttons() == Qt::RightButton) {
				QPoint dPos = m_mouseDownPos - pos;
				if (dPos.y() > 0)
					m_dBmPanDelta = 1.0;
				else if (dPos.y() < 0)
					m_dBmPanDelta = -1.0f;
				
				m_dBmPanMax -= m_dBmPanDelta;

				if (qAbs(m_dBmPanMax - m_dBmPanMin) < 10) {

					m_dBmPanMin -= m_dBmPanDelta;
					m_dBmPanMax += m_dBmPanDelta;
				}
				if (m_dBmPanMin < MINDBM) m_dBmPanMin = MINDBM;
				if (m_dBmPanMax > MAXDBM) m_dBmPanMax = MAXDBM;

				set->setWidebanddBmScaleMin(m_dBmPanMin);
				set->setWidebanddBmScaleMax(m_dBmPanMax);

				m_mouseDownPos = pos;
				m_dBmScaleUpdate = true;
				m_panGridUpdate = true;
				needUpdate = true;
			}
			else if (cursor().shape() != Qt::ArrowCursor) {
				setCursor(Qt::ArrowCursor);
			}
			break;

		case freqScaleRegion:
			if (event->buttons() == Qt::LeftButton) {

				if (m_freqScaleZoomFactor < 1.0) {

					QPoint dPos = m_mouseDownPos - pos;

					m_frequencySpan = (m_widebandMaxFrequency - m_widebandMinFrequency) * m_freqScaleZoomFactor;

					const qreal wbRange = qMax<qreal>(1.0, m_widebandMaxFrequency - m_widebandMinFrequency);
					qreal unit = (qreal)((wbRange * m_freqScaleZoomFactor) / m_freqScaleRect.width());

					m_lowerFrequency += unit * dPos.x();
					m_upperFrequency = m_lowerFrequency + m_frequencySpan;

					if (m_lowerFrequency < m_widebandMinFrequency) m_lowerFrequency = m_widebandMinFrequency;
					if (m_upperFrequency > m_widebandMaxFrequency) {

						m_upperFrequency = m_widebandMaxFrequency;
						m_lowerFrequency = (qreal)(m_widebandMaxFrequency - m_frequencySpan);
					}

					m_mouseDownPos = pos;
                    m_freqScaleUpdate = true;
                    m_panGridUpdate = true;
					needUpdate = true;
				}
				else {

					m_lowerFrequency = m_widebandMinFrequency;
					m_upperFrequency = m_widebandMaxFrequency;
				}
			}
			else if (event->buttons() == Qt::RightButton) {

				QPoint dPos = m_mouseDownPos - pos;

				if (dPos.x() > 0) {

					m_freqScaleZoomFactor += 0.005f;
				}
				else if (dPos.x() < 0)
					m_freqScaleZoomFactor -= 0.005f;

				if (m_freqScaleZoomFactor > 1.0) m_freqScaleZoomFactor = 1.0f;
				if (m_freqScaleZoomFactor < 0.15) m_freqScaleZoomFactor = 0.15f;

				const qreal wbRange = qMax<qreal>(1.0, m_widebandMaxFrequency - m_widebandMinFrequency);
				qreal unit = (qreal)((wbRange * m_freqScaleZoomFactor) / m_freqScaleRect.width());
				m_lowerFrequency -= unit * dPos.x();
				m_upperFrequency = m_lowerFrequency + m_frequencySpan;

				if (m_lowerFrequency < m_widebandMinFrequency) m_lowerFrequency = m_widebandMinFrequency;
				if (m_upperFrequency > m_widebandMaxFrequency) {

					m_upperFrequency = m_widebandMaxFrequency;
					m_lowerFrequency = (qreal)(m_widebandMaxFrequency - m_frequencySpan);
				}

				m_mouseDownPos = pos;
                m_freqScaleUpdate = true;
                m_panGridUpdate = true;
				needUpdate = true;
			}
			else if (cursor().shape() != Qt::ArrowCursor) {
				setCursor(Qt::ArrowCursor);
			}
			break;

		case elsewhere:
			break;
	}

	if (needUpdate && m_displayTime.elapsed() >= 33) {
		m_displayTime.restart();
		update();
	}
}

void QGLWidebandPanel::keyPressEvent(QKeyEvent* event) {
	
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
    update();
}

void QGLWidebandPanel::timerEvent(QTimerEvent *event) {
    if (event->timerId() == m_dprPollTimerId) {
        qreal currentDpr = devicePixelRatioF();
        if (!qFuzzyCompare(currentDpr, dpr)) { // seems like dpr is not updated on the fly.
            qDebug() << "DPR changed! Old:" << dpr << "New:" << currentDpr;
            dpr = currentDpr;
            if (m_oglTextTiny)
                m_oglTextTiny->setDevicePixelRatio(dpr);
            if (m_oglTextSmall)
                m_oglTextSmall->setDevicePixelRatio(dpr);
            if (m_oglTextNormal)
                m_oglTextNormal->setDevicePixelRatio(dpr);
            // Recreate FBOs and update geometry
            if (m_dBmScaleFBO) { delete m_dBmScaleFBO; m_dBmScaleFBO = nullptr; }
            if (m_frequencyScaleFBO) { delete m_frequencyScaleFBO; m_frequencyScaleFBO = nullptr; }
            if (m_gridFBO) { delete m_gridFBO; m_gridFBO = nullptr; }
            setupDisplayRegions(size());
            m_dBmScaleRenew = true;
            m_freqScaleRenew = true;
            m_panGridRenew = true;
            update();
        }
        return;
    }
    QOpenGLWidget::timerEvent(event);
}
 
//********************************************************************
 
void QGLWidebandPanel::setFrequency(int mode, int rx, qint64 freq) {

	Q_UNUSED (mode)

	if (rx != m_currentReceiver) return;
	if (m_frequency == freq && m_hwInterface != QSDR::SoapySDR) return;
	
	m_frequency = freq;
	if (m_hwInterface == QSDR::SoapySDR && m_freqScaleZoomFactor >= 0.999f) {
		const qreal span = qMax<qreal>(1.0, m_widebandMaxFrequency - m_widebandMinFrequency);
		m_lowerFrequency = qMax<qreal>(m_widebandMinFrequency, static_cast<qreal>(m_frequency) - span * 0.5);
		m_upperFrequency = m_lowerFrequency + span;
		if (m_upperFrequency > m_widebandMaxFrequency) {
			m_upperFrequency = m_widebandMaxFrequency;
			m_lowerFrequency = m_widebandMaxFrequency - span;
		}
	}
	m_freqScaleUpdate = true;
	m_panGridUpdate = true;

	// Spectrum timer already paints while live. Digit-wheel retunes must not
	// force NoPartialUpdate clears (Core 3.3 window flash).
	if (m_dataEngineState != QSDR::DataEngineUp && m_panTimer.elapsed() >= 50) {
		m_panTimer.restart();
		update();
	}
}

void QGLWidebandPanel::setCurrentReceiver(int value) {

	m_currentReceiver = value;
	m_frequency = set->getVfoFrequency(m_currentReceiver);
	m_freqScaleUpdate = true;
	m_panGridUpdate = true;
	update();
}

//void QGLWidebandPanel::computeDisplayBins(const float *panBuffer) {
//
//	//int newSampleSize = 0;
//	//int deltaSampleSize = 0;
//	//int idx = 0;
//	//int lIdx = 0;
//	//int rIdx = 0;
//	//qreal localMax;
//
//
//	//m_panScale = (qreal)(1.0 * newSampleSize / m_panRectWidth);
//	//m_scaleMultOld = m_displayData.scaleMult;
//	//	
//	//if (m_panScale < 0.125) {
//	//	m_displayData.scaleMult = 0.0625;
//	//}
//	//else if (m_panScale < 0.25) {
//	//	m_displayData.scaleMult = 0.125;
//	//}
//	//else if (m_panScale < 0.5) {
//	//	m_displayData.scaleMult = 0.25;
//	//}
//	//else if (m_panScale < 1.0) {
//	//	m_displayData.scaleMult = 0.5;
//	//}
//	//else {
//	//	m_displayData.scaleMult = 1.0;
//	//}
//
//	//m_panSpectrumBinsLength = (GLint)(m_displayData.scaleMult * m_panRectWidth);
//
//	//if (m_scaleMultOld != m_displayData.scaleMult) {
//
//	//	m_displayData.waterfallUpdate = true;
//	//}
//
//	//m_displayData.waterfallPixel.clear();
//	//m_displayData.waterfallPixel.resize(4 * m_panRectWidth);
//
//	//m_displayData.panadapterBins.clear();
//	//
//	//for (int i = 0; i < m_panSpectrumBinsLength; i++) {
//	//		
//	//	idx = 0;
//	//	lIdx = (int)floor((qreal)(i * m_panScale / m_displayData.scaleMult));
//	//	rIdx = (int)floor((qreal)(i * m_panScale / m_displayData.scaleMult) + m_panScale / m_displayData.scaleMult);
//	//				
//	//	// max value; later we try mean value also!
//	//	localMax = -10000.0F;
//	//	for (int j = lIdx; j < rIdx; j++) {
//
//	//		if (panBuffer[j] > localMax) {
//
//	//			localMax = panBuffer[j];
//	//			idx = j;
//	//		}
//	//	}
//	//	idx += deltaSampleSize/2;
//	//			
//	//	m_displayData.panadapterBins << panBuffer[idx] - m_displayData.dBmPanMin - m_dBmPanLogGain;
//
//	//update();
//	////updateGL();
//}

void QGLWidebandPanel::setWidebandSpectrumBuffer(const qVectorFloat &buffer) {
	m_wbSpectrumBufferLength = buffer.size();
	if (m_wbSpectrumBufferLength <= 0)
		return;

	const TWideband wbOpts = set->getWidebandOptions();
	const bool averagingEnabled = wbOpts.averaging;
	const int requestedAvgCnt = qMax(1, wbOpts.averagingCnt);

	mutex.lock();
	if (!averagingEnabled || requestedAvgCnt <= 1) {
		// Pass-through mode: clear averaging history so slider changes apply immediately.
		specAv_queue.clear();
		m_wbAverageAccum.clear();
		m_specAveragingCnt = requestedAvgCnt;
		m_scale = 1.0f;
		m_wbSpectrumBuffer = buffer;
	} else {
		if (m_specAveragingCnt != requestedAvgCnt || m_wbAverageAccum.size() != m_wbSpectrumBufferLength) {
			specAv_queue.clear();
			m_wbAverageAccum = qVectorFloat(m_wbSpectrumBufferLength, 0.0f);
		}
		m_specAveragingCnt = requestedAvgCnt;
		m_scale = 1.0f / m_specAveragingCnt;

		specAv_queue.enqueue(QVector<float>(buffer));
		if (m_wbAverageAccum.size() != m_wbSpectrumBufferLength)
			m_wbAverageAccum = qVectorFloat(m_wbSpectrumBufferLength, 0.0f);

		const QVector<float> &latest = specAv_queue.back();
		for (int i = 0; i < m_wbSpectrumBufferLength; ++i)
			m_wbAverageAccum[i] += latest.at(i);

		while (specAv_queue.size() > m_specAveragingCnt) {
			const QVector<float> oldest = specAv_queue.dequeue();
			for (int i = 0; i < m_wbSpectrumBufferLength; ++i)
				m_wbAverageAccum[i] -= oldest.at(i);
		}

		const int divisor = qMax(1, specAv_queue.size());
		m_wbSpectrumBuffer.resize(m_wbSpectrumBufferLength);
		for (int i = 0; i < m_wbSpectrumBufferLength; ++i)
			m_wbSpectrumBuffer[i] = m_wbAverageAccum.at(i) / divisor;
	}
	mutex.unlock();

//	deltaIdx = qFloor((qreal)(m_wbSpectrumBufferLength * (m_lowerFrequency / set->getMaxFrequency())));
//	frequencyScale = (qreal)(1.0f * m_scaledBufferSize / width);
//
//	if (frequencyScale < 0.125)
//		scaleMult = 0.0625;
//	else if (frequencyScale < 0.25)
//		scaleMult = 0.125;
//	else if (frequencyScale < 0.5)
//		scaleMult = 0.25;
//	else if (frequencyScale < 1.0)
//		scaleMult = 0.5;

	if (m_panTimer.elapsed() >= 50) {  // ~20 FPS — keep below receiver rate to reduce compose thrash
		m_panTimer.restart();
		update();
	}
}

void QGLWidebandPanel::setWidebandFrequencyRange(qreal lowHz, qreal highHz) {
	if (m_hwInterface != QSDR::SoapySDR)
		return;
	if (highHz <= lowHz)
		return;
	m_widebandMinFrequency = lowHz;
	m_lowerFrequency = lowHz;
	m_widebandMaxFrequency = highHz;
	m_upperFrequency = highHz;
	m_frequencySpan = m_upperFrequency - m_lowerFrequency;
	m_freqScaleZoomFactor = 1.0f;
	m_freqScaleUpdate = true;
	m_panGridUpdate = true;
	update();
}

void QGLWidebandPanel::resetWidebandSpectrumBuffer() {

	QMutexLocker lock(&mutex);
	specAv_queue.clear();
	m_wbAverageAccum.clear();
	m_wbSpectrumBuffer.resize(BIGWIDEBANDSIZE / 2);
	m_wbSpectrumBuffer.fill(-1000.0f);
	m_wbSpectrumBufferLength = m_wbSpectrumBuffer.size();
	m_widebandPanSpectrumBins.clear();
}

void QGLWidebandPanel::systemStateChanged(
	QSDR::_Error err, 
	QSDR::_HWInterfaceMode hwmode, 
	QSDR::_ServerMode mode, 
	QSDR::_DataEngineState state)
{
	Q_UNUSED (err)
	Q_UNUSED (state)

	if (m_hwInterface != hwmode)
		m_hwInterface = hwmode;

	//bool change = false;

	if (m_dataEngineState != state) {
		
		m_dataEngineState = state;
		//change = true;
	}

	//if (m_serverMode != mode)
	//	m_serverMode = mode;

	if (m_serverMode == mode)
		return;
	else {

		{
			QMutexLocker lock(&mutex);
			m_wbSpectrumBuffer.fill(-10000.0);
			m_widebandPanSpectrumBins.clear();
		}
		//memset(&m_wbSpectrumBuffer, -10000, 4 * BUFFER_SIZE * sizeof(float));
		m_serverMode = mode;
	}

	//resizeGL(width(), height());
	m_displayTime.restart();

	update();
}

void QGLWidebandPanel::graphicModeChanged(
	int rx,
	PanGraphicsMode panMode,
	WaterfallColorMode colorScheme)
{
	//Q_UNUSED (rx)

	if (rx != -1) return;

	bool change = false;

	if (m_panMode != panMode) {
		
		m_panMode = panMode;
		change = true;
	}

	if (m_waterfallMode != colorScheme) {

		m_waterfallMode = colorScheme;
		change = true;
	}

	if (!change) return;

	update();
}

 
//void QGLWidebandPanel::setSpectrumAveragingCnt(int value) {
//
//	mutex.lock();
//
//		memset(m_tmpBuf, 0, SAMPLE_BUFFER_SIZE * sizeof(float));
//
//		while (!specAv_queue.isEmpty())
//			specAv_queue.dequeue();
//
//		m_specAveragingCnt = value;
//
//		if (m_specAveragingCnt > 0)
//			m_scale = 1.0f / m_specAveragingCnt;
//		else
//			m_scale = 1.0f;
//
//	mutex.unlock();
//}

void QGLWidebandPanel::setMercuryAttenuator(HamBand band, int value) {

	Q_UNUSED(band)
	
		m_mercuryAttenuator = value;
	update();
}

void QGLWidebandPanel::setPanadapterColors() {

	m_spectrumColorsChanged = true;

	mutex.lock();
	m_bkgRed   = (GLfloat)(set->getPanadapterColors().panBackgroundColor.red() / 256.0);
	m_bkgGreen = (GLfloat)(set->getPanadapterColors().panBackgroundColor.green() / 256.0);
	m_bkgBlue  = (GLfloat)(set->getPanadapterColors().panBackgroundColor.blue() / 256.0);

	m_r	= (GLfloat)(set->getPanadapterColors().wideBandLineColor.red() / 256.0);
	m_g = (GLfloat)(set->getPanadapterColors().wideBandLineColor.green() / 256.0);
	m_b	= (GLfloat)(set->getPanadapterColors().wideBandLineColor.blue() / 256.0);

	m_rf = (GLfloat)(set->getPanadapterColors().wideBandFilledColor.red() / 256.0);
	m_gf = (GLfloat)(set->getPanadapterColors().wideBandFilledColor.green() / 256.0);
	m_bf = (GLfloat)(set->getPanadapterColors().wideBandFilledColor.blue() / 256.0);

	m_redST	  = (GLfloat)(set->getPanadapterColors().panSolidTopColor.red() / 256.0);
	m_greenST = (GLfloat)(set->getPanadapterColors().panSolidTopColor.green() / 256.0);
	m_blueST  = (GLfloat)(set->getPanadapterColors().panSolidTopColor.blue() / 256.0);

	m_redSB   = (GLfloat)(set->getPanadapterColors().panSolidBottomColor.red() / 256.0);
	m_greenSB = (GLfloat)(set->getPanadapterColors().panSolidBottomColor.green() / 256.0);
	m_blueSB  = (GLfloat)(set->getPanadapterColors().panSolidBottomColor.blue() / 256.0);

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

void QGLWidebandPanel::setPanGridStatus(bool value, int rx) {

	Q_UNUSED (rx)

	mutex.lock();

	 if (m_panGrid == value) 
		 return;
	 else
		 m_panGrid = value;

	 mutex.unlock();
	 update();
}

void QGLWidebandPanel::sampleRateChanged(int value) {

	m_sampleRate = value;
	update();
}


void QGLWidebandPanel::closeEvent(QCloseEvent *event) {

        emit closeEvent();
        QWidget::closeEvent(event);
}

void QGLWidebandPanel::showEvent(QShowEvent *event) {
        emit showEvent();
        QWidget::showEvent(event);
}

void QGLWidebandPanel::qglColor(QColor color)
{
	Q_UNUSED(color);
}

