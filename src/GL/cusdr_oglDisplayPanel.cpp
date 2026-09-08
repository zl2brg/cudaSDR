#include "Models/RadioModel.h"
#include "Models/RadioTelemetry.h"
#include "Models/SliceModel.h"
#include "Models/BandPlanManager.h"
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
#include "SMeterRenderer.h"
#include "DisplayFreqRenderer.h"
#include "DisplayStatusRenderer.h"
#include "UI/FrequencyEntryDialog.h"
#include "cusdr_glShaders.h"
#include "cusdr_glDraw.h"
#include "Util/cusdr_rigctlserver.h"
#include "Util/cusdr_tciserver.h"

#include <QGuiApplication>
#include <QOpenGLPaintDevice>
#include <QTimer>

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
	, m_oglTextTiny(nullptr)
	, m_oglTextSmall(nullptr)
	, m_oglTextSmallItalic(nullptr)
	, m_oglTextNormal(nullptr)
	, m_oglTextBig(nullptr)
	, m_oglTextBigItalic(nullptr)
	, m_oglTextFreq1(nullptr)
	, m_oglTextFreq2(nullptr)
	, m_oglTextFreqInactive1(nullptr)
	, m_oglTextFreqInactive2(nullptr)
	, m_oglTextImpact(nullptr)
	, m_smeterUpdate(true)
	, m_smeterRenew(true)
	, m_oldFreq(0)
	, m_height(155)
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
	, m_freqDigitsPosYA(48)
	, m_freqDigitsPosYB(95)
	, m_sMeterPosY(50)//(45)
	, m_sMeterHoldTime(model->slices().isEmpty() ? 1000 : model->slices().first()->sMeterHoldTime())
	, m_sMeterPrevHoldTimeMax(0)
	, m_mouseWheelFreqStep(set->getMouseWheelFreqStep(m_currentReceiver))
	, m_dBmPanMin(-130.0f)
	, m_dBmPanMax(10.0f)
	, m_unit(1.0f)
	, m_sMeterValue(0.0f)
    , m_sMeterMaxValueB(-1000.0f)
{
    for (int i = 0; i < MAX_RECEIVERS; i++) {
        m_sMeterAvgValList[i] = 0.0f;
        m_sMeterPeakValList[i] = 0.0f;
        m_sMeterHoldMaxList[i] = 0.0f;
    }
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setAutoFillBackground(false);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    setMouseTracking(true);
    setUpdateBehavior(QOpenGLWidget::PartialUpdate);
    disableVSyncOnNativeWayland(this);
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
	{
		QFont inactive1 = m_fonts.freqFont1;
		inactive1.setPixelSize(18);
		m_oglTextFreqInactive1 = new OGLText(inactive1, dpr);
		QFont inactive2 = m_fonts.freqFont2;
		inactive2.setPixelSize(14);
		m_oglTextFreqInactive2 = new OGLText(inactive2, dpr);
	}
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
	m_oglTextFreqInactive1->invalidateCache();
	m_oglTextFreqInactive2->invalidateCache();
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
	m_sMeterDisplayTime.start();

	m_smeterRenderer = new SMeterRenderer(this);
	m_freqRenderer = new DisplayFreqRenderer(this);
	m_statusRenderer = new DisplayStatusRenderer(this);
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
    delete  m_oglTextFreqInactive1;
    delete  m_oglTextFreqInactive2;
    delete  m_oglTextImpact;
    delete m_oglTextSmallItalic;
    delete m_oglTextNormal;
    delete m_oglTextBig;
    delete m_oglTextTiny;
    delete m_oglTextSmall;

    delete m_smeterRenderer;
    delete m_freqRenderer;
    delete m_statusRenderer;
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
	connect(set, &Settings::dspModeChanged,           this, [this](int rx, DSPMode mode) {
		Q_UNUSED(mode);
		if (rx == m_currentReceiver)
			update();
	});
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
            connect(slice, &SliceModel::sMeterPeakValueChanged, this, [this, slice](double value){ this->setSMeterPeakValue(slice->id(), value); });
            connect(slice, &SliceModel::sMeterHoldTimeChanged, this, &OGLDisplayPanel::setSMeterHoldTime);
            connect(slice, &SliceModel::frequencyChanged, this, [this, slice](long freq){ this->setFrequency(0, slice->id(), freq); });
            connect(slice, &SliceModel::vfoAFrequencyChanged, this, [this](qint64){ scheduleRepaint(); });
            connect(slice, &SliceModel::vfoBFrequencyChanged, this, [this](qint64){ scheduleRepaint(); });
            connect(slice, &SliceModel::activeVfoChanged, this, [this](SliceModel::ActiveVfo){ scheduleRepaint(); });
        }
	connect(set, &Settings::sMeterHoldTimeChanged,    this, &OGLDisplayPanel::setSMeterHoldTime);

	RigCtlServer *rcs = set->rigCtlServer();
	if (rcs)
		connect(rcs, &RigCtlServer::remoteControlChanged, this, &OGLDisplayPanel::setRigCtlStatus);

	TciServer *tci = set->tciServer();
	if (tci) {
		connect(tci, &TciServer::remoteControlChanged, this, &OGLDisplayPanel::setTciStatus);
		setTciStatus(tci->hasClients());
	}
}

