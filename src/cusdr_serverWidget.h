/**
* @file  cusdr_serverWidget.h
* @brief header file for hpsdr server settings widget
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2010-09-21
* MVC Refactoring ZL2BRG
*/

#ifndef _CUSDR_SERVER_WIDGET_H
#define _CUSDR_SERVER_WIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QLabel>
#include <QTableWidget>
#include <QGridLayout>
#include <QCheckBox>

#include "Util/cusdr_buttons.h"

class ServerWidget : public QWidget {
	Q_OBJECT

public:
	ServerWidget(QWidget *parent = 0);
	~ServerWidget();

	// MVC Setters
	void	addServerNIEntry(QString niName, QString ipAddress);
	void	setServerNIC(int index);
	void	setTciServerEnabled(bool enabled);
	void	setTciServerPort(quint16 port);
	void	setPorts(quint16 serverPort, quint16 listenPort, quint16 audioPort);
	void	addNICChangedConnection();

signals:
	// MVC Signals
	void	serverNICRequested(int index);
	void	tciServerEnabledRequested(bool enabled);

	void	showEvent();
	void	closeEvent();
	void	messageEvent(QString );

public slots:
	QSize	sizeHint() const;
	QSize	minimumSizeHint() const;
	
protected:
	void	closeEvent(QCloseEvent *event);
	void	showEvent(QShowEvent *event);

private:
	QStringList		niList;
	QTableWidget	*serverNITable;

	QGroupBox		*portAddressesGroup();
	QGroupBox		*serverPortAddressGroup();
	QGroupBox		*listenerPortAddressGroup();
	QGroupBox		*audioPortAddressGroup();
	QGroupBox		*tciServerGroup();
	QGroupBox		*serverNIGroupBox;

	QCheckBox		*tciEnableCheckBox;

	QGridLayout		*portGridBox;
	QComboBox		*serverNetworkInterfaces;
	
	QLabel			*labelServerPortLabel;
	QLabel			*labelServerPortText;
	QLabel			*labelListenerPortLabel;
	QLabel			*labelListenerPortText;
	QLabel			*labelAudioPortLabel;
	QLabel			*labelAudioPortText;
	QLabel			*tciPortLabel;

	QLineEdit		*le_server_address;
	QLineEdit		*le_server_port;
	QLineEdit		*le_listener_port;
	QLineEdit		*le_audio_port;

	QString			lineedit_style;

	int		m_minimumWidgetWidth;
	int		m_minimumGroupBoxWidth;
	int		m_btnSpacing;

	void	createServerNIGroup();
	void	setupConnections();

private slots:
	void	serverNICIndexChanged(int index);
	void	tciEnabledToggled(bool enabled);
};

#endif // _CUSDR_SERVER_WIDGET_H
