#include "Models/RadioModel.h"
#include "Models/RadioTelemetry.h"
#include "Models/SliceModel.h"
/**
* @file  cusdr_oglDisplayPanel.cpp
* @brief Display panel class for cuSDR
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2012-02-22
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

#define LOG_DISPLAYPANEL

#include "cusdr_oglDisplayPanel.h"
#include "UI/FrequencyEntryDialog.h"
#include "cusdr_glShaders.h"
#include "cusdr_glDraw.h"
#include "Util/cusdr_rigctlserver.h"

#include <QGuiApplication>
#include <QOpenGLPaintDevice>

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif


OGLDisplayPanel::OGLDisplayPanel(RadioModel *model, QWidget *parent)
	: QOpenGLWidget(parent)
        , m_radioModel(model)

	, set(Settings::instance())
	, m_serverMode(set->getCurrentServerMode())
	, m_hwInterface(set->getHWInterface())
	, m_dataEngineState(QSDR::DataEngineDown)
	, m_smeterUpdate(true)
	, m_smeterRenew(true)
	, m_sMeterAvg(true)
	, m_oldFreq(0)
	, m_height(120)//(120)
	, m_sMeterWidth(300)
	, m_rxRectWidth(500)
	, m_lowerRectY(12)
	, m_upperRectY(1)
	, m_syncStatus(0)
	, m_adcStatus(0)
	, m_packetLossStatus(0)
	, m_sendIQStatus(0)
	, m_recvAudioStatus(0)
	, m_receivers(set->getNumberOfReceivers())
	, m_sample_rate(set->getSampleRate()/1000)
	, m_dither(set->getMercuryDither())
	, m_random(set->getMercuryRandom())
	, m_currentReceiver(set->getCurrentReceiver())
	, m_sMeterDeform(15)
	, m_freqDigitsPosY(70)
	, m_sMeterPosY(50)//(45)
	, m_sMeterHoldTime(model->slices().isEmpty() ? 1000 : model->slices().first()->sMeterHoldTime())
	, m_sMeterPrevHoldTimeMax(0)
	, m_sMeterPrevHoldTimeMin(0)
	, m_sMeterMeanValueCnt(0)
	, m_mouseWheelFreqStep(set->getMouseWheelFreqStep(m_currentReceiver))
	, m_dBmPanMin(-130.0f)
	, m_dBmPanMax(10.0f)
	, m_unit(1.0f)
	, m_smeterVertices(256.0f)
	, m_sMeterValue((float)(-156*ONEPI/256.0f))
	, m_sMeterMeanValue(0.0f)
	, m_sMeterMaxValueA((float)(-ONEPI/2.0f))
	, m_sMeterMinValueA((float)(ONEPI/2.0f))
    , m_sMeterMaxValueB(-1000.0f)
    , m_sMeterMinValueB(1000.0f)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setAutoFillBackground(false);
    setMouseTracking(true);
	const bool isWayland = QGuiApplication::platformName().contains("wayland", Qt::CaseInsensitive);
	setUpdateBehavior(isWayland ? QOpenGLWidget::NoPartialUpdate : QOpenGLWidget::PartialUpdate);
        m_freqStringLeftPos = 20;
        setupDisplayRegions(size());
        dpr = devicePixelRatioF();
        fonts = new CFonts(this);
	m_fonts = fonts->getFonts();

	m_fonts.smallFont.setBold(true);
	m_oglTextTiny = new OGLText(m_fonts.tinyFont, dpr);
	m_oglTextSmall = new OGLText(m_fonts.smallFont, dpr);

	m_fonts.smallFont.setItalic(true);

	m_shaderProgram = nullptr;
    m_textureProgram = nullptr;
	m_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);

	m_oglTextSmallItalic = new OGLText(m_fonts.smallFont, dpr);

	m_oglTextNormal = new OGLText(m_fonts.normalFont, dpr);
	m_oglTextBig = new OGLText(m_fonts.bigFont, dpr);

	m_fonts.bigFont.setItalic(true);
	m_oglTextBigItalic = new OGLText(m_fonts.bigFont, dpr);
	m_oglTextFreq1 = new OGLText(m_fonts.freqFont1, dpr);
	m_oglTextFreq2 = new OGLText(m_fonts.freqFont2, dpr);
	m_oglTextImpact = new OGLText(m_fonts.impactFont, dpr);

	// Glyph cache must match current dpr / CharData layout (GLES text path).
	m_oglTextTiny->invalidateCache();
	m_oglTextSmall->invalidateCache();
	m_oglTextSmallItalic->invalidateCache();
	m_oglTextNormal->invalidateCache();
	m_oglTextBig->invalidateCache();
	m_oglTextBigItalic->invalidateCache();
	m_oglTextFreq1->invalidateCache();
	m_oglTextFreq2->invalidateCache();
	m_oglTextImpact->invalidateCache();

	setupConnections();
	setupTextstrings();

	set10mhzSource(set->get10MHzSource());
	set122_88mhzSource(set->get122_8MHzSource());

	QList<qint64> fList = set->getVfoFrequencies();

	for (int i = 0; i < MAX_RECEIVERS; i++) {
		const qint64 freq =
			(i < fList.size()) ? fList.at(i)
			                   : (!fList.isEmpty() ? fList.at(0) : 7000000);

		TFrequency f;
		f.frequency = freq;
		f.freqMHz = (int)(freq / 1000);
		f.freqkHz = (int)(freq % 1000);

		m_frequencyList << f;
	}
	QList<THamBandFrequencies> bandList = getHamBandFrequencies();
	const qint64 baseFreq = !m_frequencyList.isEmpty() ? m_frequencyList.at(0).frequency : 7000000;
	HamBand band = getBandFromFrequency(bandList, baseFreq);

	QList<int> mercuryAttenuators = set->getMercuryAttenuators(0);
	const int bandIndex = static_cast<int>(band);
	if (bandIndex >= 0 && bandIndex < mercuryAttenuators.size()) {
		m_mercuryAttenuator = mercuryAttenuators.at(bandIndex);
	} else {
		m_mercuryAttenuator = 0;
		qWarning() << "OGLDisplayPanel: invalid mercury attenuator index" << bandIndex
				   << "for list size" << mercuryAttenuators.size();
	}



        m_colors = set->getPanadapterColors();

    m_txdigitColor = QColor(230,40,40);
	m_digitColor = QColor(68, 68, 68);
	m_bkgColor1 = QColor(30, 30, 30);
	m_bkgColor2 = QColor(50, 50, 50);
    m_activeTextColor = QColor(166, 196, 208);
    m_glTextColor = m_activeTextColor;
	m_inactiveTextColor = QColor(68, 68, 68);//Qt::white;//
	m_textBackgroundColor = QColor(66, 96, 208);
	m_sMeterTimer.start();
	m_sMeterMaxTimer.start();
	m_sMeterMinTimer.start();
	m_sMeterDisplayTime.start();
}

OGLDisplayPanel::~OGLDisplayPanel() {
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

    if (m_sMeterTex) {
        glDeleteTextures(1, &m_sMeterTex);
    }

    delete  m_oglTextBigItalic;
    delete  m_oglTextFreq1;
    delete  m_oglTextFreq2;
    delete  m_oglTextImpact;
    delete m_oglTextSmallItalic;
    delete m_oglTextNormal;
    delete m_oglTextBig;
    delete m_oglTextTiny;
    delete m_oglTextSmall;

}

QSize OGLDisplayPanel::minimumSizeHint() const {

	return QSize(width(), m_height);
}

QSize OGLDisplayPanel::sizeHint() const {
	
	//return QSize(width(), height());
	return QSize(width(), m_height);
}

void OGLDisplayPanel::setupConnections() {

	connect(set, &Settings::systemStateChanged,       this, &OGLDisplayPanel::systemStateChanged);
	connect(set, &Settings::vfoFrequencyChanged,      this, &OGLDisplayPanel::setFrequency);
	connect(set, &Settings::numberOfRXChanged,        this, &OGLDisplayPanel::setReceivers);
	connect(set, &Settings::currentReceiverChanged,   this, &OGLDisplayPanel::setCurrentReceiver);
	connect(set, &Settings::mercuryAttenuatorChanged, this, &OGLDisplayPanel::setMercuryAttenuator);
	connect(set, &Settings::ditherChanged,            this, &OGLDisplayPanel::setDither);
	connect(set, &Settings::randomChanged,            this, &OGLDisplayPanel::setRandom);
	connect(set, &Settings::sampleRateChanged,        this, &OGLDisplayPanel::setSampleRate);
	connect(set, &Settings::mercuryVersionChanged,    this, &OGLDisplayPanel::setMercuryVersion);
	connect(set, &Settings::metisVersionChanged,      this, &OGLDisplayPanel::setMetisVersion);
	connect(set, &Settings::penelopeVersionChanged,   this, &OGLDisplayPanel::setPenelopeVersion);
	connect(set, &Settings::penelopeVersionChanged,   this, &OGLDisplayPanel::setPennylaneVersion);
	connect(set, &Settings::hermesVersionChanged,     this, &OGLDisplayPanel::setHermesVersion);
	connect(set, &Settings::src10MhzChanged,          this, &OGLDisplayPanel::set10mhzSource);
	connect(set, &Settings::src122_88MhzChanged,      this, &OGLDisplayPanel::set122_88mhzSource);
	if (RadioTelemetry* tel = m_radioModel ? m_radioModel->telemetry() : nullptr) {
		connect(tel, &RadioTelemetry::protocolSyncChanged, this, &OGLDisplayPanel::setSyncStatus);
		connect(tel, &RadioTelemetry::adcOverflowChanged, this, &OGLDisplayPanel::setADCStatus);
		connect(tel, &RadioTelemetry::packetLossChanged, this, &OGLDisplayPanel::setPacketLossStatus);
		connect(tel, &RadioTelemetry::forwardPowerChanged, this, &OGLDisplayPanel::setForwardPower);
		connect(tel, &RadioTelemetry::swrChanged, this, &OGLDisplayPanel::setSWR);
		connect(tel, &RadioTelemetry::supplyVoltageChanged, this, &OGLDisplayPanel::setSupplyVoltage);
		connect(tel, &RadioTelemetry::temperatureChanged, this, &OGLDisplayPanel::setTemperature);
		connect(tel, &RadioTelemetry::sendIQSignalChanged, this, &OGLDisplayPanel::setSendIQStatus);
		connect(tel, &RadioTelemetry::rcveIQSignalChanged, this, &OGLDisplayPanel::setRecvAudioStatus);
	}
	connect(set, &Settings::radioStateChanged,        this, &OGLDisplayPanel::setRadioState);
	connect(set, &Settings::mouseWheelFreqStepChanged,this, &OGLDisplayPanel::setMouseWheelFreqStep);
        for (auto slice : m_radioModel->slices()) {
            connect(slice, &SliceModel::sMeterValueChanged, this, [this, slice](double value){ this->setSMeterValue(slice->id(), value); });
            connect(slice, &SliceModel::sMeterHoldTimeChanged, this, &OGLDisplayPanel::setSMeterHoldTime);
            connect(slice, &SliceModel::frequencyChanged, this, [this, slice](long freq){ this->setFrequency(0, slice->id(), freq); });
        }
	connect(set, &Settings::sMeterHoldTimeChanged,    this, &OGLDisplayPanel::setSMeterHoldTime);

	RigCtlServer *rcs = set->rigCtlServer();
	if (rcs)
		connect(rcs, &RigCtlServer::remoteControlChanged, this, &OGLDisplayPanel::setRigCtlStatus);
}

void OGLDisplayPanel::setupTextstrings() {

    m_blankWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(".");
    m_blankHeight = m_oglTextSmall->fontMetrics().height()-1;

    m_pointStringWidth = m_oglTextFreq1->fontMetrics().horizontalAdvance(".");
    m_blankWidthf = m_oglTextFreq1->fontMetrics().horizontalAdvance("59.999");
    m_blankWidthf1 = m_oglTextFreq1->fontMetrics().horizontalAdvance("0");
    m_blankWidthf2 = m_oglTextFreq2->fontMetrics().horizontalAdvance("0");
    m_fUnitStringWidth = m_oglTextFreq2->fontMetrics().horizontalAdvance("MHz");

    m_versionStringWidth = m_oglTextSmall->fontMetrics().horizontalAdvance("2.22");

	m_SYNCString = QString("SYNC");
    m_syncWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(m_SYNCString);

	m_ADCString = QString("ADC");
    m_adcWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(m_ADCString);

	m_PacketLossString = QString("IP Packets");
    m_packetLossWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(m_PacketLossString);

	m_sendIQString = QString("send IQ");
    m_sendIQWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(m_sendIQString);

	m_recvAudioString = QString("recv Audio");
    m_recvAudioWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(m_recvAudioString);

	m_AttnString = QString("Attn:");
    m_AttnWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(m_AttnString);

	m_ditherString = QString("Dither");
    m_ditherWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(m_ditherString);

	m_randomString = QString("Random");
    m_randomWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(m_randomString);

	m_sampleRateString = QString("SampleRate:");
    m_sampleRateWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(m_sampleRateString);

	m_modusString = QString("Modus:");
    m_modusWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(m_modusString);

	m_10MHzString = QString("10 MHz:");
    m_10MHzWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(m_10MHzString);

	m_12288MHzString = QString("122.88 MHz:");
    m_12288MHzWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(m_12288MHzString);

	m_mercuryString = QString("Mercury ");
    m_mercuryStringWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(m_mercuryString);

	m_penelopeString = QString("Penelope ");
    m_penelopeStringWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(m_penelopeString);

	m_pennylaneString = QString("Pennylane ");
    m_pennylaneStringWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(m_pennylaneString);

	m_excaliburString = QString("Excalibur ");
    m_excaliburStringWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(m_excaliburString);

	m_metisString = QString("Metis ");
    m_metisStringWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(m_metisString);

	m_alexString = QString("Alex ");
    m_alexStringWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(m_alexString);

	m_rigCtlString = QString("RigCtl");
	m_rigCtlStringWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(m_rigCtlString);

	m_hermesString = QString("Hermes ");
    m_hermesStringWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(m_hermesString);

    m_hermesStepAttnString = QString("Hermes Step-Attn:");
    m_hermesStepAttnStringWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(m_hermesStepAttnString);
}

void OGLDisplayPanel::initializeGL() {
    initializeOpenGLFunctions();
    if (!isValid()) return;

    // --- Modern OpenGL Setup ---
    m_shaderProgram = new QOpenGLShaderProgram(this);

    if (!m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, GlShaders::coloredVertexSourceVec3())) {
        qCritical() << "Vertex shader compilation failed:" << m_shaderProgram->log();
    }

    if (!m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, GlShaders::coloredFragmentSourceVec3())) {
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
    m_shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 6);

    m_shaderProgram->enableAttributeArray(1);
    m_shaderProgram->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 3, sizeof(float) * 6);

    m_vao.release();
    m_vbo.release();

    m_textureProgram = new QOpenGLShaderProgram(this);
    m_textureProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, GlShaders::texturedQuadVertexSource());
    m_textureProgram->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                              GlShaders::texturedFragmentSource("tex"));
    m_textureProgram->bindAttributeLocation("position", 0);
    m_textureProgram->bindAttributeLocation("texCoord", 1);
    if (!m_textureProgram->link())
        qCritical() << "S-meter texture shader link failed:" << m_textureProgram->log();

  // default initialization

    //glShadeModel(GL_FLAT);
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glDisable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);


    glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // 4-byte pixel ali
    glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
}

void OGLDisplayPanel::resizeGL(int iWidth,int iHeight) {
        //m_resizeTime.restart();
    setupDisplayRegions(QSize(iWidth, iHeight));
    glViewport(0, 0, (GLsizei)iWidth * dpr, (GLsizei)iHeight * dpr);
    setProjectionOrthographic(iWidth, iHeight);
    update();

}

void OGLDisplayPanel::paintGL() {
    const qreal currentDpr = devicePixelRatioF();
    if (!qFuzzyCompare(currentDpr, dpr)) {
        dpr = currentDpr;
        m_oglTextTiny->setDevicePixelRatio(dpr);
        m_oglTextSmall->setDevicePixelRatio(dpr);
        m_oglTextSmallItalic->setDevicePixelRatio(dpr);
        m_oglTextNormal->setDevicePixelRatio(dpr);
        m_oglTextBig->setDevicePixelRatio(dpr);
        m_oglTextBigItalic->setDevicePixelRatio(dpr);
        m_oglTextFreq1->setDevicePixelRatio(dpr);
        m_oglTextFreq2->setDevicePixelRatio(dpr);
        m_oglTextImpact->setDevicePixelRatio(dpr);
        m_smeterUpdate = true;
        m_smeterRenew = true;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0, 0, 0, 1.0);
	glClear(GL_DEPTH_BUFFER_BIT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
        paintRxRegion();
        paintUpperRegion();
        paintLowerRegion();
        paintSMeter();
     //   renderSMeterB();


}

void OGLDisplayPanel::paintUpperRegion() {
	QString str;


    GLint x1 =  m_rxRect.left() + m_blankWidth;
    GLint y1 =  m_rxRect.top() ;

	// sync status
	str = QString(m_SYNCString);
    QRect rect = QRect(x1, y1, m_syncWidth + 2*m_blankWidth, m_blankHeight);
	
	switch (m_syncStatus) {

		case 0:
			drawPanelRect(rect, QColor(68, 68, 68), -2.0f);
			break;


		case 1:
			drawPanelRect(rect, QColor(56, 242, 115), -2.0f);
			break;

		case 2:
			drawPanelRect(rect, QColor(242, 56, 109), -2.0f);
			break;
	}
	qglColor(Qt::black);
	renderPanelText(m_oglTextSmallItalic, x1 + m_blankWidth, y1, m_SYNCString);
	
	// ADC status
	str = QString(m_ADCString);
    x1 += m_syncWidth + 2*m_blankWidth + 2;
	rect = QRect(x1, y1, m_adcWidth + 2*m_blankWidth, m_blankHeight);

	switch (m_adcStatus) {

		case 0:
			drawPanelRect(rect, QColor(68, 68, 68), -2.0f);
			break;

		case 1:
			drawPanelRect(rect, QColor(56, 242, 115), -2.0f);
			break;

		case 2:
			drawPanelRect(rect, QColor(242, 56, 109), -2.0f);
			break;
	}
	qglColor(Qt::black);
	renderPanelText(m_oglTextSmallItalic, x1 + m_blankWidth, y1, m_ADCString);

	// Packet loss status
	str = QString(m_PacketLossString);
    x1 += m_adcWidth + 2*m_blankWidth + 2;
    rect = QRect(x1, y1,  m_packetLossWidth + 2*m_blankWidth, m_blankHeight);
    
	switch (m_packetLossStatus) {

		case 0:
			drawPanelRect(rect, QColor(68, 68, 68), -2.0f);
			break;

		case 1:
			drawPanelRect(rect, QColor(56, 242, 115), -2.0f);
			break;

		case 2:
			drawPanelRect(rect, QColor(242, 56, 109), -2.0f);
			break;
	}
	qglColor(Qt::black);
	renderPanelText(m_oglTextSmallItalic, x1 + m_blankWidth, y1, m_PacketLossString);

	// Metis status
	str = m_metisString;
    x1 += m_packetLossWidth + 2*m_blankWidth + 2;
	{
		QString fwdStr = QString("FWD: %1 W").arg(m_fwdPowerWatts, 0, 'f', 1);
		int fwdWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(fwdStr);
		rect = QRect(x1, y1, fwdWidth + 2*m_blankWidth, m_blankHeight);
		if (m_txActive) {
            drawPanelRect(rect, QColor(56, 242, 115), -2.0f);
            qglColor(QColor(0, 0, 0));
        } else {
            drawPanelRect(rect, QColor(50, 50, 50), -2.0f);
            qglColor(QColor(100, 100, 100));
        }
        renderPanelText(m_oglTextSmallItalic,x1 + m_blankWidth, y1, fwdStr);
        x1 += fwdWidth + 5*m_blankWidth;
	}

    // SWR status
    {
        QString swrStr = QString("SWR: %1").arg(m_swr, 0, 'f', 1);
        int swrWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(swrStr);
        rect = QRect(x1, y1, swrWidth + 2*m_blankWidth, m_blankHeight);

        if (m_txActive) {
            // Transmitting: show live colour-coded SWR
            if (m_swr < 1.5)
                drawPanelRect(rect, QColor(56, 242, 115), -2.0f); // Green
            else if (m_swr < 2.5)
                drawPanelRect(rect, QColor(255, 255, 50), -2.0f); // Yellow
            else
                drawPanelRect(rect, QColor(242, 56, 109), -2.0f); // Red
            qglColor(QColor(0, 0, 0));
        } else {
            // Receiving: dim grey background
            drawPanelRect(rect, QColor(50, 50, 50), -2.0f);
            qglColor(QColor(100, 100, 100));
        }
        renderPanelText(m_oglTextSmallItalic,x1 + m_blankWidth, y1, swrStr);
        x1 += swrWidth + 5*m_blankWidth;
    }

    // Supply Voltage
    if (m_supplyVolts > 0.1) {
        QString voltStr = QString("%1V").arg(m_supplyVolts, 0, 'f', 1);
        int voltWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(voltStr);
        rect = QRect(x1, y1, voltWidth + 2*m_blankWidth, m_blankHeight);
        drawPanelRect(rect, QColor(100, 120, 140), -2.0f); // Blue-grey
        qglColor(QColor(206, 236, 248));
        renderPanelText(m_oglTextSmallItalic,x1 + m_blankWidth, y1, voltStr);
        x1 += voltWidth + 5*m_blankWidth;
    }

    // Temperature
    if (m_temperature > 0.1) {
        QString tempStr = QString("%1°C").arg(m_temperature, 0, 'f', 1);
        int tempWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(tempStr);
        rect = QRect(x1, y1, tempWidth + 2*m_blankWidth, m_blankHeight);
        drawPanelRect(rect, QColor(80, 80, 80), -2.0f); // Deep grey
        qglColor(QColor(206, 236, 248));
        renderPanelText(m_oglTextSmallItalic,x1 + m_blankWidth, y1, tempStr);
        x1 += tempWidth + 5*m_blankWidth;
    }

	if (m_hwInterface == QSDR::Metis && m_dataEngineState == QSDR::DataEngineUp)
		rect = QRect(x1, y1, m_metisStringWidth + m_versionStringWidth, m_blankHeight);
	else
		rect = QRect(x1, y1, m_metisStringWidth, m_blankHeight);
	
	if (m_hwInterface == QSDR::Metis) {

		drawPanelRect(rect, m_textBackgroundColor, -2.0f);
		if (m_dataEngineState == QSDR::DataEngineUp) {

			str.append(m_metisVersion);
			qglColor(QColor(206, 236, 248));
		}
		else
			qglColor(QColor(0, 0, 0));
	}
	else {

		drawPanelRect(rect, QColor(68, 68, 68), -2.0f);
		qglColor(QColor(0, 0, 0));
	}
	renderPanelText(m_oglTextSmallItalic,x1, y1, 1.0f, str);


	// Mercury status
	str = m_mercuryString;

	if (m_hwInterface == QSDR::Metis && m_dataEngineState == QSDR::DataEngineUp) {
		x1 += m_metisStringWidth + m_versionStringWidth + m_blankWidth;
		rect = QRect(x1, y1, m_mercuryStringWidth + m_versionStringWidth, m_blankHeight);
	}
	else {
		x1 += m_metisStringWidth + m_blankWidth;
		rect = QRect(x1, y1, m_mercuryStringWidth, m_blankHeight);
	}

    //rect = QRect(x1, y1, m_mercuryStringWidth + m_versionStringWidth, m_blankHeight);

	if (set->getMercuryPresence() && m_hwInterface == QSDR::Metis) {
		drawPanelRect(rect, m_textBackgroundColor, -2.0f);

		if (m_dataEngineState == QSDR::DataEngineUp) {
			str.append(m_mercuryVersion);
			qglColor(QColor(206, 236, 248));
		}
		else
			qglColor(QColor(0, 0, 0));
	}
	else {

		drawPanelRect(rect, QColor(68, 68, 68), -2.0f);
		qglColor(QColor(0, 0, 0));
	}

	//m_oglTextSmallItalic->renderFreqText(x1 + m_blankWidth, y1, 1.0f, str);
	renderPanelText(m_oglTextSmallItalic,x1, y1, 1.0f, str);


	// Penelope status
	str = m_penelopeString;

	if (m_hwInterface == QSDR::Metis && m_dataEngineState == QSDR::DataEngineUp)
		x1 += m_mercuryStringWidth + m_versionStringWidth + m_blankWidth;
	else
		x1 += m_mercuryStringWidth + m_blankWidth;

	if (set->getPenelopePresence() && m_hwInterface == QSDR::Metis) {
		str = m_penelopeString;

		if (m_dataEngineState == QSDR::DataEngineUp)
			rect = QRect(x1, y1, m_penelopeStringWidth + m_versionStringWidth, m_blankHeight);
		else
			rect = QRect(x1, y1, m_penelopeStringWidth, m_blankHeight);

		drawPanelRect(rect, m_textBackgroundColor, -2.0f);

		if (m_dataEngineState == QSDR::DataEngineUp) {
			str.append(m_penelopeVersion);
			qglColor(QColor(206, 236, 248));
		}
		else
			qglColor(QColor(0, 0, 0));
	}
	else if (set->getPennyLanePresence() && m_hwInterface == QSDR::Metis) {
		str = m_pennylaneString;

		if (m_dataEngineState == QSDR::DataEngineUp)
			rect = QRect(x1, y1, m_pennylaneStringWidth + m_versionStringWidth, m_blankHeight);
		else
			rect = QRect(x1, y1, m_pennylaneStringWidth, m_blankHeight);

		drawPanelRect(rect, m_textBackgroundColor, -2.0f);

		if (m_dataEngineState == QSDR::DataEngineUp) {
			str.append(m_pennylaneVersion);
			qglColor(QColor(206, 236, 248));
		}
		else
			qglColor(QColor(0, 0, 0));
	}
	else {
//        if (m_dataEngineState == QSDR::DataEngineUp && m_hwInterface == QSDR::Metis)
//            rect = QRect(x1, y1, m_penelopeStringWidth + m_versionStringWidth, m_blankHeight);
//        else
			rect = QRect(x1, y1, m_penelopeStringWidth, m_blankHeight);

        drawPanelRect(rect, QColor(68, 68, 68), -2.0f);
		qglColor(QColor(0, 0, 0));
	}

	renderPanelText(m_oglTextSmallItalic,x1, y1, 1.0f, str);


	// Hermes status
	str = m_hermesString;

    if(set->getPennyLanePresence())
        x1 += m_pennylaneStringWidth;
    else
        x1 += m_penelopeStringWidth;

    x1 += m_blankWidth;

    if (m_dataEngineState == QSDR::DataEngineUp) {

        if(m_hwInterface == QSDR::Metis) {

            if(set->getPenelopePresence() || set->getPennyLanePresence())
                x1 += m_versionStringWidth;

            rect = QRect(x1, y1,  m_hermesStringWidth, m_blankHeight);
        }
        else {
            rect = QRect(x1, y1,  m_hermesStringWidth + m_versionStringWidth, m_blankHeight);
        }
    }
	else {
        rect = QRect(x1, y1,  m_hermesStringWidth, m_blankHeight);
    }

    if (set->getHPSDRHardware() == 1) {
		drawPanelRect(rect, m_textBackgroundColor, -2.0f);

		if (m_dataEngineState == QSDR::DataEngineUp) {
			str.append(m_hermesVersion);
			qglColor(QColor(206, 236, 248));
		}
		else
			qglColor(QColor(0, 0, 0));
	}
	else {
		drawPanelRect(rect, QColor(68, 68, 68), -2.0f);
		qglColor(QColor(0, 0, 0));
	}

	//m_oglTextSmallItalic->renderFreqText(x1 + m_blankWidth, y1, 1.0f, str);
    renderPanelText(m_oglTextSmallItalic,x1, y1, 1.0f, str);


	// Excalibur status
	str = m_excaliburString;

//    if (m_dataEngineState == QSDR::DataEngineUp && set->getHermesPresence())
    if (m_dataEngineState == QSDR::DataEngineUp && m_hwInterface == QSDR::Hermes)
        x1 += m_hermesStringWidth + m_versionStringWidth + m_blankWidth;
	else
		x1 += m_hermesStringWidth + m_blankWidth;

    //rect = QRect(x1, y1, m_hermesStringWidth + m_versionStringWidth, m_blankHeight);
	rect = QRect(x1, y1, m_excaliburStringWidth, m_blankHeight);

	if (set->getExcaliburPresence() && m_hwInterface == QSDR::Metis) {

		drawPanelRect(rect, m_textBackgroundColor, -2.0f);
		if (m_dataEngineState == QSDR::DataEngineUp) {

			str.append(m_excaliburVersion);
			qglColor(QColor(206, 236, 248));
		}
		else
			qglColor(QColor(0, 0, 0));
	}
	else {

		drawPanelRect(rect, QColor(68, 68, 68), -2.0f);
		qglColor(QColor(0, 0, 0));
	}
	//m_oglTextSmallItalic->renderFreqText(x1 + m_blankWidth, y1, 1.0f, str);
    renderPanelText(m_oglTextSmallItalic,x1, y1, 1.0f, str);

	
	// Alex status
	str = m_alexString;
    //x1 += m_hermesStringWidth + m_versionStringWidth + m_blankWidth;
	x1 += m_excaliburStringWidth + m_blankWidth;

    rect = QRect(x1, y1, m_alexStringWidth + m_blankWidth, m_blankHeight);

	if (set->getAlexPresence()) {

		drawPanelRect(rect, m_textBackgroundColor, -2.0f);
		if (m_dataEngineState == QSDR::DataEngineUp) {

			str.append(m_alexVersion);
			qglColor(QColor(206, 236, 248));
		}
		else
			qglColor(QColor(0, 0, 0));
	}
	else {

		drawPanelRect(rect, QColor(68, 68, 68), -2.0f);
		qglColor(QColor(0, 0, 0));
	}
	//m_oglTextSmallItalic->renderFreqText(x1 + m_blankWidth, y1, 1.0f, str);
    renderPanelText(m_oglTextSmallItalic,x1, y1, 1.0f, str);

	// RigCtl status
	x1 += m_alexStringWidth + m_blankWidth;
	rect = QRect(x1, y1, m_rigCtlStringWidth + 2*m_blankWidth, m_blankHeight);
	if (m_rigCtlConnected) {
		drawPanelRect(rect, QColor(56, 242, 115), -2.0f);
		qglColor(QColor(0, 0, 0));
	} else {
		drawPanelRect(rect, QColor(68, 68, 68), -2.0f);
		qglColor(QColor(0, 0, 0));
	}
	renderPanelText(m_oglTextSmallItalic,x1 + m_blankWidth, y1, m_rigCtlString);
}

void OGLDisplayPanel::paintLowerRegion() {

	QString str;

	GLint x1 = m_rxRect.left() + m_blankWidth;
	GLint y2 = m_rxRect.height() - m_lowerRectY;
	
	// Attenuator
	qglColor(QColor(106, 136, 148));
	renderPanelText(m_oglTextSmallItalic,x1 + m_blankWidth, y2, m_AttnString);

    x1 += m_AttnWidth + 2*m_blankWidth;
	if (m_mercuryAttenuator == 1)
		str = "0 dB";
	else
		str = "-20 dB";

    int attnValueWidth = m_oglTextSmall->fontMetrics().tightBoundingRect(str).width();
	qglColor(m_activeTextColor);
	renderPanelText(m_oglTextSmallItalic,x1, y2, str);

	// Dither status
    x1 += attnValueWidth + 5*m_blankWidth;

	if (m_dither == 1)
		qglColor(m_activeTextColor);
	else
		qglColor(QColor(68, 68, 68));

	renderPanelText(m_oglTextSmallItalic,x1 + m_blankWidth, y2, m_ditherString);

	// Random status
    x1 += m_ditherWidth + 5*m_blankWidth;

	if (m_random == 1)
		qglColor(m_activeTextColor);
	else
		qglColor(QColor(68, 68, 68));

	renderPanelText(m_oglTextSmallItalic,x1 + m_blankWidth, y2, m_randomString);

	// Sample rate status
    x1 += m_randomWidth + 10*m_blankWidth;
	str = "%1";

	qglColor(QColor(166, 196, 208));
	renderPanelText(m_oglTextSmallItalic,x1 + m_blankWidth, y2, str.arg(m_sample_rate, 3, 10, QLatin1Char(' ')));

	int samplerateWidth = m_oglTextSmall->fontMetrics().tightBoundingRect(str.arg(m_sample_rate, 3, 10, QLatin1Char(' '))).width();
	x1 += samplerateWidth + 4*m_blankWidth;

	str = "kHz";
	int samplerateUnitWidth = m_oglTextSmall->fontMetrics().tightBoundingRect(str).width();
	renderPanelText(m_oglTextSmallItalic,x1 + m_blankWidth, y2, str);


	// server modus status
    x1 += samplerateUnitWidth + 10*m_blankWidth;
	switch (m_serverMode) {

		case QSDR::NoServerMode:

			str = "No Server mode";
			break;

		case QSDR::SDRMode:
			
			str = "SDR Mode";
			break;

	}
	int serverModeStringWidth = m_oglTextSmall->fontMetrics().tightBoundingRect(str).width();

	qglColor(QColor(166, 196, 208));
	renderPanelText(m_oglTextSmallItalic,x1 + m_blankWidth, y2, str);

	if (m_hwInterface == QSDR::Metis) {

		x1 += serverModeStringWidth + 15*m_blankWidth;

		// 10 MHz source status
		qglColor(QColor(106, 136, 148));
		renderPanelText(m_oglTextSmallItalic,x1 + m_blankWidth, y2, m_10MHzString);

		x1 += m_10MHzWidth + 4*m_blankWidth;
		qglColor(QColor(166, 196, 208));
		int src10MHStringWidth = m_oglTextSmall->fontMetrics().tightBoundingRect(m_src10mhz).width();
		renderPanelText(m_oglTextSmallItalic,x1 + m_blankWidth, y2, m_src10mhz);

		// 122.88 MHz source status
		x1 += src10MHStringWidth + 10*m_blankWidth;
		qglColor(QColor(106, 136, 148));
		renderPanelText(m_oglTextSmallItalic,x1 + m_blankWidth, y2, m_12288MHzString);

		x1 += m_12288MHzWidth + 4*m_blankWidth;
		qglColor(QColor(166, 196, 208));
		renderPanelText(m_oglTextSmallItalic,x1 + m_blankWidth, y2, m_src122_88mhz);
	}
	else if (m_hwInterface == QSDR::Hermes) {

		//x1 += serverModeStringWidth + 10*m_blankWidth;

		//qglColor(QColor(166, 196, 208));
		//m_oglTextSmallItalic->renderFreqText(x1, y2, m_hermesStepAttnString);

		//x1 += m_hermesStepAttnStringWidth + 2*m_blankWidth;
		//y2 += 1;

		//QColor triCol;
		//QColor onLCol = QColor(0x11, 0x6b, 0x7f);//QColor(26, 56, 168);
		//QColor onHCol = QColor(0x51, 0xab, 0xbf);//QColor(66, 96, 208);
		//QColor offCol = QColor(68, 68, 68);

		//if (m_dataEngineState == QSDR::DataEngineUp)
		//	triCol = QColor(156, 186, 198);
		//else
		//	triCol = QColor(68, 68, 68);

		//QRect rect = QRect(x1, y2, 9, 10);
		//drawGLTriangleLeft(rect, triCol, -2.0f);
		//x1 += 10;

		//for (int i = 0; i < 31; i++) {

		//	rect = QRect(x1 + i*4, y2, 3, 10);
		//	if (m_dataEngineState == QSDR::DataEngineUp) {

		//		if (i < 19)
		//			drawGLRect(rect, onHCol, -2.0f);
		//		else
		//			drawGLRect(rect, onLCol, -2.0f);
		//	}
		//	else
		//		drawGLRect(rect, offCol, -2.0f);
		//}

		//x1 += 124;
		//rect = QRect(x1, y2, 9, 10);
		//drawGLTriangleRight(rect, triCol, -2.0f);

		//if (m_dataEngineState == QSDR::DataEngineUp)
		//	qglColor(QColor(166, 196, 208));
		//else
		//	qglColor(QColor(0, 0, 0));

		//x1 += 12;
		//m_oglTextSmallItalic->renderFreqText(x1, y2, "-19 dB");
	}
}



void OGLDisplayPanel::paintRxRegion() {
    QString str;
    QColor fontcolor;
    QPainter painter(this);
  //  painter.setPen(Qt::red);
  //  painter.drawRect(m_rxRect);
    painter.beginNativePainting();
    GLint x1 = m_rxRect.left() + 20;
    GLint y1 = ((m_rxRect.top()) + m_freqDigitsPosY);

        // draw background
	if (m_dataEngineState == QSDR::DataEngineUp) {

        drawPanelGradientRect(m_rect, Qt::black, m_bkgColor2, false, -3.0f);
		qglColor(m_activeTextColor);
        fontcolor = m_activeTextColor;
	}
	else {

		drawPanelRect(m_rect, QColor(0, 0, 0, 255), -3.0f);
		qglColor(QColor(68, 68, 68));
		fontcolor = QColor(68, 68, 68);
	}


	str = "%1.%2";

	TFrequency currentFrequency;
	if (m_currentReceiver >= 0 && m_currentReceiver < m_frequencyList.size()) {
		currentFrequency = m_frequencyList.at(m_currentReceiver);
	} else {
		currentFrequency.frequency = 7000000;
		currentFrequency.freqMHz = 7000;
		currentFrequency.freqkHz = 0;
	}

	int f1 = currentFrequency.freqMHz; // kHz
	int f2 = currentFrequency.freqkHz; // Hz

    // Format: G.MMM.KKK
    long ghz = f1 / 1000000;
    long mhz = (f1 / 1000) % 1000;
    long khz = f1 % 1000;
    
    m_f1str = QString("%1.%2.%3")
            .arg(ghz)
            .arg(mhz, 3, 10, QLatin1Char('0'))
            .arg(khz, 3, 10, QLatin1Char('0'));

    // Suppress leading zeros/dots in f1str
    for (int i = 0; i < m_f1str.length() - 1; ++i) {
        if (m_f1str[i] == '0' || m_f1str[i] == '.') {
            m_f1str[i] = ' ';
        } else {
            break;
        }
    }

	m_freqStringLeftPos = x1;

    m_f2str = QString("%1").arg(f2, 3, 10, QLatin1Char('0'));
    painter.endNativePainting();
    painter.scale(1,1);

    renderFreqText(painter,x1,y1,m_fonts.freqFont1,m_oglTextFreq1->fontMetrics(), fontcolor, m_f1str, 0, m_digitPosition, m_blankWidthf1 );
    renderText(painter,x1, y1, m_fonts.freqFont1, fontcolor, ".");

    x1 += m_pointStringWidth;
    renderFreqText(painter,x1,y1,m_fonts.freqFont2,m_oglTextFreq2->fontMetrics(), fontcolor, m_f2str, 10, m_digitPosition, m_blankWidthf2);
    x1+= 2 * m_blankWidth;

    renderText(painter,x1, y1 - 1, m_fonts.freqFont2, fontcolor, "MHz");

        // current mouse wheel step size
        str = "step: %1";
        x1 += m_fUnitStringWidth + 3 * m_blankWidthf2;

    renderText(painter,x1, y1 , m_fonts.normalFont, fontcolor, str.arg(set->getValue1000(m_mouseWheelFreqStep, 0, "Hz")));

        // current receiver
    if (set->getRadioState() == RadioState::RX){
        str = "Rx: %1";
        renderText(painter,x1, y1 - (int)( m_fonts.fontHeightBigFont ) ,m_fonts.bigFont, fontcolor, str.arg(m_currentReceiver + 1));
    }
    else{
        str ="Tx: %1";
        renderText(painter,x1, y1 - (int)( m_fonts.fontHeightBigFont ) ,m_fonts.bigFont, m_txdigitColor, str.arg(m_currentReceiver + 1));
    }


	// frequency info
	if (m_oldFreq != currentFrequency.frequency) {

		m_bandText = getHamBandTextString(set->getHamBandTextList(), false, currentFrequency.frequency);
                m_oldFreq = currentFrequency.frequency;
        }

    renderText(painter,m_freqStringLeftPos,  y1 + (int) (m_fonts.fontHeightFreqFont1) - 10 ,m_fonts.smallFont, fontcolor, m_bandText);
        painter.end();

    }

QMatrix4x4 OGLDisplayPanel::panelProjection() const
{
    QMatrix4x4 projection;
    projection.ortho(0, size().width(), size().height(), 0, -10, 10);
    return projection;
}

void OGLDisplayPanel::drawPanelRect(const QRect &rect, const QColor &color, float z)
{
    if (rect.isEmpty())
        return;
    if (m_shaderProgram && m_shaderProgram->isLinked())
        GlDraw::drawSolidRect(this, m_shaderProgram, m_vbo, panelProjection(), rect, color, z);
    else
        drawGLRect(rect, color, z);
}

void OGLDisplayPanel::drawPanelGradientRect(const QRect &rect,
                                          const QColor &c1,
                                          const QColor &c2,
                                          bool leftToRight,
                                          float z)
{
    if (rect.isEmpty())
        return;
    if (m_shaderProgram && m_shaderProgram->isLinked())
        GlDraw::drawGradientRect(this, m_shaderProgram, m_vbo, panelProjection(), rect, c1, c2, leftToRight, z);
    else
        drawGLRect(rect, c1, c2, z, leftToRight);
}

void OGLDisplayPanel::drawSMeterNeedle(const QMatrix4x4 &projection, int x1)
{
    if (m_sMeterValue <= 0 || !m_shaderProgram || !m_shaderProgram->isLinked())
        return;

    const float x = float(x1 + int(m_sMeterValue * m_unit));
    const GlDraw::Vec3Rgb needle[2] = {
        { x, float(m_sMeterPosY) - 15.0f, 1.0f, 1.0f, 1.0f, 1.0f },
        { x, float(m_sMeterPosY) + 28.0f, 1.0f, 1.0f, 1.0f, 1.0f },
    };

    glLineWidth(2.0f);
    GlDraw::drawColoredLines(this, m_shaderProgram, m_vbo, projection, needle, 2);
}

    void OGLDisplayPanel::paintSMeter() {

        GLint width = m_smeterRect.width();
        GLint height = m_smeterRect.height();
        GLint x1 = m_smeterRect.left();
        GLint y1 = m_smeterRect.top();
        GLint y2 = y1 + height;

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);

        // Only recreate FBO if needed
        if (!m_smeterFBO || m_smeterRenew) {
            if (m_smeterFBO) {
                delete m_smeterFBO;
                m_smeterFBO = nullptr;
            }
            m_smeterFBO = new QOpenGLFramebufferObject(m_sMeterWidth, height);
            m_smeterUpdate = true; // Need to re-render after FBO recreation
            m_smeterRenew = false;
        }

        // Only re-render scale if needed
        if (m_smeterUpdate) {
            m_smeterFBO->bind();
            renderSMeterScale();
            m_smeterFBO->release();
            m_smeterUpdate = false;
        }

        const QMatrix4x4 projection = panelProjection();
        const int smeterX = m_rxRect.right() + m_sMeterOffset;

        glDisable(GL_DEPTH_TEST);

        const QRect texRect(smeterX, 0, m_sMeterWidth, height);
        if (m_textureProgram && m_textureProgram->isLinked())
            GlDraw::renderTexturedQuad(this, m_textureProgram, m_vbo, projection,
                                       texRect, m_smeterFBO->texture(), -2.0f);
        else
            renderTexture(texRect, m_smeterFBO->texture(), -2.0f);

        drawSMeterScaleLabels(projection, smeterX);

        glScissor(int(x1 * dpr), int((size().height() - y2) * dpr), int(width * dpr), int(height * dpr));
        glEnable(GL_SCISSOR_TEST);

        if (m_dataEngineState == QSDR::DataEngineUp) {

            glLineWidth(1);
            int min = (int)(m_sMeterMinValueB * m_unit);
            int max = (int)(m_sMeterMaxValueB * m_unit);
            min += min % 2;
            max += max % 2;

            QRect bar(x1 + min, m_sMeterPosY + 4, max - min, 5);
            if (min > 0 && m_shaderProgram && m_shaderProgram->isLinked())
                GlDraw::drawGradientRect(this, m_shaderProgram, m_vbo, projection, bar,
                                         QColor(255, 50, 50), QColor(255, 255, 50), true, 1.0f);

            drawSMeterNeedle(projection, x1);

            qglColor(m_activeTextColor);
            m_sMeterNumValueString = QString::number(m_sMeterOrgValue, 'f', 1);
            m_oglTextBig->renderText(projection, x1 + m_sMeterWidth - 85, 2, m_sMeterNumValueString, Qt::white);
            m_oglTextNormal->renderText(projection, x1 + m_sMeterWidth - 28, 9, QStringLiteral("dBm"), m_activeTextColor);
        }

        glDisable(GL_SCISSOR_TEST);
        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
    }

void OGLDisplayPanel::renderSMeterScale() {
	const GLint width = m_sMeterWidth;
	const GLint height = m_smeterRect.height();

	const qreal dBmRange = qAbs(m_dBmPanMax - m_dBmPanMin);
	m_unit = (dBmRange > 0) ? qreal(m_sMeterWidth / dBmRange) : 0;

	GLint savedViewport[4] = { 0, 0, 0, 0 };
	glGetIntegerv(GL_VIEWPORT, savedViewport);

	const int fboW = m_smeterFBO ? m_smeterFBO->width() : width;
	const int fboH = m_smeterFBO ? m_smeterFBO->height() : height;
	if (m_smeterFBO)
		glViewport(0, 0, fboW, fboH);

	const QRect rect(0, 0, fboW, fboH);

	QMatrix4x4 projection;
	projection.ortho(0, fboW, fboH, 0, -10, 10);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_MULTISAMPLE);
	glDisable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);

	if (m_shaderProgram && m_shaderProgram->isLinked()) {
		if (m_dataEngineState == QSDR::DataEngineUp)
			GlDraw::drawGradientRect(this, m_shaderProgram, m_vbo, projection, rect,
			                         Qt::black, m_bkgColor2, false, -3.0f);
		else
			GlDraw::drawSolidRect(this, m_shaderProgram, m_vbo, projection, rect, Qt::black, -3.0f);

		QColor col = m_activeTextColor;
		const float r = col.redF(), g = col.greenF(), b = col.blueF();
		const float posY = float(m_sMeterPosY);

		QVector<GlDraw::Vec3Rgb> scaleLines;
		scaleLines.reserve(4 + m_sMeterWidth * 2);
		scaleLines.append({ 0.0f, posY, 0.0f, r, g, b });
		scaleLines.append({ float(width - 1), posY, 0.0f, r, g, b });
		scaleLines.append({ 0.0f, posY + 12.0f, 0.0f, r, g, b });
		scaleLines.append({ float(width - 1), posY + 12.0f, 0.0f, r, g, b });

		const QColor stepCol = (m_dataEngineState == QSDR::DataEngineUp)
		                           ? QColor(126, 156, 168)
		                           : m_activeTextColor;
		const float sr = stepCol.redF(), sg = stepCol.greenF(), sb = stepCol.blueF();

		int vertexArrayLength = m_sMeterWidth;
		vertexArrayLength += vertexArrayLength % 2;
		for (int i = 0; i < vertexArrayLength; ++i) {
			scaleLines.append({ 2.0f * float(i), posY + 4.0f, 0.0f, sr, sg, sb });
			scaleLines.append({ 2.0f * float(i), posY + 9.0f, 0.0f, sr, sg, sb });
		}

		GlDraw::drawColoredLines(this, m_shaderProgram, m_vbo, projection,
		                         scaleLines.constData(), scaleLines.size());

		const QColor tickCol = (m_dataEngineState == QSDR::DataEngineUp) ? Qt::white : m_inactiveTextColor;
		const float tr = tickCol.redF(), tg = tickCol.greenF(), tb = tickCol.blueF();

		QVector<GlDraw::Vec3Rgb> tickLines;
		tickLines.reserve(52);
		for (int i = 1, z = -120; z < 10; i++, z += 10) {
			const float xMajor = float(10 * i * m_unit);
			const float xMinor = float((10 * i - 5) * m_unit);
			tickLines.append({ xMajor, posY - 4.0f, 0.0f, tr, tg, tb });
			tickLines.append({ xMajor, posY, 0.0f, tr, tg, tb });
			tickLines.append({ xMinor, posY - 2.0f, 0.0f, tr, tg, tb });
			tickLines.append({ xMinor, posY, 0.0f, tr, tg, tb });
		}
		GlDraw::drawColoredLines(this, m_shaderProgram, m_vbo, projection,
		                         tickLines.constData(), tickLines.size());

		QVector<GlDraw::Vec3Rgb> sUnitLines;
		sUnitLines.reserve(40);
		auto appendLine = [&](float x, float y1, float y2, float lr, float lg, float lb) {
			sUnitLines.append({ x, y1, 0.0f, lr, lg, lb });
			sUnitLines.append({ x, y2, 0.0f, lr, lg, lb });
		};

		for (int i = 0; i < 17; ++i) {
			if (i < 10) {
				const float x = float((6 * i + 3) * m_unit);
				appendLine(x, posY + 12.0f, posY + 17.0f, r, g, b);
			} else {
				float lr, lg, lb;
				if (m_dataEngineState == QSDR::DataEngineUp) {
					lr = 1.0f; lg = 80.0f / 255.0f; lb = 80.0f / 255.0f;
				} else {
					lr = m_inactiveTextColor.redF();
					lg = m_inactiveTextColor.greenF();
					lb = m_inactiveTextColor.blueF();
				}
				const float x = float((10 * i - 33) * m_unit);
				appendLine(x, posY + 12.0f, posY + 17.0f, lr, lg, lb);
			}
		}
		sUnitLines.append({ float(57 * m_unit + 1), posY + 12.0f, 0.0f, r, g, b });
		sUnitLines.append({ float(width - 1), posY + 12.0f, 0.0f, r, g, b });

		GlDraw::drawColoredLines(this, m_shaderProgram, m_vbo, projection,
		                         sUnitLines.constData(), sUnitLines.size());
	} else {
		if (m_dataEngineState == QSDR::DataEngineUp)
			GlDraw::drawGradientRect(this, m_shaderProgram, m_vbo, projection, rect,
			                         Qt::black, m_bkgColor2, false, -3.0f);
		else
			GlDraw::drawSolidRect(this, m_shaderProgram, m_vbo, projection, rect, Qt::black, -3.0f);
	}

	glViewport(savedViewport[0], savedViewport[1], savedViewport[2], savedViewport[3]);
}

void OGLDisplayPanel::drawSMeterScaleLabels(const QMatrix4x4 &projection, int xOffset)
{
	const QFontMetrics fm = m_oglTextNormal->fontMetrics();

	QString marker;
	for (int i = 1, z = -120; z < 10; i++, z += 10) {
		marker = QString::number(z, 'f', 0);
		const int d = fm.horizontalAdvance(marker);
		const int x = xOffset + int(10 * i * m_unit) - d / 2 - 2;

		if (z == -120 || z == -100 || z == -80 || z == -60 || z == -40 || z == -20)
			m_oglTextNormal->renderText(projection, float(x), float(m_sMeterPosY - 18), marker, m_activeTextColor);

		if (m_sMeterWidth > 500) {
			if (z == -110 || z == -90 || z == -70 || z == -50 || z == -30 || z == -10)
				m_oglTextNormal->renderText(projection, float(x), float(m_sMeterPosY - 18), marker, m_activeTextColor);
		}

		if (m_sMeterWidth > 400 && z == 0)
			m_oglTextNormal->renderText(projection, float(xOffset + int(10 * i * m_unit) - d / 2),
			                            float(m_sMeterPosY - 18), marker, m_activeTextColor);
	}

	m_oglTextSmallItalic->renderText(projection, float(xOffset + m_sMeterWidth - 25),
	                                 float(m_sMeterPosY - 16), QStringLiteral("dBm"), m_activeTextColor);

	for (int i = 0; i < 17; ++i) {
		if (i < 10) {
			marker = QStringLiteral("S1");
			const int d = fm.horizontalAdvance(marker);
			const float x = float(xOffset + int((6 * (i + 1) - d / 2 + 1) * m_unit));

			if (i == 1)
				m_oglTextNormal->renderText(projection, x, float(m_sMeterPosY + 18), marker, m_activeTextColor);
			else if (i == 3)
				m_oglTextNormal->renderText(projection, x, float(m_sMeterPosY + 18), QStringLiteral("S3"), m_activeTextColor);
			else if (i == 5)
				m_oglTextNormal->renderText(projection, x, float(m_sMeterPosY + 18), QStringLiteral("S5"), m_activeTextColor);
			else if (i == 7)
				m_oglTextNormal->renderText(projection, x, float(m_sMeterPosY + 18), QStringLiteral("S7"), m_activeTextColor);
			else if (i == 9)
				m_oglTextNormal->renderText(projection, x, float(m_sMeterPosY + 18), QStringLiteral("S9"), m_activeTextColor);
		} else {
			const int idx = xOffset + int((10 * i - 33) * m_unit);
			marker = QStringLiteral("+20");
			const int d = fm.horizontalAdvance(marker);

			if (i == 11)
				m_oglTextNormal->renderText(projection, float(idx - d / 2 - 2), float(m_sMeterPosY + 18), marker, m_activeTextColor);
			else if (i == 13)
				m_oglTextNormal->renderText(projection, float(idx - d / 2 - 2), float(m_sMeterPosY + 18), QStringLiteral("+40"), m_activeTextColor);
			else if (i == 15)
				m_oglTextNormal->renderText(projection, float(idx - d / 2 - 2), float(m_sMeterPosY + 18), QStringLiteral("+60"), m_activeTextColor);
		}
	}
}

void OGLDisplayPanel::renderSMeterB() {

    //GLint width = m_smeterRect.horizontalAdvance();
	GLint width = m_sMeterWidth;
	GLint height = m_smeterRect.height();

	//GLint x1 = m_smeterRect.left();
	GLint x1 = m_smeterRect.left() + m_sMeterOffset;
	GLint y1 = m_smeterRect.top();
	GLint x2 = x1 + width;
	GLint y2 = y1 + height;

	QFontMetrics fm = m_oglTextNormal->fontMetrics();

	int vertexArrayLength = width/2;

	// draw background
	if (m_dataEngineState == QSDR::DataEngineUp)
		drawGLRect(QRect(0, 0, x2-x1, y2-y1), Qt::black, m_bkgColor2, -3.0f, false);
	else
		drawGLRect(QRect(0, 0, x2-x1, y2-y1), Qt::black);

	glDisable(GL_MULTISAMPLE);
	glDisable(GL_LINE_SMOOTH);
	glLineWidth(1.0f);

	// Draw horizontal lines
	if (m_dataEngineState == QSDR::DataEngineUp)
		qglColor(m_activeTextColor);
	else
		qglColor(m_inactiveTextColor);

	glBegin(GL_LINES);
		glVertex3f(0,		m_sMeterPosY, 0.0);
		glVertex3f(width-1,	m_sMeterPosY, 0.0);
		glVertex3f(0,		m_sMeterPosY + 12, 0.0);
		glVertex3f(width-1,	m_sMeterPosY + 12, 0.0);
	glEnd();

	if (m_dataEngineState == QSDR::DataEngineUp)
		qglColor(QColor(100, 100, 100));
	else
		qglColor(m_inactiveTextColor);

	TGL3float *vertexArray = new TGL3float[width];

	for (int i = 0; i < vertexArrayLength; i++) {

		vertexArray[2*i].x = (GLfloat)(2.0f * i);
		vertexArray[2*i].y = (GLfloat)(m_sMeterPosY + 3);
		vertexArray[2*i].z = 0.0;

		vertexArray[2*i+1].x = (GLfloat)(2.0f * i);
		vertexArray[2*i+1].y = (GLfloat)(m_sMeterPosY + 10);
		vertexArray[2*i+1].z = 0.0;
	}

	glEnableClientState(GL_VERTEX_ARRAY);
				
	glVertexPointer(3, GL_FLOAT, 0, vertexArray);
	glDrawArrays(GL_LINES, 0, width);
	glDisableClientState(GL_VERTEX_ARRAY);

	delete[] vertexArray;

	// Draw the S1..S9 value items
	//int d;
    //int markerSpacing = 24; // spacing for the S value items
	//int markerSpacing = m_sMeterWidth/12.5f; // spacing for the S value items
	float markerSpacing = qRound(m_sMeterWidth/12.5f); // spacing for the S value items
	//DISPLAYPANEL_DEBUG << "S markerSpacing:" << markerSpacing;

	QString marker;

	if (m_dataEngineState == QSDR::DataEngineUp)
		qglColor(m_activeTextColor);
	else
		qglColor(m_inactiveTextColor);

	//for (int x = 21, y = 9, z = 0; z < 5; x += markerSpacing, y += markerSpacing, z++) {
	for (int x = markerSpacing - 3, y = x - markerSpacing/2, z = 0; z < 5; x += markerSpacing, y += markerSpacing, z++) {
		
		if (z == 0) marker = "S1";
		else if (z == 1) marker = "S3";
		else if (z == 2) marker = "S5";
		else if (z == 3) marker = "S7";
		else if (z == 4) marker = "S9";
		
		// big ticks
		glBegin(GL_LINES);
			glVertex3f(x, m_sMeterPosY + 12, 0.0);
			glVertex3f(x, m_sMeterPosY + 16, 0.0);
		glEnd();
		
		// small ticks
		glBegin(GL_LINES);
			glVertex3f(y, m_sMeterPosY + 12, 0.0);
			glVertex3f(y, m_sMeterPosY + 15, 0.0);
		glEnd();

		// S strings
        //d = fm.horizontalAdvance(marker);
		m_oglTextNormal->renderText(x-7, m_sMeterPosY + 18, marker);
	}

	// Draw the S+ value items
	//markerSpacing = 20;
	markerSpacing = qRound(m_sMeterWidth/15.0f);
	//DISPLAYPANEL_DEBUG << "S+ markerSpacing:" << markerSpacing;

	for (int x = 118 + markerSpacing, y = 128 + markerSpacing, z = 0; z < 8; x += markerSpacing, y += 2 * markerSpacing, z++) {
	//for (int x = next + markerSpacing, y = next + markerSpacing/2, z = 0; z < 8; x += markerSpacing, y += 2 * markerSpacing, z++) {
		
		if (m_dataEngineState == QSDR::DataEngineUp)
			qglColor(m_activeTextColor);
		else
			qglColor(m_inactiveTextColor);

		if (z == 0) marker = "+20";
		else if (z == 1) marker = "+40";
		else if (z == 2) marker = "+60";
		else if (z == 3) marker = "+80";
		
		// big ticks
		glBegin(GL_LINES);
			glVertex3f(x, m_sMeterPosY + 12, 0.0);
			glVertex3f(x, m_sMeterPosY + 16, 0.0);
		glEnd();
		
		if (m_dataEngineState == QSDR::DataEngineUp)
			qglColor(QColor(255, 80, 80));
		else
			qglColor(QColor(68, 68, 68));

		m_oglTextNormal->renderText(y, m_sMeterPosY + 18, marker);
	}

	// Draw the dbm items
	if (m_dataEngineState == QSDR::DataEngineUp)
			qglColor(m_activeTextColor);
		else
			qglColor(m_inactiveTextColor);

	//for (int x = 4, y = 14, z = -130; z < 10; x += markerSpacing, y += markerSpacing, z += 10) {
	for (int x = markerSpacing - 16, y = x + markerSpacing/2, z = -130; z < 10; x += markerSpacing, y += markerSpacing, z += 10) {
		
		marker = QString::number(z, 'f', 0);
        int d = fm.horizontalAdvance(marker);
		
		// big ticks
		glBegin(GL_LINES);
			glVertex3f(x, m_sMeterPosY - 4, 0.0);
			glVertex3f(x, m_sMeterPosY, 0.0);
		glEnd();

		// small ticks
		glBegin(GL_LINES);
			glVertex3f(y, m_sMeterPosY - 2, 0.0);
			glVertex3f(y, m_sMeterPosY, 0.0);
		glEnd();
		
		if (z == -120 || z == -100 || z == -80 || z == -60 || z == -40 || z == -20) 
			m_oglTextNormal->renderText(x-d/2-2, m_sMeterPosY - 18, marker);

		if (z == 0) m_oglTextNormal->renderText(x-d/2-1, m_sMeterPosY - 18, marker);
	}

	renderPanelText(m_oglTextSmallItalic,width - 25, m_sMeterPosY - 16, "dBm");

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_MULTISAMPLE);
}

void OGLDisplayPanel::setSMeterValue(int rx, double value) {

	Q_UNUSED(rx)
		float tmp;

        // Automatic offset alignment: HPSDR (Hermes/Metis) hardware uses a +140dB offset
        // to convert WDSP's raw dB values to dBm. Generic SoapySDR hardware (like RTL-SDR)
        // typically has a much lower noise floor relative to full scale.
        // A +90dB offset aligns the S-Meter with the visual Panadapter floor for Soapy mode.
        const float offset = (set->getHWInterface() == QSDR::SoapySDR) ? 90.0f : 140.0f;
		tmp = (float)value + offset;

		if (m_sMeterTimer.elapsed() > 40) {

			if (tmp < m_sMeterMinValueB) m_sMeterMinValueB = tmp;

			if (tmp > m_sMeterMaxValueB) m_sMeterMaxValueB = tmp;

			int elapsedTimeMax = m_sMeterMaxTimer.elapsed();
			if (elapsedTimeMax > m_sMeterHoldTime) {

				if (m_sMeterPrevHoldTimeMax <= 0)
					m_sMeterPrevHoldTimeMax = m_sMeterHoldTime;

				// slowly reduce the peak hold level (taken from SDRMAX3 by (c) Cathy Moss)
				m_sMeterMaxValueB -= (float)(elapsedTimeMax - m_sMeterPrevHoldTimeMax) / 15;
				m_sMeterPrevHoldTimeMax = elapsedTimeMax;

				if ((qRound(m_sMeterMaxValueB) <= qRound(tmp)) || (m_sMeterMaxValueB <= tmp)) {

					m_sMeterMaxValueB = tmp;
					m_sMeterMaxTimer.restart();
					m_sMeterPrevHoldTimeMax = 0;
				}
			}

			int elapsedTimeMin = m_sMeterMinTimer.elapsed();
			if (elapsedTimeMin > m_sMeterHoldTime) {

				if (m_sMeterPrevHoldTimeMin <= 0)
					m_sMeterPrevHoldTimeMin = m_sMeterHoldTime;

				// slowly increase the minimum hold level (taken from SDRMAX3 by (c) Cathy Moss)
				m_sMeterMinValueB += (float)(elapsedTimeMin - m_sMeterPrevHoldTimeMin) / 15;
				m_sMeterPrevHoldTimeMin = elapsedTimeMin;

				if ((qRound(m_sMeterMinValueB) >= qRound(tmp)) || (m_sMeterMinValueB >= tmp)) {

					m_sMeterMinValueB = tmp;
					m_sMeterMinTimer.restart();
					m_sMeterPrevHoldTimeMin = 0;
				}
			}

		m_sMeterValue = tmp * 0.13f + m_sMeterValue * 0.87f;

			if (m_sMeterDisplayTime.elapsed() > 200) {

                m_sMeterOrgValue = tmp - 130.0f;

				m_sMeterDisplayTime.restart();
			}
			//m_sMeterOrgValue = value - 37.7f;

		}
		update();
}


void OGLDisplayPanel::setupDisplayRegions(QSize size) {

    int width  = size.width();
	int height = size.height();

	m_sMeterWidth = (int)(0.8f*(width - m_rxRectWidth));

	if (m_sMeterWidth < 300) m_sMeterWidth = 300;
	if (m_sMeterWidth > 600) m_sMeterWidth = 600;

	int d = 320 - m_sMeterWidth;
	if (d > 0)
		m_sMeterOffset = (int)(width - m_rxRectWidth - m_sMeterWidth)/2.0f;
	else
		m_sMeterOffset = width - m_rxRectWidth - m_sMeterWidth - 40;

	//m_sMeterOffset = 0;
		
	m_rect = QRect(0, 0, width, height);
	m_rxRect = QRect(0, 0, m_rxRectWidth, height);
	m_smeterRect = QRect(m_rxRect.right() + m_sMeterOffset, 0, width - m_rxRectWidth, height);

	//DISPLAYPANEL_DEBUG << "m_sMeterWidth:" << m_sMeterWidth;
	//DISPLAYPANEL_DEBUG << "            d:" << d;
	//DISPLAYPANEL_DEBUG << "m_smeterRectWidth:" << m_smeterRect.width();
	//DISPLAYPANEL_DEBUG << "m_sMeterOffset:" << m_sMeterOffset;

	int x = m_rxRect.left() + 20;
	int y = m_rxRect.top() + m_freqDigitsPosY;

	m_freg1000000000 = QRegion(QRect(x, y -  m_fonts.fontHeightFreqFont1, m_blankWidthf1, m_fonts.fontHeightFreqFont1));
    x += m_blankWidthf1;
    m_point2         = QRegion(QRect(x, y -  m_fonts.fontHeightFreqFont1, m_pointStringWidth, m_fonts.fontHeightFreqFont1));
    x += m_pointStringWidth;
    m_freg100000000  = QRegion(QRect(x, y -  m_fonts.fontHeightFreqFont1, m_blankWidthf1, m_fonts.fontHeightFreqFont1));
    x += m_blankWidthf1;
	m_freg10000000	 = QRegion(QRect(x, y -  m_fonts.fontHeightFreqFont1, m_blankWidthf1, m_fonts.fontHeightFreqFont1));
    x += m_blankWidthf1;
	m_freg1000000	 = QRegion(QRect(x, y -  m_fonts.fontHeightFreqFont1, m_blankWidthf1, m_fonts.fontHeightFreqFont1));
    x += m_blankWidthf1;
    m_point =           QRegion(QRect(x, y -  m_fonts.fontHeightFreqFont1, m_pointStringWidth, m_fonts.fontHeightFreqFont1));
    x += m_pointStringWidth;
    m_freg100000	 = QRegion(QRect(x, y -  m_fonts.fontHeightFreqFont1, m_blankWidthf1, m_fonts.fontHeightFreqFont1));
    x += m_blankWidthf1;
    m_freg10000 	 = QRegion(QRect(x, y -  m_fonts.fontHeightFreqFont1 , m_blankWidthf1, m_fonts.fontHeightFreqFont1));
	x += m_blankWidthf1;
	m_freg1000 		 = QRegion(QRect(x, y -  m_fonts.fontHeightFreqFont1 , m_blankWidthf1, m_fonts.fontHeightFreqFont1));
    x += m_blankWidthf1;
    m_point1         = QRegion(QRect(x, y -  m_fonts.fontHeightFreqFont1, m_pointStringWidth, m_fonts.fontHeightFreqFont1));
    x += m_pointStringWidth;
    m_freg100 		 = QRegion(QRect(x, y -  m_fonts.fontHeightFreqFont2, m_blankWidthf2, m_fonts.fontHeightFreqFont2));
	x += m_blankWidthf2;
	m_freg10 		 = QRegion(QRect(x, y -  m_fonts.fontHeightFreqFont2 , m_blankWidthf2, m_fonts.fontHeightFreqFont2));
	x += m_blankWidthf2;
	m_freg1 		 = QRegion(QRect(x, y -  m_fonts.fontHeightFreqFont2 , m_blankWidthf2, m_fonts.fontHeightFreqFont2));

	m_smeterRenew = true;
}

void OGLDisplayPanel::getSelectedDigit(QPoint p) {

    static int pos;
    m_digitPosition = None;

	if (m_freg1.contains(p))
		m_digitPosition = Freq1;
	else 
	if (m_freg10.contains(p))
		m_digitPosition = Freq10;
	else 
	if (m_freg100.contains(p))
		m_digitPosition = Freq100;
	else 
    if (m_point1.contains(p))
        m_digitPosition = dp2;
    else
	if (m_freg1000.contains(p))
		m_digitPosition = Freq1000;
	else 
	if (m_freg10000.contains(p))
		m_digitPosition = Freq10000;
	else 
	if (m_freg100000.contains(p))
        m_digitPosition = Freq100000;
    else
    if (m_point.contains(p))
        m_digitPosition = dp1;
    else
	if (m_freg1000000.contains(p))
		m_digitPosition = Freq1000000;
	else 
	if (m_freg10000000.contains(p))
		m_digitPosition = Freq10000000;
    else
    if (m_freg100000000.contains(p))
        m_digitPosition = Freq100000000;
    else
    if (m_point2.contains(p))
        m_digitPosition = dp0;
    else
    if (m_freg1000000000.contains(p))
        m_digitPosition = Freq1000000000;

    // Check if the selected position is suppressed
    if (m_digitPosition != None) {
        if (m_digitPosition <= Freq1000) { // In m_f1str
            int idx = -1;
            switch(m_digitPosition) {
                case Freq1000000000: idx = 0; break;
                case dp0:            idx = 1; break;
                case Freq100000000:  idx = 2; break;
                case Freq10000000:   idx = 3; break;
                case Freq1000000:    idx = 4; break;
                case dp1:            idx = 5; break;
                case Freq100000:     idx = 6; break;
                case Freq10000:      idx = 7; break;
                case Freq1000:       idx = 8; break;
                default: break;
            }
            if (idx >= 0 && idx < m_f1str.length() && m_f1str[idx] == ' ') {
                m_digitPosition = None;
            }
        }
    }

    if (pos != m_digitPosition){
        pos=m_digitPosition;
        update();
     }
}

//***********************************************
void OGLDisplayPanel::enterEvent(QEvent *event) {

	Q_UNUSED(event)
}

void OGLDisplayPanel::leaveEvent(QEvent *event) {

	Q_UNUSED(event)
}

void OGLDisplayPanel::mousePressEvent(QMouseEvent *event) {

	QPoint pos = event->pos();
	
	//if (m_serverMode != QSDR::ExternalDSP) {
		getSelectedDigit(pos);

        if (event->button() == Qt::LeftButton && m_digitPosition != None) {
			if (m_currentReceiver < 0 || m_currentReceiver >= m_frequencyList.size()) {
				qWarning() << "OGLDisplayPanel::mousePressEvent invalid receiver index" << m_currentReceiver;
				return;
			}

			qint64 currentFreq = m_frequencyList[m_currentReceiver].frequency;
            FrequencyEntryDialog dlg(currentFreq, this);
            if (dlg.exec() == QDialog::Accepted) {
                qint64 newFreq = dlg.frequency();
                if (newFreq < (qint64)set->getMaxFrequency() && newFreq >= 0) {
                    // mode 1 in setCtrFrequency sets both Center AND VFO to the same value,
                    // which effectively zeroes the NCO offset.
                    set->setCtrFrequency(1, m_currentReceiver, newFreq);
                }
            }
            return;
        }

		switch (m_digitPosition) {

			case Freq1:
				if (event->buttons() == Qt::LeftButton) {
					if (set->getMouseWheelFreqStep(m_currentReceiver) == 1.0)
						set->setMouseWheelFreqStep(m_currentReceiver, 5.0);
					else
						set->setMouseWheelFreqStep(m_currentReceiver, 1.0);
				}
				break;

			case Freq10:
				if (event->buttons() == Qt::LeftButton) {
					if (set->getMouseWheelFreqStep(m_currentReceiver) == 10.0)
						set->setMouseWheelFreqStep(m_currentReceiver, 50.0);
					else
						set->setMouseWheelFreqStep(m_currentReceiver, 10.0);
				}
				break;

			case Freq100:
				if (event->buttons() == Qt::LeftButton) {
					if (set->getMouseWheelFreqStep(m_currentReceiver) == 100.0)
						set->setMouseWheelFreqStep(m_currentReceiver, 500.0);
					else
						set->setMouseWheelFreqStep(m_currentReceiver, 100.0);
				}
				break;
	
			case Freq1000:
				if (event->buttons() == Qt::LeftButton) {
					if (set->getMouseWheelFreqStep(m_currentReceiver) == 1000.0)
                        set->setMouseWheelFreqStep(m_currentReceiver, 5000.0);
                    else if (set->getMouseWheelFreqStep(m_currentReceiver) == 5000.0)
                        set->setMouseWheelFreqStep(m_currentReceiver, 9000.0);
                    else
						set->setMouseWheelFreqStep(m_currentReceiver, 1000.0);
				}
				break;

			case Freq10000:
				if (event->buttons() == Qt::LeftButton) {
					if (set->getMouseWheelFreqStep(m_currentReceiver) == 10000.0)
						set->setMouseWheelFreqStep(m_currentReceiver, 50000.0);
					else
						set->setMouseWheelFreqStep(m_currentReceiver, 10000.0);
				}
				break;
	
			case Freq100000:
				if (event->buttons() == Qt::LeftButton) {
					if (set->getMouseWheelFreqStep(m_currentReceiver) == 100000.0)
						set->setMouseWheelFreqStep(m_currentReceiver, 500000.0);
					else
						set->setMouseWheelFreqStep(m_currentReceiver, 100000.0);
				}
				break;

			case Freq1000000:
				if (event->buttons() == Qt::LeftButton) {
					if (set->getMouseWheelFreqStep(m_currentReceiver) == 1000000.0)
						set->setMouseWheelFreqStep(m_currentReceiver, 5000000.0);
					else
						set->setMouseWheelFreqStep(m_currentReceiver, 1000000.0);
				}
				break;

			case Freq10000000:
				if (event->buttons() == Qt::LeftButton) {
					if (set->getMouseWheelFreqStep(m_currentReceiver) == 10000000.0)
						set->setMouseWheelFreqStep(m_currentReceiver, 50000000.0);
					else
						set->setMouseWheelFreqStep(m_currentReceiver, 10000000.0);
				}
				break;

            case Freq100000000:
                if (event->buttons() == Qt::LeftButton)
                    set->setMouseWheelFreqStep(m_currentReceiver, 100000000.0);
                break;

            case Freq1000000000:
                if (event->buttons() == Qt::LeftButton)
                    set->setMouseWheelFreqStep(m_currentReceiver, 1000000000.0);
                break;

			case None:
				return;
		}

	//}

	QWidget::mousePressEvent(event);
}

void OGLDisplayPanel::mouseReleaseEvent(QMouseEvent *event) {

	Q_UNUSED(event)
}

void OGLDisplayPanel::mouseMoveEvent(QMouseEvent *event) {

	QPoint pos = event->pos();

	QColor oldDigitColor = m_digitColor;

    if (m_dataEngineState != QSDR::DataEngineUp)
    {
        m_digitColor = QColor(98, 98, 98);
        return;
    }

		getSelectedDigit(pos);
		switch (m_digitPosition) {

			case Freq1:
				setCursor(Qt::PointingHandCursor);
                    m_digitColor = QColor(136, 166, 178);
                break;

			case Freq10:
				setCursor(Qt::PointingHandCursor);
                    m_digitColor = QColor(136, 166, 178);
                break;

			case Freq100:
				setCursor(Qt::PointingHandCursor);
                    m_digitColor = QColor(136, 166, 178);
                break;

			case Freq1000:
				setCursor(Qt::PointingHandCursor);
                    m_digitColor = QColor(136, 166, 178);
                break;

			case Freq10000:
				setCursor(Qt::PointingHandCursor);
                    m_digitColor = QColor(136, 166, 178);
                break;

			case Freq100000:
				setCursor(Qt::PointingHandCursor);
                    m_digitColor = QColor(136, 166, 178);
                break;

			case Freq1000000:
				setCursor(Qt::PointingHandCursor);
					m_digitColor = QColor(136, 166, 178);
			break;

			case Freq10000000:
				setCursor(Qt::PointingHandCursor);
					m_digitColor = QColor(136, 166, 178);
				break;

			case Freq100000000:
			setCursor(Qt::PointingHandCursor);
			m_digitColor = QColor(136, 166, 178);
			break;

			case Freq1000000000:
			setCursor(Qt::PointingHandCursor);
			m_digitColor = QColor(136, 166, 178);
			break;

			case None:

				setCursor(Qt::ArrowCursor);
					m_digitColor = QColor(106, 136, 148);

				break;
		}
	//}

    if (oldDigitColor != m_digitColor) update();

	QOpenGLWidget::mouseMoveEvent(event);
}

void OGLDisplayPanel::wheelEvent(QWheelEvent * event) {
		qint64 deltaF = 0;
		switch (m_digitPosition) {
			case Freq1:
				deltaF = 1;
				break;

			case Freq10:
				deltaF = 10;
				break;

			case Freq100:
				deltaF = 100;
				break;

			case Freq1000:
				deltaF = 1000;
				break;

			case Freq10000:
				deltaF = 10000;
				break;

			case Freq100000:
				deltaF = 100000;
				break;

			case Freq1000000:
				deltaF = 1000000;
				break;

			case Freq10000000:
				deltaF = 10000000;
				break;

            case Freq100000000:
                deltaF = 100000000;
                break;

            case Freq1000000000:
                deltaF = 1000000000;
                break;

			case None:
				return;
		}

        int  numDegrees = event->angleDelta().y()/ 8;
        int  numSteps = numDegrees / 15;
		
		if (m_currentReceiver < 0 || m_currentReceiver >= m_frequencyList.size()) {
			qWarning() << "OGLDisplayPanel::wheelEvent invalid receiver index" << m_currentReceiver;
			return;
		}

		qint64 currentFreq = m_frequencyList[m_currentReceiver].frequency;
        qint64 newFreq = currentFreq + (qint64)numSteps * deltaF;

		if (newFreq < (qint64)set->getMaxFrequency() && newFreq >= 0) {

			if (set->getPanLockedStatus(m_currentReceiver)) {

				qint64 ctrf = set->getCtrFrequency(m_currentReceiver);
				int s = set->getSampleRate()/2;
				if (newFreq > ctrf + s)
					newFreq = ctrf + s;
				else if (newFreq < ctrf - s)
					newFreq = ctrf - s;

                set->setVFOFrequency(0, m_currentReceiver, newFreq);
			}
            else {
                set->setCtrFrequency(0, m_currentReceiver, newFreq);
                set->setVFOFrequency(0, m_currentReceiver, newFreq);
            }
		}
	event->accept();
	QOpenGLWidget::wheelEvent(event);
}

void OGLDisplayPanel::keyPressEvent(QKeyEvent* event) {

	Q_UNUSED(event)
}

void OGLDisplayPanel::closeEvent(QCloseEvent *event) {

	Q_UNUSED(event)
}
/*
void OGLDisplayPanel::showEvent(QShowEvent *event) {

	Q_UNUSED(event)
}
*/

