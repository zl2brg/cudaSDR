/**
* @file  cusdr_displayTabWidget.cpp
* @brief Display settings tab widget class for cuSDR
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2012-10-30
*/

/*
 *   
 *   Copyright 2010 - 2012 Hermann von Hasseln, DL3HVH
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
#include <QVBoxLayout>

#include "cusdr_displayTabWidget.h"


#define	btn_height		15
#define	btn_width		74
#define	btn_width2		52
#define	btn_widths		40


DisplayTabWidget::DisplayTabWidget(QWidget *parent)
	: QTabWidget(parent)
	, set(Settings::instance())
	, m_minimumWidgetWidth(set->getMinimumWidgetWidth())
	, m_minimumGroupBoxWidth(set->getMinimumGroupBoxWidth())
	, m_3DDialog(nullptr)
	, m_3DPanel(nullptr)
{
	setStyleSheet(set->getTabWidgetStyle());
	setContentsMargins(4, 4, 4, 0);
	setMouseTracking(true);
	
	m_displayWidget = new DisplayOptionsWidget(this);
	m_colorWidget = new ColorOptionsWidget(this);
	m_3DWidget = new Options3DWidget(this);

	this->addTab(m_displayWidget, " Display ");
	this->addTab(m_colorWidget, " Colors ");
	this->addTab(m_3DWidget, " 3D View ");

	setTabEnabled(0, true);
	setTabEnabled(1, true);
	setTabEnabled(2, true);
	setupConnections();
}

DisplayTabWidget::~DisplayTabWidget() {
    // Clean up 3D dialog and panel first
    if (m_3DPanel) {
        disconnect(set, 0, m_3DPanel, 0);
    }
    
    if (m_3DDialog) {
        delete m_3DDialog;
        m_3DDialog = nullptr;
        m_3DPanel = nullptr; // Panel is deleted with dialog since it's a child
    }

    disconnect(this, 0, 0, 0);
    disconnect(set, 0, this, 0);
    
    delete m_displayWidget;
    delete m_colorWidget;
    delete m_3DWidget;
}

QSize DisplayTabWidget::sizeHint() const {
	
	return QSize(m_minimumWidgetWidth, height());
}

QSize DisplayTabWidget::minimumSizeHint() const {

	return QSize(m_minimumWidgetWidth, height());
}

void DisplayTabWidget::setupConnections() {

	CHECKED_CONNECT(
		set,
		SIGNAL(systemStateChanged(
					QObject *,
					QSDR::_Error,
					QSDR::_HWInterfaceMode,
					QSDR::_ServerMode,
					QSDR::_DataEngineState)),
		this,
		SLOT(systemStateChanged(
					QObject *,
					QSDR::_Error,
					QSDR::_HWInterfaceMode,
					QSDR::_ServerMode,
					QSDR::_DataEngineState)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(alexPresenceChanged(bool)),
		this,
		SLOT(setAlexPresence(bool)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(penelopePresenceChanged(bool)),
		this,
		SLOT(setPennyPresence(bool)));

	CHECKED_CONNECT(
		set, 
		SIGNAL(penelopePresenceChanged(bool)),
		this,
		SLOT(setPennyPresence(bool)));

	// Connect 3D options widget
	CHECKED_CONNECT(
		m_3DWidget,
		SIGNAL(show3DPanadapterChanged(bool)),
		this,
		SLOT(show3DPanadapter(bool)));
}

void DisplayTabWidget::systemStateChanged(
	QObject *sender,					/*!<[in] the sender of the signal. */
	QSDR::_Error err,					/*!<[in] error state. */
	QSDR::_HWInterfaceMode hwmode,		/*!<[in] HPSDR interface (Metis, Hermes, none). */
	QSDR::_ServerMode mode,				/*!<[in] server mode. */
	QSDR::_DataEngineState state		/*!<[in] data engine state. */
) {
	Q_UNUSED (sender)
	Q_UNUSED (err)

	//if (sender == this) return;

	if (m_hwInterface != hwmode)
		m_hwInterface = hwmode;

	if (m_hwInterface == QSDR::Hermes)
		setTabEnabled(2, true);
	else
		setTabEnabled(2, false);

	if (m_serverMode != mode)
		m_serverMode = mode;

	if (m_dataEngineState != state)
		m_dataEngineState = state;
}

void DisplayTabWidget::setPennyPresence(bool value) {

	//setTabEnabled(1, value);
	setTabEnabled(2, value);
	//setTabEnabled(3, value);
}

