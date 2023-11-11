/*
 *
 *   Copyright 2010,2011,2022, Simon Eatough ZL2BRG based on Hermann von Hasseln's DL3HVH Work
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



#ifndef CUSDR_SETUPWIDGET_H
#define CUSDR_SETUPWIDGET_H

#include "cusdr_settings.h"
#include "cusdr_hpsdrWidget.h"
#include "cusdr_networkWidget.h"
#include "cusdr_transmitTabWidget.h"
#include "cusdr_alexTabWidget.h"
#include "cusdr_extCtrlWidget.h"
#include "cusdr_displayTabWidget.h"
#include "ui_cusdr_setupwidget.h"


#include <QTabWidget>



namespace Ui {
class cusdr_setupWidget;
}

class cusdr_SetupWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit cusdr_SetupWidget(QWidget *parent = nullptr);
    ~cusdr_SetupWidget();

private:
    Ui::setupWidget *ui;
public slots:
    void	addNICChangedConnection();

protected:
    QSize	sizeHint() const;
    QSize	minimumSizeHint() const;

    void	closeEvent(QCloseEvent *event);
    void	showEvent(QShowEvent *event);
    void	enterEvent(QEvent *event);
    void	leaveEvent(QEvent *event);
    void	mouseMoveEvent(QMouseEvent *event);
    void	mousePressEvent(QMouseEvent *event);
    void	mouseReleaseEvent(QMouseEvent *event);

private:
    Settings			*set;

    QSDR::_Error				m_error;
    QSDR::_ServerMode			m_serverMode;
    QSDR::_HWInterfaceMode		m_hwInterface;
    QSDR::_DataEngineState		m_dataEngineState;
    HPSDRWidget			*m_hpsdrWidget;
    NetworkWidget		*m_networkWidget;
    AlexTabWidget		*m_alexTabWidget;
    ExtCtrlWidget		*m_extCtrlWidget;
    TransmitTabWidget   *m_transmitTabWidget;
    tx_settings_dialog  *m_txsettingsWidget;
    DisplayTabWidget    *m_displaytabWidget;

    QString				m_message;

    int		m_minimumWidgetWidth;
    int		m_minimumGroupBoxWidth;

    void	setupConnections();

private slots:
            void systemStateChanged(
                    QObject *sender,
                    QSDR::_Error err,
    QSDR::_HWInterfaceMode hwmode,
            QSDR::_ServerMode mode,
    QSDR::_DataEngineState state);

    void setAlexPresence(bool value);
    void setPennyPresence(bool value);

    signals:
            void	showEvent(QObject *sender);
    void	closeEvent(QObject *sender);
    void	messageEvent(QString message);

};

#endif // CUSDR_SETUPWIDGET_H
