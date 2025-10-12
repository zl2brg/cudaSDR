/**
* @file  cusdr_3DOptionsWidget.cpp
* @brief 3D display options widget implementation for cuSDR
* @author Simon Brown, ZL2BRG
* @version 0.1
* @date 2024-12-28
*/

#include "cusdr_3DOptionsWidget.h"

#include <QBoxLayout>
#include <QGridLayout>
#include <QSpacerItem>

#define	btn_height		15
#define	btn_width		74
#define	btn_width2		52
#define	btn_widths		40

Options3DWidget::Options3DWidget(QWidget *parent)
    : QWidget(parent)
    , set(Settings::instance())
    , m_minimumWidgetWidth(set->getMinimumWidgetWidth())
    , m_minimumGroupBoxWidth(set->getMinimumGroupBoxWidth())
{
    setMinimumWidth(m_minimumWidgetWidth);
    setContentsMargins(4, 8, 4, 0);
    setMouseTracking(true);

    createGeneralGroup();
    createRenderingGroup();
    createPerformanceGroup();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(4, 8, 4, 0);
    mainLayout->addWidget(m_generalGroup);
    mainLayout->addWidget(m_renderingGroup);
    mainLayout->addWidget(m_performanceGroup);
    mainLayout->addStretch();
    setLayout(mainLayout);

    setupConnections();
}

Options3DWidget::~Options3DWidget() {
    disconnect(this, 0, 0, 0);
    disconnect(set, 0, this, 0);
}

QSize Options3DWidget::sizeHint() const {
    return QSize(m_minimumWidgetWidth, height());
}

QSize Options3DWidget::minimumSizeHint() const {
    return QSize(m_minimumWidgetWidth, height());
}

void Options3DWidget::setupConnections() {
    // Connect to settings changes if needed
}

void Options3DWidget::createGeneralGroup() {
    m_generalGroup = new QGroupBox("3D Display", this);
    m_generalGroup->setMinimumWidth(m_minimumGroupBoxWidth);
    m_generalGroup->setStyleSheet(set->getWidgetStyle());
    m_generalGroup->setFont(QFont("Arial", 8));

    m_enable3DCheckBox = new QCheckBox("Enable 3D Panadapter", this);
    m_enable3DCheckBox->setChecked(false); // Default to disabled for safety
    m_enable3DCheckBox->setFont(QFont("Arial", 8));

    connect(m_enable3DCheckBox, &QCheckBox::toggled, this, &Options3DWidget::enable3DChanged);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(8, 12, 8, 8);
    layout->addWidget(m_enable3DCheckBox);
    m_generalGroup->setLayout(layout);
}