void OGLDisplayPanel::timerEvent(QTimerEvent *event) {

	Q_UNUSED(event)
}

void OGLDisplayPanel::setSMeterHoldTime(int value) {

	m_sMeterHoldTime = value;
}

void OGLDisplayPanel::setSyncStatus(int value) {

	m_syncStatus = value;

    //QElapsedTimer::singleShot(50, this, SLOT(updateSyncStatus()));
}

void OGLDisplayPanel::updateSyncStatus() {

	if (m_dataEngineState == QSDR::DataEngineUp)
		m_syncStatus = 1;
	else
		m_syncStatus = 0;

}

void OGLDisplayPanel::setADCStatus(int value) {

	m_adcStatus = value;

    QTimer::singleShot(500, this, &OGLDisplayPanel::updateADCStatus);
}

void OGLDisplayPanel::updateADCStatus() {

	if (m_dataEngineState == QSDR::DataEngineUp)
		m_adcStatus = 1;
	else
		m_adcStatus = 0;

}

void OGLDisplayPanel::setPacketLossStatus(int value) {

	m_packetLossStatus = value;
    QTimer::singleShot(100, this, &OGLDisplayPanel::updatePacketLossStatus);
}

void OGLDisplayPanel::updatePacketLossStatus() {

	if (m_dataEngineState == QSDR::DataEngineUp)
		m_packetLossStatus = 1;
	else
		m_packetLossStatus = 0;
}

