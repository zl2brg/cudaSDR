#include "Models/SliceModel.h"
#include "noisefilterwidget.h"
#include "ui_noisefilterwidget.h"
#include <QSignalBlocker>

#define btn_height              15
#define btn_width               70
#define btn_widths              32
#define btn_width2              52
#define btn_width3              60



NoiseFilterWidget::NoiseFilterWidget(SliceModel *model, QWidget *parent)
    : QWidget(parent)
    , m_sliceModel(model)
    , ui(new Ui::NoiseFilterWidget)
    , set(Settings::instance())
    , m_serverMode(set->getCurrentServerMode())
    , m_hwInterface(set->getHWInterface())
    , m_dataEngineState(set->getDataEngineState())
    //, m_panadapterMode(set->getPanadapterMode())
    //, m_waterColorScheme(set->getWaterfallColorScheme())
    , m_rx(model->id())
    , m_minimumGroupBoxWidth(set->getMinimumGroupBoxWidth())
    , m_btnSpacing(5)
    , m_fontHeight(0)
    , m_maxFontWidth(0)
    , m_currentReceiver(model->id())
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
    getSettings();
    setMouseTracking(true);
    setContentsMargins(4, 4, 4, 4);
    setWindowOpacity(0.9);
//    ui->frame->setFrameStyle(1);

}

NoiseFilterWidget::~NoiseFilterWidget()
{
    delete ui;
}



void NoiseFilterWidget::setCurrentReceiver(int rx) {
    if (m_rx == rx) return;
    m_rx = rx;
    getSettings();
}

void NoiseFilterWidget::setupConnections() {
    connect(m_sliceModel, &SliceModel::nrModeChanged, this, [this](int value) {
        QSignalBlocker blocker(ui->nrModeComboBox);
        ui->nrModeComboBox->setCurrentIndex(value);
    });
    connect(m_sliceModel, &SliceModel::nbModeChanged, this, [this](int value) {
        QSignalBlocker blocker(ui->nbModeComboBox);
        ui->nbModeComboBox->setCurrentIndex(value);
    });
    connect(m_sliceModel, &SliceModel::nr2GainMethodChanged, this, [this](int value) {
        QSignalBlocker blocker(ui->nr2GainComboBox);
        ui->nr2GainComboBox->setCurrentIndex(value);
    });
    connect(m_sliceModel, &SliceModel::nrAgcChanged, this, [this](int value) {
        QSignalBlocker blocker1(ui->preAGCCheckBox);
        QSignalBlocker blocker2(ui->postAGCCheckBox);
        ui->preAGCCheckBox->setChecked(value == 0);
        ui->postAGCCheckBox->setChecked(value != 0);
    });
    connect(m_sliceModel, &SliceModel::nr2AeChanged, this, [this](bool value) {
        QSignalBlocker blocker(ui->nr2aeCheckBox);
        ui->nr2aeCheckBox->setChecked(value);
    });

    connect(m_sliceModel, &SliceModel::anfChanged, this, [this](bool value) {
        QSignalBlocker blocker(ui->anfCheckBox);
        ui->anfCheckBox->setChecked(value);
    });

    connect(m_sliceModel, &SliceModel::snbChanged, this, [this](bool value) {
        QSignalBlocker blocker(ui->snbCheckBox);
        ui->snbCheckBox->setChecked(value);
    });
    
    connect(m_sliceModel, &SliceModel::nr2NpeMethodChanged, this, [this](int value) {
        QSignalBlocker blocker1(ui->omsCheckBox);
        QSignalBlocker blocker2(ui->mmseCheckBox);
        ui->omsCheckBox->setChecked(value == 0);
        ui->mmseCheckBox->setChecked(value != 0);
    });

    CHECKED_CONNECT(ui->nbModeComboBox, &QComboBox::currentIndexChanged, this, &NoiseFilterWidget::nbModeChanged);
    CHECKED_CONNECT(ui->nrModeComboBox, &QComboBox::currentIndexChanged, this, &NoiseFilterWidget::nfModeChanged);
    CHECKED_CONNECT(ui->nr2GainComboBox, &QComboBox::currentIndexChanged, this, &NoiseFilterWidget::nr2GainChanged);
    CHECKED_CONNECT(ui->nr2aeCheckBox, &QCheckBox::toggled, this, &NoiseFilterWidget::nr2aeChanged);
    CHECKED_CONNECT(ui->snbCheckBox, &QCheckBox::toggled, this, &NoiseFilterWidget::snbChanged);
    CHECKED_CONNECT(ui->anfCheckBox, &QCheckBox::toggled, this, &NoiseFilterWidget::anfChanged);
    CHECKED_CONNECT(ui->omsCheckBox, &QCheckBox::toggled, this, &NoiseFilterWidget::omsChanged);
    CHECKED_CONNECT(ui->mmseCheckBox, &QCheckBox::toggled, this, &NoiseFilterWidget::mmseChanged);
    CHECKED_CONNECT(ui->preAGCCheckBox, &QCheckBox::toggled, this, &NoiseFilterWidget::preAgcChanged);
    CHECKED_CONNECT(ui->postAGCCheckBox, &QCheckBox::toggled, this, &NoiseFilterWidget::postAgcChanged);
}




