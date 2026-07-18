#ifndef _CUSDR_ALEX_FILTER_WIDGET_H
#define _CUSDR_ALEX_FILTER_WIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QBoxLayout>

#include "Util/cusdr_buttons.h"
#include "Util/cusdr_led.h"
#include "cusdr_settings.h"
#include "cusdr_fonts.h"

class AlexFilterWidget : public QWidget {
	Q_OBJECT

public:
	AlexFilterWidget(QWidget *parent = 0);
	~AlexFilterWidget();

	// MVC View Interface Setters
	void	setAlexConfig(quint16 config);
	void	setAlexStates(const QList<int>& states);
	void	setAlexManualState(bool manual);
	void	setFrequencies(const QList<long>& hpfLo, const QList<long>& hpfHi, const QList<long>& lpfLo, const QList<long>& lpfHi);
	void	setFrequency(int mode, int rx, qint64 frequency);
	void	setCurrentReceiver(int rx);

signals:
	// MVC View Interface Signals
	void	manualFilterRequested(bool manual);
	void	alexConfigurationRequested(quint16 config);
	void	hpfLoFrequencyRequested(int filter, long value);
	void	hpfHiFrequencyRequested(int filter, long value);
	void	lpfLoFrequencyRequested(int filter, long value);
	void	lpfHiFrequencyRequested(int filter, long value);

	void	showEvent();
	void	closeEvent();
	void	messageEvent(QString);

private:
	void	createHPFGroup();
	void	createLPFGroup();
	void 	initAlexValues();
	void	setFilterValues();
	void	setAlexConfiguration(double frequency);

	QSDR::_ServerMode		m_serverMode;
	QSDR::_HWInterfaceMode	m_hwInterface;
	QSDR::_DataEngineState	m_dataEngineState;

	QGroupBox*	HPFGroup;
	QGroupBox*	LPFGroup;

	QList<QDoubleSpinBox *>		m_HPFLoSpinBoxList;
	QList<QDoubleSpinBox *>		m_HPFHiSpinBoxList;
	QList<QDoubleSpinBox *>		m_LPFLoSpinBoxList;
	QList<QDoubleSpinBox *>		m_LPFHiSpinBoxList;

	QList<QHLed *>				m_HPFActiveBtnList;
	QList<QHLed *>				m_LPFActiveBtnList;

	QList<QLabel *>			m_HPFLabelList;
	QList<QLabel *>			m_LPFLabelList;
	QList<AeroButton *>		m_HPFBtnList;

	QList<int>					m_alexStates;

	QList<QPair<qreal, qreal> >	m_HPFFrequencyRangeLoList;
	QList<QPair<qreal, qreal> >	m_HPFFrequencyRangeHiList;
	QList<QPair<qreal, qreal> >	m_LPFFrequencyRangeLoList;
	QList<QPair<qreal, qreal> >	m_LPFFrequencyRangeHiList;

	QList<qreal>			m_HPFLoDefaultFrequencyList;
	QList<qreal>			m_HPFHiDefaultFrequencyList;
	QList<qreal>			m_LPFLoDefaultFrequencyList;
	QList<qreal>			m_LPFHiDefaultFrequencyList;

	CFonts		*fonts;
	TFonts		m_fonts;

	QLabel 		*mhz55HPFLabel;

	AeroButton 	*manualFilterBtn;
	AeroButton 	*defaultValuesBtn;
	AeroButton 	*bypassAllHPFBtn;
	AeroButton 	*lowNoise6mAmpBtn;
	AeroButton 	*hpf13MHzBtn;
	AeroButton 	*hpf20MHzBtn;
	AeroButton 	*hpf9_5MHzBtn;
	AeroButton 	*hpf6_5MHzBtn;
	AeroButton 	*hpf1_5MHzBtn;

	QColor 		btnOff;
	QColor 		btnOn;

	quint16	m_alexConfig;

	long	m_frequency;
	int		m_minimumWidgetWidth;
	int		m_minimumGroupBoxWidth;
	int		m_hpfFilters;
	int		m_lpfFilters;
	int		m_receiver;

	bool	bypassAll;
	bool 	lowNoise6m;
	bool 	hpf13MHz;
	bool 	hpf20MHz;
	bool 	hpf9_5MHz;
	bool 	hpf6_5MHz;
	bool 	hpf1_5MHz;

private slots:
	// Internal UI slots
	void hpfLoSpinBoxValueChanged(double value);
	void hpfHiSpinBoxValueChanged(double value);
	void lpfLoSpinBoxValueChanged(double value);
	void lpfHiSpinBoxValueChanged(double value);

	void manualFilterBtnClicked();
	void defaultValuesBtnClicked();
	void bypassAllHPFBtnClicked();
	void lowNoise6mAmpBtnClicked();
	void hpf13MHzBtnClicked();
	void hpf20MHzBtnClicked();
	void hpf9_5MHzBtnClicked();
	void hpf6_5MHzBtnClicked();
	void hpf1_5MHzBtnClicked();
};

#endif // _CUSDR_ALEX_FILTER_WIDGET_H