void Options3DWidget::createRenderingGroup() {
    m_renderingGroup = new QGroupBox("Rendering Options", this);
    m_renderingGroup->setMinimumWidth(m_minimumGroupBoxWidth);
    m_renderingGroup->setStyleSheet(set->getWidgetStyle());
    m_renderingGroup->setFont(QFont("Arial", 8));

    // Height scale
    m_heightScaleLabel = new QLabel("Height Scale: 10.0", this);
    m_heightScaleLabel->setFont(QFont("Arial", 8));
    m_heightScaleSlider = new QSlider(Qt::Horizontal, this);
    m_heightScaleSlider->setRange(1, 50);
    m_heightScaleSlider->setValue(10);
    m_heightScaleSlider->setTickPosition(QSlider::TicksBelow);
    m_heightScaleSlider->setTickInterval(10);

    connect(m_heightScaleSlider, &QSlider::valueChanged, this, &Options3DWidget::heightScaleChanged);

    // Frequency scale
    m_frequencyScaleLabel = new QLabel("Frequency Scale: 1.0", this);
    m_frequencyScaleLabel->setFont(QFont("Arial", 8));
    m_frequencyScaleSlider = new QSlider(Qt::Horizontal, this);
    m_frequencyScaleSlider->setRange(1, 50);
    m_frequencyScaleSlider->setValue(10); // 1.0 scale = value 10
    m_frequencyScaleSlider->setTickPosition(QSlider::TicksBelow);
    m_frequencyScaleSlider->setTickInterval(10);

    connect(m_frequencyScaleSlider, &QSlider::valueChanged, this, &Options3DWidget::frequencyScaleChanged);

    // Time scale
    m_timeScaleLabel = new QLabel("Time Scale: 1.0", this);
    m_timeScaleLabel->setFont(QFont("Arial", 8));
    m_timeScaleSlider = new QSlider(Qt::Horizontal, this);
    m_timeScaleSlider->setRange(1, 50);
    m_timeScaleSlider->setValue(10); // 1.0 scale = value 10
    m_timeScaleSlider->setTickPosition(QSlider::TicksBelow);
    m_timeScaleSlider->setTickInterval(10);

    connect(m_timeScaleSlider, &QSlider::valueChanged, this, &Options3DWidget::timeScaleChanged);

    // Display options
    m_showGridCheckBox = new QCheckBox("Show Grid", this);
    m_showGridCheckBox->setChecked(true);
    m_showGridCheckBox->setFont(QFont("Arial", 8));

    m_showAxesCheckBox = new QCheckBox("Show Axes", this);
    m_showAxesCheckBox->setChecked(true);
    m_showAxesCheckBox->setFont(QFont("Arial", 8));

    m_wireframeModeCheckBox = new QCheckBox("Wireframe Mode", this);
    m_wireframeModeCheckBox->setChecked(false);
    m_wireframeModeCheckBox->setFont(QFont("Arial", 8));

    // Contour mode
    m_showContoursCheckBox = new QCheckBox("Show Contours", this);
    m_showContoursCheckBox->setChecked(false);
    m_showContoursCheckBox->setFont(QFont("Arial", 8));
    
    // Contour interval (dB between contours)
    m_contourIntervalLabel = new QLabel("Contour Interval: 10.0 dB", this);
    m_contourIntervalLabel->setFont(QFont("Arial", 8));
    m_contourIntervalSlider = new QSlider(Qt::Horizontal, this);
    m_contourIntervalSlider->setRange(5, 20);  // 0.5 to 2.0 (x10)
    m_contourIntervalSlider->setValue(10);  // 1.0 default (x10)
    m_contourIntervalSlider->setMinimumWidth(160);
    
    // Contour minimum level
    m_contourMinLevelLabel = new QLabel("Contour Min Level: -140 dB", this);
    m_contourMinLevelLabel->setFont(QFont("Arial", 8));
    m_contourMinLevelSlider = new QSlider(Qt::Horizontal, this);
    m_contourMinLevelSlider->setRange(-180, -80);  // -180 to -80 dB
    m_contourMinLevelSlider->setValue(-140);  // -140 dB default
    m_contourMinLevelSlider->setMinimumWidth(160);
    
    // Waterfall dBm offset
    m_waterfallOffsetLabel = new QLabel("Waterfall Offset: 0 dB", this);
    m_waterfallOffsetLabel->setFont(QFont("Arial", 8));
    m_waterfallOffsetSlider = new QSlider(Qt::Horizontal, this);
    m_waterfallOffsetSlider->setRange(-50, 50);  // -50 to +50 dB offset
    m_waterfallOffsetSlider->setValue(0);  // 0 dB default
    m_waterfallOffsetSlider->setMinimumWidth(160);

    connect(m_showGridCheckBox, &QCheckBox::toggled, this, &Options3DWidget::showGridChanged);
    connect(m_showAxesCheckBox, &QCheckBox::toggled, this, &Options3DWidget::showAxesChanged);
    connect(m_wireframeModeCheckBox, &QCheckBox::toggled, this, &Options3DWidget::wireframeModeChanged);
    connect(m_showContoursCheckBox, &QCheckBox::toggled, this, &Options3DWidget::showContoursChanged);
    connect(m_contourIntervalSlider, &QSlider::valueChanged, this, &Options3DWidget::contourIntervalChanged);
    connect(m_contourMinLevelSlider, &QSlider::valueChanged, this, &Options3DWidget::contourMinLevelChanged);
    connect(m_waterfallOffsetSlider, &QSlider::valueChanged, this, &Options3DWidget::waterfallOffsetChanged);

    QGridLayout *layout = new QGridLayout;
    layout->setContentsMargins(8, 12, 8, 8);
    layout->setSpacing(4);
    layout->addWidget(m_heightScaleLabel, 0, 0);
    layout->addWidget(m_heightScaleSlider, 1, 0);
    layout->addWidget(m_frequencyScaleLabel, 2, 0);
    layout->addWidget(m_frequencyScaleSlider, 3, 0);
    layout->addWidget(m_timeScaleLabel, 4, 0);
    layout->addWidget(m_timeScaleSlider, 5, 0);
    layout->addWidget(m_showGridCheckBox, 6, 0);
    layout->addWidget(m_showAxesCheckBox, 7, 0);
    layout->addWidget(m_wireframeModeCheckBox, 8, 0);
    layout->addWidget(m_showContoursCheckBox, 9, 0);
    layout->addWidget(m_contourIntervalLabel, 10, 0);
    layout->addWidget(m_contourIntervalSlider, 11, 0);
    layout->addWidget(m_contourMinLevelLabel, 12, 0);
    layout->addWidget(m_contourMinLevelSlider, 13, 0);
    layout->addWidget(m_waterfallOffsetLabel, 14, 0);
    layout->addWidget(m_waterfallOffsetSlider, 15, 0);
    m_renderingGroup->setLayout(layout);
}

