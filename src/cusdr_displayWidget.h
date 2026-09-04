#ifndef _CUSDR_DISPLAY_OPTIONS_WIDGET_H
#define _CUSDR_DISPLAY_OPTIONS_WIDGET_H

#include <QWidget>
#include <QPainter>
#include <QComboBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QLabel>
#include <QBoxLayout>
#include <QRadioButton>
#include <QCheckBox>

#include "Util/cusdr_buttons.h"
#include "cusdr_settings.h"
#include "cusdr_fonts.h"

class RadioModel;

class DisplayOptionsWidget : public QWidget {
	Q_OBJECT

public:
	DisplayOptionsWidget(RadioModel *model, QWidget* parent = nullptr);
	~DisplayOptionsWidget();

	// MVC View Interface Setters
	void	setFramesPerSecond(int fps);
	void	setSpectrumAveragingCnt(int avg);
	void	setWidebandAveragingCnt(int val);
	void	setWaterfallTime(int val);
	void	setWaterfallOffsetLo(int val);
	void	setWaterfallOffsetHi(int val);
	void	setSMeterHoldTime(int val);
	void	setPanSMeter(bool show);
	void	setPanFreq(bool show);
	void	setCallsign(const QString& callsign);
	void	setPanadapterMode(PanGraphicsMode mode);
	void	setWaterfallColorMode(WaterfallColorMode mode);
	void	setPanAveragingMode(PanAveragingMode mode);
	void	setPanDetectorMode(PanDetectorMode mode);
	void	setfftSize(int size);
	void	setfmsqLevel(int val);

	int		currentReceiver() const { return m_currentReceiver; }

signals:
	// MVC View Interface Signals
	void	receiverChanged(int rx);
	void	framesPerSecondRequested(int rx, int val);
	void	spectrumAveragingCntRequested(int rx, int val);
	void	waterfallTimeRequested(int rx, int val);
	void	waterfallOffsetLoRequested(int rx, int val);
	void	waterfallOffsetHiRequested(int rx, int val);
	void	sMeterHoldTimeRequested(int val);
	void	panSMeterRequested(bool show);
	void	panFreqRequested(bool show);
	void	callsignRequested(const QString& val);
	void	graphicsStateRequested(int rx, int panadapterMode, int waterColorMode);
	void	panAveragingModeRequested(int rx, int mode);
	void	panDetectorModeRequested(int rx, int mode);
	void	fftSizeRequested(int rx, int size);
	void	fmsqLevelRequested(int rx, int val);

	void	averagingModeChanged(bool value);

public slots:
	QSize	sizeHint() const;
	QSize	minimumSizeHint() const;
	void	setCurrentReceiver(int rx);

private:
    RadioModel*                             m_radioModel;

	QSDR::_ServerMode			m_serverMode;
	QSDR::_HWInterfaceMode		m_hwInterface;
	QSDR::_DataEngineState		m_dataEngineState;

	PanGraphicsMode				m_panadapterMode;
	PanGraphicsMode				m_wbPanadapterMode;
	WaterfallColorMode			m_waterColorMode;
    PanAveragingMode            m_panAveragingMode;
    PanDetectorMode             m_panDetectorMode;

	TWideband				m_widebandOptions;
	
	QString					m_menu_style;
	QString					m_callSingText;

	CFonts*					fonts;
	TFonts					m_fonts;

	QGroupBox*				m_fpsGroupBox;
	QGroupBox*				m_panSpectrumOptions;
	QGroupBox*				m_widebandPanOptions;
	QGroupBox*				m_waterfallSpectrumOptions;
	QGroupBox*				m_wideBandSpectrumOptions;
	QGroupBox*				m_sMeterOptions;
	QGroupBox*				m_callSignEditor;

	QLineEdit*				callSignLineEdit;

	QSlider*				m_fpsSlider;
	QSlider*				m_avgSlider;
	QSlider*				m_wbAvgSlider;
	QSlider*				m_fmSqlevel;

