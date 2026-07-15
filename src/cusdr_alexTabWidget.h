#ifndef _CUSDR_ALEX_TABWIDGET_H
#define _CUSDR_ALEX_TABWIDGET_H

#include <QTabWidget>

#include "Util/cusdr_buttons.h"
#include "cusdr_settings.h"
#include "cusdr_alexAntennaWidget.h"
#include "cusdr_alexFilterWidget.h"

class AlexTabWidget : public QTabWidget {
	Q_OBJECT

public:
	AlexTabWidget(QWidget *parent = 0);
	~AlexTabWidget();

public slots:
	QSize	sizeHint() const;
	QSize	minimumSizeHint() const;
	void	addNICChangedConnection();
	
private:
	AlexAntennaWidget		*m_alexAntennaWidget;
	AlexFilterWidget		*m_alexFilterWidget;

	int		m_minimumWidgetWidth;
	int		m_minimumGroupBoxWidth;

	void	setupConnections();
};

#endif // _CUSDR_ALEX_TABWIDGET_H
