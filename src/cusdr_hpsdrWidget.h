#ifndef _CUSDR_HPSDR_WIDGET_H
#define _CUSDR_HPSDR_WIDGET_H

#include <QWidget>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>

#include "Util/cusdr_buttons.h"
#include "cusdr_settings.h"

#ifdef LOG_HPSDR_WIDGET
#   define HPSDR_WIDGET_DEBUG qDebug().nospace() << "HPSDRWidget::\t"
#else
#   define HPSDR_WIDGET_DEBUG nullDebug()
#endif

class HPSDRWidget : public QWidget {
	Q_OBJECT

public:
	HPSDRWidget(QWidget *parent = 0);
	~HPSDRWidget();

	// MVC View Interface Setters
	void	setHwInterface(QSDR::_HWInterfaceMode mode);
	void	setHpsdrHardware(int hw);
	void	setNumberOfReceivers(int count);
	void	setFirmwareCheck(bool check);
	void	set10MhzSource(int src);
	void	set122_88MhzSource(int src);
	void	setSampleRate(int rate);
	void	setMercuryPresence(bool pres);
	void	setPenelopePresence(bool pres);
	void	setPennyLanePresence(bool pres);
	void	setAlexPresence(bool pres);
	void	setExcaliburPresence(bool pres);
	void	setCurrentMetisCard(const TNetworkDevicecard& card);

signals:
	// MVC View Interface Signals
	void	hwInterfaceRequested(QSDR::_HWInterfaceMode mode);
	void	hpsdrHardwareRequested(int hw);
	void	numberOfReceiversRequested(int count);
	void	firmwareCheckRequested(bool check);
	void	src10MhzRequested(int src);
	void	src122_88MhzRequested(int src);
	void	sampleRateRequested(int rate);
	void	mercuryPresenceRequested(bool pres);
	void	penelopePresenceRequested(bool pres);
	void	pennyLanePresenceRequested(bool pres);
	void	alexPresenceRequested(bool pres);
	void	excaliburPresenceRequested(bool pres);

	void	messageEvent(QString message);

private:
	QGroupBox       *hpsdrHardwareBtnGroup();
	QGroupBox       *receiversExclusiveBtnGroup();
	QGroupBox       *source10MhzExclusiveGroup;
	QGroupBox       *source122_88MhzExclusiveGroup;
	QGroupBox       *sampleRateExclusiveGroup();
	QGroupBox       *numberOfReceiversGroup();

	QGroupBox       *m_hpsdrHardwareGroupBox;
	QGroupBox       *m_sampleRateGroupBox;
	QGroupBox       *m_numberOfReceiversGroupBox;

	QComboBox       *m_receiverComboBox;

	QLabel		*m_fwCheckLabel;
	QLabel		*m_receiversLabel;
	QLabel		*m_detectedBoardLabel;
	
	QString		m_message;

	AeroButton	*hermesPresenceBtn;
	AeroButton	*modulesPresenceBtn;
	AeroButton	*penelopePresenceBtn;
	AeroButton	*pennyPresenceBtn;
	AeroButton	*mercuryPresenceBtn;
	AeroButton	*alexPresenceBtn;
	AeroButton	*excaliburPresenceBtn;
	AeroButton	*firmwareCheckBtn;
	AeroButton	*atlasBtn;
	AeroButton	*penelopeBtn;
	AeroButton	*mercuryBtn;
	AeroButton	*penelope2Btn;
	AeroButton	*mercury2Btn;
	AeroButton	*samplerate48Btn;
	AeroButton	*samplerate96Btn;
	AeroButton	*samplerate192Btn;
	AeroButton	*samplerate384Btn;
    AeroButton	*samplerate768Btn;
    AeroButton	*samplerate1536Btn;

	QList<AeroButton *>	hardwareBtnList;
	QList<AeroButton *>	source10MhzBtnList;
	QList<AeroButton *>	viewBtnList;
	QList<QString> sources10Mhz;
	QList<AeroButton *>	samplerateBtnList;

	QSDR::_ServerMode		m_serverMode;
	QSDR::_HWInterfaceMode	m_hwInterface;
	QSDR::_HWInterfaceMode	m_hwInterfaceTemp;
	QSDR::_DataEngineState	m_dataEngineState;

	bool	m_firmwareCheck;
    bool    m_antialiased;
	int		m_minimumWidgetWidth;
	int		m_minimumGroupBoxWidth;
	int		m_numberOfReceivers;
	int		m_hpsdrHardware;
	int		m_socketBufferSize;

	void	createSource10MhzExclusiveGroup();
	void	createSource122_88MhzExclusiveGroup();
	void	hwInterfaceChanged();
	void	disableButtons();
	void	enableButtons();
	void	updateDetectedBoardLabel(TNetworkDevicecard card);

private slots:
	// Internal UI slots
	void	setHPSDRHardware();
	void	source10MhzChanged();
	void	source122_88MhzChanged();
	void	sampleRateChanged();
	void	receiverComboBoxChanged(int index);
	void	hpsdrHardwareChanged();
	void	penelopePresenceChanged();
	void	pennyPresenceChanged();
	void	mercuryPresenceChanged();
	void	alexPresenceChanged();
	void	excaliburPresenceChanged();
};

#endif // _CUSDR_HPSDR_WIDGET_H
