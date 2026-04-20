#include "noisefilterwidget.h"
#include "ui_noisefilterwidget.h"
#include <QSignalBlocker>

#define	btn_height		15
#define	btn_width		70
#define	btn_widths		32
#define	btn_width2		52
#define	btn_width3		60



NoiseFilterWidget::NoiseFilterWidget(QWidget *parent, int rx)
    : QWidget(parent)
    , ui(new Ui::NoiseFilterWidget)
    , set(Settings::instance())
    , m_serverMode(set->getCurrentServerMode())
    , m_hwInterface(set->getHWInterface())
    , m_dataEngineState(set->getDataEngineState())
    //, m_panadapterMode(set->getPanadapterMode())
    //, m_waterColorScheme(set->getWaterfallColorScheme())
    , m_rx(rx >= 0 ? rx : set->getCurrentReceiver())
    , m_minimumGroupBoxWidth(set->getMinimumGroupBoxWidth())
    , m_btnSpacing(5)
    , m_fontHeight(0)
    , m_maxFontWidth(0)
    , m_currentReceiver(m_rx)
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
    connect(set, &Settings::noiseFilterChanged, this, [this](int rx, int) {
        if (rx == m_rx) getSettings();
    });
    connect(set, &Settings::noiseBlankerChanged, this, [this](int rx, int) {
        if (rx == m_rx) getSettings();
    });
    connect(set, &Settings::nr2GainMethodChanged, this, [this](int rx, int) {
        if (rx == m_rx) getSettings();
    });
    connect(set, &Settings::nr2NpeMethodChanged, this, [this](int rx, int) {
        if (rx == m_rx) getSettings();
    });
    connect(set, &Settings::nrAgcChanged, this, [this](int rx, int) {
        if (rx == m_rx) getSettings();
    });
    connect(set, &Settings::nr2AeChanged, this, [this](int rx, bool) {
        if (rx == m_rx) getSettings();
    });
    connect(set, &Settings::snbChanged, this, [this](int rx, bool) {
        if (rx == m_rx) getSettings();
    });
    connect(set, &Settings::anfChanged, this, [this](int rx, bool) {
        if (rx == m_rx) getSettings();
    });

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

    ui->nrModeComboBox->setCurrentIndex(set->getnrMode(m_rx));
    ui->nbModeComboBox->setCurrentIndex(set->getnbMode(m_rx));
    ui->nr2GainComboBox->setCurrentIndex(set->getNr2GainMethod(m_rx));
    ui->snbCheckBox->setChecked(set->getSnb(m_rx));
    ui->anfCheckBox->setChecked(set->getAnf(m_rx));
    int nr2Npe = set->getNr2NpeMethod(m_rx);
    ui->omsCheckBox->setChecked(nr2Npe == 0);
    ui->mmseCheckBox->setChecked(nr2Npe != 0);
    int agcMode = set->getNrAGC(m_rx);
    ui->preAGCCheckBox->setChecked(agcMode == 0);
    ui->postAGCCheckBox->setChecked(agcMode != 0);
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
    if (!value) return;

    {
        const QSignalBlocker blockOms(ui->omsCheckBox);
        const QSignalBlocker blockMmse(ui->mmseCheckBox);
        ui->omsCheckBox->setChecked(true);
        ui->mmseCheckBox->setChecked(false);
    }
    set->setNR2NpeMethod(m_rx, 0);
}

void NoiseFilterWidget::mmseChanged(bool value) {
    if (!value) return;

    {
        const QSignalBlocker blockOms(ui->omsCheckBox);
        const QSignalBlocker blockMmse(ui->mmseCheckBox);
        ui->omsCheckBox->setChecked(false);
        ui->mmseCheckBox->setChecked(true);
    }
    set->setNR2NpeMethod(m_rx, 1);
}


void NoiseFilterWidget::preAgcChanged(bool value) {
    if (!value) return;

    {
        const QSignalBlocker blockPreAgc(ui->preAGCCheckBox);
        const QSignalBlocker blockPostAgc(ui->postAGCCheckBox);
        ui->preAGCCheckBox->setChecked(true);
        ui->postAGCCheckBox->setChecked(false);
    }
    set->setNRAgc(m_rx, 0);
}


void NoiseFilterWidget::postAgcChanged(bool value) {
    if (!value) return;

    {
        const QSignalBlocker blockPreAgc(ui->preAGCCheckBox);
        const QSignalBlocker blockPostAgc(ui->postAGCCheckBox);
        ui->preAGCCheckBox->setChecked(false);
        ui->postAGCCheckBox->setChecked(true);
    }
    set->setNRAgc(m_rx, 1);
}
