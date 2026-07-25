/**
* @file  cusdr_oglDistancePanel.cpp
* @brief distance panel class for cuSDR
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2012-02-14
*/

/*
 *   Copyright 2012 Hermann von Hasseln, DL3HVH
 *
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

#include "cusdr_oglDistancePanel.h"
#include "cusdr_glShaders.h"
#include <QGuiApplication>
#include <QMatrix4x4>
#include <QVarLengthArray>
#include <QOpenGLPaintDevice>


#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

QGLDistancePanel::QGLDistancePanel(QWidget *parent)
	: QOpenGLWidget( parent)

	, set(Settings::instance())
	, m_serverMode(set->getCurrentServerMode())
	, m_hwInterface(set->getHWInterface())
	, m_dataEngineState(QSDR::DataEngineDown)
	, m_panMode(set->getPanadapterMode(0))
	, m_mousePos(QPoint(-1, -1))
	, m_mouseDownPos(QPoint(-1, -1))
	, m_specAveragingCnt(set->getSpectrumAveragingCnt(0))
	, m_freqRulerDisplayWidth(0)
	, m_panSpectrumMinimumHeight(70)
	, m_spectrumUpdate(false)
	, m_showZerodBmLine(false)
	, m_spectrumVertexColorUpdate(false)
	, m_spectrumColorsChanged(true)
	, m_spectrumAveraging(set->getSpectrumAveraging(0))
	, m_spectrumAveragingOld(m_spectrumAveraging)
	, m_crossHairCursor(false)
	, m_panGrid(set->getPanGridStatus(0))
	, m_freqRulerPosition(0.5)
	, m_distRulerDisplayDelta(0.0)
	, m_distRulerDisplayDeltaStep(500.0)
	, m_distRulerMaxDist(150000.0)
	, m_freqScaleZoomFactor(1.0f)
	, m_distScaleZoomFactor(1.0) // 0.2 .. 1.0
	, m_scaleMult(1.0f)
	, m_filterLowerFrequency(-3050.0)
	, m_filterUpperFrequency(-150.0)
	, m_snapMouse(3)
	, m_sampleRate(set->getSampleRate())
{
    m_shaderProgram = nullptr;
    m_textureProgram = nullptr;
    m_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);

	setMouseTracking(true);
	setFocusPolicy(Qt::StrongFocus);

	setupDisplayRegions(size());
	m_oldWidth = size().width();
	
	m_frequency = set->getVfoFrequency(0);

	const HamBand band = set->getCurrentHamBand(0);
	m_dBmPanMin = set->getdBmPanScaleMin(0, band);
	m_dBmPanMax = set->getdBmPanScaleMax(0, band);

	m_filterLowerFrequency = set->getFilterLo(0);
	m_filterUpperFrequency = set->getFilterHi(0);

	m_dBmScalePanadapterRenew = true;
	m_dBmScalePanadapterUpdate = true;
	m_freqScalePanadapterRenew = true;
	m_freqScalePanadapterUpdate = true;
	m_panGridRenew = true;
	m_panGridUpdate = true;

	fonts = new CFonts(this);
	m_fonts = fonts->getFonts();

	m_oglTextTiny = new OGLText(m_fonts.tinyFont);
	m_oglTextSmall = new OGLText(m_fonts.smallFont);
	m_oglTextNormal = new OGLText(m_fonts.normalFont);

	timer = 0;

	setupConnections();

	m_displayTime.start();
	m_resizeTime.start();
	//freqChangeTimer.start();
	
	memset(m_spectrumBuffer, -10000, 4 * BUFFER_SIZE * sizeof(float));
	memset(m_distanceSpectrumBuffer, -10000, 16 * BUFFER_SIZE * sizeof(float));

	m_kilometersPerGate = 0.5f * 3E5 / (m_sampleRate/m_downRate);

	m_dBmPanLogGain = 69; // allow user to calibrate this value

	m_dBmDistMin = set->getdBmDistScaleMin();
	m_dBmDistMax = set->getdBmDistScaleMax();
	m_dBmDistScaleMin = m_dBmDistMin;
	m_dBmDistScaleMax = m_dBmDistMax;

	m_panSpectrumBinsLength = 0;	
	
	/*m_bkgRed   = (GLfloat)(set->getPanadapterColors().panBackgroundColor.red() / 256.0);
	m_bkgGreen = (GLfloat)(set->getPanadapterColors().panBackgroundColor.green() / 256.0);
	m_bkgBlue  = (GLfloat)(set->getPanadapterColors().panBackgroundColor.blue() / 256.0);*/

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

	m_frequencyScaleFBO = 0;
	m_dBmScaleFBO = 0;
	m_panadapterGridFBO = 0;
	m_textureFBO = 0;
	
	if (m_specAveragingCnt > 0)
		m_scale = 1.0f / m_specAveragingCnt;
	else
		m_scale = 1.0f;

	memset(m_tmpBuf, 0, SAMPLE_BUFFER_SIZE * sizeof(float));
}

QGLDistancePanel::~QGLDistancePanel() {

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
	
	makeCurrent();
	glFinish();

	if (m_frequencyScaleFBO) {

		delete m_frequencyScaleFBO;
		m_frequencyScaleFBO = 0;
	}

	if (m_textureFBO) {

		delete m_textureFBO;
		m_textureFBO = 0;
	}

	if (m_dBmScaleFBO) {

		delete m_dBmScaleFBO;
		m_dBmScaleFBO = 0;
	}

	if (m_panadapterGridFBO) {

		delete m_panadapterGridFBO;
		m_panadapterGridFBO = 0;
	}

	while (!specAv_queue.isEmpty())
		specAv_queue.dequeue();
}

//QSize QGLDistancePanel::minimumSizeHint() const {
//	
//	//return QSize(1000, 1024);
//	return QSize(width(), height());
//}

QSize QGLDistancePanel::sizeHint() const {
	
	return QSize(width(), height());
}

void QGLDistancePanel::setupConnections() {

	CHECKED_CONNECT(
		set,
		&Settings::systemStateChanged,
		this,
		&QGLDistancePanel::systemStateChanged);

	CHECKED_CONNECT(
		set, 
		&Settings::graphicModeChanged,
		this, 
		&QGLDistancePanel::graphicModeChanged);

	CHECKED_CONNECT(
		set, 
		&Settings::freqRulerPositionChanged, 
		this, 
		&QGLDistancePanel::freqRulerPositionChanged);

	CHECKED_CONNECT(
		set, 
		&Settings::sampleRateChanged, 
		this, 
		&QGLDistancePanel::sampleRateChanged);

	CHECKED_CONNECT(
		set, 
		&Settings::filterFrequenciesChanged, 
		this, 
		&QGLDistancePanel::setFilterFrequencies);

    /*
	CHECKED_CONNECT_OPT(
		set, 
		&Settings::chirpSpectrumBufferChanged,
		this,
		&QGLDistancePanel::distanceSpectrumBufferChanged,
		Qt::DirectConnection);

	CHECKED_CONNECT(
		set, 
		&Settings::chirpFFTShowChanged,
		this,
		&QGLDistancePanel::setChirpFFTShow);

	CHECKED_CONNECT(
		set, 
		SIGNAL(waterfallTimeChanged(int, int)),
		this,
		SLOT(setWaterfallTime(int, int)));

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
    */

	CHECKED_CONNECT(
		set, 
		&Settings::spectrumAveragingChanged, 
		this, 
		&QGLDistancePanel::setSpectrumAveraging);

	CHECKED_CONNECT(
		set, 
		&Settings::spectrumAveragingCntChanged, 
		this, 
		&QGLDistancePanel::setSpectrumAveragingCnt);

	CHECKED_CONNECT(
		set, 
		&Settings::panGridStatusChanged,
		this,
		&QGLDistancePanel::setPanGridStatus);

	CHECKED_CONNECT(
		set, 
		&Settings::panadapterColorChanged,
		this,
		&QGLDistancePanel::setPanadapterColors);
}

void QGLDistancePanel::initializeGL() {

	if (!isValid()) return;
    initializeOpenGLFunctions();

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
    m_vbo.setUsagePattern(QOpenGLBuffer::StreamDraw);

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
        qCritical() << "Distance panel texture shader link failed:" << m_textureProgram->log();
    }

	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	m_cnt = 0;
}

QMatrix4x4 QGLDistancePanel::panelProjection() const
{
	QMatrix4x4 projection;
	projection.ortho(0, width(), height(), 0, -10, 10);
	return projection;
}

void QGLDistancePanel::drawPanelRect(const QRect &rect, const QColor &color, float z)
{
	if (rect.isEmpty())
		return;
	m_vao.bind();
	if (m_shaderProgram && m_shaderProgram->isLinked())
		GlDraw::drawSolidRect(this, m_shaderProgram, m_vbo, panelProjection(), rect, color, z);
}

void QGLDistancePanel::drawPanelGradientRect(const QRect &rect, const QColor &c1, const QColor &c2,
                                             bool leftToRight, float z)
{
	if (rect.isEmpty())
		return;
	m_vao.bind();
	if (m_shaderProgram && m_shaderProgram->isLinked())
		GlDraw::drawGradientRect(this, m_shaderProgram, m_vbo, panelProjection(),
		                         rect, c1, c2, leftToRight, z);
}

void QGLDistancePanel::drawCachedTexture(const QRect &rect, GLuint texId, float z)
{
	if (rect.isEmpty() || !texId)
		return;
	m_vao.bind();
	if (m_textureProgram && m_textureProgram->isLinked())
		GlDraw::renderTexturedQuad(this, m_textureProgram, m_vbo, panelProjection(), rect, texId, z);
}

void QGLDistancePanel::renderPanelText(OGLText *text, float x, float y, const QString &str)
{
	renderPanelText(text, x, y, 0.0f, str);
}

void QGLDistancePanel::renderPanelText(OGLText *text, float x, float y, float z, const QString &str)
{
	if (!text)
		return;
	text->renderText(panelProjection(), x, y, z, str, m_glTextColor);
}

void QGLDistancePanel::paintGL() {

	drawPanelRect(QRect(0, 0, width(), height()), QColor(0, 0, 0), -5.0f);
}
 
