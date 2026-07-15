#include "Models/RadioModel.h"
#include "Models/RadioTelemetry.h"
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

DisplayTabWidget::DisplayTabWidget(RadioModel *model, QWidget *parent)
	: QTabWidget(parent)
        , m_radioModel(model)
	, m_minimumWidgetWidth(250)
	, m_minimumGroupBoxWidth(240)
	, m_3DDockWidget(nullptr)
	, m_3DPanel(nullptr)
{
	setContentsMargins(4, 4, 4, 0);
	setMouseTracking(true);
	setUsesScrollButtons(true);
	tabBar()->setExpanding(false);
	
	m_displayWidget = new DisplayOptionsWidget(m_radioModel, this);
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
    if (m_3DDockWidget) {
        delete m_3DDockWidget;
        m_3DDockWidget = nullptr;
        m_3DPanel = nullptr;
    }

    disconnect(0, 0, 0);
    
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
	CHECKED_CONNECT(m_3DWidget, &Options3DWidget::show3DPanadapterChanged, this, &DisplayTabWidget::syncShow3DPanadapterUi);
}

void DisplayTabWidget::systemStateChanged(
	QSDR::_Error err,
	QSDR::_HWInterfaceMode hwmode,
	QSDR::_ServerMode mode,
	QSDR::_DataEngineState state
) {
	Q_UNUSED(err)
	m_hwInterface = hwmode;
	m_serverMode = mode;
	m_dataEngineState = state;
}

void DisplayTabWidget::setPennyPresence(bool value) {
	Q_UNUSED(value)
}

void DisplayTabWidget::setAlexPresence(bool value) {
	Q_UNUSED(value)
}

void DisplayTabWidget::addNICChangedConnection() {
}

void DisplayTabWidget::create3DDockWidget(QWidget *mainWindow) {
	if (!m_3DDockWidget && mainWindow) {
		m_3DDockWidget = new QDockWidget(tr("3D Panadapter"), mainWindow);
		m_3DDockWidget->setObjectName("OGL3DPanelDock");
		m_3DDockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
		m_3DDockWidget->setFeatures(QDockWidget::DockWidgetClosable | 
		                            QDockWidget::DockWidgetFloatable | 
		                            QDockWidget::DockWidgetMovable);
		m_3DDockWidget->setMinimumSize(800, 400);
		
		m_3DPanel = new QGL3DPanel(m_3DDockWidget, 0);
		m_3DPanel->setMinimumSize(400, 300);
		m_3DDockWidget->setWidget(m_3DPanel);
		
		if (m_radioModel && m_radioModel->telemetry()) {
			connect(m_radioModel->telemetry(), &RadioTelemetry::spectrumBufferChanged,
			        m_3DPanel, &QGL3DPanel::setSpectrumBuffer);
		}
			
		// Emit signal so controller can connect Settings signals
		emit panel3DCreated(m_3DPanel);
		
		CHECKED_CONNECT(m_3DWidget, &Options3DWidget::heightScaleValueChanged, m_3DPanel, &QGL3DPanel::setHeightScale);
		CHECKED_CONNECT(m_3DWidget, &Options3DWidget::frequencyScaleValueChanged, m_3DPanel, &QGL3DPanel::setFrequencyScale);
		CHECKED_CONNECT(m_3DWidget, &Options3DWidget::timeScaleValueChanged, m_3DPanel, &QGL3DPanel::setTimeScale);
		CHECKED_CONNECT(m_3DWidget, &Options3DWidget::updateIntervalValueChanged, m_3DPanel, &QGL3DPanel::setUpdateRate);
		CHECKED_CONNECT(m_3DWidget, &Options3DWidget::showGridValueChanged, m_3DPanel, &QGL3DPanel::setShowGrid);
		CHECKED_CONNECT(m_3DWidget, &Options3DWidget::showAxesValueChanged, m_3DPanel, &QGL3DPanel::setShowAxes);
		CHECKED_CONNECT(m_3DWidget, &Options3DWidget::wireframeModeValueChanged, m_3DPanel, &QGL3DPanel::setWireframeMode);
		CHECKED_CONNECT(m_3DWidget, &Options3DWidget::waterfallOffsetValueChanged, m_3DPanel, &QGL3DPanel::setWaterfallOffset);
		
		m_3DWidget->emitInitialValues();

		connect(m_3DDockWidget, &QDockWidget::visibilityChanged, this, [this](bool visible) {
			if (visible != m_3DWidget->is3DEnabled())
				syncShow3DPanadapterUi(visible);
		});
	}
}

void DisplayTabWidget::syncShow3DPanadapterUi(bool enabled) {
	m_3DWidget->setShow3DPanadapterChecked(enabled, false);
	show3DPanadapter(enabled);
}

void DisplayTabWidget::show3DPanadapter(bool enabled) {
	if (enabled) {
		if (m_3DDockWidget) {
			m_3DDockWidget->setVisible(true);
			m_3DDockWidget->show();
			m_3DDockWidget->raise();
			m_3DDockWidget->activateWindow();
		}
	} else {
		if (m_3DDockWidget) {
			m_3DDockWidget->hide();
		}
	}
}

void DisplayTabWidget::closeEvent(QCloseEvent *event) {
	emit closeEvent();
	QWidget::closeEvent(event);
}

void DisplayTabWidget::showEvent(QShowEvent *event) {
	emit showEvent();
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
