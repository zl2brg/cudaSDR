#define LOG_COLOROPTIONS_WIDGET

#include "cusdr_colorsWidget.h"

#define	btn_height		15
#define	btn_width		70
#define	btn_widths		38
#define	btn_width2		52
#define	btn_width3		60

ColorOptionsWidget::ColorOptionsWidget(QWidget *parent)
	: QWidget(parent)
	, m_colorTriangle(new QtColorTriangle(this))
	, m_minimumWidgetWidth(250)
	, m_minimumGroupBoxWidth(240)
	, m_btnSpacing(5)
	, m_currentReceiver(0)
	, m_btnChooserHit(0)
	, m_sampleRate(48000)
{
	setContentsMargins(4, 0, 4, 0);
	setMouseTracking(true);

	createColorChooserWidget();

    mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(0,0,0,0);
	mainLayout->addSpacing(8);

    hbox1 = new QHBoxLayout;
	hbox1->setSpacing(0);
    hbox1->setContentsMargins(0,0,0,0);
	hbox1->addStretch();
	hbox1->addWidget(m_colorChooserWidget);
	hbox1->addStretch();

	mainLayout->addLayout(hbox1);
	mainLayout->addStretch();
	setLayout(mainLayout);
}

ColorOptionsWidget::~ColorOptionsWidget() {
	disconnect(0, 0, 0);
}

QSize ColorOptionsWidget::sizeHint() const {
	return {m_minimumWidgetWidth, height()};
}

QSize ColorOptionsWidget::minimumSizeHint() const {
	return {m_minimumWidgetWidth, height()};
}

