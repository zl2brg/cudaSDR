#include "cusdr_alexTabWidget.h"

#define	btn_height		22
#define	btn_width		74
#define	btn_width2		52
#define	btn_widths		40

AlexTabWidget::AlexTabWidget(QWidget *parent)
	: QTabWidget(parent)
	, m_minimumWidgetWidth(0)
	, m_minimumGroupBoxWidth(0)
{
	setContentsMargins(4, 4, 4, 0);
	setMouseTracking(true);
	
	m_alexAntennaWidget = new AlexAntennaWidget(this);
	m_alexFilterWidget = new AlexFilterWidget(this);

	this->addTab(m_alexAntennaWidget, "Antenna");
	this->addTab(m_alexFilterWidget, "Filter");

	setupConnections();
}

AlexTabWidget::~AlexTabWidget() {
	disconnect(0, 0, 0);
}

QSize AlexTabWidget::sizeHint() const {
	return QTabWidget::sizeHint();
}

QSize AlexTabWidget::minimumSizeHint() const {
	return QTabWidget::minimumSizeHint();
}

void AlexTabWidget::setupConnections() {
}

void AlexTabWidget::addNICChangedConnection() {
}
