/*
 *
 *   Copyright 2022, Simon Eatough ZL2BRG, based on Hermann von Hasseln's DL3HVH work
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
#include "cusdr_setupwidget.h"
#include "ui_cusdr_setupwidget.h"


#define	btn_height		15
#define	btn_width		74
#define	btn_width2		52
#define	btn_widths		40




cusdr_SetupWidget::cusdr_SetupWidget(QWidget *parent)
: QTabWidget(parent)
, set(Settings::instance())
, ui(new Ui::setupWidget)
, m_minimumGroupBoxWidth(set->getMinimumGroupBoxWidth())


{
    setMinimumWidth(DOCK_MIN_WIDTH);
    setMaximumWidth(DOCK_MAX_WIDTH);
    setContentsMargins(4, 4, 4, 0);
    setMouseTracking(true);
    m_networkWidget = new NetworkWidget(this);
    m_hpsdrWidget = new HPSDRWidget(this);
    m_alexTabWidget = new AlexTabWidget(this);
    m_extCtrlWidget = new ExtCtrlWidget(this);
    m_txsettingsWidget = new tx_settings_dialog(this);
    m_displaytabWidget = new DisplayTabWidget(this);
   addTab(m_hpsdrWidget, " Config ");
   addTab(m_networkWidget, " Network ");
   addTab(m_extCtrlWidget, " Ext Ctrl ");
   addTab(m_alexTabWidget, " Alex ");
   addTab(m_txsettingsWidget, " Tx Settings ");
   this->addTab(m_displaytabWidget," Display Ctrl");
if (!set->getPenelopePresence() && !set->getPennyLanePresence() && (set->getHWInterface() != QSDR::Hermes)) {

}

if (!set->getAlexPresence())

setupConnections();
}

cusdr_SetupWidget::~cusdr_SetupWidget()
{
    delete m_displaytabWidget;

    disconnect(set, 0, this, 0);
    disconnect(this, 0, 0, 0);
}


QSize cusdr_SetupWidget::sizeHint() const {

    return QSize(m_minimumWidgetWidth, height());
}

QSize cusdr_SetupWidget::minimumSizeHint() const {

    return QSize(m_minimumWidgetWidth, height());
}

void cusdr_SetupWidget::setupConnections() {

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
            SIGNAL(pennyLanePresenceChanged(bool)),
            this,
            SLOT(setPennyPresence(bool)));
}

void cusdr_SetupWidget::systemStateChanged(
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

void cusdr_SetupWidget::setPennyPresence(bool value) {

    //setTabEnabled(1, value);
    setTabEnabled(2, value);
    //setTabEnabled(3, value);
}

void cusdr_SetupWidget::setAlexPresence(bool value) {

    setTabEnabled(3, value);
}

void cusdr_SetupWidget::addNICChangedConnection() {

    m_networkWidget->addNICChangedConnection();
}

void cusdr_SetupWidget::closeEvent(QCloseEvent *event) {

    emit closeEvent(this);
    QWidget::closeEvent(event);
}

void cusdr_SetupWidget::showEvent(QShowEvent *event) {

    emit showEvent(this);
    QWidget::showEvent(event);
}

void cusdr_SetupWidget::enterEvent(QEvent *event) {

    Q_UNUSED(event)
}

void cusdr_SetupWidget::leaveEvent(QEvent *event) {

    Q_UNUSED(event)
}

void cusdr_SetupWidget::mouseMoveEvent(QMouseEvent *event) {

    Q_UNUSED(event)
}

void cusdr_SetupWidget::mousePressEvent(QMouseEvent *event) {

    Q_UNUSED(event)
}

void cusdr_SetupWidget::mouseReleaseEvent(QMouseEvent *event) {

    Q_UNUSED(event)
}