//****************************************************
// painting modes
void QGLDistancePanel::paintReceiverDisplay() {
 
	QRect mouse_rect(0, 0, 100, 100);
	mouse_rect.moveCenter(m_mousePos);

	drawPanadapter();
	drawPanHorizontalScale();
	drawPanVerticalScale();

	if (m_panGrid)
		drawPanadapterGrid();

	drawPanFilter();
	
	if (m_mouseRegion == panadapterRegion && m_crossHairCursor)
		drawCrossHair();

	m_oldFreq = m_frequency;
 }

void QGLDistancePanel::paintChirpWSPRDisplay() {

	//drawGLBackground(m_distanceSpectrumRect);

	distanceSpectrumBufferMutex.lock();
	drawDistanceSpectrum();
	distanceSpectrumBufferMutex.unlock();

	drawDistHorizontalScale();
	drawDistVerticalScale();
	
	//// chirp distance spectrum
	//m_distancePanadapter->drawGLDistanceRuler(
	//				m_freqScaleDistancePanRect,
	//				m_freqScalePanadapterRect,
	//				m_mouseRegion,
	//				m_distRulerZoomFactor,
	//				m_distRulerDisplayDelta,
	//				m_kilometersPerGate,
	//				m_showChirpFFT,
	//				m_chirpBufferLength);

	//m_distancePanadapter->drawGLdBmScale(
	//				m_dBmScaleDistancePanRect,
	//				m_distanceSpectrumRect,
	//				m_mouseRegion,
	//				m_dBmDistScaleMin,
	//				m_dBmDistScaleMax,
	//				m_showZerodBmLine);

	//m_distancePanadapter->drawGLGrid(m_distanceSpectrumRect, m_dBmScaleDistancePanRect);
}

void QGLDistancePanel::drawPanadapter() {
    float dpr = (float)devicePixelRatio();
	GLint vertexArrayLength = (GLint)m_panadapterBins.size();
    if (vertexArrayLength == 0) return;

	GLint height = m_panRect.height();
	GLint x1 = m_panRect.left();
	GLint y1 = m_panRect.top();
	GLint x2 = x1 + m_panRect.width();
	GLint y2 = y1 + m_panRect.height();

	// y scale
	qreal dBmRange = qAbs(m_dBmPanMax - m_dBmPanMin);
	float yScale = m_panRect.height() / (float)dBmRange;
	float yScaleColor = 10.0f / (float)dBmRange;
	float yTop = (float) y2;
	
	if (m_dataEngineState == QSDR::DataEngineUp)
		glClear(GL_DEPTH_BUFFER_BIT);
	else
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_LINE_SMOOTH);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glLineWidth(1);

    // --- Modern OpenGL Rendering ---
    if (!m_shaderProgram || !m_shaderProgram->isLinked()) return;

    m_shaderProgram->bind();
    
    QMatrix4x4 projection;
    projection.ortho(0, size().width(), size().height(), 0, -10, 10);
    m_shaderProgram->setUniformValue("matrix", projection);

	// draw background
	if (m_dataEngineState == QSDR::DataEngineUp) {
		if (m_panGrid) {
			drawPanelGradientRect(m_panRect,
			                      QColor::fromRgbF(0.15f, 0.15f, 0.3f),
			                      QColor::fromRgbF(0.15f, 0.15f, 0.51f),
			                      false, -3.0f);
		} else {
			drawPanelGradientRect(m_panRect,
			                      QColor::fromRgbF(0.05f, 0.05f, 0.2f),
			                      QColor::fromRgbF(0.05f, 0.05f, 0.31f),
			                      false, -3.0f);
		}
	} else {
		drawPanelRect(m_panRect, QColor(30, 30, 50, 155), -3.0f);
	}

	// set a scissor box
	glScissor((int)(x1 * dpr), (int)((size().height() - y2) * dpr), (int)((x2 - x1) * dpr), (int)(height * dpr));
	glEnable(GL_SCISSOR_TEST);
    spectrumBufferMutex.lock();

    struct VertexData {
        float x, y, z;
        float r, g, b, a;
    };

    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue("matrix", projection);

	switch (m_panMode) {

		case (PanGraphicsMode) FilledLine: {
            QVarLengthArray<VertexData> data(vertexArrayLength * 2);
			for (int i = 0; i < vertexArrayLength; i++) {
                float vx = (float)(i/m_scaleMult);
                float vy = (float)(yTop - yScale * m_panadapterBins.at(i));

                data[2*i] = { vx, vy, -2.5f, m_redF, m_greenF, m_blueF, 0.4f };
                data[2*i+1] = { vx, yTop, -2.5f, 0.3f * m_redF, 0.3f * m_greenF, 0.3f * m_blueF, 0.2f };
			}
			
            m_vao.bind();
            m_vbo.bind();
            m_vbo.allocate(data.data(), (int)(data.size() * sizeof(VertexData)));
            m_shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 7);
            m_shaderProgram->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 4, sizeof(float) * 7);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 2*vertexArrayLength);

            QVarLengthArray<VertexData> lineData(vertexArrayLength);
            for (int i = 0; i < vertexArrayLength; i++) {
                float binVal = m_panadapterBins.at(i);
                lineData[i] = { (float)(i/m_scaleMult), (float)(yTop - yScale * binVal), -1.0f, 
                                m_red * (yScaleColor * binVal), m_green * (yScaleColor * binVal), m_blue * (yScaleColor * binVal), 1.0f };
            }
            m_vbo.allocate(lineData.data(), (int)(lineData.size() * sizeof(VertexData)));
            m_shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 7);
            m_shaderProgram->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 4, sizeof(float) * 7);
			glDrawArrays(GL_LINE_STRIP, 0, vertexArrayLength);
            m_vao.release();
			break;
        }

		case (PanGraphicsMode) Line: {
            QVarLengthArray<VertexData> data(vertexArrayLength);
			for (int i = 0; i < vertexArrayLength; i++) {
                float binVal = m_panadapterBins.at(i);
				data[i] = { (float)(i/m_scaleMult), (float)(yTop - yScale * binVal), -1.0f,
                            m_red * (yScaleColor * binVal), m_green * (yScaleColor * binVal), m_blue * (yScaleColor * binVal), 1.0f };
			}
		
            m_vao.bind();
            m_vbo.bind();
            m_vbo.allocate(data.data(), (int)(data.size() * sizeof(VertexData)));
            m_shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 7);
            m_shaderProgram->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 4, sizeof(float) * 7);
			glDrawArrays(GL_LINE_STRIP, 0, vertexArrayLength);
            m_vao.release();
			break;
        }

		case (PanGraphicsMode) Solid: {
			glDisable(GL_MULTISAMPLE);
			glDisable(GL_LINE_SMOOTH);

            QVarLengthArray<VertexData> data(vertexArrayLength * 2);
			for (int i = 0; i < vertexArrayLength; i++) {
                float vx = (float)(i/m_scaleMult);
                float vy = (float)(yTop - yScale * m_panadapterBins.at(i));
                data[2*i]   = { vx, vy,   -2.0f, m_redST, m_greenST, m_blueST, 1.0f };
                data[2*i+1] = { vx, yTop, -2.0f, m_redSB, m_greenSB, m_blueSB, 1.0f };
			}
			
            m_vao.bind();
            m_vbo.bind();
            m_vbo.allocate(data.data(), (int)(data.size() * sizeof(VertexData)));
            m_shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 7);
            m_shaderProgram->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 4, sizeof(float) * 7);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 2*vertexArrayLength);
            m_vao.release();
			break;
        }
	}
    spectrumBufferMutex.unlock();
    m_shaderProgram->release();
	glDisable(GL_SCISSOR_TEST);
} 

void QGLDistancePanel::drawPanVerticalScale() {

	if (!m_dBmScalePanRect.isValid()) return;

	int width = m_dBmScalePanRect.width();
	int height = m_dBmScalePanRect.height();
	if (width <= 0 || height <= 0) return;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	
	if (!m_dBmScaleFBO || m_dBmScalePanadapterUpdate || m_dBmScalePanadapterRenew)	{

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

		glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
		
		m_dBmScalePanadapterUpdate = false;
		m_dBmScalePanadapterRenew = false;
	}
		
	drawCachedTexture(m_dBmScalePanRect, m_dBmScaleFBO->texture(), 0.0f);
}

void QGLDistancePanel::drawPanHorizontalScale() {

	if (!m_freqScalePanRect.isValid()) return;

	int width = m_freqScalePanRect.width();
	int height = m_freqScalePanRect.height();
	if (width <= 0 || height <= 0) return;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	if (!m_frequencyScaleFBO || m_freqScalePanadapterUpdate || m_freqScalePanadapterRenew) {

		if (!m_frequencyScaleFBO || m_freqScalePanadapterRenew) {

			if (m_frequencyScaleFBO) {
			
				delete m_frequencyScaleFBO;
				m_frequencyScaleFBO = 0;
			}

            m_frequencyScaleFBO = new QOpenGLFramebufferObject(width, height);
		}

        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glViewport(0, 0, width, height);

		m_frequencyScaleFBO->bind();
			renderPanHorizontalScale();
		m_frequencyScaleFBO->release();

        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
		
		m_freqScalePanadapterUpdate = false;
		m_freqScalePanadapterRenew = false;
	}

	drawCachedTexture(m_freqScalePanRect, m_frequencyScaleFBO->texture(), 0.0f);
}