	QComboBox*              m_panAverageCombo;
    QComboBox*              m_panDetectorCombo;
	QComboBox*              m_fftSizeCombo;

    QSpinBox*				m_waterfallLoOffsetSpinBox;
	QSpinBox*				m_waterfallHiOffsetSpinBox;
	QSpinBox*				m_waterfallTimeSpinBox;
	QSpinBox*				m_sMeterHoldTimeSpinBox;

	QLabel*					m_fpsLabel;
	QLabel*					m_fpsLevelLabel;
	QLabel*					m_avgLabel;
	QLabel*					m_wbAvgLabel;
	QLabel*					m_avgLevelLabel;
	QLabel*					m_wbAvgLevelLabel;
	QLabel*					m_resolutionLabel;
	QLabel*					m_waterfallTimeLabel;
	QLabel*					m_waterfallLoOffsetLabel;
	QLabel*					m_waterfallHiOffsetLabel;
	QLabel*					m_sMeterHoldTimeLabel;
	QLabel*                 m_panAvgModeLabel;
    QLabel*                 m_panDetModeLabel;
	QLabel*					m_fftLabel;
	QLabel*					m_sqlabel;


	AeroButton*				m_PanLineBtn;
	AeroButton*				m_PanFilledLineBtn;
	AeroButton*				m_PanSolidBtn;
	AeroButton*				m_wbPanLineBtn;
	AeroButton*				m_wbPanFilledLineBtn;
	AeroButton*				m_wbPanSolidBtn;
	AeroButton*				m_setCallSignBtn;
	AeroButton*				m_waterfallSimpleBtn;
	AeroButton*				m_waterfallEnhancedBtn;
	AeroButton*				m_waterfallSpectranBtn;

	QList<AeroButton* >		m_panadapterBtnList;
	QList<AeroButton* >		m_wbpanadapterBtnList;
	QList<AeroButton* >		m_waterfallColorBtnList;

	QList<QRadioButton *>	m_sMeterTypeRadioBtnList;
	QCheckBox*				m_panSMeterCheckBox;
	QCheckBox*				m_panFreqCheckBox;

	int		m_fontHeight;
	int		m_maxFontWidth;

	bool	m_antialiased;
	bool	m_mouseOver;
	int		m_minimumWidgetWidth;
	int		m_minimumGroupBoxWidth;
	int		m_btnSpacing;

	int		m_currentReceiver;
	int		m_btnChooserHit;
	int		m_panStyle;
	int		m_framesPerSecond;
	int		m_avgValue;
	int		m_wbAvgValue;
	int		m_sampleRate;
	int		m_waterfallTime;
	int		m_sMeterHoldTime;
	bool	m_panSMeter = true;
	bool	m_panFreq = true;
	int     m_panAvMode;
    int     m_panDetMode;
    int     m_fftSize;

	void	createFPSGroupBox();
	void	createPanSpectrumOptions();
	void	createWidebandPanOptions();
	void	createWaterfallSpectrumOptions();
	void	createSMeterOptions();
	void	createCallSignEditor();

private slots:
	// Internal UI slots
	void	panModeChanged();
	void	wbPanModeChanged();
	void	waterfallColorChanged();
	void	sMeterChanged();
	void	waterfallTimeChanged(int value);
	void	waterfallLoOffsetChanged(int value);
	void	waterfallHiOffsetChanged(int value);
	void	sMeterHoldTimeChanged(int value);
	void	panSMeterChanged(bool value);
	void	panFreqChanged(bool value);
	void 	fpsValueChanged(int value);
	void	averagingFilterCntChanged(int value);
	void	wbAveragingFilterCntChanged(int value);
	void	callSignTextChanged(const QString &text);
	void	callSignChanged();
	void    panAverageModeChanged(int value);
    void    panDetectorModeChanged(int value);
	void    fftSizeChanged(int value);
	void 	sqLevelChanged(int);
};

#endif // _CUSDR_DISPLAY_OPTIONS_WIDGET_H
