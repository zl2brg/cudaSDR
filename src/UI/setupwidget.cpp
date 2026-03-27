#include "setupwidget.h"
#include "ui_setupwidget.h"

#define	btn_height		20
#define	btn_width		70
#define	btn_widths		32
#define	btn_width2		52
#define	btn_width3		60

SetupWidget::SetupWidget(QDialog *parent)
    : QDialog(parent)
    , set(Settings::instance())
    , m_serverMode(set->getCurrentServerMode())
    , m_hwInterface(set->getHWInterface())
    , m_dataEngineState(set->getDataEngineState())
        //, m_panadapterMode(set->getPanadapterMode())
        //, m_waterColorScheme(set->getWaterfallColorScheme())
    , m_minimumWidgetWidth(set->getMinimumWidgetWidth())
    , m_minimumGroupBoxWidth(set->getMinimumGroupBoxWidth())
    , m_btnSpacing(5)
    , m_rx(set->getCurrentReceiver())
    , m_mouseOver(false)
    , ui(new Ui::SetupWidget)

{
    setContentsMargins(4, 0, 4, 0);
    this->setStyleSheet(set->getSDRStyle());
    ui->setupUi(this);

//    ui->alex_btn->setupModeBtn(0,btn_width3,btn_height,set->getMiniButtonStyle());
//    ui->hermes_btn->setupModeBtn(0,btn_width3,btn_height,set->getMiniButtonStyle());
//    ui->hw_none_btn->setupModeBtn(0,btn_width3,btn_height,set->getMiniButtonStyle());
//    ui->mercury_btn->setupModeBtn(0,btn_width3,btn_height,set->getMiniButtonStyle());
//    ui->modules_btn->setupModeBtn(0,btn_width3,btn_height,set->getMiniButtonStyle());
//    ui->excalabur_btn->setupModeBtn(0,btn_width3,btn_height,set->getMiniButtonStyle());
//    ui->penelopue_btn->setupModeBtn(0,btn_width3,btn_height,set->getMiniButtonStyle());
//    ui->pennylane_btn->setupModeBtn(0,btn_width3,btn_height,set->getMiniButtonStyle());
//   ui->hw_network_btn->setupModeBtn(0,btn_width3,btn_height,set->getMiniButtonStyle());
//    ui->hw_none_btn->setupModeBtn(0,btn_width3,btn_height,set->getMiniButtonStyle());
//    ui->sample_48k_btn->setupModeBtn(0,btn_width3,btn_height,set->getMiniButtonStyle());
//    ui->sample_96k_btn->setupModeBtn(0,btn_width3,btn_height,set->getMiniButtonStyle());
//    ui->sample_192k_btn->setupModeBtn(0,btn_width3,btn_height,set->getMiniButtonStyle());
//    ui->sample_384k_btn->setupModeBtn(0,btn_width3,btn_height,set->getMiniButtonStyle());
//    ui->sample_48k_btn->setupModeBtn(0,btn_width3,btn_height,set->getMiniButtonStyle());
//    ui->firmware_check_btn->setupModeBtn(0,btn_width3,btn_height,set->getMiniButtonStyle());
//    ui->skt_buffer_enable_btn->setupModeBtn(0,btn_width3,btn_height,set->getMiniButtonStyle());
//    ui->sample_48k_btn->setupModeBtn(0,btn_width3,btn_height,set->getMiniButtonStyle());
//    ui->hpsdr_search_btn->setupModeBtn(0,btn_width3,btn_height,set->getMiniButtonStyle());
//    ui->firmware_check_btn->setupModeBtn(0,btn_width3,btn_height,set->getMiniButtonStyle());

    ui->tabWidget->setStyleSheet(set->getTabWidgetStyle());
    ui->tabWidget->setTabEnabled(1, true);
    ui->tabWidget->setTabEnabled(2, true);
    ui->tabWidget->setTabEnabled(3, true);


    ui->audio->setStyleSheet(set->getWidgetStyle());
    ui->hpsdr->setStyleSheet(set->getWidgetStyle());
    ui->general->setStyleSheet(set->getWidgetStyle());

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

    ui->groupBox->setStyleSheet(set->getWidgetStyle());
    ui->groupBox_2->setStyleSheet(set->getWidgetStyle());
    ui->groupBox_3->setStyleSheet(set->getWidgetStyle());
    ui->groupBox_4->setStyleSheet(set->getWidgetStyle());

    ui->ipaddr_combo->setFont(QFont("Arial", 8));
    ui->ipaddr_combo->setStyleSheet(set->getComboBoxStyle());
    ui->ipaddr_combo->setFont(QFont("Arial", 8));
    ui->ipaddr_combo->setStyleSheet(set->getComboBoxStyle());

    ui->hpsdr_ip_combo->setFont(QFont("Arial", 8));
    ui->hpsdr_ip_combo->setStyleSheet(set->getComboBoxStyle());
    ui->hpsdr_ip_combo->setFont(QFont("Arial", 8));
    ui->hpsdr_ip_combo->setStyleSheet(set->getComboBoxStyle());

    ui->skt_buffer_size_combo->setFont(QFont("Arial", 8));
    ui->skt_buffer_size_combo->setStyleSheet(set->getComboBoxStyle());
    ui->skt_buffer_size_combo->setFont(QFont("Arial", 8));
    ui->skt_buffer_size_combo->setStyleSheet(set->getComboBoxStyle());



/*
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
//    ui->frame->setFrameStyle(1);
*/
}

SetupWidget::~SetupWidget()
{
    delete ui;
}

QSize SetupWidget::sizeHint() const {

    return QSize(m_minimumWidgetWidth, height());
}

QSize SetupWidget::minimumSizeHint() const {

    return QSize(m_minimumWidgetWidth, height());
}


void SetupWidget::setupConnections() {

}




void SetupWidget::systemStateChanged(
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

void SetupWidget::getSettings() {
    /*
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
*/
}

