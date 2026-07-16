#include "noisefilterwidget.h"
#include "ui_noisefilterwidget.h"
#include <QSignalBlocker>

#define btn_height              15
#define btn_width               70
#define btn_widths              32
#define btn_width2              52
#define btn_width3              60

NoiseFilterWidget::NoiseFilterWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NoiseFilterWidget)
    , m_rx(0)
    , m_minimumGroupBoxWidth(240)
    , m_btnSpacing(5)
    , m_fontHeight(0)
    , m_maxFontWidth(0)
    , m_currentReceiver(0)
    , m_mouseOver(false)
{
    setContentsMargins(4, 0, 4, 0);
    ui->setupUi(this);
    ui->anfCheckBox->setFont(QFont("Arial", 8));

    ui->label->setFrameStyle(QFrame::Box | QFrame::Raised);
    ui->label->setFont(QFont("Arial", 8));

    ui->label_2->setFrameStyle(QFrame::Box | QFrame::Raised);
    ui->label_2->setFont(QFont("Arial", 8));

    ui->label_3->setFrameStyle(QFrame::Box | QFrame::Raised);
    ui->label_3->setFont(QFont("Arial", 8));

    ui->label_4->setFrameStyle(QFrame::Box | QFrame::Raised);
    ui->label_4->setFont(QFont("Arial", 8));

    ui->label_5->setFrameStyle(QFrame::Box | QFrame::Raised);
    ui->label_5->setFont(QFont("Arial", 8));

    ui->nbModeComboBox->setFont(QFont("Arial", 8));
    ui->nrModeComboBox->setFont(QFont("Arial", 8));

    ui->nr2GainComboBox->setFont(QFont("Arial", 8));

    ui->mmseCheckBox->setFont(QFont("Arial", 8));

    ui->omsCheckBox->setFont(QFont("Arial", 8));

    ui->postAGCCheckBox->setFont(QFont("Arial", 8));

    ui->preAGCCheckBox->setFont(QFont("Arial", 8));

    ui->nr2aeCheckBox->setFont(QFont("Arial", 8));
    
    setupConnections();
    setMouseTracking(true);
    setContentsMargins(4, 4, 4, 4);
    setWindowOpacity(0.9);
}

NoiseFilterWidget::~NoiseFilterWidget()
{
    delete ui;
}

void NoiseFilterWidget::setupConnections() {
    connect(ui->nbModeComboBox, &QComboBox::currentIndexChanged, this, &NoiseFilterWidget::nbModeChanged);
    connect(ui->nrModeComboBox, &QComboBox::currentIndexChanged, this, &NoiseFilterWidget::nfModeChanged);
    connect(ui->nr2GainComboBox, &QComboBox::currentIndexChanged, this, &NoiseFilterWidget::nr2GainChanged);
    connect(ui->nr2aeCheckBox, &QCheckBox::toggled, this, &NoiseFilterWidget::nr2aeChanged);
    connect(ui->snbCheckBox, &QCheckBox::toggled, this, &NoiseFilterWidget::snbChanged);
    connect(ui->anfCheckBox, &QCheckBox::toggled, this, &NoiseFilterWidget::anfChanged);
    connect(ui->omsCheckBox, &QCheckBox::toggled, this, &NoiseFilterWidget::omsChanged);
    connect(ui->mmseCheckBox, &QCheckBox::toggled, this, &NoiseFilterWidget::mmseChanged);
    connect(ui->preAGCCheckBox, &QCheckBox::toggled, this, &NoiseFilterWidget::preAgcChanged);
    connect(ui->postAGCCheckBox, &QCheckBox::toggled, this, &NoiseFilterWidget::postAgcChanged);
}

void NoiseFilterWidget::systemStateChanged(
        QSDR::_Error err,
        QSDR::_HWInterfaceMode hwmode,
        QSDR::_ServerMode mode,
        QSDR::_DataEngineState state)
{
    Q_UNUSED (err)
    Q_UNUSED (hwmode)
    Q_UNUSED (mode)
    Q_UNUSED (state)
    update();
}

void NoiseFilterWidget::nbModeChanged(int value) {
    emit nbModeRequested(value);
}

void NoiseFilterWidget::nfModeChanged(int value){
    emit nrModeRequested(value);
}

void NoiseFilterWidget::nr2GainChanged(int value) {
    emit nr2GainMethodRequested(value);
}

void NoiseFilterWidget::anfChanged(bool value) {
    emit anfRequested(value);
}

void NoiseFilterWidget::snbChanged(bool value) {
    emit snbRequested(value);
}

void NoiseFilterWidget::nr2aeChanged(bool value) {
    emit nr2AeRequested(value);
}

void NoiseFilterWidget::omsChanged(bool value) {
    if (!value) return;
    emit nr2NpeMethodRequested(0);
}

void NoiseFilterWidget::mmseChanged(bool value) {
    if (!value) return;
    emit nr2NpeMethodRequested(1);
}

void NoiseFilterWidget::preAgcChanged(bool value) {
    if (!value) return;
    emit nrAgcRequested(0);
}

void NoiseFilterWidget::postAgcChanged(bool value) {
    if (!value) return;
    emit nrAgcRequested(1);
}

void NoiseFilterWidget::setNrMode(int value) {
    QSignalBlocker blocker(ui->nrModeComboBox);
    ui->nrModeComboBox->setCurrentIndex(value);
}

void NoiseFilterWidget::setNbMode(int value) {
    QSignalBlocker blocker(ui->nbModeComboBox);
    ui->nbModeComboBox->setCurrentIndex(value);
}

void NoiseFilterWidget::setNr2GainMethod(int value) {
    QSignalBlocker blocker(ui->nr2GainComboBox);
    ui->nr2GainComboBox->setCurrentIndex(value);
}

void NoiseFilterWidget::setNr2NpeMethod(int value) {
    QSignalBlocker blocker1(ui->omsCheckBox);
    QSignalBlocker blocker2(ui->mmseCheckBox);
    ui->omsCheckBox->setChecked(value == 0);
    ui->mmseCheckBox->setChecked(value != 0);
}

void NoiseFilterWidget::setNrAgc(int value) {
    QSignalBlocker blocker1(ui->preAGCCheckBox);
    QSignalBlocker blocker2(ui->postAGCCheckBox);
    ui->preAGCCheckBox->setChecked(value == 0);
    ui->postAGCCheckBox->setChecked(value != 0);
}

void NoiseFilterWidget::setNr2Ae(bool value) {
    QSignalBlocker blocker(ui->nr2aeCheckBox);
    ui->nr2aeCheckBox->setChecked(value);
}

void NoiseFilterWidget::setSnb(bool value) {
    QSignalBlocker blocker(ui->snbCheckBox);
    ui->snbCheckBox->setChecked(value);
}

void NoiseFilterWidget::setAnf(bool value) {
    QSignalBlocker blocker(ui->anfCheckBox);
    ui->anfCheckBox->setChecked(value);
}