void OGLDisplayPanel::setupTextstrings() {

    m_blankWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(".");
    m_blankHeight = m_oglTextSmall->fontMetrics().height()-1;

    m_pointStringWidth = m_oglTextFreq1->fontMetrics().horizontalAdvance(".");
    m_blankWidthf = m_oglTextFreq1->fontMetrics().horizontalAdvance("59.999");
    m_blankWidthf1 = m_oglTextFreq1->fontMetrics().horizontalAdvance("0");
    m_blankWidthf2 = m_oglTextFreq2->fontMetrics().horizontalAdvance("0");
    m_blankWidthInactive1 = m_oglTextFreqInactive1->fontMetrics().horizontalAdvance("0");
    m_blankWidthInactive2 = m_oglTextFreqInactive2->fontMetrics().horizontalAdvance("0");
    m_pointStringWidthInactive = m_oglTextFreqInactive1->fontMetrics().horizontalAdvance(".");
    m_fUnitStringWidth = m_oglTextFreq2->fontMetrics().horizontalAdvance("MHz");
    m_fUnitStringWidthInactive = m_oglTextFreqInactive2->fontMetrics().horizontalAdvance("MHz");
    m_vfoLabelWidth = m_oglTextBig->fontMetrics().horizontalAdvance("B") + 8;

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

	m_tciString = QString("TCI");
	m_tciStringWidth = m_oglTextSmall->fontMetrics().horizontalAdvance(m_tciString);

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

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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
        m_oglTextFreqInactive1->setDevicePixelRatio(dpr);
        m_oglTextFreqInactive2->setDevicePixelRatio(dpr);
        m_oglTextImpact->setDevicePixelRatio(dpr);
        m_smeterUpdate = true;
        m_smeterRenew = true;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	if (m_freqRenderer)
		m_freqRenderer->paintRxRegion();
	if (m_statusRenderer) {
		m_statusRenderer->paintUpperRegion();
		m_statusRenderer->paintLowerRegion();
	}
	if (m_smeterRenderer)
		m_smeterRenderer->paintSMeter();
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
    m_vao.bind();
    if (m_shaderProgram && m_shaderProgram->isLinked())
        GlDraw::drawSolidRect(this, m_shaderProgram, m_vbo, panelProjection(), rect, color, z);
}

