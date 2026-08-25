/**
* @file  cusdr_oglReceiverPanel.h
* @brief receiver panel header file for cuSDR
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2011-03-14
*/

/*
 *   Copyright 2010, 2011 Hermann von Hasseln, DL3HVH
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

#ifndef _CUSDR_QGL_RECEIVERPANEL_H
#define _CUSDR_QGL_RECEIVERPANEL_H

#include "cusdr_oglUtils.h"
#include "cusdr_oglInfo.h"
#include "cusdr_settings.h"
#include "cusdr_fonts.h"
#include "Util/cusdr_buttons.h"
#include "cusdr_oglText.h"
#include "cusdr_radioPopupWidget.h"
#include "WaterfallRenderer.h"
#include "PanadapterRenderer.h"
#include "OverlayRenderer.h"

#include <QWheelEvent>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLPaintDevice>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

class RadioPopupController;


#ifdef LOG_GRAPHICS
#   define GRAPHICS_DEBUG qDebug().nospace() << "ReceiverPanel::\t"
#else
#   define GRAPHICS_DEBUG nullDebug()
#endif


class SliceModel;
class QGLReceiverPanel : public QOpenGLWidget, protected QOpenGLFunctions {

    Q_OBJECT

public:
    QGLReceiverPanel(SliceModel *model, QWidget *parent = nullptr);
	~QGLReceiverPanel();

	RadioPopupWidget* getRadioPopupWidget() const { return radioPopup; }

public slots:
	QSize minimumSizeHint() const;
	QSize sizeHint() const;

	//void setSpectrumBuffer(const float* buffer, int size);
	//void setSpectrumBuffer(const qVectorFloat& buffer);
	void setSpectrumBuffer(int rx, const qVectorFloat& buffer);
	void setCtrFrequency(int mode, int rx, qint64 freq);
	void setVFOFrequency(int mode, int rx, qint64 freq);

protected:
    void initializeGL();
    void resizeGL(int iWidth, int iHeight);
    void paintGL();
    
	void enterEvent(QEnterEvent *event) override;
	void leaveEvent(QEvent *event) override;
	void mousePressEvent(QMouseEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent * event );
	void keyPressEvent(QKeyEvent* event);
	void qglColor(QColor color);

private:
    SliceModel*                             m_sliceModel;
    Settings*                                       set;

	QSDR::_ServerMode			m_serverMode;
	QSDR::_HWInterfaceMode		m_hwInterface;
	QSDR::_DataEngineState		m_dataEngineState;

	CFonts*						fonts;
	TFonts						m_fonts;

	RadioPopupWidget*			radioPopup;
	RadioPopupController*       radioPopupController = nullptr;
	ADCMode						m_adcMode;
	AGCMode						m_agcMode;
	DSPMode						m_dspMode;
	PanGraphicsMode				m_panMode;
	WaterfallColorMode			m_waterfallMode;

    QElapsedTimer						m_displayTime;
    QElapsedTimer						m_resizeTime;
    QElapsedTimer						freqChangeTimer;

	QString						m_bandText;
	QString						m_adcModeString;
	QString						m_agcModeString;
	QString						m_dspModeString;
	QString						m_filterWidthString;	

	TScale						m_frequencyScale;
	TScale						m_dBmScale;
	TScale						m_secScale;
	
	QVector<qreal>					m_panadapterBins;
	QVector<qreal>					m_panPeakHoldBins;
	QVarLengthArray<float>			m_waterfallPixel;

	QQueue<QVector<float> >			specAv_queue;

	QOpenGLFramebufferObject*	m_frequencyScaleFBO;
	QOpenGLFramebufferObject*	m_dBmScaleFBO;
	QOpenGLFramebufferObject*	m_secScaleWaterfallFBO;

    QOpenGLShaderProgram      *m_shaderProgram;
    QOpenGLShaderProgram      *m_textureProgram;
    QOpenGLBuffer              m_vbo;
    QOpenGLVertexArrayObject   m_vao;

    QMatrix4x4 panelProjection() const;
    qreal displayedFrequencySpanHz() const;
    float displayedZoomFactor() const;
    void ensurePanelViewport();
    void syncTextDevicePixelRatio();
    void drawPanelRect(const QRect &rect, const QColor &color, float z = 0.0f);
    void renderPanelText(OGLText *text, float x, float y, const QString &str);
    void renderPanelText(OGLText *text, float x, float y, float z, const QString &str);
    void drawCachedTexture(const QRect &rect, GLuint texId, float z = 0.0f);

    QColor m_glTextColor;
    qreal m_panelDpr;

	QRect						m_panRect;
	QRect						m_bandPlanRect;
	QRect						m_dBmScalePanRect;
	QRect						m_freqScalePanRect;
	QRect						m_waterfallRect;
	QRect						m_secScaleWaterfallRect;
	QRect						m_filterRect;
	QRect						m_agcButtonRect;
	QRect						m_lockedPanButtonRect;
	QRect						m_vfoToMidButtonRect;
	QRect						m_midToVfoButtonRect;
	QRect						m_clickVFOButtonRect;
	QRect						m_cwTextRect;
	bool						m_dragCwText = false;
	QPoint						m_cwDragStartMouse;
	bool						m_hasCustomCwBoxPos = false;
	QPoint						m_cwBoxPos;
	
	OGLText*					m_oglTextTiny;
	OGLText*					m_oglTextSmall;
	OGLText*					m_oglTextNormal;
	OGLText*					m_oglTextFreq1;
	OGLText*					m_oglTextFreq2;
	OGLText*					m_oglTextBig1;
	OGLText*					m_oglTextBig2;
	OGLText*					m_oglTextHuge;

	QPoint						m_mousePos;
	QPoint						m_oldMousePos;
	QPoint						m_mouseLastPos;
	QPoint						m_mouseDownPos;
	QPoint						m_rulerMouseDownPos;
	QPoint						m_cameraAngle;

	QColor						m_waterfallLoColor;
	QColor						m_waterfallHiColor;
	QColor						m_waterfallMidColor;
	QColor						m_gridColor;
	QColor						m_darkColor;
	
	QMutex						mutex;
	QMutex						spectrumBufferMutex;
	QVector<float>				m_cachedSpectrumBuffer;
    qreal                       dpr;
    QPainter                    painter;
	enum Region {

		freqScalePanadapterRegion,
		panadapterRegion,
		dBmScalePanadapterRegion,
		waterfallRegion,
		filterRegion,
		filterRegionLow,
		filterRegionHigh,
		agcButtonRegion,
		agcThresholdLine,
		agcHangLine,
		agcFixedGainLine,
		//lockedPanButtonRegion,
		//vfoToMidButtonRegion,
		//midToVfoButtonRegion,
		//clickVfoButtonRegion,
		elsewhere,
		out
	};
    
	GLint		m_panRectWidth;
	GLint		m_panSpectrumBinsLength;

	GLint		m_filterLeft;
	GLint		m_filterRight;
	GLint		m_filterTop;
	GLint		m_filterBottom;

	GLfloat		m_agcThresholdPixel;
	GLfloat		m_agcHangLevelPixel;
	GLfloat		m_agcFixedGainLevelPixel;

	GLfloat		m_bkgRed;
	GLfloat		m_bkgGreen;
	GLfloat		m_bkgBlue;

	GLfloat		m_red;
	GLfloat		m_green;
	GLfloat		m_blue;

	GLfloat		m_redF;
	GLfloat		m_greenF;
	GLfloat		m_blueF;


	GLfloat		m_redST;
	GLfloat		m_greenST;
	GLfloat		m_blueST;

	GLfloat		m_redSB;
	GLfloat		m_greenSB;
	GLfloat		m_blueSB;

	GLfloat		m_redGrid;
	GLfloat		m_greenGrid;
	GLfloat		m_blueGrid;
	
    WaterfallRenderer* m_waterfallRenderer;
    PanadapterRenderer* m_panadapterRenderer;
    OverlayRenderer* m_overlayRenderer;

    int			m_bigHeight;
	int			m_bigWidth;
	int			m_receiver;
	//int			m_frequencyRxOnRx;
	int			m_spectrumSize;
	int			m_sampleSize;
	int			m_oldSampleSize;
	int			m_oldWidth;
	int			m_oldPanRectHeight;
	int			m_specAveragingCnt;
	int			m_currentReceiver;
	int			m_waterfallAlpha;
	int			m_waterfallOffsetLo;
	int			m_waterfallOffsetHi;
	int			m_freqRulerDisplayWidth;
	int			m_displayTop;
	int			m_dBmPanLogGain;
	int			m_panSpectrumMinimumHeight;
	int			m_mouseRegion;
	int			m_oldMouseRegion;
	int			m_oldMousePosX;
	int			m_snapMouse;
	int			m_panDisplayMode;
	int			m_sampleRate;
	int			m_downRate;
	int			m_mercuryAttenuator;
	int			m_haircrossOffsetRight;
	int			m_haircrossOffsetLeft;
	int			m_haircrossMaxRight;
	int			m_haircrossMinTop;
	int			m_displayCenterlineHeight;
	int			m_adcStatus;
	int			m_fps;
	int			m_filterWidth;
	int			m_fftMult;

	long		m_centerFrequency;
	long		m_vfoFrequency;
	long		m_deltaFrequency;
	long		m_otherFrequency;
	//long		m_oldFreq;

	bool		m_smallSize;
	bool		m_spectrumVertexColorUpdate;
	bool		m_dBmScalePanadapterRenew;
	bool		m_dBmScalePanadapterUpdate;
	bool		m_freqScalePanadapterRenew;
	bool		m_freqScalePanadapterUpdate;
	bool		m_secScaleWaterfallUpdate;
	bool		m_secScaleWaterfallRenew;
	bool		m_panGridRenew;
	bool		m_panGridUpdate;
	bool		m_waterfallDisplayUpdate;
	bool		m_spectrumColorsChanged;
	bool		m_spectrumAveraging;
	//bool		m_spectrumAveragingOld;
	bool		m_crossHair;
	bool		m_crossHairCursor;
	bool		m_panGrid;
	bool		m_peakHold;
	bool		m_peakHoldBufferResize;
	bool		m_filterChanged;
	bool		m_showFilterLeftBoundary;
	bool		m_showFilterRightBoundary;
	bool		m_highlightFilter;
	bool		m_showAGCLines;
	bool		m_agcHangEnabled;
	bool		m_dragMouse;
	bool		m_dragDBmScale;
	bool		m_dragFreqScale;
	bool		m_dragFreqScaleZoom;
	bool		m_panLocked;
	bool		m_clickVFO;
	
	qreal		m_yScaleFactor;
	qreal		m_panFrequencyScale;
	qreal		m_freqScaleZoomFactor;
	qreal		m_dBmPanMin;
	qreal		m_dBmPanMax;
	qreal		m_dBmPanDelta;
	qreal		m_mouseWheelFreqStep;
	qreal		m_secWaterfallMin;
	qreal		m_secWaterfallMax;
	qreal		m_panScale;
	qreal		m_scaleMult;
	qreal		m_scaleMultOld;
	qreal		m_filterLowerFrequency;
	qreal		m_filterUpperFrequency;
	qreal		m_mouseDownFilterFrequencyLo;
	qreal		m_mouseDownFilterFrequencyHi;
	qreal		m_filterLo;
	qreal		m_filterHi;
	qreal		m_deltaF;
	qreal		m_agcThresholdNew;
	qreal		m_agcThresholdOld;
	qreal		m_mouseDownAGCThreshold;
	qreal		m_agcHangLevelNew;
	qreal		m_agcHangLevelOld;
	qreal		m_mouseDownAGCHangLevel;
	qreal		m_agcFixedGain;
	qreal		m_mouseDownFixedGainLevel;

	float		m_scale;
	float		m_cameraDistance;
	float		m_freqRulerPosition;
	

	//******************************************************************
	void	setupConnections();

	void	saveGLState();
	void	restoreGLState();

	// drawing
	void	paintReceiverDisplay();
	void	paint3DPanadapterMode();

	void	drawPanadapter();
	void	drawBandPlanStrip();
	void 	drawPanVerticalScale();
	void 	drawPanHorizontalScale();
	void 	drawPanadapterGrid();
	void 	drawPanFilter();
	void 	drawCenterLine();
	void 	drawWaterfall();
	void 	drawWaterfallVerticalScale();
	void	drawCrossHair();
	void 	drawReceiverInfo();
	void	drawAGCControl();
	void	drawVFOControl();
	void	drawCwDecoderHUD();

	void 	updateFrequencyRuler();
	void 	updateDBmRuler();
	void 	renderPanVerticalScale();
	void 	renderPanHorizontalScale();
	void 	renderPanadapterGrid();
	void 	renderWaterfallVerticalScale();

	//void	computeDisplayBins(const QVector<float>& panBuffer, const float* waterfallBuffer);
	//void	computeDisplayBins(QVector<float> &buffer);
	void	computeDisplayBins(QVector<float>& panBuffer, QVector<float>& waterfallBuffer);
	void	recomputeDisplayBinsFromCache();
	qint64	findPeakFrequencyNear(qint64 targetFreq, int searchRadiusHz, bool *found = nullptr) const;
	void 	showText(float x, float y, float z, const QString &text, bool smallText);
	void	showRadioPopup(bool value);

private slots:
	void	systemStateChanged(
					QSDR::_Error err, 
					QSDR::_HWInterfaceMode hwmode, 
					QSDR::_ServerMode mode, 
					QSDR::_DataEngineState state);

	void	graphicModeChanged(
					int rx,
					PanGraphicsMode panMode,
					WaterfallColorMode waterfallColorMode);

	void	setSpectrumSize(int value);
	void	setCurrentReceiver(int value);
	void 	setHamBand(int rx, bool byButton, HamBand band);
	void	setFilterFrequencies(int rx, qreal lo, qreal hi);
	void	setMercuryAttenuator(HamBand band, int value);
	void	setupDisplayRegions(QSize size);
	
	void	setSpectrumAveraging(int rx, bool value);
	void	setSpectrumAveragingCnt(int value);
	void	setVfoToMidFrequency();
	void	setMidToVfoFrequency();
	void	setPanGridStatus(bool value, int rx);
	void	setPeakHoldStatus(bool value, int rx);
	void	resetPeakHoldBins();
	void	setPanLockedStatus(bool value, int rx);
	void	setClickVFOStatus(bool value, int rx);
	void	setHairCrossStatus(bool value, int rx);
	void	setPanadapterColors();
	void	getRegion(QPoint p);
	void	freqRulerPositionChanged(int rx, float pos);
	void	sampleRateChanged(int value);
	void	setWaterfallOffesetLo(int rx, int value);
	void	setWaterfallOffesetHi(int rx, int value);
	void	setdBmScaleMin(int rx, qreal value);
	void	setdBmScaleMax(int rx, qreal value);
	void	setMouseWheelFreqStep(int, qreal);

	void 	setADCStatus(int value);
	void 	updateADCStatus();
	void	setFramesPerSecond(int rx, int value);
	void	setDSPMode(int rx, DSPMode mode);
	void	setADCMode(int rx, ADCMode mode);
	void	setAGCMode(int rx, AGCMode mode, bool hangEnabled);
	void 	setAGCLineLevels(int rx, qreal thresh, qreal hang);
	void	setAGCLineFixedLevel(int rx, qreal value);
	void	setAGCLinesStatus(bool value, int rx);


signals:
	void	showEvent();
	void	closeEvent();
	void	messageEvent(QString msg);
	void	coordChanged(int x,int y);
};

#endif  // _CUSDR_QGL_RECEIVERPANEL_H
