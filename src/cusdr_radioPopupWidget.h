/*
* @file cusdr_radioPopupWidget.h
* @brief Radio control popup widget header file for cuSDR
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2012-08-22
* QT6 update ZL2BRG
*/

#ifndef _CUSDR_RADIO_POPUP_WIDGET_H
#define _CUSDR_RADIO_POPUP_WIDGET_H

#ifdef LOG_RADIOPOPUP
#define RADIOPOPUP_DEBUG qDebug().nospace() << "RadioPopup::\t"
#else
#define RADIOPOPUP_DEBUG nullDebug()
#endif

#include <QtWidgets>
#include <QTabWidget>
#include <QLabel>

#include "Util/cusdr_buttons.h"
#include "cusdr_settings.h"
#include "cusdr_fonts.h"
#include "cusdr_agcWidget.h"
#include "noisefilterwidget.h"

class SliceModel;

class RadioPopupWidget : public QWidget {
	Q_OBJECT

public:
	RadioPopupWidget(SliceModel *model, QWidget *parent = 0);
	~RadioPopupWidget();

	bool getSpectrumAveraging() { return m_spectrumAveraging; }
	bool getPanGridStatus()		{ return m_panGrid; }
	bool getPeakHoldStatus()	{ return m_peakHold; }
	AGCOptionsWidget* agcOptionsWidget() const { return m_popupAgcWidget; }
	NoiseFilterWidget* noiseFilterWidget() const { return m_noiseFilterWidget; }

	// MVC View Interface Setters
	void setSingleAdcDevice(bool single);
	void setBandFrequencyList(const QList<THamBandFrequencies>& list);
	void setHamBand(HamBand band);
	void setDSPModeList(const QList<DSPMode>& list);
	void setDSPMode(DSPMode mode);
	void setCtrFrequency(qint64 frequency);
	void setVfoFrequency(qint64 frequency);
	void setADCMode(ADCMode mode);
	void setAGCMode(AGCMode mode);
	void setDefaultFilterMode(TDefaultFilterMode mode);
	void setFilterFrequencies(qreal low, qreal high);
	void setSpectrumAveraging(bool enabled);
	void setPanGrid(bool enabled);
	void setPeakHold(bool enabled);
	void setPanLocked(bool enabled);
	void setClickVFO(bool enabled);
	void setHairCross(bool enabled);
	void setPanadapterMode(PanGraphicsMode mode);
	void setWaterfallColorMode(WaterfallColorMode mode);
	void setLastFrequencies(const QList<qint64>& ctrFreqs, const QList<qint64>& vfoFreqs);
	void setFreeDVMode(int mode);
	void setFreeDVStatus(bool sync, float snr, quint64 rxFrames, quint64 txFrames);
	void setAGCShowLines(bool enabled);
	int getReceiver() const { return m_receiver; }

signals:
	// MVC View Interface Signals
	void hamBandRequested(int rx, HamBand band);
	void vfoFrequencyRequested(int rx, qint64 val);
	void freeDVModeRequested(int rx, int mode);
	void dspModeRequested(int rx, DSPMode mode);
	void filterFrequenciesRequested(int rx, qreal low, qreal high);
	void adcModeRequested(int rx, ADCMode mode);
	void agcModeRequested(int rx, AGCMode mode);
	void agcShowLinesRequested(int rx, bool enabled);
	void spectrumAveragingRequested(int rx, bool enabled);
	void panGridRequested(int rx, bool enabled);
	void peakHoldRequested(int rx, bool enabled);
	void panLockedRequested(int rx, bool enabled);
	void clickVFORequested(int rx, bool enabled);
	void hairCrossRequested(int rx, bool enabled);
	void graphicsStateRequested(int rx, PanGraphicsMode panMode, WaterfallColorMode waterMode);

	void showEvent();
	void hideEvent();
	void closeEvent();
	void newMessage(QString msg);
	void midToVfoBtnEvent();
	void vfoToMidBtnEvent();

public slots:
	QSize	minimumSizeHint() const;

	void systemStateChanged(
			QSDR::_Error err, 
			QSDR::_HWInterfaceMode hwmode, 
			QSDR::_ServerMode mode, 
			QSDR::_DataEngineState state);