// Corners are built from per-scanline insets: drawSolidRect is the only fill primitive here.
void OGLDisplayPanel::drawPanelRoundedRect(const QRect &rect, const QColor &color, int radius, float z)
{
    if (rect.isEmpty())
        return;
    const int r = qBound(0, radius, qMin(rect.width(), rect.height()) / 2);
    drawPanelRect(QRect(rect.left(), rect.top() + r, rect.width(), rect.height() - 2 * r), color, z);

    for (int i = 0; i < r; ++i) {
        const double dy = r - i - 0.5;
        const int dx = int(qRound(r - std::sqrt(qMax(0.0, double(r) * r - dy * dy))));
        const int w = rect.width() - 2 * dx;
        if (w <= 0)
            continue;
        drawPanelRect(QRect(rect.left() + dx, rect.top() + i, w, 1), color, z);
        drawPanelRect(QRect(rect.left() + dx, rect.bottom() - i, w, 1), color, z);
    }
}

void OGLDisplayPanel::drawPanelRoundedRectOutline(const QRect &rect, const QColor &color,
                                                  int radius, float z)
{
    if (rect.isEmpty() || !m_shaderProgram || !m_shaderProgram->isLinked())
        return;

    const float r = float(qBound(0, radius, qMin(rect.width(), rect.height()) / 2));
    // Half-pixel offsets keep the single-pixel outline from straddling two columns/rows.
    const float x1 = float(rect.left()) + 0.5f;
    const float y1 = float(rect.top()) + 0.5f;
    const float x2 = float(rect.right()) + 0.5f;
    const float y2 = float(rect.bottom()) + 0.5f;

    QVector<QPointF> pts;
    const int segments = 4;
    const struct { float cx, cy, a0; } corners[4] = {
        { x1 + r, y1 + r, float(M_PI) },            // top-left
        { x2 - r, y1 + r, 1.5f * float(M_PI) },     // top-right
        { x2 - r, y2 - r, 0.0f },                   // bottom-right
        { x1 + r, y2 - r, 0.5f * float(M_PI) },     // bottom-left
    };
    for (const auto &c : corners) {
        for (int i = 0; i <= segments; ++i) {
            const float a = c.a0 + 0.5f * float(M_PI) * float(i) / float(segments);
            pts.append(QPointF(double(c.cx + r * std::cos(a)), double(c.cy + r * std::sin(a))));
        }
    }

    const float cr = float(color.redF()), cg = float(color.greenF());
    const float cb = float(color.blueF()), ca = float(color.alphaF());
    QVector<GlDraw::Vec3Rgba> verts;
    verts.reserve(pts.size() * 2);
    for (int i = 0; i < pts.size(); ++i) {
        const QPointF &a = pts.at(i);
        const QPointF &b = pts.at((i + 1) % pts.size());
        verts.append({ float(a.x()), float(a.y()), z, cr, cg, cb, ca });
        verts.append({ float(b.x()), float(b.y()), z, cr, cg, cb, ca });
    }

    m_vao.bind();
    glLineWidth(1.0f);
    GlDraw::drawColoredRgbaLines(this, m_shaderProgram, m_vbo, panelProjection(),
                                verts.constData(), verts.size());
}

void OGLDisplayPanel::drawPanelGradientRect(const QRect &rect,
                                          const QColor &c1,
                                          const QColor &c2,
                                          bool leftToRight,
                                          float z)
{
    if (rect.isEmpty())
        return;
    m_vao.bind();
    if (m_shaderProgram && m_shaderProgram->isLinked())
        GlDraw::drawGradientRect(this, m_shaderProgram, m_vbo, panelProjection(), rect, c1, c2, leftToRight, z);
}