void Options3DWidget::createPerformanceGroup() {
    m_performanceGroup = new QGroupBox("Performance", this);
    m_performanceGroup->setMinimumWidth(m_minimumGroupBoxWidth);
    m_performanceGroup->setStyleSheet(set->getWidgetStyle());
    m_performanceGroup->setFont(QFont("Arial", 8));

    // Update interval
    m_updateIntervalLabel = new QLabel("Update Rate: 20 FPS", this);
    m_updateIntervalLabel->setFont(QFont("Arial", 8));
    m_updateIntervalSlider = new QSlider(Qt::Horizontal, this);
    m_updateIntervalSlider->setRange(10, 60); // 10-60 FPS
    m_updateIntervalSlider->setValue(20);
    m_updateIntervalSlider->setTickPosition(QSlider::TicksBelow);
    m_updateIntervalSlider->setTickInterval(10);

    connect(m_updateIntervalSlider, &QSlider::valueChanged, this, &Options3DWidget::updateIntervalChanged);

    // Quality preset
    QLabel *qualityLabel = new QLabel("Quality Preset:", this);
    qualityLabel->setFont(QFont("Arial", 8));
    m_qualityComboBox = new QComboBox(this);
    m_qualityComboBox->addItems(QStringList() << "Performance" << "Balanced" << "Quality");
    m_qualityComboBox->setCurrentIndex(1); // Balanced default
    m_qualityComboBox->setFont(QFont("Arial", 8));

    QGridLayout *layout = new QGridLayout;
    layout->setContentsMargins(8, 12, 8, 8);
    layout->setSpacing(4);
    layout->addWidget(m_updateIntervalLabel, 0, 0);
    layout->addWidget(m_updateIntervalSlider, 1, 0);
    layout->addWidget(qualityLabel, 2, 0);
    layout->addWidget(m_qualityComboBox, 3, 0);
    m_performanceGroup->setLayout(layout);
}

void Options3DWidget::enable3DChanged() {
    bool enabled = m_enable3DCheckBox->isChecked();
    
    // Enable/disable other controls based on 3D enable state
    m_renderingGroup->setEnabled(enabled);
    m_performanceGroup->setEnabled(enabled);
    
    emit show3DPanadapterChanged(enabled);
}

void Options3DWidget::heightScaleChanged(int value) {
    float scale = static_cast<float>(value);
    m_heightScaleLabel->setText(QString("Height Scale: %1").arg(scale, 0, 'f', 1));
    emit heightScaleValueChanged(scale);
}

void Options3DWidget::frequencyScaleChanged(int value) {
    float scale = static_cast<float>(value) / 10.0f; // Convert to 0.1 - 5.0 range
    m_frequencyScaleLabel->setText(QString("Frequency Scale: %1").arg(scale, 0, 'f', 1));
    emit frequencyScaleValueChanged(scale);
}

void Options3DWidget::timeScaleChanged(int value) {
    float scale = static_cast<float>(value) / 10.0f; // Convert to 0.1 - 5.0 range
    m_timeScaleLabel->setText(QString("Time Scale: %1").arg(scale, 0, 'f', 1));
    emit timeScaleValueChanged(scale);
}

void Options3DWidget::updateIntervalChanged(int value) {
    int interval = 1000 / value; // Convert FPS to milliseconds
    m_updateIntervalLabel->setText(QString("Update Rate: %1 FPS").arg(value));
    qDebug() << "3D Options: Update rate slider changed to" << value << "FPS (" << interval << "ms)";
    emit updateIntervalValueChanged(interval);
}

void Options3DWidget::showGridChanged() {
    emit showGridValueChanged(m_showGridCheckBox->isChecked());
}

void Options3DWidget::showAxesChanged() {
    emit showAxesValueChanged(m_showAxesCheckBox->isChecked());
}

void Options3DWidget::wireframeModeChanged() {
    emit wireframeModeValueChanged(m_wireframeModeCheckBox->isChecked());
}

void Options3DWidget::showContoursChanged() {
    emit showContoursValueChanged(m_showContoursCheckBox->isChecked());
}

void Options3DWidget::contourIntervalChanged(int value) {
    float interval = (float)value;  // Direct value mapping: 5-20 dB
    m_contourIntervalLabel->setText(QString("Contour Interval: %1 dB").arg(interval, 0, 'f', 1));
    emit contourIntervalValueChanged(interval);
}

void Options3DWidget::contourMinLevelChanged(int value) {
    float minLevel = (float)value;  // Direct value mapping: -180 to -80 dB
    m_contourMinLevelLabel->setText(QString("Contour Min Level: %1 dB").arg(minLevel, 0, 'f', 0));
    emit contourMinLevelValueChanged(minLevel);
}

void Options3DWidget::waterfallOffsetChanged(int value) {
    float offset = (float)value;  // Direct value mapping: -50 to +50 dB
    m_waterfallOffsetLabel->setText(QString("Waterfall Offset: %1 dB").arg(offset, 0, 'f', 0));
    emit waterfallOffsetValueChanged(offset);
}