void QGLDistancePanel::drawPanadapterGrid() {

	if (!m_panRect.isValid()) return;

	int width = m_panRect.width();
	int height = m_panRect.height();
	if (width <= 0 || height <= 0) return;

	glDisable(GL_MULTISAMPLE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	
	if (!m_panadapterGridFBO || m_panGridUpdate || m_panGridRenew)	{

		if (!m_panadapterGridFBO || m_panGridRenew) {

			if (m_panadapterGridFBO) {
			
				delete m_panadapterGridFBO;
				m_panadapterGridFBO = 0;
			}

            m_panadapterGridFBO = new QOpenGLFramebufferObject(width, height);
		}

        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glViewport(0, 0, width, height);

		m_panadapterGridFBO->bind();
			renderPanadapterGrid();
		m_panadapterGridFBO->release();

        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
		
		m_panGridUpdate = false;
		m_panGridRenew = false;
	}

	drawCachedTexture(m_panRect, m_panadapterGridFBO->texture(), -2.0f);
	glEnable(GL_MULTISAMPLE);
}

void QGLDistancePanel::drawPanFilter() {

	qreal freqLo = m_filterLowerFrequency / m_sampleRate;
	qreal freqHi = m_filterUpperFrequency / m_sampleRate;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	QColor color = QColor(150, 150, 150, 100);

	GLint x1 = m_panRect.left() + qRound((qreal)(m_panRect.width()/2.0f) + freqLo * m_panRect.width() / m_freqScaleZoomFactor);
	GLint x2 = m_panRect.left() + qRound((qreal)(m_panRect.width()/2.0f) + freqHi * m_panRect.width() / m_freqScaleZoomFactor);
	GLint y1 = m_panRect.top() + 1;
	GLint y2 = m_panRect.top() + m_panRect.height() - 1;

	QRect filterRect = QRect(x1, y1, x2 - x1, y2 - y1);

	if ((x1 >= m_panRect.left() && x1 <= m_panRect.right()) ||
		(x2 >= m_panRect.left() && x2 <= m_panRect.right()) ||
		(x1 < m_panRect.left() && x2 > m_panRect.right()))
	{
		if (filterRect.height() > 5) {
			// Translucent fill via rgba shader vertices
			if (m_shaderProgram && m_shaderProgram->isLinked()) {
				struct VertexData { float x, y, z, r, g, b, a; };
				const float r = color.redF(), g = color.greenF(), b = color.blueF(), a = color.alphaF();
				const float z = 3.0f;
				const VertexData quad[4] = {
					{ float(filterRect.left()), float(filterRect.top()), z, r, g, b, a },
					{ float(filterRect.right() + 1), float(filterRect.top()), z, r, g, b, a },
					{ float(filterRect.left()), float(filterRect.bottom() + 1), z, r, g, b, a },
					{ float(filterRect.right() + 1), float(filterRect.bottom() + 1), z, r, g, b, a },
				};
				m_shaderProgram->bind();
				m_shaderProgram->setUniformValue("matrix", panelProjection());
				m_vao.bind();
				m_vbo.bind();
				m_vbo.allocate(quad, int(sizeof(quad)));
				m_shaderProgram->enableAttributeArray(0);
				m_shaderProgram->enableAttributeArray(1);
				m_shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 7);
				m_shaderProgram->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 4, sizeof(float) * 7);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				m_vao.release();
				m_shaderProgram->release();
			}
		}
	}

	// draw a line for the display center
	y1 = m_panRect.top() + 3;
	y2 = m_panRect.top() + (m_panRect.height() - 3);

	if (y2 > y1 + 3) {

		GLint x = m_panRect.width()/2;
		color = set->getPanadapterColors().panCenterLineColor;

		glDisable(GL_MULTISAMPLE);
		glLineWidth(1);

		const GlDraw::Vec3Rgb centerLine[2] = {
			{ float(x), float(y1), 4.0f, color.redF(), color.greenF(), color.blueF() },
			{ float(x), float(y2), 4.0f, color.redF(), color.greenF(), color.blueF() },
		};
		m_vao.bind();
		GlDraw::drawColoredLines(this, m_shaderProgram, m_vbo, panelProjection(), centerLine, 2);
		glEnable(GL_MULTISAMPLE);
	}
}

void QGLDistancePanel::drawCrossHair() {

	QRect rect(0, m_panRect.top(), width(), height() - m_panRect.top());

	int x = m_mousePos.x();
	int y = m_mousePos.y();

	glDisable(GL_MULTISAMPLE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);

    if (!m_shaderProgram || !m_shaderProgram->isLinked()) return;

	// set a scissor box
    qreal dpr = devicePixelRatioF();
	glScissor((int)(rect.left() * dpr), (int)((size().height() - rect.bottom()) * dpr), (int)((rect.width() - 1) * dpr), (int)(rect.height() * dpr));
	glEnable(GL_SCISSOR_TEST);

    m_shaderProgram->bind();
    QMatrix4x4 projection;
    projection.ortho(0, size().width(), size().height(), 0, -10, 10);
    m_shaderProgram->setUniformValue("matrix", projection);

    struct VertexData {
        float x, y, z;
        float r, g, b, a;
    };

    QList<VertexData> crossHairLines;
    float r = 1.0f, g = 1.0f, b = 1.0f; 

	// horizontal line
    crossHairLines.append({ (float)m_dBmScalePanRect.right() - 2.0f, (float)y, 4.0f, r, g, b, 0.3f });
    crossHairLines.append({ (float)rect.right() - 1.0f, (float)y, 4.0f, r, g, b, 0.3f });

	// vertical line
    crossHairLines.append({ (float)x, (float)rect.top() + 1.0f, 4.0f, r, g, b, 0.3f });
    crossHairLines.append({ (float)x, (float)rect.bottom() - 1.0f, 4.0f, r, g, b, 0.3f });

	// cross hair
    crossHairLines.append({ (float)x     , (float)y - 20.0f, 5.0f, r, g, b, 0.7f });
    crossHairLines.append({ (float)x     , (float)y + 20.0f, 5.0f, r, g, b, 0.7f });
    crossHairLines.append({ (float)x - 20.0f, (float)y, 5.0f, r, g, b, 0.7f });
    crossHairLines.append({ (float)x + 20.0f, (float)y, 5.0f, r, g, b, 0.7f });

    if (!crossHairLines.isEmpty()) {
        m_vao.bind();
        m_vbo.bind();
        m_vbo.allocate(crossHairLines.data(), crossHairLines.size() * (int)sizeof(VertexData));
        m_shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 7);
        m_shaderProgram->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 4, sizeof(float) * 7);
        glDrawArrays(GL_LINES, 0, crossHairLines.size());
        m_vao.release();
    }
    m_shaderProgram->release();

	// text only on panadapter
	QString str;
	m_glTextColor = QColor(255, 255, 255, 255);

	int dx = m_panRect.width()/2 - x;
	qreal unit = (qreal)((m_sampleRate * m_freqScaleZoomFactor) / m_panRect.width());
	qreal df = unit * dx;
	qreal frequency = m_frequency - df;
	
	str = frequencyString(-df, true);
	if (x > m_panRect.width() - 85)
		renderPanelText(m_oglTextSmall, x - 90, y - 30, 5.0f, str);
	else
		renderPanelText(m_oglTextSmall, x + 4, y - 30, 5.0f, str);

	str = frequencyString(frequency);
	if (x > m_panRect.width() - 85)
		renderPanelText(m_oglTextSmall, x - 90, y - 18, 5.0f, str);
	else
		renderPanelText(m_oglTextSmall, x + 4, y - 18, 5.0f, str);

	if (m_mouseRegion == panadapterRegion) {

		qreal dBm = m_dBmPanMax - ((m_dBmPanMax - m_dBmPanMin) * ((qreal)(y - m_panRect.top()) / m_panRect.height()));
		str = QString::number(dBm, 'f', 1) + " dBm";
		if (x > m_panRect.width() - 85)
			renderPanelText(m_oglTextSmall, x - 90, y + 6, 5.0f, str);
		else
			renderPanelText(m_oglTextSmall, x + 4, y + 6, 5.0f, str);
	}

	// disable scissor box
	glDisable(GL_SCISSOR_TEST);

	glEnable(GL_MULTISAMPLE);
}

//************
void QGLDistancePanel::drawDistanceSpectrum() {

	float localMax;

	GLint displayWidth = (GLint)m_distanceSpectrumRect.width();
	
	double distScale = 0;
	
	distScale = (double)(1.0f * qRound(m_chirpBufferLength * m_distScaleZoomFactor) / m_distanceSpectrumRect.width());
	if (distScale < 1.0) distScale = 1.0;
	
	if (distScale < 0)	{

		drawPanelRect(m_distanceSpectrumRect, Qt::black);
		return;
	}
	
	qreal dBmRange = (m_dBmDistMax - m_dBmDistMin) * m_distScaleZoomFactor;

	float yScale = m_distanceSpectrumRect.height() / dBmRange;
	float yTop = m_distanceSpectrumRect.top() + m_distanceSpectrumRect.height();
	
	int idx = 0;
	int lIdx = 0;
	int rIdx = 0;

    qreal dpr = devicePixelRatioF();
	glScissor((int)(m_distanceSpectrumRect.left() * dpr), (int)((size().height() - m_distanceSpectrumRect.bottom() - 1) * dpr), (int)(m_distanceSpectrumRect.width() * dpr), (int)(m_distanceSpectrumRect.height() * dpr));
	glEnable(GL_SCISSOR_TEST);

	GLint vertexArrayLength = displayWidth;

	glLineWidth(1);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (!m_shaderProgram || !m_shaderProgram->isLinked() || vertexArrayLength <= 0) {
		glDisable(GL_SCISSOR_TEST);
		return;
	}

	m_redD = (GLfloat)(set->getPanadapterColors().distanceLineColor.red() / 256.0);
	m_greenD = (GLfloat)(set->getPanadapterColors().distanceLineColor.green() / 256.0);
	m_blueD = (GLfloat)(set->getPanadapterColors().distanceLineColor.blue() / 256.0);

	struct VertexData {
		float x, y, z;
		float r, g, b, a;
	};

	QVarLengthArray<VertexData> data(vertexArrayLength);
	for (int i = 0; i < vertexArrayLength; i++) {
	
		lIdx = (int)floor((qreal)(i * distScale));
		rIdx = (int)floor((qreal)(i * distScale) + distScale);

		localMax = -10000.0F;
		for (int j = lIdx; j < rIdx; j++) {
			if (m_distanceSpectrumBuffer[j] > localMax) {

				localMax = m_distanceSpectrumBuffer[j];
				idx = j;
			}
		}

		data[i] = {
			float(i),
			float(yTop - yScale * (m_distanceSpectrumBuffer[idx] - m_dBmDistMin)),
			0.0f,
			m_redD, m_greenD, m_blueD, 1.0f
		};
	}

	m_shaderProgram->bind();
	m_shaderProgram->setUniformValue("matrix", panelProjection());
	m_vao.bind();
	m_vbo.bind();
	m_vbo.allocate(data.data(), (int)(data.size() * sizeof(VertexData)));
	m_shaderProgram->enableAttributeArray(0);
	m_shaderProgram->enableAttributeArray(1);
	m_shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 7);
	m_shaderProgram->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 4, sizeof(float) * 7);
	glDrawArrays(GL_LINE_STRIP, 0, vertexArrayLength);
	m_vao.release();
	m_shaderProgram->release();
	
	glDisable(GL_SCISSOR_TEST);
}