void OGLDisplayPanel::setForwardPower(qreal watts) {

	m_fwdPowerWatts = watts;
	update();
}

void OGLDisplayPanel::setSWR(qreal swr) {

    m_swr = swr;
    update();
}

void OGLDisplayPanel::setRadioState(RadioState state) {

    m_txActive = (state == RadioState::MOX || state == RadioState::TUNE);
    update();
}

void OGLDisplayPanel::setSupplyVoltage(qreal volts) {
    m_supplyVolts = volts;
    update();
}

void OGLDisplayPanel::setTemperature(qreal temp) {
    m_temperature = temp;
    update();
}

void OGLDisplayPanel::setRigCtlStatus(bool active) {
    m_rigCtlConnected = active;
    update();
}

void OGLDisplayPanel::setSendIQStatus(int value) {

	m_sendIQStatus = value;
}

void OGLDisplayPanel::setRecvAudioStatus(int value) {

	m_recvAudioStatus = value;
}

void OGLDisplayPanel::setReceivers(int value) {

	m_receivers = value;
}

void OGLDisplayPanel::setSampleRate(int value) {

	m_sample_rate = value / 1000;
}

void OGLDisplayPanel::setMercuryAttenuator(HamBand band,int value) {

	Q_UNUSED (band)

	m_mercuryAttenuator = value;
}

