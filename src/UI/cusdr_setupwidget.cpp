#include "Models/RadioModel.h"
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
#include <QScrollArea>

#include "cusdr_setupwidget.h"
#include "Controllers/NetworkSettingsController.h"
#include "Controllers/HpsdrSettingsController.h"
#include "Controllers/ExtCtrlSettingsController.h"
#include "Controllers/AlexSettingsController.h"
#include "Controllers/TransmitSettingsController.h"
#include "Controllers/DisplaySettingsController.h"
#include "Controllers/SetupController.h"
#ifdef HAVE_SOAPYSDR
#include "Controllers/RadioSettingsController.h"
#endif


#define	btn_height		22
#define	btn_width		74
#define	btn_width2		52
#define	btn_widths		40

cusdr_SetupWidget::cusdr_SetupWidget(RadioModel *model, QWidget *parent)
: QTabWidget(parent)
    , m_radioModel(model)
, set(Settings::instance())
, m_minimumWidgetWidth(0)
, m_minimumGroupBoxWidth(set->getMinimumGroupBoxWidth())
{
    setContentsMargins(4, 4, 4, 0);
    setMouseTracking(true);
    m_networkWidget = new NetworkWidget(this);
    m_networkSettingsController = new NetworkSettingsController(this);
    m_networkSettingsController->bind(m_networkWidget, set);
    m_hpsdrWidget = new HPSDRWidget(this);
    m_hpsdrSettingsController = new HpsdrSettingsController(this);
    m_hpsdrSettingsController->bind(m_hpsdrWidget, set);
    m_alexTabWidget = new AlexTabWidget(this);
    m_alexSettingsController = new AlexSettingsController(this);
    m_alexSettingsController->bind(m_alexTabWidget, m_radioModel, set);
    m_extCtrlWidget = new ExtCtrlWidget(this);
    m_extCtrlSettingsController = new ExtCtrlSettingsController(this);
    m_extCtrlSettingsController->bind(m_extCtrlWidget, m_radioModel, set);
    m_txsettingsWidget = new tx_settings_dialog(this);
    m_transmitSettingsController = new TransmitSettingsController(this);
    m_transmitSettingsController->bind(m_txsettingsWidget, m_radioModel ? m_radioModel->transmit() : nullptr, set);
    m_displaytabWidget = new DisplayTabWidget(m_radioModel, this);
    m_displaySettingsController = new DisplaySettingsController(this);
    m_displaySettingsController->bind(m_displaytabWidget, m_radioModel, set);
#ifdef HAVE_SOAPYSDR
    m_radioSettingsWidget = new cusdr_radioSettingsWidget(this);
    m_radioSettingsController = new RadioSettingsController(this);
    m_radioSettingsController->bind(m_radioSettingsWidget, set);
#endif

    auto addScrollableTab = [this](QWidget* widget, const QString& title) {
        QScrollArea* scrollArea = new QScrollArea(this);
        scrollArea->setWidget(widget);
        scrollArea->setWidgetResizable(true);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scrollArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
        scrollArea->setFrameShape(QFrame::NoFrame);
        this->addTab(scrollArea, title);
    };

    addScrollableTab(m_hpsdrWidget, " Config ");
    addScrollableTab(m_networkWidget, " Network ");
    addScrollableTab(m_extCtrlWidget, " Ext Ctrl ");
    addScrollableTab(m_alexTabWidget, " Alex ");
    addScrollableTab(m_txsettingsWidget, " Tx Settings ");
    // DisplayTabWidget has its own tabs (incl. 3D View) and internal scroll areas — do not wrap in QScrollArea
    // or the tab bar scrolls away with the first tab's content.
    this->addTab(m_displaytabWidget, tr(" Display Ctrl"));
#ifdef HAVE_SOAPYSDR
    if (QWidget *radioPage = m_radioSettingsWidget->detachRadioConfigPage())
        addScrollableTab(radioPage, " Radio ");
#endif

    m_setupController = new SetupController(this);
    m_setupController->bind(this, set);
}

cusdr_SetupWidget::~cusdr_SetupWidget()
{
    delete m_displaytabWidget;
    disconnect(0, 0, 0);
}


QSize cusdr_SetupWidget::sizeHint() const {

    return QTabWidget::sizeHint().expandedTo(QSize(420, 0));
}

QSize cusdr_SetupWidget::minimumSizeHint() const {

    return QTabWidget::minimumSizeHint().expandedTo(QSize(320, 0));
}



void cusdr_SetupWidget::addNICChangedConnection() {

    m_networkWidget->addNICChangedConnection();
}

void cusdr_SetupWidget::closeEvent(QCloseEvent *event) {

    emit closeEvent();
    QWidget::closeEvent(event);
}

void cusdr_SetupWidget::showEvent(QShowEvent *event) {

    emit showEvent();
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