void ColorOptionsWidget::createColorChooserWidget() {
	m_resetBtn = new AeroButton("Reset", this);
	m_resetBtn->setRoundness(0);
	m_resetBtn->setFixedSize (btn_width, btn_height);
	m_resetBtn->setBtnState(AeroButton::OFF);

	CHECKED_CONNECT(m_resetBtn, &AeroButton::clicked, this, &ColorOptionsWidget::resetColors);

	m_okBtn = new AeroButton("Save", this);
	m_okBtn->setRoundness(0);
	m_okBtn->setFixedSize (btn_width, btn_height);
	m_okBtn->setBtnState(AeroButton::OFF);

	CHECKED_CONNECT(m_okBtn, &AeroButton::clicked, this, &ColorOptionsWidget::acceptColors);

	m_setPanBackground = new AeroButton("Pan Background", this);
	m_setPanBackground->setRoundness(0);
	m_setPanBackground->setFixedSize (130, btn_height);
	m_setPanBackground->setBtnState(AeroButton::OFF);
	m_changeColorBtnList.append(m_setPanBackground);

	m_setPanCenterLine = new AeroButton("Pan Grid Center Line", this);
	m_setPanCenterLine->setRoundness(0);
	m_setPanCenterLine->setFixedSize (130, btn_height);
	m_setPanCenterLine->setBtnState(AeroButton::OFF);
	m_changeColorBtnList.append(m_setPanCenterLine);

	m_setPanLine = new AeroButton("Pan Line", this);
	m_setPanLine->setRoundness(0);
	m_setPanLine->setFixedSize (130, btn_height);
	m_setPanLine->setBtnState(AeroButton::OFF);
	m_changeColorBtnList.append(m_setPanLine);

	m_setPanLineFilling = new AeroButton("Pan Line Filling", this);
	m_setPanLineFilling->setRoundness(0);
	m_setPanLineFilling->setFixedSize (130, btn_height);
	m_setPanLineFilling->setBtnState(AeroButton::OFF);
	m_changeColorBtnList.append(m_setPanLineFilling);

	m_setPanSolidTop = new AeroButton("Pan Solid Top", this);
	m_setPanSolidTop->setRoundness(0);
	m_setPanSolidTop->setFixedSize (130, btn_height);
	m_setPanSolidTop->setBtnState(AeroButton::OFF);
	m_changeColorBtnList.append(m_setPanSolidTop);

	m_setPanSolidBottom = new AeroButton("Pan Solid Bottom", this);
	m_setPanSolidBottom->setRoundness(0);
	m_setPanSolidBottom->setFixedSize (130, btn_height);
	m_setPanSolidBottom->setBtnState(AeroButton::OFF);
	m_changeColorBtnList.append(m_setPanSolidBottom);

	m_setWaterfall = new AeroButton("Waterfall", this);
	m_setWaterfall->setRoundness(0);
	m_setWaterfall->setFixedSize (130, btn_height);
	m_setWaterfall->setBtnState(AeroButton::OFF);
	m_changeColorBtnList.append(m_setWaterfall);

	m_setWideBandLine = new AeroButton("Wide Band Line", this);
	m_setWideBandLine->setRoundness(0);
	m_setWideBandLine->setFixedSize (130, btn_height);
	m_setWideBandLine->setBtnState(AeroButton::OFF);
	m_changeColorBtnList.append(m_setWideBandLine);

	m_setWideBandFilling = new AeroButton("Wide Band Line Filling", this);
	m_setWideBandFilling->setRoundness(0);
	m_setWideBandFilling->setFixedSize (130, btn_height);
	m_setWideBandFilling->setBtnState(AeroButton::OFF);
	m_changeColorBtnList.append(m_setWideBandFilling);

	m_setWideBandSolidTop = new AeroButton("Wide Band Solid Top", this);
	m_setWideBandSolidTop->setRoundness(0);
	m_setWideBandSolidTop->setFixedSize (130, btn_height);
	m_setWideBandSolidTop->setBtnState(AeroButton::OFF);
	m_changeColorBtnList.append(m_setWideBandSolidTop);

	m_setWideBandSolidBottom = new AeroButton("Wide Band Solid Bottom", this);
	m_setWideBandSolidBottom->setRoundness(0);
	m_setWideBandSolidBottom->setFixedSize (130, btn_height);
	m_setWideBandSolidBottom->setBtnState(AeroButton::OFF);
	m_changeColorBtnList.append(m_setWideBandSolidBottom);

	m_setDistanceLine = new AeroButton("Distance Line", this);
	m_setDistanceLine->setRoundness(0);
	m_setDistanceLine->setFixedSize (130, btn_height);
	m_setDistanceLine->setBtnState(AeroButton::OFF);
	m_changeColorBtnList.append(m_setDistanceLine);

	m_setDistanceLineFilling = new AeroButton("Distance Line Filling", this);
	m_setDistanceLineFilling->setRoundness(0);
	m_setDistanceLineFilling->setFixedSize (130, btn_height);
	m_setDistanceLineFilling->setBtnState(AeroButton::OFF);
	m_changeColorBtnList.append(m_setDistanceLineFilling);

	m_setGridLine = new AeroButton("Grid Line", this);
	m_setGridLine->setRoundness(0);
	m_setGridLine->setFixedSize (130, btn_height);
	m_setGridLine->setBtnState(AeroButton::OFF);
	m_changeColorBtnList.append(m_setGridLine);

	m_setPanFilter = new AeroButton("Pan Filter", this);
	m_setPanFilter->setRoundness(0);
	m_setPanFilter->setFixedSize (130, btn_height);
	m_setPanFilter->setBtnState(AeroButton::OFF);
	m_changeColorBtnList.append(m_setPanFilter);

	foreach(AeroButton *btn, m_changeColorBtnList) {
		CHECKED_CONNECT(btn, &AeroButton::clicked, this, &ColorOptionsWidget::colorChooserChanged);
	}

	CHECKED_CONNECT(m_colorTriangle, &QtColorTriangle::colorChanged, this, &ColorOptionsWidget::triangleColorChanged);

	hbox = new QHBoxLayout;
	hbox->setSpacing(4);
	hbox->addWidget(m_resetBtn);
	hbox->addWidget(m_okBtn);

	vbox = new QVBoxLayout;
	vbox->setSpacing(4);
	vbox->addWidget(m_colorTriangle);
	vbox->addLayout(hbox);

	grid1 = new QGridLayout;
	grid1->setSpacing(2);

	grid1->addWidget(m_setPanBackground, 0, 0);
	grid1->addWidget(m_setPanCenterLine, 0, 1);
	grid1->addWidget(m_setPanLine, 1, 0);
	grid1->addWidget(m_setPanLineFilling, 1, 1);
	grid1->addWidget(m_setPanSolidTop, 2, 0);
	grid1->addWidget(m_setPanSolidBottom, 2, 1);
	grid1->addWidget(m_setWaterfall, 3, 0);
	grid1->addWidget(m_setWideBandLine, 3, 1);
	grid1->addWidget(m_setWideBandFilling, 4, 0);
	grid1->addWidget(m_setWideBandSolidTop, 4, 1);
	grid1->addWidget(m_setWideBandSolidBottom, 5, 0);
	grid1->addWidget(m_setDistanceLine, 5, 1);
	grid1->addWidget(m_setDistanceLineFilling, 6, 0);
	grid1->addWidget(m_setGridLine, 6, 1);
	grid1->addWidget(m_setPanFilter, 7, 0);

	QGridLayout *gridLayout = new QGridLayout;
	gridLayout->setSpacing(2);

	gridLayout->addLayout(vbox, 0, 0, Qt::AlignCenter);
	gridLayout->addLayout(grid1, 1, 0, Qt::AlignCenter);

	m_colorChooserWidget = new QGroupBox(tr("Spectrum Color Options"), this);
	m_colorChooserWidget->setMinimumWidth(m_minimumGroupBoxWidth);
	m_colorChooserWidget->setLayout(gridLayout);
	m_colorChooserWidget->setFont(QFont("Arial", 8));
}