void DisplayTabWidget::setAlexPresence(bool value) {

	setTabEnabled(3, value);
}

void DisplayTabWidget::addNICChangedConnection() {

	//m_networkWidget->addNICChangedConnection();
}

void DisplayTabWidget::show3DPanadapter(bool enabled) {
	if (enabled) {
		if (!m_3DDialog) {
			// Create the 3D dialog window
			m_3DDialog = new QDialog(this);
			m_3DDialog->setWindowTitle("3D Panadapter - cuSDR");
			m_3DDialog->setModal(false);
			m_3DDialog->resize(800, 600);
			
			// Create the 3D panel for receiver 0
			m_3DPanel = new QGL3DPanel(m_3DDialog, 0);
			
			// Set up layout
			QVBoxLayout *layout = new QVBoxLayout;
			layout->setContentsMargins(0, 0, 0, 0);
			layout->addWidget(m_3DPanel);
			m_3DDialog->setLayout(layout);
			
			// Connect to real spectrum data
			CHECKED_CONNECT(
				set,
				SIGNAL(spectrumBufferChanged(int, const qVectorFloat&)),
				m_3DPanel,
				SLOT(setSpectrumBuffer(int, const qVectorFloat&)));
				
			// Connect to frequency changes
			CHECKED_CONNECT(
				set,
				SIGNAL(ctrFrequencyChanged(QObject*, int, int, long)),
				m_3DPanel,
				SLOT(setCtrFrequency(QObject*, int, int, long)));
				
			CHECKED_CONNECT(
				set,
				SIGNAL(vfoFrequencyChanged(QObject*, int, int, long)),
				m_3DPanel,
				SLOT(setVFOFrequency(QObject*, int, int, long)));
			
			// Connect 3D options widget controls to 3D panel
			CHECKED_CONNECT(
				m_3DWidget,
				SIGNAL(heightScaleValueChanged(float)),
				m_3DPanel,
				SLOT(setHeightScale(float)));
				
			CHECKED_CONNECT(
				m_3DWidget,
				SIGNAL(frequencyScaleValueChanged(float)),
				m_3DPanel,
				SLOT(setFrequencyScale(float)));
				
			CHECKED_CONNECT(
				m_3DWidget,
				SIGNAL(timeScaleValueChanged(float)),
				m_3DPanel,
				SLOT(setTimeScale(float)));
				
			CHECKED_CONNECT(
				m_3DWidget,
				SIGNAL(updateIntervalValueChanged(int)),
				m_3DPanel,
				SLOT(setUpdateRate(int)));
				
			CHECKED_CONNECT(
				m_3DWidget,
				SIGNAL(showGridValueChanged(bool)),
				m_3DPanel,
				SLOT(setShowGrid(bool)));
				
			CHECKED_CONNECT(
				m_3DWidget,
				SIGNAL(showAxesValueChanged(bool)),
				m_3DPanel,
				SLOT(setShowAxes(bool)));
				
			CHECKED_CONNECT(
				m_3DWidget,
				SIGNAL(wireframeModeValueChanged(bool)),
				m_3DPanel,
				SLOT(setWireframeMode(bool)));
				
			CHECKED_CONNECT(
				m_3DWidget,
				SIGNAL(waterfallOffsetValueChanged(float)),
				m_3DPanel,
				SLOT(setWaterfallOffset(float)));
				
		}
		
		if (m_3DDialog) {
			m_3DDialog->show();
		}
	} else {
		if (m_3DDialog) {
			m_3DDialog->hide();
		}
	}
}

void DisplayTabWidget::closeEvent(QCloseEvent *event) {

	emit closeEvent(this);
	QWidget::closeEvent(event);
}

void DisplayTabWidget::showEvent(QShowEvent *event) {

	emit showEvent(this);
	QWidget::showEvent(event);
}

void DisplayTabWidget::enterEvent(QEvent *event) {

	Q_UNUSED(event)
}

void DisplayTabWidget::leaveEvent(QEvent *event) {

	Q_UNUSED(event)
}

void DisplayTabWidget::mouseMoveEvent(QMouseEvent *event) {

	Q_UNUSED(event)
}

void DisplayTabWidget::mousePressEvent(QMouseEvent *event) {

	Q_UNUSED(event)
}

void DisplayTabWidget::mouseReleaseEvent(QMouseEvent *event) {

	Q_UNUSED(event)
}


