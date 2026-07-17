/**
* @file cusdr_hpsdrTabWidget.cpp
* @brief Hardware settings widget class for cuSDR
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2010-09-21
*/

/*
 *   
 *   Copyright 2010, 2011 Hermann von Hasseln, DL3HVH
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
 
#include <QtGui>
#include <QMenu>
#include <QFileDialog>
#include <QDebug>

#include "cusdr_hpsdrTabWidget.h"
#include "Controllers/SetupController.h"


#define	btn_height		15
#define	btn_width		74
#define	btn_width2		52
#define	btn_widths		40


HPSDRTabWidget::HPSDRTabWidget(QWidget *parent) 
	: QTabWidget(parent)
	, set(Settings::instance())
	, m_minimumWidgetWidth(500)
	, m_minimumGroupBoxWidth(set->getMinimumGroupBoxWidth())
{
	setMinimumWidth(520);
	setContentsMargins(4, 4, 4, 0);
	setMouseTracking(true);

//	m_hpsdrWidget = new HPSDRWidget(this);
	m_networkWidget = new NetworkWidget(this);
 //   m_transmitTabWidget = new TransmitTabWidget(this);
    //m_alexTabWidget = new AlexTabWidget(this);
//	m_extCtrlWidget = new ExtCtrlWidget(this);
 //   m_txsettingsWidget = new tx_settings_dialog(this);

//	this->addTab(m_hpsdrWidget, " Config ");
    this->addTab(m_networkWidget, " Network ");
 //   this->addTab(m_transmitTabWidget, " Transmit ");
//	this->addTab(m_extCtrlWidget, " Ext Ctrl ");
//	this->addTab(m_alexTabWidget, " Alex ");
 //   this->addTab(m_txsettingsWidget, " Tx settings ");
	setTabEnabled(1, true);

	m_setupController = new SetupController(this);
	m_setupController->bind(this, set);
}

HPSDRTabWidget::~HPSDRTabWidget() {
	disconnect(0, 0, 0);
}

QSize HPSDRTabWidget::sizeHint() const {
	
	return QSize(m_minimumWidgetWidth, height());
}

QSize HPSDRTabWidget::minimumSizeHint() const {

	return QSize(m_minimumWidgetWidth, height());
}



void HPSDRTabWidget::addNICChangedConnection() {

	m_networkWidget->addNICChangedConnection();
}

void HPSDRTabWidget::closeEvent(QCloseEvent *event) {

	emit closeEvent();
	QWidget::closeEvent(event);
}

void HPSDRTabWidget::showEvent(QShowEvent *event) {

	emit showEvent();
	QWidget::showEvent(event);
}

void HPSDRTabWidget::enterEvent(QEvent *event) {

	Q_UNUSED(event)
}

void HPSDRTabWidget::leaveEvent(QEvent *event) {

	Q_UNUSED(event)
}

void HPSDRTabWidget::mouseMoveEvent(QMouseEvent *event) {

	Q_UNUSED(event)
}

void HPSDRTabWidget::mousePressEvent(QMouseEvent *event) {

	Q_UNUSED(event)
}

void HPSDRTabWidget::mouseReleaseEvent(QMouseEvent *event) {

	Q_UNUSED(event)
}


