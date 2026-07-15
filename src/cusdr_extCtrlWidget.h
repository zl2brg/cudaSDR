#ifndef _CUSDR_PENNY_WIDGET_H
#define _CUSDR_PENNY_WIDGET_H

#include <QGroupBox>
#include <QList>

#include "Util/cusdr_buttons.h"
#include "cusdr_settings.h"

class ExtCtrlWidget : public QWidget {
	Q_OBJECT

public:
	ExtCtrlWidget(QWidget *parent = 0);
	~ExtCtrlWidget();

	// MVC View Interface Setters
	void	setPennyOCEnabled(bool enabled);
	void	setRxPins(const QList<int>& pins);
	void	setTxPins(const QList<int>& pins);

signals:
	// MVC View Interface Signals
	void	pennyOCEnabledRequested(bool enabled);
	void	rxPinsRequested(const QList<int>& pins);
	void	txPinsRequested(const QList<int>& pins);

	void	showEvent();
	void	closeEvent();
	void	messageEvent(QString);

private:
	QGroupBox*	receivePinsGroup;
	QGroupBox*	transmitPinsGroup;	

	AeroButton*	enableBtn;
	
	QList<QList<AeroButton *> >	receivePinsBtnMatrix;
	QList<QList<AeroButton *> >	transmitPinsBtnMatrix;

	QList<int>	m_rxPins;
	QList<int>	m_txPins;

	bool	m_pennyOCEnabled;

	int		m_minimumWidgetWidth;
	int		m_minimumGroupBoxWidth;

	void	createReceivePinsGroup();
	void	createTransmitPinsGroup();
	void	setValues();
	uchar	getMask(int value);

private slots:
	// Internal UI slots
	void	enable();
	void	receivePinsBtnClicked();
	void	transmitPinsBtnClicked();
};

#endif // _CUSDR_PENNY_WIDGET_H