void OGLDisplayPanel::setDither(int value) {

	m_dither = value;
}

void OGLDisplayPanel::setRandom(int value) {

	m_random = value;
}

void OGLDisplayPanel::setCurrentReceiver(int value) {
	if (value < 0 || value >= m_frequencyList.size()) {
		qWarning() << "OGLDisplayPanel::setCurrentReceiver invalid index" << value
				   << "list size" << m_frequencyList.size();
		return;
	}

	m_currentReceiver = value;
}

void OGLDisplayPanel::setFrequency(int mode,int rx, qint64 freq) {

	Q_UNUSED (mode)
	//Q_UNUSED (rx)

	//m_oldFreq = freq;

	TFrequency f;
	f.frequency = freq;
	f.freqMHz = (int)(freq / 1000);
	f.freqkHz = (int)(freq % 1000);

	//frequency1 = (int)(freq / 1000);
	//frequency2 = (int)(freq % 1000);

	if (rx < 0) {
		qWarning() << "OGLDisplayPanel::setFrequency invalid rx" << rx;
		return;
	}

	if (rx >= MAX_RECEIVERS) {
		qWarning() << "OGLDisplayPanel::setFrequency out-of-range rx" << rx;
		return;
	}

	if (rx >= m_frequencyList.size()) {
		m_frequencyList.resize(rx + 1);
	}

	m_frequencyList[rx] = f;
	update();

}

