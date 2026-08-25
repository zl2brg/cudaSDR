#ifndef _CUSDR_DISPLAY_TABWIDGET_H
#define _CUSDR_DISPLAY_TABWIDGET_H

#include <QDialog>
#include <QDockWidget>
#include <QTabWidget>

#include "Util/cusdr_buttons.h"
#include "cusdr_settings.h"
#include "cusdr_displayWidget.h"
#include "cusdr_colorsWidget.h"
#include "cusdr_3DOptionsWidget.h"
#include "GL/cusdr_ogl3DPanel.h"

class DisplayTabWidget : public QTabWidget {
	Q_OBJECT

public:
	DisplayTabWidget(RadioModel *model, QWidget *parent = nullptr);
	~DisplayTabWidget();

public slots:
	QSize	sizeHint() const;
	QSize	minimumSizeHint() const;
	void	addNICChangedConnection();
	void	show3DPanadapter(bool enabled);
	void	create3DDockWidget(QWidget *mainWindow);

protected:
	void	closeEvent(QCloseEvent *event);
	void	showEvent(QShowEvent *event);
	void	enterEvent(QEvent *event);
	void	leaveEvent(QEvent *event);
	void	mouseMoveEvent(QMouseEvent *event);
	void	mousePressEvent(QMouseEvent *event);
	void	mouseReleaseEvent(QMouseEvent *event);

private:
    RadioModel*                             m_radioModel;

	QSDR::_Error				m_error;
	QSDR::_ServerMode			m_serverMode;
	QSDR::_HWInterfaceMode		m_hwInterface;
	QSDR::_DataEngineState		m_dataEngineState;

	ColorOptionsWidget			*m_colorWidget;
	Options3DWidget				*m_3DWidget;
	
	int		m_minimumWidgetWidth;
	int		m_minimumGroupBoxWidth;
	
	// 3D panel dockable window
	QDockWidget					*m_3DDockWidget;
	QGL3DPanel					*m_3DPanel;

public:
	QDockWidget* get3DDockWidget() const { return m_3DDockWidget; }
	QGL3DPanel* get3DPanel() const { return m_3DPanel; }

private:
	QString						m_message;

	void	setupConnections();
	void	syncShow3DPanadapterUi(bool enabled);

private slots:
	void systemStateChanged(
		QSDR::_Error err,
		QSDR::_HWInterfaceMode hwmode,
		QSDR::_ServerMode mode,
		QSDR::_DataEngineState state);

	void setAlexPresence(bool value);
	void setPennyPresence(bool value);
	
signals:
	void	panel3DCreated(QGL3DPanel *panel);
	void	showEvent();
	void	closeEvent();
	void	messageEvent(QString message);
};

#endif // _CUSDR_DISPLAY_TABWIDGET_H