void QGLDistancePanel::drawDistHorizontalScale() {

	if (m_freqScaleDistancePanRect.isEmpty()) return;

	int rulerFontHeight;
	int rulerFontMaxWidth;
	double maxSpanDist;

	if (m_showChirpFFT) {

		rulerFontHeight = m_fonts.smallFontMetrics->tightBoundingRect(".0Hz").height();
		rulerFontMaxWidth = m_fonts.smallFontMetrics->boundingRect("000000").width();
		maxSpanDist = m_sampleRate / 4;
	}
	else {

		rulerFontHeight = m_fonts.smallFontMetrics->tightBoundingRect(".0kmM").height();
		rulerFontMaxWidth = m_fonts.smallFontMetrics->boundingRect("0000000").width();
		maxSpanDist = m_chirpBufferLength * m_kilometersPerGate;
	}

	qreal kilometerSpan = maxSpanDist * m_distScaleZoomFactor;
	
	qreal lowerDist = 0;
	qreal upperDist = 0;

	if (m_showChirpFFT)
		lowerDist = -maxSpanDist/2;
	else {

		lowerDist = m_dBmDistDelta;
		if (lowerDist < 0.0) lowerDist = 0.0;
	}
	
	upperDist = lowerDist + kilometerSpan;
	if (upperDist > maxSpanDist) {
		
		upperDist = maxSpanDist;
		lowerDist = maxSpanDist - kilometerSpan;
	}

	qreal unit = (float)(m_freqScaleDistancePanRect.width() / kilometerSpan);
	
	m_distanceScale = getXRuler(m_freqScaleDistancePanRect, rulerFontMaxWidth, unit, lowerDist, upperDist);

	drawPanelRect(m_freqScaleDistancePanRect, Qt::black);

	int		offset_X		= -1;
	int		textOffset_y	= 17;
	double	distScale		= 1;

	QRect scaledTextRect(0, textOffset_y, 1, rulerFontHeight);
	scaledTextRect.moveLeft(m_freqScaleDistancePanRect.width() - scaledTextRect.width());

	const QColor tickColor = (m_mouseRegion == freqScaleDistancePanRegion)
		? QColor::fromRgbF(0.8f, 0.92f, 0.97f)
		: QColor::fromRgbF(0.65f, 0.76f, 0.81f);

	QVector<GlDraw::Vec3Rgb> tickLines;
	int len = m_distanceScale.mainPointPositions.length();
	if (len > 0) {
		tickLines.reserve(len * 2 + m_distanceScale.subPointPositions.length() * 2);
		const float top = float(m_freqScaleDistancePanRect.top());
		for (int i = 0; i < len; i++) {
			const float x = float(m_distanceScale.mainPointPositions.at(i));
			tickLines.append({ x, top + 1.0f, 0.0f, tickColor.redF(), tickColor.greenF(), tickColor.blueF() });
			tickLines.append({ x, top + 4.0f, 0.0f, tickColor.redF(), tickColor.greenF(), tickColor.blueF() });
		}

		m_glTextColor = tickColor;
		for (int i = 0; i < len; i++) {
		
			QString str = QString::number(m_distanceScale.mainPoints.at(i) / distScale, 'f', 0);
            int textWidth = m_fonts.smallFontMetrics->horizontalAdvance(str);
			QRect textRect(m_distanceScale.mainPointPositions.at(i) + offset_X - (textWidth / 2), textOffset_y, textWidth, rulerFontHeight);

			if (textRect.left() < 0 || textRect.right() >= scaledTextRect.left()) continue;

			renderPanelText(m_oglTextSmall, float(textRect.x()), float(textRect.y()), str);
		}
	}

	len = m_distanceScale.subPointPositions.length();
	if (len > 0) {
		const float top = float(m_freqScaleDistancePanRect.top());
		for (int i = 0; i < len; i++) {
			const float x = float(m_distanceScale.subPointPositions.at(i));
			tickLines.append({ x, top + 1.0f, 0.0f, tickColor.redF(), tickColor.greenF(), tickColor.blueF() });
			tickLines.append({ x, top + 3.0f, 0.0f, tickColor.redF(), tickColor.greenF(), tickColor.blueF() });
		}
	}

	if (!tickLines.isEmpty()) {
		glLineWidth(1);
		m_vao.bind();
		GlDraw::drawColoredLines(this, m_shaderProgram, m_vbo, panelProjection(),
		                         tickLines.constData(), tickLines.size());
	}
}

void QGLDistancePanel::drawDistVerticalScale() {

	if (m_dBmScaleDistancePanRect.isEmpty()) return;

	int spacing = 5;
	int fontHeight = m_fonts.smallFontMetrics->tightBoundingRect(".0dBm").height() + spacing;
	int fontMaxWidth = m_fonts.smallFontMetrics->boundingRect("-000.0").width();

	qreal dBmRange = qAbs(m_dBmDistScaleMax - m_dBmDistScaleMin);
	qreal unit = (float)(m_dBmScaleDistancePanRect.height() / dBmRange);

	m_dBmScale = getYRuler(m_dBmScaleDistancePanRect, fontHeight, unit, m_dBmDistScaleMin, m_dBmDistScaleMax);
	
	drawPanelRect(m_dBmScaleDistancePanRect, QColor(60, 60, 60, 80));

	QRect textRect(0, 0, fontMaxWidth, fontHeight);
	textRect.moveRight(14);
	int yOld = -textRect.height();

	const QColor tickColor = (m_mouseRegion == dBmScaleDistancePanRegion)
		? QColor::fromRgbF(0.8f, 0.92f, 0.97f)
		: QColor::fromRgbF(0.65f, 0.76f, 0.81f);
	const QColor subColor = (m_mouseRegion == dBmScaleDistancePanRegion)
		? QColor::fromRgbF(0.5f, 0.62f, 0.67f)
		: QColor::fromRgbF(0.35f, 0.46f, 0.51f);

	QVector<GlDraw::Vec3Rgb> tickLines;
	int len = m_dBmScale.mainPointPositions.length();
	
	if (len > 0) {
		const float left = float(m_dBmScaleDistancePanRect.left());
		for (int i = 0; i < len; i++) {
			const float y = float(m_dBmScale.mainPointPositions.at(i));
			tickLines.append({ left,     y, 0.0f, tickColor.redF(), tickColor.greenF(), tickColor.blueF() });
			tickLines.append({ left + 4, y, 0.0f, tickColor.redF(), tickColor.greenF(), tickColor.blueF() });
		}

		m_glTextColor = tickColor;
		for (int i = 0; i < len; i++) {

			textRect.moveTop(m_dBmScale.mainPointPositions.at(i) + textRect.height()/3);

			if (textRect.y() >= yOld && 
				textRect.bottom() <= (m_dBmScaleDistancePanRect.top() + m_dBmScaleDistancePanRect.height() - textRect.height()) &&
				m_dBmScale.mainPointPositions.at(i) > 10 + m_dBmScaleDistancePanRect.top())
			{
				QString str = QString::number(m_dBmScale.mainPoints.at(i), 'f', 1);
				renderPanelText(m_oglTextSmall,
				                float(textRect.right() + m_dBmScaleDistancePanRect.left()),
				                float(textRect.y()), str);
				yOld = textRect.bottom();
			}
		
			if (qRound(m_dBmScale.mainPoints.at(i)) == 0 && m_showZerodBmLine) {
		
				int zerodBmLine = m_dBmScale.mainPointPositions.at(i);
				if (zerodBmLine > m_dBmScaleDistancePanRect.top() && zerodBmLine < m_dBmScaleDistancePanRect.bottom()) {
					const float zy = float(zerodBmLine);
					tickLines.append({ float(m_distanceSpectrumRect.left()), zy, 0.0f, 0.2f, 0.87f, 0.87f });
					tickLines.append({ float(m_distanceSpectrumRect.width() - m_dBmScaleDistancePanRect.width() + 4), zy, 0.0f, 0.2f, 0.87f, 0.87f });
				}
			}
		}
	}

	if (m_dBmScale.subPointPositions.length() > 0) {
		const float left = float(m_dBmScaleDistancePanRect.left());
		for (int i = 0; i < m_dBmScale.subPointPositions.length(); i++) {
			const float y = float(m_dBmScale.subPointPositions.at(i));
			tickLines.append({ left,     y, 0.0f, subColor.redF(), subColor.greenF(), subColor.blueF() });
			tickLines.append({ left + 2, y, 0.0f, subColor.redF(), subColor.greenF(), subColor.blueF() });
		}
	}

	if (!tickLines.isEmpty()) {
		glLineWidth(1);
		m_vao.bind();
		GlDraw::drawColoredLines(this, m_shaderProgram, m_vbo, panelProjection(),
		                         tickLines.constData(), tickLines.size());
	}
}

//**********************************************************************************************
// The algorithms of the scale functions renderPanVerticalScale() and renderPanHorizontalScale() 
// are taken from SDRMAXIII (c) Catherine Moss, with permission.