void OGLDisplayPanel::set10mhzSource(int value) {

	switch (value) {
		case 0:
			m_src10mhz = "Atlas";
			break;
		case 1:
			m_src10mhz = "Penelope";
			break;
		case 2:
			m_src10mhz = "Mercury";
			break;
	}
}

void OGLDisplayPanel::set122_88mhzSource(int value) {

	switch (value) {
		case 0:
			m_src122_88mhz = "Penelope";
			break;
		case 1:
			m_src122_88mhz = "Mercury";
			break;
	}
}

void OGLDisplayPanel::setMercuryVersion(int value) {

	m_mercuryVersion.setNum(value/10);
	m_mercuryVersion.append(".");
	QString str;
	m_mercuryVersion.append(str.setNum(value%10));
}

void OGLDisplayPanel::setPenelopeVersion(int value) {

	m_penelopeVersion.setNum(value/10);
	m_penelopeVersion.append(".");
	QString str;
	m_penelopeVersion.append(str.setNum(value%10));
}

void OGLDisplayPanel::setPennylaneVersion(int value) {

	m_pennylaneVersion.setNum(value/10);
	m_pennylaneVersion.append(".");
	QString str;
	m_pennylaneVersion.append(str.setNum(value%10));
}

void OGLDisplayPanel::setHermesVersion(int value) {

	m_hermesVersion.setNum(value/10);
	m_hermesVersion.append(".");
	QString str;
	m_hermesVersion.append(str.setNum(value%10));
}

