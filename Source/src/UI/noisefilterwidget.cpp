#include "noisefilterwidget.h"
#include "ui_noisefilterwidget.h"

#define	btn_height		15
#define	btn_width		70
#define	btn_widths		32
#define	btn_width2		52
#define	btn_width3		60



NoiseFilterWidget::NoiseFilterWidget(QWidget *parent)
    : QWidget(parent)
    , set(Settings::instance())
    , m_serverMode(set->getCurrentServerMode())
    , m_hwInterface(set->getHWInterface())
    , m_dataEngineState(set->getDataEngineState())
    , m_rx(set->getCurrentReceiver())

        //, m_panadapterMode(set->getPanadapterMode())
        //, m_waterColorScheme(set->getWaterfallColorScheme())
    , m_minimumWidgetWidth(set->getMinimumWidgetWidth())
    , m_minimumGroupBoxWidth(set->getMinimumGroupBoxWidth())

    , m_btnSpacing(5)
    , m_mouseOver(false)
    , ui(new Ui::NoiseFilterWidget)

{
    setContentsMargins(4, 0, 4, 0);
    ui->setupUi(this);
    ui->snbCheckBox->setStyleSheet(set->getCheckBoxStyle());
    ui->anfCheckBox->setStyleSheet(set->getCheckBoxStyle());
    ui->anfCheckBox->setFont(QFont("Arial", 8));

    ui->label->setFrameStyle(QFrame::Box | QFrame::Raised);
    ui->label->setStyleSheet(set->getLabelStyle());
    ui->label->setFont(QFont("Arial", 8));

    ui->label_2->setFrameStyle(QFrame::Box | QFrame::Raised);
    ui->label_2->setStyleSheet(set->getLabelStyle());
    ui->label_2->setFont(QFont("Arial", 8));

    ui->label_3->setFrameStyle(QFrame::Box | QFrame::Raised);
    ui->label_3->setStyleSheet(set->getLabelStyle());
    ui->label_3->setFont(QFont("Arial", 8));

    ui->label_4->setFrameStyle(QFrame::Box | QFrame::Raised);
    ui->label_4->setStyleSheet(set->getLabelStyle());
    ui->label_4->setFont(QFont("Arial", 8));

    ui->label_5->setFrameStyle(QFrame::Box | QFrame::Raised);
    ui->label_5->setStyleSheet(set->getLabelStyle());
    ui->label_5->setFont(QFont("Arial", 8));

    ui->nbModeComboBox->setFont(QFont("Arial", 8));
    ui->nbModeComboBox->setStyleSheet(set->getComboBoxStyle());
    ui->nrModeComboBox->setFont(QFont("Arial", 8));
    ui->nrModeComboBox->setStyleSheet(set->getComboBoxStyle());

    ui->nr2GainComboBox->setFont(QFont("Arial", 8));
    ui->nr2GainComboBox->setStyleSheet(set->getComboBoxStyle());

    ui->mmseCheckBox->setFont(QFont("Arial", 8));
    ui->mmseCheckBox->setStyleSheet(set->getCheckBoxStyle());

    ui->omsCheckBox->setFont(QFont("Arial", 8));
    ui->omsCheckBox->setStyleSheet(set->getCheckBoxStyle());

    ui->postAGCCheckBox->setFont(QFont("Arial", 8));
    ui->postAGCCheckBox->setStyleSheet(set->getCheckBoxStyle());

    ui->preAGCCheckBox->setFont(QFont("Arial", 8));
    ui->preAGCCheckBox->setStyleSheet(set->getCheckBoxStyle());

    ui->nr2aeCheckBox->setFont(QFont("Arial", 8));
    ui->nr2aeCheckBox->setStyleSheet(set->getCheckBoxStyle());
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

QSize NoiseFilterWidget::sizeHint() const {

    return QSize(m_minimumWidgetWidth, height());
}

QSize NoiseFilterWidget::minimumSizeHint() const {

    return QSize(m_minimumWidgetWidth, height());
}



void NoiseFilterWidget::setCurrentReceiver(QObject *sender, int rx) {
    Q_UNUSED(sender)
    Q_UNUSED(rx)
}

void NoiseFilterWidget::setupConnections() {
    CHECKED_CONNECT(
            ui->nrModeComboBox ,
            SIGNAL(currentIndexChanged(int)),
            this,
            SLOT(nfModeChanged(int)));

    CHECKED_CONNECT(
            ui->nbModeComboBox ,
            SIGNAL(currentIndexChanged(int)),
            this,
            SLOT(nbModeChanged(int)));

    CHECKED_CONNECT(
            ui->nr2GainComboBox ,
            SIGNAL(currentIndexChanged(int)),
            this,
            SLOT(nr2GainChanged(int)));

    CHECKED_CONNECT(
            ui->snbCheckBox ,
            SIGNAL(toggled(bool)),
            this,
            SLOT(snbChanged(bool)));

    CHECKED_CONNECT(
            ui->anfCheckBox ,
            SIGNAL(toggled(bool)),
            this,
            SLOT(anfChanged(bool)));

    CHECKED_CONNECT(
            ui->omsCheckBox ,
            SIGNAL(toggled(bool)),
            this,
            SLOT(omsChanged(bool)));

    CHECKED_CONNECT(
            ui->mmseCheckBox ,
            SIGNAL(toggled(bool)),
            this,
            SLOT(mmseChanged(bool)));

    CHECKED_CONNECT(
            ui->preAGCCheckBox ,
            SIGNAL(toggled(bool)),
            this,
            SLOT(preAgcChanged(bool)));

    CHECKED_CONNECT(
            ui->postAGCCheckBox ,
            SIGNAL(toggled(bool)),
            this,
            SLOT(postAgcChanged(bool)));

    CHECKED_CONNECT(
            ui->nr2aeCheckBox ,
            SIGNAL(toggled(bool)),
            this,
            SLOT(nr2aeChanged(bool)));

}




void NoiseFilterWidget::systemStateChanged(
        QObject *sender,
        QSDR::_Error err,
        QSDR::_HWInterfaceMode hwmode,
        QSDR::_ServerMode mode,
        QSDR::_DataEngineState state)
{
    Q_UNUSED (sender)
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
    ui->nrModeComboBox->setCurrentIndex(set->getnrMode(m_rx));
    ui->nbModeComboBox->setCurrentIndex(set->getnbMode(m_rx));
    ui->nr2GainComboBox->setCurrentIndex(set->getNr2GainMethod(m_rx));
    ui->snbCheckBox->setChecked(set->getSnb(m_rx));
    ui->anfCheckBox->setChecked(set->getAnf(m_rx));
    int nr2Npe = set->getNr2NpeMethod(m_rx);
    if (nr2Npe == 0) ui->omsCheckBox->setChecked(1);
    else ui->mmseCheckBox->setChecked(1);
    int agcMode = set->getNrAGC(m_rx);
    if (agcMode == 0) ui->preAGCCheckBox->setChecked(1);
    else ui->postAGCCheckBox->setChecked(1);
    ui->nr2aeCheckBox->setChecked(set->getNr2ae(m_rx));
//    ui->mmseCheckBox->setChecked();
}


void NoiseFilterWidget::nbModeChanged(int value) {
    set->setNoiseBlankerMode(m_rx,value);
}

void NoiseFilterWidget::nfModeChanged(int value){
    set->setNoiseFilterMode(m_rx,value);
}

void NoiseFilterWidget::nr2GainChanged(int value) {
    set->setNR2GainMethod(m_rx,value);
}

void NoiseFilterWidget::npeModeChanged(int value) {
    set->setNR2NpeMethod(m_rx,value);
}

void NoiseFilterWidget::agcProcChanged(int value) {
    set->setNRAgc(m_rx,value);
}

void NoiseFilterWidget::anfChanged(bool value) {
    set->setAnf(m_rx,value);
}

void NoiseFilterWidget::snbChanged(bool value) {
    set->setSnb(m_rx,value);
}

void NoiseFilterWidget::nr2aeChanged(bool value) {
    set->setNR2Ae(m_rx,value);
}

void NoiseFilterWidget::omsChanged(bool value) {
 if (value)
    set->setNR2NpeMethod(m_rx,0);
 else
    set->setNR2NpeMethod(m_rx,1);
}

void NoiseFilterWidget::mmseChanged(bool value) {
    set->setNR2NpeMethod(m_rx,value);
}


void NoiseFilterWidget::preAgcChanged(bool value) {
    set->setNRAgc(m_rx,!value);
}


void NoiseFilterWidget::postAgcChanged(bool value) {
    set->setNRAgc(m_rx,value);
}