void QGLDistancePanel::renderPanVerticalScale() {

	QString str;
	int spacing = 7;
	int fontHeight = m_fonts.smallFontMetrics->tightBoundingRect(".0dBm").height() + spacing;
    int fontMaxWidth= m_fonts.smallFontMetrics->boundingRect("-000.0").width();

	GLint width = m_dBmScalePanRect.width();
	GLint height = m_dBmScalePanRect.height();

	qreal unit = (qreal)(m_dBmScalePanRect.height() / qAbs(m_dBmPanMax - m_dBmPanMin));

	m_dBmScale = getYRuler2(m_dBmScalePanRect, fontHeight, unit, m_dBmPanMin, m_dBmPanMax);

	QOpenGLPaintDevice paintDevice(m_dBmScaleFBO->size());
	painter.begin(&paintDevice);

    QRect textRect(0, 0, fontMaxWidth, fontHeight);
	textRect.moveLeft(3);
	int yOld = -textRect.height();

	int len		= m_dBmScale.mainPointPositions.length();
	int sublen	= m_dBmScale.subPointPositions.length();

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

	painter.setPen(QPen(QColor(191, 219, 232)));
	painter.setFont(m_oglTextSmall->font());
	for (int i = 0; i < len; i++) {
		textRect.moveBottom(m_dBmScale.mainPointPositions.at(i) + textRect.height()/2);
		if (textRect.y() >= yOld && textRect.bottom() <= (m_dBmScalePanRect.height() - textRect.height())) {
			str = QString::number(m_dBmScale.mainPoints.at(i), 'f', 1);
			painter.drawText(textRect.x() + fontMaxWidth - m_fonts.smallFontMetrics->tightBoundingRect(str).width(),
			                 textRect.y() + fontHeight, str);
			yOld = textRect.bottom();
		}
	}

	textRect.moveTop(m_dBmScalePanRect.height() - textRect.height());
	painter.setPen(QPen(QColor(239, 56, 109)));
	str = QString("dBm");
	painter.drawText(textRect.x(), textRect.y() + fontHeight, str);
	painter.end();
}

void QGLDistancePanel::renderPanHorizontalScale() {

	int fontHeight = m_fonts.smallFontMetrics->tightBoundingRect(".0kMGHz").height();
	int fontMaxWidth = m_fonts.smallFontMetrics->boundingRect("000.000.0").width();

	qreal freqSpan = (qreal)(m_sampleRate * m_freqScaleZoomFactor);
	qreal lowerFreq = (qreal)m_frequency - freqSpan / 2;
	qreal upperFreq = (qreal)m_frequency + freqSpan / 2;
	qreal unit = (qreal)(m_freqScalePanRect.width() / freqSpan);

	m_frequencyScale = getXRuler(m_freqScalePanRect, fontMaxWidth, unit, lowerFreq, upperFreq);

	int		offset_X		= -1;
	int		textOffset_y	= 5;
	double	freqScale		= 1;

	QString fstr = QString(" Hz ");
	if (upperFreq >= 1e6) { freqScale = 1e6; fstr = QString("  MHz "); }
	else
	if (upperFreq >= 1e3) { freqScale = 1e3; fstr = QString("  kHz "); }

	QOpenGLPaintDevice paintDevice(m_frequencyScaleFBO->size());
	painter.begin(&paintDevice);

	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.fillRect(0, 0, m_freqScalePanRect.width(), m_freqScalePanRect.height(), QColor(0, 0, 0, 255));
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

	QRect scaledTextRect(0, textOffset_y, 1, m_freqScalePanRect.height());
    scaledTextRect.setWidth(m_fonts.smallFontMetrics->horizontalAdvance(fstr));
	scaledTextRect.moveLeft(m_freqScalePanRect.width() - scaledTextRect.width());

	painter.setPen(QPen(QColor(166, 194, 206), 3, Qt::SolidLine, Qt::FlatCap));
	int len = m_frequencyScale.mainPointPositions.length();
	if (len > 0) {
		for (int i = 0; i < len; i++) {
			const int x = m_frequencyScale.mainPointPositions.at(i);
			painter.drawLine(x, 1, x, 4);
		}

		painter.setPen(QPen(QColor(166, 194, 206)));
		painter.setFont(m_oglTextSmall->font());
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

            int text_width = m_fonts.smallFontMetrics->horizontalAdvance(str);
			QRect textRect(m_frequencyScale.mainPointPositions.at(i) + offset_X - (text_width / 2), textOffset_y, text_width, fontHeight);

			if (textRect.left() < 0 || textRect.right() >= scaledTextRect.left()) continue;
			
			painter.drawText(textRect.x(), textRect.y() + fontHeight, str);
		}
	}

	if (m_frequencyScale.subPointPositions.length() > 0) {
		painter.setPen(QPen(QColor(166, 194, 206), 1, Qt::SolidLine, Qt::FlatCap));
		for (int i = 0; i < m_frequencyScale.subPointPositions.length(); i++) {
			const int x = m_frequencyScale.subPointPositions.at(i);
			painter.drawLine(x, 1, x, 3);
		}
	}

	painter.setPen(QPen(QColor(239, 56, 109)));
	painter.setFont(m_oglTextSmall->font());
	painter.drawText(m_freqScalePanRect.width() - 30, textOffset_y + fontHeight, fstr);
	painter.end();
}

void QGLDistancePanel::renderPanadapterGrid() {

	glClear(GL_COLOR_BUFFER_BIT);
    if (!m_shaderProgram || !m_shaderProgram->isLinked()) return;

	glLineWidth(1.0f);

    m_shaderProgram->bind();
    QMatrix4x4 projection;
    projection.ortho(0, m_panRect.width(), m_panRect.height(), 0, -10, 10);
    m_shaderProgram->setUniformValue("matrix", projection);

    struct VertexData {
        float x, y, z;
        float r, g, b, a;
    };

    QList<VertexData> gridLines;
    float r = 0.45f, g = 0.56f, b = 0.61f;
    float a = 0.5f;

	// vertical lines
	int len = m_frequencyScale.mainPointPositions.length();
	if (len > 0) {
		GLint x1 = m_panRect.left();
		GLint x2 = 1;
		if (m_dBmScalePanRect.isValid()) x2 += m_dBmScalePanRect.width();

		float y1 = 1.0f;
		float y2 = (float)m_panRect.bottom() - 1.0f;

		for (int i = 0; i < len; i++) {
			GLint x = m_frequencyScale.mainPointPositions.at(i);
			if (x < x2) continue;
			float vx = (float)(x + x1);
            gridLines.append({ vx, y1, 0.0f, r, g, b, a });
            gridLines.append({ vx, y2, 0.0f, r, g, b, a });
		}
	}

	// horizontal lines
	len = m_dBmScale.mainPointPositions.length();
	if (len > 0) {
		float vx1 = (float)(m_panRect.left() + m_dBmScalePanRect.width());
		float vx2 = (float)m_panRect.right();
		
		for (int i = 0; i < len; i++) {
			float vy = (float)m_dBmScale.mainPointPositions.at(i);
            gridLines.append({ vx1, vy, 0.0f, r, g, b, a });
            gridLines.append({ vx2, vy, 0.0f, r, g, b, a });
		}
	}

    if (!gridLines.isEmpty()) {
        m_vao.bind();
        m_vbo.bind();
        m_vbo.allocate(gridLines.data(), gridLines.size() * (int)sizeof(VertexData));
        m_shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 7);
        m_shaderProgram->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 4, sizeof(float) * 7);
        glDrawArrays(GL_LINES, 0, gridLines.size());
        m_vao.release();
    }

    m_shaderProgram->release();
}
 

//********************************************************************

