/**
* @file  cusdr_oglDisplayPanel.h
* @brief display panel header file for cuSDR 
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

#ifndef _CUSDR_QGL_DISPLAYPANEL_H
#define _CUSDR_QGL_DISPLAYPANEL_H

#include "cusdr_oglUtils.h"
#include "cusdr_oglInfo.h"
#include "cusdr_settings.h"
#include "cusdr_fonts.h"
#include "cusdr_oglText.h"

#include <QWheelEvent>
#include <QOpenGLWidget>
#include <QtOpenGL/QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMenu>
#include <QMenuBar>


#ifdef LOG_DISPLAYPANEL
#   define DISPLAYPANEL_DEBUG qDebug().nospace() << "DisplayPanel::\t"
#else
#   define DISPLAYPANEL_DEBUG nullDebug()
#endif


class RadioModel;
class OGLDisplayPanel : public QOpenGLWidget, protected QOpenGLFunctions {

    Q_OBJECT

public:
    OGLDisplayPanel(RadioModel *model, QWidget *parent = nullptr);
	~OGLDisplayPanel();
    void renderFreqText(QPainter &painter,GLint &x1, GLint  &y1, QFont &font,QFontMetrics  fontMetrics, QColor fontcolor, const QString freqstr, int digit, int digit_pos);
    void renderText(QPainter &painter, int x, int y, QFont &font, QColor fontcolor, const QString &text);

	QSize minimumSizeHint() const;
	QSize sizeHint() const;

public slots:
	void setSampleRate(int value);
	void setFrequency(int mode, int rx, long freq);

protected:
    void initializeGL();
    void resizeGL(int iWidth, int iHeight);
    void paintGL();
    
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent * event );
	void keyPressEvent(QKeyEvent* event);
	void closeEvent(QCloseEvent *event);
    void timerEvent(QTimerEvent *);
    void qglColor(QColor color);
    void renderPanelText(OGLText *text, float x, float y, const QString &str);
    void renderPanelText(OGLText *text, float x, float y, float z, const QString &str);
    void saveGLState();
    void restoreGLState();

private:
    QOpenGLShaderProgram      *m_shaderProgram;
    QOpenGLShaderProgram      *m_textureProgram;
    QOpenGLBuffer              m_vbo;
    QOpenGLVertexArrayObject   m_vao;

    QMatrix4x4 panelProjection() const;
    void drawPanelRect(const QRect &rect, const QColor &color, float z = 0.0f);
    void drawPanelGradientRect(const QRect &rect, const QColor &c1, const QColor &c2,
                               bool leftToRight, float z = 0.0f);
    void drawSMeterNeedle(const QMatrix4x4 &projection, int x1);
    void drawSMeterScaleLabels(const QMatrix4x4 &projection, int xOffset);

    RadioModel*                             m_radioModel;
	Settings*					set;

    qreal dpr;

	QSDR::_ServerMode			m_serverMode;
	QSDR::_HWInterfaceMode		m_hwInterface;
	QSDR::_DataEngineState		m_dataEngineState;
	QList<TFrequency>			m_frequencyList;

	TPanadapterColors			m_colors;
	TScale						m_dBmScale;

	CFonts		*fonts;
	TFonts		m_fonts;

	QMutex		m_mutex;

    OGLText		*m_oglTextTiny;
	OGLText		*m_oglTextSmall;
	OGLText		*m_oglTextSmallItalic;
	OGLText		*m_oglTextNormal;
	OGLText		*m_oglTextBig;
	OGLText		*m_oglTextBigItalic;
	OGLText		*m_oglTextFreq1;
	OGLText		*m_oglTextFreq2;
	OGLText		*m_oglTextImpact;

	QRect		m_rect;
	QRect		m_rxRect;
	QRect		m_smeterRect;
	
	QString		m_src10mhz;
	QString		m_src122_88mhz;
	QString		m_mercuryVersion;
	QString		m_penelopeVersion;
	QString		m_pennylaneVersion;
	QString		m_hermesVersion;
	QString		m_excaliburVersion;
	QString		m_metisVersion;
	QString		m_alexVersion;
	QString		m_mercuryString;
	QString		m_penelopeString;
	QString		m_pennylaneString;
	QString		m_excaliburString;
	QString		m_metisString;
	QString		m_alexString;
	QString		m_hermesString;
	QString		m_hermesStepAttnString;

	QString		m_SYNCString;
	QString		m_ADCString;
	QString		m_PacketLossString;
	QString		m_rigCtlString;
	QString		m_fwdPowerString;
	QString		m_sendIQString;
	QString		m_recvAudioString;
	QString		m_AttnString;
	QString		m_ditherString;
	QString		m_randomString;
	QString		m_sampleRateString;
	QString		m_modusString;
	QString		m_10MHzString;
	QString		m_12288MHzString;
	QString		m_sMeterNumValueString;

	QString		m_bandText;

	QRegion		m_freg1;
	QRegion		m_freg10;
	QRegion		m_freg100;
	QRegion		m_freg1000;
	QRegion		m_freg10000;
	QRegion		m_freg100000;
	QRegion		m_freg1000000;
	QRegion		m_freg10000000;
    QRegion		m_point;
    QRegion		m_point1;


    QColor      m_txdigitColor;
	QColor		m_digitColor;
	QColor		m_bkgColor1;
	QColor		m_bkgColor2;
	QColor		m_activeTextColor;
	QColor		m_glTextColor;
	QColor		m_inactiveTextColor;
	QColor		m_textBackgroundColor;

    QElapsedTimer		m_sMeterTimer;
    QElapsedTimer		m_sMeterMaxTimer;
    QElapsedTimer		m_sMeterMinTimer;
    QElapsedTimer		m_sMeterDisplayTime;

	enum Region {

		upperRegion,
		lowerRegion,
		rxRegion,
		smeterRegion,
		hpsdrRegion,
		elsewhere,
		out
	};

	enum FreqDigit {
        Freq10000000,
        Freq1000000,
        dp1,
        Freq100000,
        Freq10000,
        Freq1000,
        dp2,
        Freq100,
        Freq10,
        Freq1,
        None,


    };

	GLuint	m_sMeterTex;
	bool	m_smeterUpdate;
	bool	m_smeterRenew;
	bool	m_sMeterAvg;


	long	m_oldFreq;

	int		m_height;
	int		m_sMeterWidth;
	int		m_sMeterOffset;
	int		m_rxRectWidth;
	int		m_lowerRectY;
	int		m_upperRectY;
    int		m_digitPosition = None;
	int		m_syncStatus;
	int		m_adcStatus;
	int		m_packetLossStatus;
	qreal	m_fwdPowerWatts = 0.0;
    bool    m_txActive = false;
    bool    m_rigCtlConnected = false;
    qreal   m_swr = 1.0;
    qreal   m_supplyVolts = 0.0;
    qreal   m_temperature = 0.0;
	int		m_sendIQStatus;
	int		m_recvAudioStatus;
	int		m_receivers;
	int		m_sample_rate;
	int		m_mercuryAttenuator;
	int		m_dither;
	int		m_random;
	int		m_currentReceiver;
	
	int		m_pointStringWidth;
	int		m_blankWidth;
	int		m_blankWidthf;
	int		m_blankWidthf1;
	int		m_blankWidthf2;
	int		m_fUnitStringWidth;
	int		m_blankHeight;
	int		m_freqStringLeftPos;
	int		m_versionStringWidth;
	int		m_syncWidth;
	int		m_adcWidth;
	int		m_packetLossWidth;
	int		m_fwdPowerWidth;
	int		m_sendIQWidth;
	int		m_recvAudioWidth;
	int		m_metisStringWidth;
	int		m_mercuryStringWidth;
	int		m_penelopeStringWidth;
	int		m_pennylaneStringWidth;
	int		m_hermesStringWidth;
	int		m_hermesStepAttnStringWidth;
	int		m_alexStringWidth;
	int		m_excaliburStringWidth;
	int		m_rigCtlStringWidth;
	int		m_AttnWidth;
	int		m_ditherWidth;
	int		m_randomWidth;
	int		m_sampleRateWidth;
	int		m_modusWidth;
	int		m_10MHzWidth;
	int		m_sMeterDeform;
	int		m_12288MHzWidth;
	int		m_freqDigitsPosY;
	int		m_sMeterPosY;
	int		m_sMeterHoldTime;
	int		m_sMeterPrevHoldTimeMax;
	int		m_sMeterPrevHoldTimeMin;
	int		m_sMeterMeanValueCnt;
    QOpenGLFramebufferObject * m_smeterFBO =nullptr;

	qreal	m_mouseWheelFreqStep;
	qreal	m_dBmPanMin;
	qreal	m_dBmPanMax;
	qreal	m_unit;
	
	float	m_smeterVertices;
	float	m_sMeterValue;
	float	m_sMeterMeanValue;
	float	m_sMeterOrgValue;
	float	m_sMeterMaxValueA;
	float	m_sMeterMinValueA;
	float	m_sMeterMaxValueB;
	float	m_sMeterMinValueB;

	//*************************
	void	setupConnections();
	void	setupTextstrings();
	void	paintUpperRegion();
	void	paintLowerRegion();
	void	paintRxRegion();
	
	void	paintSMeter();
	void	renderSMeterScale();
	void	renderSMeterB();
	
	void	getSelectedDigit(QPoint p);
	
private slots:
	void	systemStateChanged(
				QSDR::_Error err, 
				QSDR::_HWInterfaceMode hwmode, 
				QSDR::_ServerMode mode, 
				QSDR::_DataEngineState state);

	void	setupDisplayRegions(QSize size);

	void	setSyncStatus(int value);
	void	setADCStatus(int value);
	void	setPacketLossStatus(int value);
	void	setForwardPower(qreal watts);
    void	setSWR(qreal swr);
    void	setSupplyVoltage(qreal volts);
    void	setRadioState(RadioState state);
    void	setTemperature(qreal temp);
	void	setSendIQStatus(int value);
	void	setRecvAudioStatus(int value);
	void	setCurrentReceiver(int value);
	void	setMercuryAttenuator(HamBand band, int value);
	void	setReceivers(int value);
	void	setDither(int value);
	void	setRandom(int value);
	void	set10mhzSource(int value);
	void	set122_88mhzSource(int value);

	void	setHermesVersion(int value);
	void	setRigCtlStatus(bool active);
	void	setMercuryVersion(int value);
	void	setPenelopeVersion(int value);
	void 	setPennylaneVersion(int value);
	void	setMetisVersion(int value);
	void	setExcaliburVersion(int value);
	void	setAlexVersion(int value);

	void	setMouseWheelFreqStep(int rx, qreal value);

	void	setSMeterValue(int rx, double value);
	void	setSMeterHoldTime(int value);
	void	updateSyncStatus();
	void	updateADCStatus();
	void	updatePacketLossStatus();

signals:
	void showEvent();
	void closeEvent();
	void messageEvent(QString msg);
};


#endif // _CUSDR_QGL_DISPLAYPANEL_H
