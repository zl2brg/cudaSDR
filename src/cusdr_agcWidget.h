/*
* @file  cusdr_agcWidget.h
* @brief AGC options widget header file for cuSDR
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2012-09-29
* QT6 update ZL2BRG
*/

#ifndef _CUSDR_AGC_OPTIONS_WIDGET_H
#define _CUSDR_AGC_OPTIONS_WIDGET_H

#include <QWidget>
#include <QPainter>
#include <QComboBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QLabel>
#include <QSlider>

#include "Util/cusdr_buttons.h"
#include "cusdr_settings.h"
#include "cusdr_fonts.h"

class AGCOptionsWidget : public QWidget {
	Q_OBJECT

public:
	AGCOptionsWidget(QWidget *parent = nullptr);
	~AGCOptionsWidget();

	// MVC Setters
	void setAGCMode(AGCMode mode);
	void setAGCSlope(int value);
	void setAGCMaximumGain(int value);
	void setAGCAttackTime(int value);
	void setAGCDecayTime(int value);
	void setAGCHangTime(int value);
	void setAGCFixedGain(int value);
	void setAGCHangThreshold(int value);
	void setReceiver(int rx) { m_currentReceiver = rx; }
	int getReceiver() const { return m_currentReceiver; }

signals:
	// MVC Signals
	void agcModeRequested(int rx, AGCMode mode);
	void agcSlopeRequested(int rx, int value);
	void agcMaximumGainRequested(int rx, int value);
	void agcAttackTimeRequested(int rx, int value);
	void agcDecayTimeRequested(int rx, int value);
	void agcHangTimeRequested(int rx, int value);
	void agcFixedGainRequested(int rx, int value);
	void agcHangThresholdRequested(int rx, int value);

	void	showEvent();
	void	closeEvent();
	void	messageEvent(QString );

public slots:
	QSize	sizeHint() const;
	QSize	minimumSizeHint() const;

	void	systemStateChanged(
					QSDR::_Error err, 
					QSDR::_HWInterfaceMode hwmode, 
					QSDR::_ServerMode mode, 
					QSDR::_DataEngineState state);

private:
	QSDR::_ServerMode			m_serverMode;
	QSDR::_HWInterfaceMode		m_hwInterface;
	QSDR::_DataEngineState		m_dataEngineState;

	AGCMode				m_agcMode;

	CFonts		*fonts;
	TFonts		m_fonts;

	QString			m_menu_style;
	QString			m_callSingText;

	QGroupBox	*agcModeGroupBox;
	QGroupBox	*agcOptionsGroupBox;

	AeroButton	*agcOFF;
	AeroButton	*agcLONG;
	AeroButton	*agcSLOW;
	AeroButton	*agcMED;
	AeroButton	*agcFAST;
	AeroButton	*agcUSER;

	QList<AeroButton *>	agcModeBtnList;

	QLineEdit		*callSignLineEdit;

	QSlider			*m_hangThresholdSlider;

	QSpinBox		*m_slopeSpinBox;
	QSpinBox		*m_maxGainSpinBox;
	QSpinBox		*m_attackTimeSpinBox;
	QSpinBox		*m_decayTimeSpinBox;
	QSpinBox		*m_hangTimeSpinBox;
	QSpinBox		*m_fixedGainSpinBox;

	QLabel			*m_slopeLabel;
	QLabel			*m_maxGainLabel;
	QLabel			*m_attackTimeLabel;
	QLabel			*m_decayTimeLabel;
	QLabel			*m_hangTimeLabel;
	QLabel			*m_fixedGainLabel;
	QLabel			*m_hangThresholdLabel;
	QLabel			*m_hangThresholdValueLabel;

	int		m_minimumWidgetWidth;
	int		m_minimumGroupBoxWidth;
	int		m_btnSpacing;
	int		m_fontHeight;
	int		m_maxFontWidth;
	int		m_currentReceiver;
	int		m_sampleRate;
	int		m_agcHangThreshold;

	bool	m_mouseOver;

	qreal	m_agcMaxGain;
	qreal	m_agcAttackTime;
	qreal	m_agcDecayTime;
	qreal	m_agcHangTime;
	qreal	m_agcFixedGain;

	void	setupConnections();
	void 	createAgcModeBtnGroup();
	void 	createAgcOptionsGroup();

private slots:
	void 	agcModeChangedByBtn();
	void	slopeChanged(int value);
	void	maxGainChanged(int value);
	void	attackTimeChanged(int value);
	void	decayTimeChanged(int value);
	void	hangTimeChanged(int value);
	void	fixedGainChanged(int value);
	void	hangThresholdValueChanged(int value);
};

#endif // _CUSDR_AGC_OPTIONS_WIDGET_H