void OGLDisplayPanel::setSMeterValue(int rx, double value) {
	if (rx < 0 || rx >= MAX_RECEIVERS)
		return;

	const float offset = (set->getHWInterface() == QSDR::SoapySDR) ? 90.0f : 140.0f;
	const float tmp = (float)value + offset;

	// Fast-attack, smooth-decay analog meter ballistics per receiver
	if (tmp > m_sMeterAvgValList[rx]) {
		m_sMeterAvgValList[rx] = tmp * 0.80f + m_sMeterAvgValList[rx] * 0.20f;
	} else {
		m_sMeterAvgValList[rx] = tmp * 0.15f + m_sMeterAvgValList[rx] * 0.85f;
	}

	if (rx == m_currentReceiver) {
		m_sMeterValue = m_sMeterAvgValList[rx];
		if (m_sMeterDisplayTime.elapsed() > 60) {
			m_sMeterOrgValue = (set->getHWInterface() == QSDR::SoapySDR) ? (tmp - 90.0f) : (tmp - 140.0f);
			m_sMeterDisplayTime.restart();
		}
		scheduleRepaint();
	}
}

void OGLDisplayPanel::setSMeterPeakValue(int rx, double value) {
	if (rx < 0 || rx >= MAX_RECEIVERS)
		return;

	const float offset = (set->getHWInterface() == QSDR::SoapySDR) ? 90.0f : 140.0f;
	const float tmp = (float)value + offset;
	m_sMeterPeakValList[rx] = tmp;

	if (tmp > m_sMeterHoldMaxList[rx]) {
		m_sMeterHoldMaxList[rx] = tmp;
		if (rx == m_currentReceiver) {
			m_sMeterMaxTimer.restart();
			m_sMeterPrevHoldTimeMax = 0;
		}
	}

	if (rx == m_currentReceiver) {
		const int elapsedTimeMax = m_sMeterMaxTimer.elapsed();
		if (elapsedTimeMax > m_sMeterHoldTime) {
			if (m_sMeterPrevHoldTimeMax <= 0)
				m_sMeterPrevHoldTimeMax = m_sMeterHoldTime;

			m_sMeterHoldMaxList[rx] -= (float)(elapsedTimeMax - m_sMeterPrevHoldTimeMax) / 15.0f;
			m_sMeterPrevHoldTimeMax = elapsedTimeMax;

			if (m_sMeterHoldMaxList[rx] <= tmp) {
				m_sMeterHoldMaxList[rx] = tmp;
				m_sMeterMaxTimer.restart();
				m_sMeterPrevHoldTimeMax = 0;
			}
		}
		m_sMeterMaxValueB = m_sMeterHoldMaxList[rx];
	}
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

	rebuildAllFreqDigitHitRegions();

	m_smeterRenew = true;
}

QString OGLDisplayPanel::freqMhzDisplayString(qint64 frequencyHz) const
{
	if (m_freqRenderer)
		return m_freqRenderer->freqMhzDisplayString(frequencyHz);
	return QString();
}

void OGLDisplayPanel::splitFreqDisplay(qint64 frequencyHz, QString *f1str, QString *f2str) const
{
	if (m_freqRenderer)
		m_freqRenderer->splitFreqDisplay(frequencyHz, f1str, f2str);
}

SliceModel *OGLDisplayPanel::currentSlice() const
{
	if (!m_radioModel)
		return nullptr;
	if (m_currentReceiver < 0 || m_currentReceiver >= m_radioModel->slices().size())
		return nullptr;
	return m_radioModel->slices().at(m_currentReceiver);
}

qint64 OGLDisplayPanel::vfoMemoryHz(DigitVfo which) const
{
	if (SliceModel *slice = currentSlice()) {
		return (which == DigitVfoB) ? slice->vfoBFrequency() : slice->vfoAFrequency();
	}
	if (m_currentReceiver >= 0 && m_currentReceiver < m_frequencyList.size())
		return m_frequencyList.at(m_currentReceiver).frequency;
	return 7000000;
}

void OGLDisplayPanel::activateDigitVfo(DigitVfo which)
{
	SliceModel *slice = currentSlice();
	if (!slice || which == DigitVfoNone)
		return;
	const SliceModel::ActiveVfo target = (which == DigitVfoB) ? SliceModel::VfoB : SliceModel::VfoA;
	if (slice->activeVfo() == target)
		return;
	slice->setActiveVfo(target);
	set->setVfoFrequencyVisible(m_currentReceiver, slice->frequency());
}

