#ifndef _CUSDR_ALEX_ANTENNA_WIDGET_H
#define _CUSDR_ALEX_ANTENNA_WIDGET_H

#include <QGroupBox>
#include <QList>

#include "Util/cusdr_buttons.h"
#include "cusdr_settings.h"
#include "cusdr_fonts.h"

class AlexAntennaWidget : public QWidget {
	Q_OBJECT

public:
	AlexAntennaWidget(QWidget *parent = 0);
	~AlexAntennaWidget();

	// MVC View Interface Setters
	void	setAlexConfig(quint16 config);
	void	setAlexStates(const QList<int>& states);

signals:
	// MVC View Interface Signals
	void	alexStateRequested(int index, int state);

	void	showEvent();
	void	closeEvent();
	void	messageEvent(QString);

private:
	void	createAntennasGroup();
	void 	setAlexValues();

	QSDR::_ServerMode		m_serverMode;
	QSDR::_HWInterfaceMode	m_hwInterface;
	QSDR::_DataEngineState	m_dataEngineState;

	QGroupBox				*antennaGroup;

	QList<AeroButton *>		antenna1BtnList;
	QList<AeroButton *>		antenna2BtnList;
	QList<AeroButton *>		antenna3BtnList;

	QList<QList<AeroButton *> >	bandBtnMatrix;

	QList<AeroButton *>	rx1BtnList;
	QList<AeroButton *>	rx2BtnList;
	QList<AeroButton *>	xvBtnList;

	QList<QList<AeroButton *> >	bandBtnRxMatrix;

	QList<AeroButton *>	tx1BtnList;
	QList<AeroButton *>	tx2BtnList;
	QList<AeroButton *>	tx3BtnList;

	QList<QList<AeroButton *> >	bandBtnTxMatrix;

	QList<int>					m_alexStates;

	CFonts	*fonts;
	TFonts	m_fonts;

	quint16	m_alexConfig;
	int		m_numberOfBands;
	int		m_minimumWidgetWidth;
	int		m_minimumGroupBoxWidth;

private slots:
	// Internal UI slots
	void	antBtnClicked();
	void	rxAuxBtnClicked();
	void	txAntBtnClicked();
};

#endif // _CUSDR_ALEX_ANTENNA_WIDGET_H