void OGLDisplayPanel::setMetisVersion(int value) {

	QString str;
	switch (m_hwInterface) {

		case QSDR::Metis:

			m_metisVersion.setNum(value/10);
			m_metisVersion.append(".");
			m_metisVersion.append(str.setNum(value%10));
			break;

		case QSDR::Hermes:
		case QSDR::NoInterfaceMode:
    default:
			break;
	}
	
}

void OGLDisplayPanel::setExcaliburVersion(int value) {

	m_excaliburVersion.setNum(value/10);
	m_excaliburVersion.append(".");
	QString str;
	m_excaliburVersion.append(str.setNum(value%10));
}

void OGLDisplayPanel::setAlexVersion(int value) {

	m_alexVersion.setNum(value/10);
	m_alexVersion.append(".");
	QString str;
	m_alexVersion.append(str.setNum(value%10));
}

void OGLDisplayPanel::setMouseWheelFreqStep(int rx, qreal value) {

	if (rx == m_currentReceiver)
		m_mouseWheelFreqStep = value;

}

void OGLDisplayPanel::systemStateChanged(
    QSDR::_Error err, 
	QSDR::_HWInterfaceMode hwmode, 
	QSDR::_ServerMode mode, 
	QSDR::_DataEngineState state)
{
	Q_UNUSED (err)

	//m_mutex.lock();
	if (m_serverMode != mode) {

		m_serverMode = mode;

	}

	if (m_hwInterface != hwmode)
		m_hwInterface = hwmode;

	if (m_dataEngineState != state)
		m_dataEngineState = state;

	if (state == QSDR::DataEngineDown) {

		m_sMeterMaxValueB = -1000.0f;
		m_sMeterMinValueB =  1000.0f;

		m_sMeterTimer.restart();
		m_sMeterDisplayTime.restart();
		m_sMeterMaxTimer.restart();
		m_sMeterMinTimer.restart();

		if (m_radioModel && m_radioModel->telemetry())
			m_radioModel->telemetry()->setProtocolSync(0);

        QTimer::singleShot(50, this, &OGLDisplayPanel::updateADCStatus);
        QTimer::singleShot(50, this, &OGLDisplayPanel::updateSyncStatus);
        QTimer::singleShot(50, this, &OGLDisplayPanel::updatePacketLossStatus);

		//resizeGL(width(), height());
	}
    else if (state == QSDR::DataEngineUp) {

        resizeGL(width(), height());
    }

	m_smeterUpdate = true;
	m_smeterRenew = true;

}