void OGLDisplayPanel::tuneDigitVfoTo(DigitVfo which, qint64 frequencyHz)
{
	activateDigitVfo(which);
	SliceModel *slice = currentSlice();
	if (!slice)
		return;

	if (which == DigitVfoB)
		slice->setVfoBFrequency(frequencyHz);
	else
		slice->setVfoAFrequency(frequencyHz);

	if (set->getPanLockedStatus(m_currentReceiver)) {
		qint64 ctrf = set->getCtrFrequency(m_currentReceiver);
		const int s = set->getSampleRate() / 2;
		if (frequencyHz > ctrf + s)
			frequencyHz = ctrf + s;
		else if (frequencyHz < ctrf - s)
			frequencyHz = ctrf - s;
		set->setVFOFrequency(0, m_currentReceiver, frequencyHz);
	} else {
		// Unlocked pan: digit wheel moves LO with the dial (legacy behaviour).
		set->setCtrFrequency(0, m_currentReceiver, frequencyHz);
		set->setVFOFrequency(0, m_currentReceiver, frequencyHz);
	}
}

QRect OGLDisplayPanel::vfoLabelRect(int yBaseline) const
{
	const QFontMetrics fm = m_oglTextBig->fontMetrics();
	return QRect(m_rxRect.left() + 4, yBaseline - fm.ascent() - 2,
	             qMax(m_vfoLabelWidth, 16), fm.height() + 4);
}

void OGLDisplayPanel::rebuildAllFreqDigitHitRegions()
{
	if (!m_oglTextFreq1 || !m_oglTextFreqInactive1)
		return;

	SliceModel *slice = currentSlice();
	const bool bActive = slice && slice->activeVfo() == SliceModel::VfoB;
	const int originX = m_rxRect.left() + 12 + m_vfoLabelWidth;

	const QRect labelA = vfoLabelRect(m_rxRect.top() + m_freqDigitsPosYA);
	const QRect labelB = vfoLabelRect(m_rxRect.top() + m_freqDigitsPosYB);

	QString f1A = m_f1strA;
	QString f1B = m_f1strB;
	if (f1A.isEmpty())
		f1A = freqMhzDisplayString(vfoMemoryHz(DigitVfoA));
	if (f1B.isEmpty())
		f1B = freqMhzDisplayString(vfoMemoryHz(DigitVfoB));

	updateFreqDigitHitRegions(m_hitA, originX, m_rxRect.top() + m_freqDigitsPosYA,
	                          f1A, !bActive, labelA);
	updateFreqDigitHitRegions(m_hitB, originX, m_rxRect.top() + m_freqDigitsPosYB,
	                          f1B, bActive, labelB);
}

