#ifndef _CUSDR_NETWORK_WIDGET_H
#define _CUSDR_NETWORK_WIDGET_H

#include <QWidget>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QTabWidget>

#include "Util/cusdr_buttons.h"
#include "cusdr_settings.h"

#ifdef LOG_NETWORK_WIDGET
#   define NETWORK_WIDGET_DEBUG qDebug().nospace() << "NetworkWidget::\t"
#else
#   define NETWORK_WIDGET_DEBUG nullDebug()
#endif

class NetworkWidget : public QTabWidget {
	Q_OBJECT

public:
	NetworkWidget(QWidget *parent = 0);
	~NetworkWidget();

	// MVC View Interface Setters
	void	addNICChangedConnection();
	void	setHwInterface(QSDR::_HWInterfaceMode mode);
	void	setSocketBufSize(int size);
	void	setManualSocketBufferSize(bool manual);
	void	setMetisCardsList(const QList<TNetworkDevicecard>& list, const TNetworkDevicecard& active);
#ifdef HAVE_SOAPYSDR
	void	setSoapyDevicesList(const QList<TSoapyDevice>& list, const TSoapyDevice& active);
#endif
	void	addDeviceNICEntry(QString niName, QString ipAddress);
	void	setDeviceNIC(int index);
	void	setCurrentNetworkDevice(TNetworkDevicecard card);
#ifdef HAVE_SOAPYSDR
	void    setCurrentSoapyDevice(TSoapyDevice device);
#endif
	void	setDataEngineRunning(bool running);

signals:
	// MVC View Interface Signals
	void	hwInterfaceModeRequested(QSDR::_HWInterfaceMode mode);
	void	socketBufferSizeRequested(int size);
	void	manualSocketBufferSizeRequested(bool manual);
	void	nicInterfaceSelected(int index);
	void	searchDevicesRequested();
	void	currentHpsdrDeviceSelected(const TNetworkDevicecard& card);
#ifdef HAVE_SOAPYSDR
	void	currentSoapyDeviceSelected(const TSoapyDevice& dev);
#endif

	void	messageEvent(QString message);

private:
	QString		m_message;
	QList<TNetworkDevicecard>	m_deviceCards;

	QGroupBox	*hpsdrInterfaceExclusiveBtnGroup();
	QGroupBox	*receiversExclusiveBtnGroup();
	QGroupBox	*source10MhzExclusiveGroup;
	QGroupBox	*source122_88MhzExclusiveGroup;
	QGroupBox	*deviceNIGroupBox;
	QGroupBox	*searchNetworkDeviceGroupBox;
	QGroupBox	*socketBufferSizeGroupBox;
	
	QComboBox	*networkDeviceInterfaces;
	QComboBox	*deviceCombo;
	QComboBox	*socketBufferSizes;
	QComboBox	*m_receiverComboBox;

	QLabel		*socketBufferSizeLabel;

	AeroButton	*networkPresenceBtn;
#ifdef HAVE_SOAPYSDR
	AeroButton  *soapyBtn;
#endif
	AeroButton	*noHWBtn;

	AeroButton	*searchBtn;
	AeroButton	*socketBufSizeBtn;

	QSDR::_ServerMode		m_serverMode;
	QSDR::_HWInterfaceMode	m_hwInterface;
	QSDR::_HWInterfaceMode	m_hwInterfaceTemp;
	QSDR::_DataEngineState	m_dataEngineState;

	int		m_minimumWidgetWidth;
	int		m_minimumGroupBoxWidth;
	int		m_numberOfReceivers;
	int		m_hpsdrHardware;
	int		m_socketBufferSize;
	int     m_discoveryPassId;

	void	createDeviceNetworkInterfaceGroup();
	void	createDeviceSearchGroup();
	void	hwInterfaceChanged();
	void	disableButtons();
	void	enableButtons();

private slots:
	// Internal UI slots
	void	interfaceBtnClicked();
	void	searchBtnClicked();
	void	socketBufSizeBtnClicked();
	void	onSocketBufferSizeChanged(int index);
	void	onDeviceNICChanged(int index);
	void	deviceSelected(int index);
};

#endif // _CUSDR_NETWORK_WIDGET_H