void NoiseFilterWidget::systemStateChanged(
        QSDR::_Error err,
        QSDR::_HWInterfaceMode hwmode,
        QSDR::_ServerMode mode,
        QSDR::_DataEngineState state)
{
    Q_UNUSED (err)

    if (m_hwInterface != hwmode)
        m_hwInterface = hwmode;


    if (m_serverMode != mode)
        m_serverMode = mode;

    if (m_dataEngineState != state)
        m_dataEngineState = state;

    update();
}

void NoiseFilterWidget::getSettings() {
    const QSignalBlocker blockNrMode(ui->nrModeComboBox);
    const QSignalBlocker blockNbMode(ui->nbModeComboBox);
    const QSignalBlocker blockNr2Gain(ui->nr2GainComboBox);
    const QSignalBlocker blockSnb(ui->snbCheckBox);
    const QSignalBlocker blockAnf(ui->anfCheckBox);
    const QSignalBlocker blockOms(ui->omsCheckBox);
    const QSignalBlocker blockMmse(ui->mmseCheckBox);
    const QSignalBlocker blockPreAgc(ui->preAGCCheckBox);
    const QSignalBlocker blockPostAgc(ui->postAGCCheckBox);
    const QSignalBlocker blockNr2Ae(ui->nr2aeCheckBox);

    ui->nrModeComboBox->setCurrentIndex(m_sliceModel->nrMode());
    ui->nbModeComboBox->setCurrentIndex(m_sliceModel->nbMode());
    ui->nr2GainComboBox->setCurrentIndex(m_sliceModel->nr2GainMethod());
    ui->snbCheckBox->setChecked(m_sliceModel->snb());
    ui->anfCheckBox->setChecked(m_sliceModel->anf());
    int nr2Npe = m_sliceModel->nr2NpeMethod();
    ui->omsCheckBox->setChecked(nr2Npe == 0);
    ui->mmseCheckBox->setChecked(nr2Npe != 0);
    int agcMode = m_sliceModel->nrAgc();
    ui->preAGCCheckBox->setChecked(agcMode == 0);
    ui->postAGCCheckBox->setChecked(agcMode != 0);
    ui->nr2aeCheckBox->setChecked(m_sliceModel->nr2Ae());
}


void NoiseFilterWidget::nbModeChanged(int value) {
    m_sliceModel->setNbMode(value);
}

void NoiseFilterWidget::nfModeChanged(int value){
    m_sliceModel->setNrMode(value);
}

void NoiseFilterWidget::nr2GainChanged(int value) {
    m_sliceModel->setNr2GainMethod(value);
}

void NoiseFilterWidget::npeModeChanged(int value) {
    m_sliceModel->setNr2NpeMethod(value);
}

void NoiseFilterWidget::agcProcChanged(int value) {
    m_sliceModel->setNrAgc(value);
}

void NoiseFilterWidget::anfChanged(bool value) {
    m_sliceModel->setAnf(value);
}

void NoiseFilterWidget::snbChanged(bool value) {
    m_sliceModel->setSnb(value);
}

void NoiseFilterWidget::nr2aeChanged(bool value) {
    m_sliceModel->setNr2Ae(value);
}

void NoiseFilterWidget::omsChanged(bool value) {
    if (!value) return;
    m_sliceModel->setNr2NpeMethod(0);
}

void NoiseFilterWidget::mmseChanged(bool value) {
    if (!value) return;
    m_sliceModel->setNr2NpeMethod(1);
}


void NoiseFilterWidget::preAgcChanged(bool value) {
    if (!value) return;
    m_sliceModel->setNrAgc(0);
}


void NoiseFilterWidget::postAgcChanged(bool value) {
    if (!value) return;
    m_sliceModel->setNrAgc(1);
}