void OGLDisplayPanel::updateFreqDigitHitRegions(FreqDigitHitRegions &out, int originX, int yBaseline,
                                                const QString &f1str, bool large,
                                                const QRect &labelRect)
{
	OGLText *text1 = large ? m_oglTextFreq1 : m_oglTextFreqInactive1;
	OGLText *text2 = large ? m_oglTextFreq2 : m_oglTextFreqInactive2;
	const int digitW1 = large ? m_blankWidthf1 : m_blankWidthInactive1;
	const int digitW2 = large ? m_blankWidthf2 : m_blankWidthInactive2;
	const int pointW = large ? m_pointStringWidth : m_pointStringWidthInactive;

	const int freq1Ascent = text1 ? text1->fontMetrics().ascent() : m_fonts.fontHeightFreqFont1;
	const int freq2Ascent = text2 ? text2->fontMetrics().ascent() : m_fonts.fontHeightFreqFont2;
	const int freq1Height = text1 ? text1->fontMetrics().height() : m_fonts.fontHeightFreqFont1;
	const int freq2Height = text2 ? text2->fontMetrics().height() : m_fonts.fontHeightFreqFont2;
	const int y1 = yBaseline - freq1Ascent;
	const int y2 = yBaseline - freq2Ascent;

	out.label = QRegion(labelRect);

	int x = originX;
	auto slot = [&](int strIndex, int width, int top, int height) {
		if (strIndex >= 0 && strIndex < f1str.length() && f1str.at(strIndex) == QLatin1Char(' '))
			return QRegion();
		const QRegion region(QRect(x, top, width, height));
		x += width;
		return region;
	};

	out.freg1000000000 = slot(0, digitW1, y1, freq1Height);
	out.point2         = slot(1, pointW, y1, freq1Height);
	out.freg100000000  = slot(2, digitW1, y1, freq1Height);
	out.freg10000000   = slot(3, digitW1, y1, freq1Height);
	out.freg1000000    = slot(4, digitW1, y1, freq1Height);
	out.point          = slot(5, pointW, y1, freq1Height);
	out.freg100000     = slot(6, digitW1, y1, freq1Height);
	out.freg10000      = slot(7, digitW1, y1, freq1Height);
	out.freg1000       = slot(8, digitW1, y1, freq1Height);
	out.point1         = slot(-1, pointW, y1, freq1Height);
	out.freg100        = slot(-1, digitW2, y2, freq2Height);
	out.freg10         = slot(-1, digitW2, y2, freq2Height);
	out.freg1          = slot(-1, digitW2, y2, freq2Height);
}

