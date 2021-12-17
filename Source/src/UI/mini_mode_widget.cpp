#include "mini_mode_widget.h"
#include "ui_mini_mode_widget.h"

#define	btn_height		20
#define	btn_width		70
#define	btn_widths		32
#define	btn_width2		52
#define	btn_width3		60

MiniModeWidget::MiniModeWidget(QWidget *parent)
    :baseWidget(parent)
    , m_serverMode(set->getCurrentServerMode())
    , m_hwInterface(set->getHWInterface())
    , m_dataEngineState(set->getDataEngineState())
    //, m_panadapterMode(set->getPanadapterMode())
    //, m_waterColorScheme(set->getWaterfallColorScheme())

, ui(new Ui::MiniModeWidget)


{
    setContentsMargins(4, 0, 4, 0);
    ui->setupUi(this);
    setStyleSheet(set->getDialogStyle());
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setupModeBtn(ui->mode_lsb);
    setupModeBtn(ui->mode_usb);
    setupModeBtn(ui->mode_dsb);
    setupModeBtn(ui->mode_cwl);
    setupModeBtn(ui->mode_cwu);
    setupModeBtn(ui->mode_fm);
    setupModeBtn(ui->mode_am);
    setupModeBtn(ui->mode_digu);
    setupModeBtn(ui->mode_spec);
    setupModeBtn(ui->mode_digl);
    setupModeBtn(ui->mode_sam);
    setupModeBtn(ui->mode_freedv);
    adjustSize();






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
/*
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

*/

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

MiniModeWidget::~MiniModeWidget()
{
    m_mode_btnList.clear();
    m_filter_btnList.clear();
    delete ui;
}



void MiniModeWidget::setupConnections() {

}




void MiniModeWidget::systemStateChanged(
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

void MiniModeWidget::getSettings() {
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


void MiniModeWidget::modeChange(){
    qDebug() << "Mode change";

}

void MiniModeWidget::test(){
    qDebug() << "test Mode change";

}
void MiniModeWidget::FilterbtnCallback(){

}

void MiniModeWidget::BandbtnCallback(){


}

void MiniModeWidget::ModebtnCallback(){
    AeroButton *button = qobject_cast<AeroButton *>(sender());
    int btn = m_mode_btnList.indexOf(button);

            foreach(AeroButton *btn, m_mode_btnList) {

            btn->setBtnState(AeroButton::OFF);
            btn->update();
        }

    set->setDSPMode(this, m_receiver, (DSPMode) btn);
    //m_dspModeList[m_hamBand] = (DSPMode) btn;
    //filterChanged(this, m_receiver, m_filterLo, m_filterHi);
    //filterGroupChanged((DSPMode) btn);

    button->setBtnState(AeroButton::ON);
    button->update();

}