void QGLDistancePanel::getRegion(QPoint p) {

	QRect mouse_rect(0, 0, 100, 100);
	mouse_rect.moveCenter(p);

	if (m_freqScalePanRect.contains(p)) {

		m_mouseRegion = freqScalePanadapterRegion;
		
		if (m_displayTime.elapsed() >= 50) {
			
			m_displayTime.restart();
		}
	}
	else if (m_dBmScalePanRect.contains(p)) {

		m_mouseRegion = dBmScalePanadapterRegion;

		if (m_displayTime.elapsed() >= 50) {
			
			m_displayTime.restart();
		}
	}
	else if (abs(p.x() - m_filterRect.left()) < m_snapMouse &&
			 m_panRect.contains(p)
	) {
		m_mouseRegion = filterRegionLow;
		m_mouseDownFilterFrequencyLo = m_filterLowerFrequency;
	}

	else if (abs(p.x() - m_filterRect.right()) < m_snapMouse &&
			 m_panRect.contains(p)
	) {
		m_mouseRegion = filterRegionHigh;
		m_mouseDownFilterFrequencyHi = m_filterUpperFrequency;
	}

	else if (m_panRect.contains(p)) {

		m_mouseRegion = panadapterRegion;

		if (m_displayTime.elapsed() >= 50) {
			
			m_displayTime.restart();
		}
	}
	
	else if (m_freqScaleDistancePanRect.contains(p)) {		
		
		m_mouseRegion = freqScaleDistancePanRegion;
		m_distRulerUpdate = true;

		if (m_displayTime.elapsed() >= 50) {
			
			m_displayTime.restart();
		}
	}

	else if (m_dBmScaleDistancePanRect.contains(p)) {

		m_mouseRegion = dBmScaleDistancePanRegion;

		if (m_displayTime.elapsed() >= 50) {
			
			m_displayTime.restart();
		}
	}
	else if (m_distanceSpectrumRect.contains(p)) {

		m_mouseRegion = distancePanRegion;

		if (m_displayTime.elapsed() >= 50) {
			
			m_displayTime.restart();
		}
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

void QGLDistancePanel::resizeGL(int iWidth, int iHeight) {

	int width = (int)(iWidth/2) * 2;
	int height = iHeight;

	if (width != m_oldWidth) {

		m_freqScalePanadapterRenew = true;
		m_panGridRenew = true;

		m_oldWidth = width;
	}

	m_spectrumVertexColorUpdate = true;
	//m_displayData.size = QSize(width, height);

	glFinish();

	m_resizeTime.restart();
	setupDisplayRegions(QSize(width, height));
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
}

void QGLDistancePanel::setupDisplayRegions(QSize size) {

	m_displayTop = 0;
	int freqScaleRectHeight = 20;
	int dBmScaleWidth = 45;
	int distScaleRectHeight = 20;

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
	}

	m_oldPanRectHeight = m_panRect.height();
	m_panRectWidth = (GLint)m_panRect.width();
	
	m_freqScaleDistancePanRect = QRect(
									0,
									size.height() - m_displayTop - distScaleRectHeight + 1, 
									size.width(),
									distScaleRectHeight);

	m_distanceSpectrumRect = QRect(
								m_freqScalePanRect.left(),
								m_freqScalePanRect.bottom(),
								m_freqScalePanRect.width(),
								size.height() - m_displayTop - m_freqScalePanRect.bottom() - distScaleRectHeight + 1);

	m_distancePanRect = QRect(
							m_freqScalePanRect.left(),
							m_freqScalePanRect.bottom(),
							m_freqScalePanRect.width(),
							size.height() - m_displayTop - m_freqScalePanRect.bottom() - distScaleRectHeight + 1);
					
	m_dBmScaleDistancePanRect = QRect(
									m_distanceSpectrumRect.right() - dBmScaleWidth, 
									m_freqScalePanRect.bottom(), 
									dBmScaleWidth, 
									size.height() - m_displayTop - m_freqScalePanRect.bottom() - distScaleRectHeight + 1);

	m_dBmScalePanRect = QRect(
						m_panRect.left(), 
						m_displayTop, 
						45, 
						m_panRect.height());

	m_freqScalePanadapterUpdate = true;
	m_dBmScalePanadapterUpdate = true;
	m_panGridUpdate = true;
	
	//***************************************************************************
	/*GRAPHICS_DEBUG << "***************************************************************************";
	GRAPHICS_DEBUG << "total size" << size.height();
	GRAPHICS_DEBUG << "sizes (top, bottom, height):";
	GRAPHICS_DEBUG << "panRect: " << m_panRect.top() << " " << m_panRect.bottom() << " " << m_panRect.height();
	GRAPHICS_DEBUG << "freqScalePanRect: " << m_freqScalePanRect.top() << " " << m_freqScalePanRect.bottom() << " " << m_freqScalePanRect.height();
	GRAPHICS_DEBUG << "dBmScalePanRect: " << m_dBmScalePanRect.top() << " " << m_dBmScalePanRect.bottom() << " " << m_dBmScalePanRect.height();
	GRAPHICS_DEBUG << "m_distanceSpectrumRect: " << m_distanceSpectrumRect.top() << " " << m_distanceSpectrumRect.bottom() << " " << m_distanceSpectrumRect.height();
	GRAPHICS_DEBUG << "m_freqScaleDistancePanRect: " << m_freqScaleDistancePanRect.top() << " " << m_freqScaleDistancePanRect.bottom() << " " << m_freqScaleDistancePanRect.height();
	GRAPHICS_DEBUG << "m_dBmScaleDistancePanRect: " << m_dBmScaleDistancePanRect.top() << " " << m_dBmScaleDistancePanRect.bottom() << " " << m_dBmScaleDistancePanRect.height();
	GRAPHICS_DEBUG << "";*/
}

//********************************************************************
// HMI control
 
void QGLDistancePanel::enterEvent(QEnterEvent *event) {

	setFocus(Qt::MouseFocusReason);

	m_mousePos = QPoint(-1, -1);
	m_mouseRegion = elsewhere;

	QOpenGLWidget::enterEvent(event);
}

void QGLDistancePanel::leaveEvent(QEnterEvent *event) {

	m_mousePos = QPoint(-1, -1);
	m_mouseRegion = elsewhere;

	QOpenGLWidget::leaveEvent(event);
}

void QGLDistancePanel::wheelEvent(QWheelEvent* event) {
	
	//GRAPHICS_DEBUG << "wheelEvent";
    QPoint pos = event->angleDelta();

	if (event->buttons() == Qt::NoButton) getRegion(pos);

	double freqStep = set->getMouseWheelFreqStep(0);

	switch (m_mouseRegion) {

		case panadapterRegion:

			double delta = 0;
            if (event->angleDelta().rx() < 0) delta = -freqStep;
			else
            if (event->angleDelta().rx() > 0) delta =  freqStep;

			if (m_frequency + delta > set->getMaxFrequency())
				m_frequency = set->getMaxFrequency();
			else
			if (m_frequency + delta < 0)
				m_frequency = 0;
			else
				// snap to the frequency step
				m_frequency = (long)(qRound((m_frequency + delta) / qAbs(freqStep)) * qAbs(freqStep));

			//set->setFrequency(true, 0, m_frequency);
			set->setVFOFrequency(0, 0, m_frequency);

			/*if (m_spectrumAveragingOld) {

				m_spectrumAveragingOld = true;
				m_spectrumAveraging = false;

				freqChangeTimer.restart();
			}*/
			break;
	}

}

void QGLDistancePanel::mousePressEvent(QMouseEvent* event) {
	
	//GRAPHICS_DEBUG << "mousePressEvent";
	m_mousePos = event->pos();
	m_mouseDownPos = m_mousePos;

	getRegion(m_mousePos);

	if (m_mouseRegion == panadapterRegion) {

		if (event->buttons() == Qt::RightButton) {

			if (m_crossHairCursor) {

				m_crossHairCursor = false;
				setCursor(Qt::ArrowCursor);
			}
			else {

				m_crossHairCursor = true;
				setCursor(Qt::BlankCursor);
			}	
		}
		else if (event->buttons() == Qt::LeftButton && m_crossHairCursor) {

			double freqStep = set->getMouseWheelFreqStep(0);
			int dx = m_panRect.width()/2 - m_mousePos.x();
			qreal unit = (qreal)((m_sampleRate * m_freqScaleZoomFactor) / m_panRect.width());
			m_frequency -= unit * dx;

			m_frequency = (long)(qRound(m_frequency / qAbs(freqStep)) * qAbs(freqStep));
				
			//set->setFrequency(true, 0, m_frequency);
			set->setVFOFrequency(0, 0, m_frequency);
		}
	}
	else if (m_mouseRegion == freqScalePanadapterRegion) {

		m_rulerMouseDownPos = m_freqScalePanRect.topLeft();
		
		if (event->buttons() == Qt::RightButton) setCursor(Qt::SplitHCursor);
		return;
	}
	else if (m_mouseRegion == freqScaleDistancePanRegion) {

		m_rulerMouseDownPos = m_freqScaleDistancePanRect.topLeft();
		
		if (event->buttons() == Qt::RightButton) setCursor(Qt::SplitHCursor);
		return;
	}
	else if (m_mouseRegion == dBmScalePanadapterRegion) {

		m_rulerMouseDownPos = m_dBmScaleDistancePanRect.topLeft();

		if (event->buttons() == Qt::RightButton) setCursor(Qt::SplitVCursor);
		return;
	}
	else if (m_mouseRegion == dBmScaleDistancePanRegion) {

		m_rulerMouseDownPos = m_dBmScaleDistancePanRect.topLeft();

		if (event->buttons() == Qt::RightButton) setCursor(Qt::SplitVCursor);
		return;
	}
}

void QGLDistancePanel::mouseReleaseEvent(QMouseEvent *event) {

	//GRAPHICS_DEBUG << "mouseReleaseEvent";
	m_mousePos = event->pos();
	m_mouseDownPos = m_mousePos;

	getRegion(m_mousePos);

	if (m_mouseRegion == freqScalePanadapterRegion) {

		return;
	}
	else if (m_mouseRegion == freqScaleDistancePanRegion) {

		return;
	}
	else if (m_mouseRegion == dBmScaleDistancePanRegion) {

		if (event->button() == Qt::LeftButton && m_showZerodBmLine) {

			m_showZerodBmLine = false;
		}
		else if (event->button() == Qt::RightButton) {
		}
		return;
	}
}

void QGLDistancePanel::mouseMoveEvent(QMouseEvent* event) {
	
	//GRAPHICS_DEBUG << "mouseMoveEvent";
	QPoint pos = event->pos();
	m_mousePos = event->pos();

	if (event->buttons() == Qt::NoButton) getRegion(pos);

	switch (m_mouseRegion) {

		case distancePanRegion:
			//GRAPHICS_DEBUG << "distancePanRegion";
			setCursor(Qt::ArrowCursor);
			break;

		case panadapterRegion:
			//GRAPHICS_DEBUG << "panadapterRegion Rx:" << m_receiver;
			if (m_crossHairCursor)
				setCursor(Qt::BlankCursor);
			else
				setCursor(Qt::ArrowCursor);

			if (event->buttons() == Qt::LeftButton) {

				QPoint dPos = m_mouseDownPos - pos;
				
				qreal unit = (qreal)((m_sampleRate * m_freqScaleZoomFactor) / m_freqScalePanRect.width());
				qreal deltaFreq = unit * dPos.x();
				
				long newFrequency = m_frequency + deltaFreq;
				if (newFrequency > set->getMaxFrequency())
					newFrequency = set->getMaxFrequency();
				else
				if (newFrequency + deltaFreq < 0)
					newFrequency = 0;
				else
					m_frequency += deltaFreq;

				//set->setFrequency(true, 0, m_frequency);
				set->setVFOFrequency(0, 0, m_frequency);
				m_mouseDownPos = pos;
			}
			break;

		case dBmScalePanadapterRegion:
			//GRAPHICS_DEBUG << "dBmScalePanadapterRegion";
			if (event->buttons() == Qt::LeftButton) {

				QPoint dPos = m_mouseDownPos - pos;
				qreal unit = (qreal)(qAbs(m_dBmPanMax - m_dBmPanMin) / m_panRect.height());
				
				qreal newMin = m_dBmPanMin - unit * dPos.y();
				qreal newMax = m_dBmPanMax - unit * dPos.y();

				if (newMin > MINDBM && newMax < MAXDBM) {

					m_dBmPanMin = newMin;
					m_dBmPanMax = newMax;

					set->setdBmPanScaleMin(0, m_dBmPanMin);
					set->setdBmPanScaleMax(0, m_dBmPanMax);
				}
				
				m_mouseDownPos = pos;
				m_dBmScalePanadapterUpdate = true;
				m_panGridUpdate = true;
			}
			else
			if (event->buttons() == Qt::RightButton) {

				QPoint dPos = m_mouseDownPos - pos;
				if (dPos.y() > 0)
					m_dBmPanDelta = 1.0;
				else if (dPos.y() < 0)
					m_dBmPanDelta = -1.0f;
				
				m_dBmPanMin += m_dBmPanDelta;
				m_dBmPanMax -= m_dBmPanDelta;

				if (qAbs(m_dBmPanMax - m_dBmPanMin) < 10) {

					m_dBmPanMin -= m_dBmPanDelta;
					m_dBmPanMax += m_dBmPanDelta;
				}
				if (m_dBmPanMin < MINDBM) m_dBmPanMin = MINDBM;
				if (m_dBmPanMax > MAXDBM) m_dBmPanMax = MAXDBM;

				set->setdBmPanScaleMin(0, m_dBmPanMin);
				set->setdBmPanScaleMax(0, m_dBmPanMax);

				m_mouseDownPos = pos;
				m_dBmScalePanadapterUpdate = true;
				m_panGridUpdate = true;
			}
			else
				setCursor(Qt::ArrowCursor);
			break;

		case dBmScaleDistancePanRegion:
			//GRAPHICS_DEBUG << "dBmScaleDistancePanRegion";
			if (event->buttons() == Qt::LeftButton &&
				event->modifiers() == Qt::ControlModifier) {

				m_showZerodBmLine = true;
				QPoint dPos = m_mouseDownPos - pos;

				qreal unit = (qreal)(qAbs(m_dBmDistScaleMax - m_dBmDistScaleMin) / m_distanceSpectrumRect.height());
				
				qreal newMin = m_dBmDistScaleMin - unit * dPos.y();
				qreal newMax = m_dBmDistScaleMax - unit * dPos.y();

				if (newMin > MINDISTDBM && newMax < MAXDISTDBM) {

					m_dBmDistScaleMin = newMin;
					m_dBmDistScaleMax = newMax;
				}
				m_mouseDownPos = pos;
			}
			else
			if (event->buttons() == Qt::LeftButton) {

				QPoint dPos = m_mouseDownPos - pos;

				qreal unit = (qreal)(qAbs(m_dBmDistMax - m_dBmDistMin) / m_distanceSpectrumRect.height());
				
				qreal newMin = m_dBmDistMin - unit * dPos.y();
				qreal newMax = m_dBmDistMax - unit * dPos.y();

				if (newMin > MINDISTDBM && newMax < MAXDISTDBM) {

					m_dBmDistMin = newMin;
					m_dBmDistMax = newMax;
				}
				m_mouseDownPos = pos;

				m_dBmDistScaleMin = m_dBmDistMin;
				m_dBmDistScaleMax = m_dBmDistMax;
			}
			else
			if (event->buttons() == Qt::RightButton) {

				QPoint dPos = m_mouseDownPos - pos;
				if (dPos.y() > 0)
					m_dBmDistDelta = 1.0;
				else if (dPos.y() < 0)
					m_dBmDistDelta = -1.0f;
				
				m_dBmDistMin += m_dBmDistDelta;
				m_dBmDistMax -= m_dBmDistDelta;

				if (qAbs(m_dBmDistMax - m_dBmDistMin) < 10) {

					m_dBmDistMin -= m_dBmDistDelta;
					m_dBmDistMax += m_dBmDistDelta;
				}
				if (m_dBmDistMin < MINDISTDBM) m_dBmDistMin = MINDISTDBM;
				if (m_dBmDistMax > MAXDISTDBM)  m_dBmDistMax =  MAXDISTDBM;

				m_dBmDistScaleMin = m_dBmDistMin;
				m_dBmDistScaleMax = m_dBmDistMax;
				
				m_mouseDownPos = pos;
			}
			else
				setCursor(Qt::ArrowCursor);
			break;

		case freqScalePanadapterRegion:
			//GRAPHICS_DEBUG << "freqScalePanadapterRegion Rx" << m_receiver;
			if (event->buttons() == Qt::LeftButton) {
				
				QPoint dPos = m_mouseDownPos - pos;
				int bottom_y = height() - m_freqScalePanRect.height();
				int new_y = m_rulerMouseDownPos.y() - dPos.y();
				
				if (new_y < m_panRect.top() + m_panSpectrumMinimumHeight) 
					new_y = m_panRect.top() + m_panSpectrumMinimumHeight;
				if (new_y > bottom_y) 
					new_y = bottom_y;
				
				m_freqRulerPosition = (float)(new_y - m_panRect.top()) / (bottom_y - m_panRect.top());
				set->setFreqRulerPosition(0, m_freqRulerPosition);
			}
			else
			if (event->buttons() == Qt::RightButton) {

				QPoint dPos = m_mouseDownPos - pos;
				if (dPos.x() > 0)
					m_freqScaleZoomFactor += 0.01;
				else if (dPos.x() < 0)
					m_freqScaleZoomFactor -= 0.01;

				if (m_freqScaleZoomFactor > 1.0) m_freqScaleZoomFactor = 1.0;
				//if (m_freqScaleZoomFactor < 0.05) m_freqScaleZoomFactor = 0.05;
				if (m_freqScaleZoomFactor < 0.15) m_freqScaleZoomFactor = 0.15;

				m_mouseDownPos = pos;
				m_freqScalePanadapterUpdate = true;
				m_panGridUpdate = true;
			}
			else
				setCursor(Qt::ArrowCursor);
			break;

		case freqScaleDistancePanRegion:
			//GRAPHICS_DEBUG << "freqScaleDistancePanRegion";
			if (event->buttons() == Qt::LeftButton) {

				if (m_distScaleZoomFactor < 1.0) {
					QPoint dPos = m_mouseDownPos - pos;
					if (dPos.x() > 0)
						m_distRulerDisplayDelta += m_distRulerDisplayDeltaStep;
					else if (dPos.x() < 0)
						m_distRulerDisplayDelta -= m_distRulerDisplayDeltaStep;

					if (m_distRulerDisplayDelta < 0)
						m_distRulerDisplayDelta = 0.0;

					if (m_distRulerDisplayDelta > m_chirpBufferLength * m_kilometersPerGate)
						m_distRulerDisplayDelta -= m_distRulerDisplayDeltaStep;

					m_mouseDownPos = pos;
				}
				else
					m_distRulerDisplayDelta = 0.0;
			}
			else if (event->buttons() == Qt::RightButton) {
				
				QPoint dPos = m_mouseDownPos - pos;
				if (dPos.x() > 0)
					m_distScaleZoomFactor += 0.005;
				else if (dPos.x() < 0)
					m_distScaleZoomFactor -= 0.005;

				if (m_distScaleZoomFactor > 1.0) m_distScaleZoomFactor = 1.0;
				if (m_distScaleZoomFactor < 0.1) m_distScaleZoomFactor = 0.1;

				m_mouseDownPos = pos;
			}
			else
				setCursor(Qt::ArrowCursor);
			break;

		case filterRegionLow:

			setCursor(Qt::SizeHorCursor);
			if (event->buttons() == Qt::LeftButton) {

				QPoint dPos = m_mouseDownPos - pos;
				qreal dFreq = (qreal)(dPos.x() * m_sampleRate * m_freqScaleZoomFactor) / m_panRect.width();

				m_filterLowerFrequency = qRound(m_mouseDownFilterFrequencyLo - dFreq);
				set->setRXFilter(0, m_filterLowerFrequency, m_filterUpperFrequency);
			}
			break;

		case filterRegionHigh:

			setCursor(Qt::SizeHorCursor);
			if (event->buttons() == Qt::LeftButton) {

				QPoint dPos = m_mouseDownPos - pos;
				qreal dFreq = (qreal)(dPos.x() * m_sampleRate * m_freqScaleZoomFactor) / m_panRect.width();

				m_filterUpperFrequency = qRound(m_mouseDownFilterFrequencyHi - dFreq);
				set->setRXFilter(0, m_filterLowerFrequency, m_filterUpperFrequency);
			}
			break;

		case elsewhere:
			//GRAPHICS_DEBUG << "elsewhere";
			
			break;
	}

	// Idle mouse moves must not force NoPartialUpdate clears — spectrum frames
	// already redraw overlays. Only refresh while a button drag is active.
	if (event->buttons() != Qt::NoButton && m_displayTime.elapsed() >= 33) {
		m_displayTime.restart();
		update();
	}
}

void QGLDistancePanel::keyPressEvent(QKeyEvent* event) {
	
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
}

void QGLDistancePanel::timerEvent(QTimerEvent *) {

}
 
//********************************************************************
 
void QGLDistancePanel::setFrequency(bool value, qint64 freq) {

	Q_UNUSED(value)
	
	m_frequency = freq;
	//m_displayData.frequency = freq;
	m_freqScalePanadapterUpdate = true;
	m_panGridUpdate = true;

}

void QGLDistancePanel::setFilterFrequencies(int rx, qreal lo, qreal hi) {

	if (rx == 0) {
		
		m_filterLowerFrequency = lo;
		m_filterUpperFrequency = hi;
	}
}

void QGLDistancePanel::freqRulerPositionChanged(float pos, int rx) {

	if (rx == 0) {
		
		m_freqRulerPosition = pos;

		setupDisplayRegions(size());
	}
}

void QGLDistancePanel::setSpectrumBuffer(const float *buffer) {

	if (m_spectrumAveraging) {
	
		QVector<float>	m_specBuf(SAMPLE_BUFFER_SIZE);

		//spectrumBufferMutex.lock();

		memcpy(
			(float *) m_specBuf.data(),
			(float *) &buffer[0],
			SAMPLE_BUFFER_SIZE * sizeof(float));

		specAv_queue.enqueue(m_specBuf);
		if (specAv_queue.size() <= m_specAveragingCnt) {
	
			for (int i = 0; i < SAMPLE_BUFFER_SIZE; i++)
				m_tmpBuf[i] += specAv_queue.last().data()[i];

			//spectrumBufferMutex.unlock();
			return;
		}
	
		for (int i = 0; i < SAMPLE_BUFFER_SIZE; i++) {

				m_tmpBuf[i] -= specAv_queue.first().at(i);
				m_tmpBuf[i] += specAv_queue.last().at(i);
				m_avgBuf[i] = m_tmpBuf[i] * m_scale;
		}

		computeDisplayBins(m_avgBuf);
		specAv_queue.dequeue();
	
		//spectrumBufferMutex.unlock();
	}
	else
		computeDisplayBins(buffer);
}

void QGLDistancePanel::computeDisplayBins(const float *panBuffer) {

	int newSampleSize = 0;
	int deltaSampleSize = 0;
	int idx = 0;
	int lIdx = 0;
	int rIdx = 0;
	qreal localMax;
	newSampleSize = (int)floor(4 * BUFFER_SIZE * m_freqScaleZoomFactor);
	deltaSampleSize = 4 * BUFFER_SIZE - newSampleSize;

	/*if (deltaSampleSize%2 != 0) {
		deltaSampleSize += 1;
		newSampleSize -= 1;
	}*/

	m_panScale = (qreal)(1.0 * newSampleSize / m_panRectWidth);
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

	/*if (bins != binsOld) {

		GRAPHICS_DEBUG << "newSampleSize" << newSampleSize;
		GRAPHICS_DEBUG << "m_panScale" << m_panScale;
		GRAPHICS_DEBUG << "bins:" << bins;
	}*/

	m_panadapterBins.clear();
	
	for (int i = 0; i < m_panSpectrumBinsLength; i++) {
			
		idx = 0;
		lIdx = (int)floor((qreal)(i * m_panScale / m_scaleMult));
		rIdx = (int)floor((qreal)(i * m_panScale / m_scaleMult) + m_panScale / m_scaleMult);
					
		// max value; later we try mean value also!
		localMax = -10000.0F;
		for (int j = lIdx; j < rIdx; j++) {

			if (panBuffer[j] > localMax) {

				localMax = panBuffer[j];
				idx = j;
			}
		}
		idx += deltaSampleSize/2;
				
		m_panadapterBins << panBuffer[idx] - m_dBmPanMin - m_dBmPanLogGain;
	}
}

void QGLDistancePanel::setDistanceSpectrumBuffer(int sampleRate, qint64 length, const float *buffer) {

	Q_UNUSED(sampleRate)
	Q_UNUSED(length)
	Q_UNUSED(buffer)
}

void QGLDistancePanel::distanceSpectrumBufferChanged(int sampleRate, qint64 length, const float *buffer) {

	Q_UNUSED(sampleRate)

	distanceSpectrumBufferMutex.lock();
		m_kilometersPerGate = 1.0f * 3E5 / sampleRate;
		//m_kilometersPerGate = 0.5f * 3E5 / 12000;
		m_chirpBufferLength = length;
		memcpy(m_distanceSpectrumBuffer, buffer, m_chirpBufferLength * sizeof(float));
	distanceSpectrumBufferMutex.unlock();
}

// get waterfall colors - taken from PowerSDR/KISS Konsole
//QColor QGLDistancePanel::getWaterfallColorAtPixel(qreal value) {
//
//	QColor color;
//	//int r = 0; int g = 0; int b = 0;
//	int r, g, b;
//	int lowerThreshold = (int)m_dBmPanMin - m_waterfallOffsetLo;
//	int upperThreshold = (int)m_dBmPanMax + m_waterfallOffsetHi;
//
//	float offset;
//	float globalRange;
//	float localRange;
//	float percent;
//	
//	switch (m_waterColorScheme) {
//
//		case QSDRGraphics::simple:
//
//			if (value <= lowerThreshold)
//				color = m_waterfallLoColor;
//			else 
//			if (value >= upperThreshold)
//					color = QColor(255, 255, 255);//m_waterfallHiColor;
//			else {
//
//				percent = (value - lowerThreshold) / (upperThreshold - lowerThreshold);
//				if (percent <= 0.5)	{ // use a gradient between low and mid colors
//				
//					percent *= 2;
//
//					r = (int)((1 - percent) * m_waterfallLoColor.red()   + percent * m_waterfallMidColor.red());
//					g = (int)((1 - percent) * m_waterfallLoColor.green() + percent * m_waterfallMidColor.green());
//					b = (int)((1 - percent) * m_waterfallLoColor.blue()  + percent * m_waterfallMidColor.blue());
//				}
//				else {	// use a gradient between mid and high colors
//
//					percent = (float)(percent - 0.5) * 2;
//
//					r = (int)((1 - percent) * m_waterfallMidColor.red()   + percent * 255);//m_waterfallHiColor.red());
//					g = (int)((1 - percent) * m_waterfallMidColor.green() + percent * 255);//m_waterfallHiColor.green());
//					b = (int)((1 - percent) * m_waterfallMidColor.blue()  + percent * 255);//m_waterfallHiColor.blue());
//				}
//
//				if (r > 255) r = 255;
//				if (g > 255) g = 255;
//				if (b > 255) b = 255;
//				color = QColor(r, g, b, m_waterfallAlpha);
//			}
//
//			break;
//
//		case QSDRGraphics::enhanced:
//
//			if (value <= lowerThreshold)
//				color = m_waterfallLoColor;
//			else 
//			if (value >= upperThreshold)
//					color = m_waterfallHiColor;
//			else {
//
//				offset = value - lowerThreshold;
//				globalRange = offset / m_waterfallColorRange; // value from 0.0 to 1.0 where 1.0 is high and 0.0 is low.
//
//				if (globalRange < (float)2/9) { // background to blue
//
//					localRange = globalRange / ((float)2/9);
//					r = (int)((1.0 - localRange) * m_waterfallLoColor.red());
//					g = (int)((1.0 - localRange) * m_waterfallLoColor.green());
//					b = (int)(m_waterfallLoColor.blue() + localRange * (255 - m_waterfallLoColor.blue()));
//				}
//				else 
//				if (globalRange < (float)3/9) { // blue to blue-green
//
//					localRange = (globalRange - (float)2/9) / ((float)1/9);
//					r = 0;
//					g = (int)(localRange * 255);
//					b = 255;
//				}
//				else 
//				if (globalRange < (float)4/9) { // blue-green to green
//
//					localRange = (globalRange - (float)3/9) / ((float)1/9);
//					r = 0;
//					g = 255;
//					b = (int)((1.0 - localRange) * 255);
//				}
//				else 
//				if (globalRange < (float)5/9) { // green to red-green
//
//					localRange = (globalRange - (float)4/9) / ((float)1/9);
//					r = (int)(localRange * 255);
//					g = 255;
//					b = 0;
//				}
//				else 
//				if (globalRange < (float)7/9) { // red-green to red
//
//					localRange = (globalRange - (float)5/9) / ((float)2/9);
//					r = 255;
//					g = (int)((1.0 - localRange) * 255);
//					b = 0;
//				}
//				else 
//				if (globalRange < (float)8/9) { // red to red-blue
//
//					localRange = (globalRange - (float)7/9) / ((float)1/9);
//					r = 255;
//					g = 0;
//					b = (int)(localRange * 255);
//				}
//				else { // red-blue to purple end
//
//					localRange = (globalRange - (float)8/9) / ((float)1/9);
//					r = (int)((0.75 + 0.25 * (1.0 - localRange)) * 255);
//					g = (int)(localRange * 255 * 0.5);
//					b = 255;
//				}
//				if (r > 255) r = 255;
//				if (g > 255) g = 255;
//				if (b > 255) b = 255;
//				if (r < 0) r = 0;
//				if (g < 0) g = 0;
//				if (b < 0) b = 0;
//				color = QColor(r, g, b, m_waterfallAlpha);
//			}
//
//			break;
//
//		case QSDRGraphics::spectran:
//
//			break;
//	}
//	
//	return color;
//}

void QGLDistancePanel::setChirpFFTShow(bool value) {

	distanceSpectrumBufferMutex.lock();
		m_showChirpFFT = value;
	distanceSpectrumBufferMutex.unlock();
}
 
void QGLDistancePanel::systemStateChanged(
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

	if (m_serverMode != mode) {

		//memset(m_wbSpectrumBuffer, -10000, 4 * BUFFER_SIZE * sizeof(float));
		memset(m_spectrumBuffer, -10000, 4 * BUFFER_SIZE * sizeof(float));
		memset(m_distanceSpectrumBuffer, -10000, 16 * BUFFER_SIZE * sizeof(float));
		m_serverMode = mode;
	}

	//resizeGL(width(), height());
	m_displayTime.restart();

}

 
void QGLDistancePanel::graphicModeChanged(
	int rx,
	PanGraphicsMode panMode,
	WaterfallColorMode colorScheme)
{
	Q_UNUSED (colorScheme)
	Q_UNUSED (rx)

	if (m_panMode != panMode)
		m_panMode = panMode;

	/*if (m_waterColorScheme != colorScheme) {

		m_waterColorScheme = colorScheme;
		change = true;
	}*/

	//if (!change) return;

}

 void QGLDistancePanel::setSpectrumAveraging(bool value) {

	 spectrumBufferMutex.lock();

	 if (m_spectrumAveraging == value) 
		 return;
	 else
		 m_spectrumAveraging = value;

	 spectrumBufferMutex.unlock();
 }

void QGLDistancePanel::setSpectrumAveragingCnt(int value) {

	spectrumBufferMutex.lock();

		memset(m_tmpBuf, 0, SAMPLE_BUFFER_SIZE * sizeof(float));

		while (!specAv_queue.isEmpty())
			specAv_queue.dequeue();

		m_specAveragingCnt = value;

		if (m_specAveragingCnt > 0)
			m_scale = 1.0f / m_specAveragingCnt;
		else
			m_scale = 1.0f;

	spectrumBufferMutex.unlock();
}

void QGLDistancePanel::setPanGridStatus(bool value) {

	spectrumBufferMutex.lock();

	 if (m_panGrid == value) 
		 return;
	 else
		 m_panGrid = value;

	 spectrumBufferMutex.unlock();

}

void QGLDistancePanel::sampleRateChanged(int value) {

	m_sampleRate = value;
}

void QGLDistancePanel::setPanadapterColors() {

	m_spectrumColorsChanged = true;

	mutex.lock();
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

	m_redD   = (GLfloat)(set->getPanadapterColors().distanceLineColor.red() / 256.0);
	m_greenD = (GLfloat)(set->getPanadapterColors().distanceLineColor.green() / 256.0);
	m_blueD  = (GLfloat)(set->getPanadapterColors().distanceLineColor.blue() / 256.0);

	mutex.unlock();
	update();
}

//void QGLDistancePanel::setWaterfallTime(int rx, int value) {
//
//	if (rx == 0)
//		m_waterfallTime = value;
//}

//void QGLDistancePanel::setWaterfallOffesetLo(int rx, int value) {
//
//	if (rx == 0)
//		m_waterfallOffsetLo = value;
//	
//	update();
//}
//
//void QGLDistancePanel::setWaterfallOffesetHi(int rx, int value) {
//
//	if (rx = 0)
//		m_waterfallOffsetHi = value;
//	
//	update();
//}

void QGLDistancePanel::closeEvent(QCloseEvent *event) {

	emit closeEvent();
	QWidget::closeEvent(event);
}

void QGLDistancePanel::showEvent(QShowEvent *event) {

	emit showEvent();
	QWidget::showEvent(event);
}