bool OGLDisplayPanel::hitTestDigit(const FreqDigitHitRegions &regs, const QString &f1str,
                                   QPoint p, int *digitOut) const
{
	int digit = None;
	if (regs.freg1.contains(p))
		digit = Freq1;
	else if (regs.freg10.contains(p))
		digit = Freq10;
	else if (regs.freg100.contains(p))
		digit = Freq100;
	else if (regs.point1.contains(p))
		digit = dp2;
	else if (regs.freg1000.contains(p))
		digit = Freq1000;
	else if (regs.freg10000.contains(p))
		digit = Freq10000;
	else if (regs.freg100000.contains(p))
		digit = Freq100000;
	else if (regs.point.contains(p))
		digit = dp1;
	else if (regs.freg1000000.contains(p))
		digit = Freq1000000;
	else if (regs.freg10000000.contains(p))
		digit = Freq10000000;
	else if (regs.freg100000000.contains(p))
		digit = Freq100000000;
	else if (regs.point2.contains(p))
		digit = dp0;
	else if (regs.freg1000000000.contains(p))
		digit = Freq1000000000;

	if (digit != None && digit <= Freq1000) {
		int idx = -1;
		switch (digit) {
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
		if (idx >= 0 && idx < f1str.length() && f1str[idx] == ' ')
			digit = None;
	}

	if (digitOut)
		*digitOut = digit;
	return digit != None;
}

void OGLDisplayPanel::getSelectedDigit(QPoint p) {

	static int pos;
	static int posVfo;
	m_digitPosition = None;
	m_digitVfo = DigitVfoNone;

	int digit = None;
	if (hitTestDigit(m_hitA, m_f1strA, p, &digit)) {
		m_digitPosition = digit;
		m_digitVfo = DigitVfoA;
	} else if (hitTestDigit(m_hitB, m_f1strB, p, &digit)) {
		m_digitPosition = digit;
		m_digitVfo = DigitVfoB;
	} else if (m_hitA.label.contains(p)) {
		m_digitVfo = DigitVfoA;
	} else if (m_hitB.label.contains(p)) {
		m_digitVfo = DigitVfoB;
	}

	if (pos != m_digitPosition || posVfo != m_digitVfo) {
		pos = m_digitPosition;
		posVfo = m_digitVfo;
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

	getSelectedDigit(pos);

	if (event->button() == Qt::LeftButton && m_digitVfo != DigitVfoNone) {
		// Single click on A or B label/digits — select VFO A or B as active VFO
		activateDigitVfo(static_cast<DigitVfo>(m_digitVfo));
	}

	if (event->button() == Qt::LeftButton && m_digitPosition != None) {
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
				break;
		}
	}

	QWidget::mousePressEvent(event);
}

void OGLDisplayPanel::mouseReleaseEvent(QMouseEvent *event) {

	Q_UNUSED(event)
}

void OGLDisplayPanel::mouseDoubleClickEvent(QMouseEvent *event) {

	if (event->button() == Qt::LeftButton) {
		QPoint pos = event->pos();
		getSelectedDigit(pos);

		if (m_digitVfo != DigitVfoNone) {
			if (m_currentReceiver < 0 || m_currentReceiver >= m_frequencyList.size()) {
				qWarning() << "OGLDisplayPanel::mouseDoubleClickEvent invalid receiver index" << m_currentReceiver;
				return;
			}

			const DigitVfo which = static_cast<DigitVfo>(m_digitVfo);
			activateDigitVfo(which);
			qint64 currentFreq = vfoMemoryHz(which);
			FrequencyEntryDialog dlg(currentFreq, this);
			if (dlg.exec() == QDialog::Accepted) {
				qint64 newFreq = dlg.frequency();
				if (newFreq < (qint64)set->getMaxFrequency() && newFreq >= 0)
					tuneDigitVfoTo(which, newFreq);
			}
			return;
		}
	}

	QOpenGLWidget::mouseDoubleClickEvent(event);
}

void OGLDisplayPanel::mouseMoveEvent(QMouseEvent *event) {

	QPoint pos = event->pos();
	const int oldDigit = m_digitPosition;
	const int oldVfo = m_digitVfo;

    if (m_dataEngineState != QSDR::DataEngineUp)
    {
        m_digitColor = QColor(98, 98, 98);
        return;
    }

		getSelectedDigit(pos);
		Qt::CursorShape wantCursor = Qt::ArrowCursor;
		switch (m_digitPosition) {

			case Freq1:
			case Freq10:
			case Freq100:
			case Freq1000:
			case Freq10000:
			case Freq100000:
			case Freq1000000:
			case Freq10000000:
			case Freq100000000:
			case Freq1000000000:
				wantCursor = Qt::PointingHandCursor;
				m_digitColor = QColor(136, 166, 178);
				break;

			case None:
				wantCursor = (m_digitVfo != DigitVfoNone) ? Qt::PointingHandCursor : Qt::ArrowCursor;
				m_digitColor = QColor(106, 136, 148);
				break;
		}

		if (cursor().shape() != wantCursor)
			setCursor(wantCursor);

	// Highlight comes from m_digitPosition in renderFreqText — only repaint on change.
	if (oldDigit != m_digitPosition || oldVfo != m_digitVfo)
		scheduleRepaint();

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

		const DigitVfo which = (m_digitVfo == DigitVfoB) ? DigitVfoB : DigitVfoA;
		qint64 currentFreq = vfoMemoryHz(which);
        qint64 newFreq = currentFreq + (qint64)numSteps * deltaF;

		if (newFreq < (qint64)set->getMaxFrequency() && newFreq >= 0)
			tuneDigitVfoTo(which, newFreq);

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

void OGLDisplayPanel::scheduleRepaint()
{
	// Hermes telemetry can fire power/SWR/volts/temp in one status frame — each used
	// to call update() and flash every QOpenGLWidget. Coalesce to ≤ ~20 FPS.
	if (m_repaintPending)
		return;
	m_repaintPending = true;
	const int delayMs = qMax(0, 50 - static_cast<int>(m_sMeterTimer.elapsed()));
	QTimer::singleShot(delayMs, this, [this]() {
		m_repaintPending = false;
		m_sMeterTimer.restart();
		update();
	});
}

void OGLDisplayPanel::setForwardPower(qreal watts) {
	m_fwdPowerWatts = watts;
	if (!m_txActive) {
		m_fwdPowerWattsSmooth = 0.0;
		m_txMetersArmed = false;
		// RX ADC noise must not repaint the strip every C&C frame.
		return;
	}

	// Dual-slope IIR smoothing: fast attack (0.5), smooth decay (0.94 / 0.06)
	if (watts > m_fwdPowerWattsSmooth)
		m_fwdPowerWattsSmooth = 0.5 * m_fwdPowerWattsSmooth + 0.5 * watts;
	else
		m_fwdPowerWattsSmooth = 0.94 * m_fwdPowerWattsSmooth + 0.06 * watts;

	// Ignore PA ADC noise; once armed, keep meters up for SSB valleys until RX.
	static constexpr qreal kTxMeterPowerFloorW = 0.1;
	if (!m_txMetersArmed && m_fwdPowerWattsSmooth >= kTxMeterPowerFloorW)
		m_txMetersArmed = true;

	scheduleRepaint();
}

void OGLDisplayPanel::setSWR(qreal swr) {
	m_swr = swr;
	if (!m_txActive) {
		m_swrSmooth = 1.0;
		return;
	}

	if (swr > m_swrSmooth)
		m_swrSmooth = 0.5 * m_swrSmooth + 0.5 * swr;
	else
		m_swrSmooth = 0.94 * m_swrSmooth + 0.06 * swr;

	if (m_txMetersArmed)
		scheduleRepaint();
}

void OGLDisplayPanel::setRadioState(RadioState state) {
	const bool tx = (state == RadioState::MOX || state == RadioState::TUNE);
	if (m_txActive == tx)
		return;
	m_txActive = tx;
	if (!m_txActive) {
		m_fwdPowerWattsSmooth = 0.0;
		m_swrSmooth = 1.0;
		m_txMetersArmed = false;
	}
	scheduleRepaint();
}

void OGLDisplayPanel::setSupplyVoltage(qreal volts) {
	if (qFuzzyCompare(m_supplyVolts, volts))
		return;
    m_supplyVolts = volts;
    scheduleRepaint();
}

void OGLDisplayPanel::setTemperature(qreal temp) {
	if (qFuzzyCompare(m_temperature, temp))
		return;
    m_temperature = temp;
    scheduleRepaint();
}

void OGLDisplayPanel::setRigCtlStatus(bool active) {
	if (m_rigCtlConnected == active)
		return;
    m_rigCtlConnected = active;
    scheduleRepaint();
}

void OGLDisplayPanel::setTciStatus(bool active) {
	if (m_tciConnected == active)
		return;
	m_tciConnected = active;
	scheduleRepaint();
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
	if (value < MAX_RECEIVERS) {
		m_sMeterValue = m_sMeterAvgValList[value];
		m_sMeterMaxValueB = m_sMeterHoldMaxList[value];
		const float offset = (set->getHWInterface() == QSDR::SoapySDR) ? 90.0f : 140.0f;
		m_sMeterOrgValue = (m_sMeterValue > 0.0f) ? (m_sMeterValue - offset) : -140.0f;
	}
	scheduleRepaint();
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

	if (m_frequencyList[rx].frequency == freq)
		return;

	m_frequencyList[rx] = f;
	scheduleRepaint();

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

		m_sMeterTimer.restart();
		m_sMeterDisplayTime.restart();
		m_sMeterMaxTimer.restart();

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


void OGLDisplayPanel::saveGLState()
{
}

void OGLDisplayPanel::restoreGLState()
{
}


void OGLDisplayPanel::renderFreqText(OGLText *text, GLint &x1, GLint y1, const QColor &fontcolor,
                                     const QString &freqstr, int digit, int digit_pos, int fixed_width)
{
	if (m_freqRenderer)
		m_freqRenderer->renderFreqText(text, x1, y1, fontcolor, freqstr, digit, digit_pos, fixed_width);
}