	bool showPopupWidget(QPoint position);
		
protected:
	void showEvent(QShowEvent *event);
	void hideEvent(QHideEvent *event);
	void closeEvent(QCloseEvent *event);
	void paintEvent(QPaintEvent *event);
	void resizeEvent(QResizeEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
    void enterEvent(QEnterEvent *event);
    void leaveEvent(QEnterEvent *event);
    bool event(QEvent *event);

private:
    SliceModel*                             m_sliceModel;

	CFonts*					fonts;
	TFonts					m_fonts;

	PanGraphicsMode			m_panadapterMode;
	WaterfallColorMode		m_waterfallColorMode;

	QVBoxLayout*	optionsVBox;
	QVBoxLayout*	bandVBox;
	QVBoxLayout*	adcVBox;
	QVBoxLayout*	modeVBox;
	QVBoxLayout*	agcVBox;

	QWidget*		filterAWidget;
	QWidget*		filterBWidget;
	QWidget*		filterCWidget;

	QStackedLayout*	m_filterStackedLayout;
	QStackedWidget*	m_filterStackedWidget;

	QList<AeroButton *>	filterBtnListA;
	QList<AeroButton *>	filterBtnListB;
	QList<AeroButton *>	filterBtnListC;
	QList<AeroButton *>	panadapterBtnList;
	QList<AeroButton *>	waterfallBtnList;
	
	QLabel*			m_optionsLabel;

	AeroButton*		stickyBtn;

	AeroButton*		lockPanBtn;
	AeroButton*		clickVfoBtn;
	AeroButton*		showCrossBtn;
	AeroButton*		midToVfoBtn;
	AeroButton*		vfoToMidBtn;
	AeroButton*		avgBtn;
	AeroButton*		peakHoldBtn;
	AeroButton*		gridBtn;

	AeroButton*		m_PanLineBtn;
	AeroButton*		m_PanFilledLineBtn;
	AeroButton*		m_PanSolidBtn;
	AeroButton*		m_WaterfallSimpleBtn;
	AeroButton*		m_WaterfallEnhancedBtn;

    AeroButton*		band2200mBtn;
    AeroButton*		band630mBtn;
    AeroButton*		band160mBtn;
    AeroButton*		band80mBtn;
    AeroButton*		band60mBtn;
	AeroButton*		band40mBtn;
	AeroButton*		band30mBtn;
	AeroButton*		band20mBtn;
	AeroButton*		band17mBtn;
	AeroButton*		band15mBtn;
	AeroButton*		band12mBtn;
	AeroButton*		band10mBtn;
	AeroButton*		band6mBtn;
	AeroButton*		band2mBtn;
	AeroButton*		band125cmBtn;
	AeroButton*		band70cmBtn;
	AeroButton*		band33cmBtn;
	AeroButton*		band23cmBtn;
	AeroButton*		band13cmBtn;
	AeroButton*		band10cmBtn;
	AeroButton*		band5cmBtn;
	AeroButton*		bandGenBtn;

	QList<AeroButton *>	bandBtnList;

	AeroButton*		lsbBtn;
	AeroButton*		usbBtn;
	AeroButton*		dsbBtn;
	AeroButton*		cwlBtn;
	AeroButton*		cwuBtn;
	AeroButton*		fmnBtn;
	AeroButton*		amBtn;
	AeroButton*		diguBtn;
	AeroButton*		specBtn;
	AeroButton*		diglBtn;
	AeroButton*		samBtn;
	AeroButton*		drmBtn;
	AeroButton*		dstarBtn = nullptr;
	QComboBox*		m_freeDVModeCombo;
	QLabel*			m_freeDVStatusLabel;

	QList<AeroButton *>	dspModeBtnList;

	AeroButton*		adc1Btn;
	AeroButton*		adc2Btn;

	QList<AeroButton *>	adcModeBtnList;

	AeroButton*		showAGCLines;
	AeroButton*		agcOFF;
	AeroButton*		agcLONG;
	AeroButton*		agcSLOW;
	AeroButton*		agcMED;
	AeroButton*		agcFAST;
	AeroButton*		agcUSER;

	QList<AeroButton *>	agcModeBtnList;
	
	AeroButton*		filter1kBtnA;
	AeroButton*		filter1k8BtnA;
	AeroButton*		filter2k1BtnA;
	AeroButton*		filter2k4BtnA;
	AeroButton*		filter2k7BtnA;
	AeroButton*		filter2k9BtnA;
	AeroButton*		filter3k3BtnA;
	AeroButton*		filter3k8BtnA;
	AeroButton*		filter4k4BtnA;
	AeroButton*		filter5kBtnA;
	AeroButton*		filterVar1BtnA;
	AeroButton*		filterVar2BtnA;

	AeroButton*		filter2k4BtnB;
	AeroButton*		filter2k9BtnB;
	AeroButton*		filter3k1BtnB;
	AeroButton*		filter4kBtnB;
	AeroButton*		filter5k2BtnB;
	AeroButton*		filter6k6BtnB;
	AeroButton*		filter8kBtnB;
	AeroButton*		filter10kBtnB;
	AeroButton*		filter12kBtnB;
	AeroButton*		filter16kBtnB;
	AeroButton*		filterVar1BtnB;
	AeroButton*		filterVar2BtnB;

	AeroButton*		filter25BtnC;
	AeroButton*		filter50BtnC;
	AeroButton*		filter100BtnC;
	AeroButton*		filter250BtnC;
	AeroButton*		filter400BtnC;
	AeroButton*		filter500BtnC;
	AeroButton*		filter600BtnC;
	AeroButton*		filter750BtnC;
	AeroButton*		filter800BtnC;
	AeroButton*		filter1kBtnC;
	AeroButton*		filterVar1BtnC;
	AeroButton*		filterVar2BtnC;
	
	QLabel*			m_rxLabel;

	AGCOptionsWidget*	m_popupAgcWidget;
	NoiseFilterWidget*	m_noiseFilterWidget;
	QTabWidget*			m_popupTabWidget;

	QList<DSPMode>		m_dspModeList;

	HamBand				m_hamBand;
	ADCMode				m_adcMode;
	AGCMode				m_agcMode;
	TDefaultFilterMode	m_filterMode;

	QPoint				m_mouseDownPos;
	QPoint				m_mouseDownWindowPos;

	QList<qint64>       m_lastCtrFrequencyList;
	QList<qint64>       m_lastVfoFrequencyList;
	QList<THamBandFrequencies>  m_bandFrequencyList;

	qint64	m_ctrFrequency;
	qint64	m_vfoFrequency;

	bool	m_sticky;
	bool	m_spectrumAveraging;
	bool	m_panGrid;
	bool	m_peakHold;
	bool	m_panLocked;
	bool	m_clickVFO;
	bool	m_showCross;

	qreal	m_filterLo;
	qreal	m_filterHi;

	int		m_receiver;
	int		m_currentRx;
	int		current_band;
	int		current_dsp_mode;
	bool	m_singleAdcDevice;
	int		m_minimumWidgetWidth;
	int		m_minimumGroupBoxWidth;
    QTimer* m_closeTimer;

	void setupConnections();
	void createBackground(QSize size);
	void updateAdcAvailability();

private slots:
	void	graphicModeChanged(
					int rx,
					PanGraphicsMode panMode,
					WaterfallColorMode waterfallColorMode);

	void setSticky();
	void createOptionsBtnGroup();
	void createBandBtnGroup();
	void createAdcBtnGroup();
	void createModeBtnGroup();
	void createAgcBtnGroup();
	void createFilterBtnWidgetA();
	void createFilterBtnWidgetB();
	void createFilterBtnWidgetC();

	void avgBtnClicked();
	void gridBtnClicked();
	void peakHoldBtnClicked();
	void panLockedBtnClicked();
	void clickVfoBtnClicked();
	void hairCrossBtnClicked();
	void midToVfoBtnClicked();
	void vfoToMidBtnClicked();
	void panModeChanged();
	void waterfallModeChanged();

	void bandChangedByBtn();
	void freeDVModeSelectionChanged(int index);
	void dspModeChangedByBtn();
	void adcModeChangedByBtn();
	void agcModeChangedByBtn();
	void agcShowLinesChanged();
	void filterChangedByBtn();
	void filterGroupChanged(DSPMode mode);
	void updateFreeDVControls();

	void bandChanged(int rx, bool byButton, HamBand band);
	void freeDVModeChanged(int rx, int mode);
	void freeDVStatusChanged(int rx, bool sync, float snr, quint64 rxFrames, quint64 txFrames);
	void dspModeChanged(int rx, DSPMode mode);
	void filterChanged(int rx, qreal low, qreal high);
	void adcModeChanged(int rx, ADCMode mode);
	void agcModeChanged(int rx, AGCMode mode, bool hang);
	void loadReceiverState(int rx);
	void setCurrentReceiver(int value);
	void ctrFrequencyChanged(int mode, int rx, qint64 frequency);
	void vfoFrequencyChanged(int mode, int rx, qint64 frequency);
};

#endif // _CUSDR_RADIO_POPUP_WIDGET_H