void OGLDisplayPanel::qglColor(QColor color)
{
    m_glTextColor = color;
    glColor4f(color.redF(), color.greenF(), color.blueF(), color.alphaF());
}

void OGLDisplayPanel::renderPanelText(OGLText *text, float x, float y, const QString &str)
{
    if (text)
        text->renderText(x, y, str, m_glTextColor);
}

void OGLDisplayPanel::renderPanelText(OGLText *text, float x, float y, float z, const QString &str)
{
    if (text)
        text->renderText(x, y, z, str, m_glTextColor);
}


void OGLDisplayPanel::saveGLState() {

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
}

void OGLDisplayPanel::restoreGLState() {

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();
}


void OGLDisplayPanel::renderFreqText(QPainter &painter, GLint &x1, GLint &y1,QFont &font,QFontMetrics fontMetrics, QColor fontcolor, const QString freqstr,int digit,int digit_pos, int fixed_width) {
    int len = freqstr.length();
    QColor freqdigitcolor;


    for (int x = 0; x < len; x++) {

        int current_pos = x + digit;
        bool isDot = (current_pos == dp0 || current_pos == dp1 || current_pos == dp2);

        if (set->getRadioState() > RadioState::RX) {
            freqdigitcolor = m_txdigitColor;
        } else

        if (current_pos == digit_pos)
            freqdigitcolor = QColor(106, 236, 248);
        else freqdigitcolor = fontcolor;

        if (freqstr.at(x) != ' ') {
            renderText(painter,x1, y1, font, freqdigitcolor, freqstr.at(x));
        }

        if (isDot) {
            x1 += m_pointStringWidth;
        } else if (fixed_width > 0) {
            x1 += fixed_width;
        } else {
            x1 += (fontMetrics.horizontalAdvance(freqstr.at(x)));
        }
    }
}


void OGLDisplayPanel::renderText(QPainter &painter, int x,int y, QFont &font, QColor fontcolor, const QString &text ) {
        painter.save();
        painter.setPen(fontcolor);
        painter.setFont(font);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawText(int(x) ,int(y), text);
        painter.restore();
        }