void ColorOptionsWidget::setPanadapterColors(const TPanadapterColors& colors) {
	m_panadapterColors = colors;
	m_oldPanadapterColors = colors;

	m_setPanBackground->setColorOn(colors.panBackgroundColor);
	m_setPanCenterLine->setColorOn(colors.panCenterLineColor);
	m_setPanLine->setColorOn(colors.panLineColor);
	m_setPanLineFilling->setColorOn(colors.panLineFilledColor);
	m_setPanSolidTop->setColorOn(colors.panSolidTopColor);
	m_setPanSolidBottom->setColorOn(colors.panSolidBottomColor);
	m_setWaterfall->setColorOn(colors.waterfallColor);
	m_setWideBandLine->setColorOn(colors.wideBandLineColor);
	m_setWideBandFilling->setColorOn(colors.wideBandFilledColor);
	m_setWideBandSolidTop->setColorOn(colors.wideBandSolidTopColor);
	m_setWideBandSolidBottom->setColorOn(colors.wideBandSolidBottomColor);
	m_setDistanceLine->setColorOn(colors.distanceLineColor);
	m_setDistanceLineFilling->setColorOn(colors.distanceLineFilledColor);
	m_setGridLine->setColorOn(colors.gridLineColor);
	m_setPanFilter->setColorOn(colors.panFilterColor);

	foreach(AeroButton *btn, m_changeColorBtnList) {
		btn->update();
	}
}

void ColorOptionsWidget::colorChooserChanged() {
	AeroButton *button = qobject_cast<AeroButton *>(sender());
	int btnHit = m_changeColorBtnList.indexOf(button);

	if (btnHit >= 0) {
		foreach(AeroButton *btn, m_changeColorBtnList) {
			btn->setBtnState(AeroButton::OFF);
			btn->update();
		}

		button->setBtnState(AeroButton::ON);
		button->update();

		m_btnChooserHit = btnHit;
	} else {
		return;
	}

	switch (m_btnChooserHit) {
		case 0:
			m_colorTriangle->setColor(m_panadapterColors.panBackgroundColor);
			break;
		case 1:
			m_colorTriangle->setColor(m_panadapterColors.panCenterLineColor);
			break;
		case 2:
			m_colorTriangle->setColor(m_panadapterColors.panLineColor);
			break;
		case 3:
			m_colorTriangle->setColor(m_panadapterColors.panLineFilledColor);
			break;
		case 4:
			m_colorTriangle->setColor(m_panadapterColors.panSolidTopColor);
			break;
		case 5:
			m_colorTriangle->setColor(m_panadapterColors.panSolidBottomColor);
			break;
		case 6:
			m_colorTriangle->setColor(m_panadapterColors.waterfallColor);
			break;
		case 7:
			m_colorTriangle->setColor(m_panadapterColors.wideBandLineColor);
			break;
		case 8:
			m_colorTriangle->setColor(m_panadapterColors.wideBandFilledColor);
			break;
		case 9:
			m_colorTriangle->setColor(m_panadapterColors.wideBandSolidTopColor);
			break;
		case 10:
			m_colorTriangle->setColor(m_panadapterColors.wideBandSolidBottomColor);
			break;
		case 11:
			m_colorTriangle->setColor(m_panadapterColors.distanceLineColor);
			break;
		case 12:
			m_colorTriangle->setColor(m_panadapterColors.distanceLineFilledColor);
			break;
		case 13:
			m_colorTriangle->setColor(m_panadapterColors.gridLineColor);
			break;
		case 14:
			m_colorTriangle->setColor(m_panadapterColors.panFilterColor);
			break;
	}
}

void ColorOptionsWidget::resetColors() {
	m_panadapterColors = m_oldPanadapterColors;

	foreach(AeroButton *btn, m_changeColorBtnList) {
		btn->setBtnState(AeroButton::OFF);
		btn->update();
	}
	emit panadapterColorsRequested(m_panadapterColors);
}

void ColorOptionsWidget::acceptColors() {
	m_oldPanadapterColors = m_panadapterColors;
	emit panadapterColorsRequested(m_panadapterColors);
}

void ColorOptionsWidget::triangleColorChanged(QColor color) {
	m_currentColor = color;
	m_changeColorBtnList.at(m_btnChooserHit)->setColorOn(color);
	m_changeColorBtnList.at(m_btnChooserHit)->update();

	switch (m_btnChooserHit) {
		case 0:
			m_panadapterColors.panBackgroundColor = color;
			break;
		case 1:
			m_panadapterColors.panCenterLineColor = color;
			break;
		case 2:
			m_panadapterColors.panLineColor = color;
			break;
		case 3:
			m_panadapterColors.panLineFilledColor = color;
			break;
		case 4:
			m_panadapterColors.panSolidTopColor = color;
			break;
		case 5:
			m_panadapterColors.panSolidBottomColor = color;
			break;
		case 6:
			m_panadapterColors.waterfallColor = color;
			break;
		case 7:
			m_panadapterColors.wideBandLineColor = color;
			break;
		case 8:
			m_panadapterColors.wideBandFilledColor = color;
			break;
		case 9:
			m_panadapterColors.wideBandSolidTopColor = color;
			break;
		case 10:
			m_panadapterColors.wideBandSolidBottomColor = color;
			break;
		case 11:
			m_panadapterColors.distanceLineColor = color;
			break;
		case 12:
			m_panadapterColors.distanceLineFilledColor = color;
			break;
		case 13:
			m_panadapterColors.gridLineColor = color;
			break;
		case 14:
			m_panadapterColors.panFilterColor = color;
			break;
	}
	emit panadapterColorsRequested(m_panadapterColors);
}
