#include "radio_widget.h"
#include "ui_radio_ctrl.h"


namespace Ui {
    class  RadioCtrl;
}


RadioCtrl::RadioCtrl(QWidget *parent, int rx)
    : baseWidget(parent)
    , set(Settings::instance())
    , m_serverMode(set->getCurrentServerMode())
    , m_hwInterface(set->getHWInterface())
    , m_dataEngineState(set->getDataEngineState())
    , m_btnSpacing(2)

        //, m_panadapterMode(set->getPanadapterMode())
        //, m_waterColorScheme(set->getWaterfallColorScheme())
    , m_minimumWidgetWidth(set->getMinimumWidgetWidth())
    , m_minimumGroupBoxWidth(set->getMinimumGroupBoxWidth())
    , m_mouseOver(false)
    , ui(new Ui::RadioCtrl)
{
    ui->setupUi(this);

    setContentsMargins(4, 0, 4, 0);
    setFilterWidget();
    setBandWidget();
    setModeWidget();
    CHECKED_CONNECT(
        set,
        SIGNAL(systemStateChanged(
                    QSDR::_Error,
                    QSDR::_HWInterfaceMode,
                    QSDR::_ServerMode,
                    QSDR::_DataEngineState)),
        this,
        SLOT(systemStateChanged(
                    QSDR::_Error,
                    QSDR::_HWInterfaceMode,
                    QSDR::_ServerMode,
                    QSDR::_DataEngineState)));


     CHECKED_CONNECT(
     set,
     SIGNAL(dspModeChanged(
                int,
                DSPMode)),
    this,
    SLOT(dspModeChanged(
             int,
             DSPMode)));

     CHECKED_CONNECT(
         set,
         SIGNAL(vfoFrequencyChanged(int, int, long)),
         this,
         SLOT(vfoFrequencyChanged(int, int, long)));

     CHECKED_CONNECT(
         set,
         SIGNAL(hamBandChanged(int, bool, HamBand)),
         this,
         SLOT(bandChanged(int, bool, HamBand)));

qDebug() << "RX" << m_receiver;

setFilterWidget();
setModeWidget();
setBandWidget();
dspModeChanged(0, m_dspModeList.at(m_hamBand));



}

void RadioCtrl::setFilterWidget(){

    setupFilterBtn(ui->filter_1);
    setupFilterBtn(ui->filter_2);
    setupFilterBtn(ui->filter_3);
    setupFilterBtn(ui->filter_4);
    setupFilterBtn(ui->filter_5);
    setupFilterBtn(ui->filter_6);
    setupFilterBtn(ui->filter_7);
    setupFilterBtn(ui->filter_8);
    setupFilterBtn(ui->filter_9);
    setupFilterBtn(ui->filter_10);
    setupFilterBtn(ui->filter_var1);
    setupFilterBtn(ui->filter_var2);
    updateFilterWidget();
    adjustSize();

}


void RadioCtrl::setModeWidget(){
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

}


void RadioCtrl::setBandWidget(){
    setupBandBtn(ui->bnd_2200m);
    setupBandBtn(ui->bnd_630m);
    setupBandBtn(ui->bnd_160m);
    setupBandBtn(ui->bnd_80m);
    setupBandBtn(ui->bnd_60m);
    setupBandBtn(ui->bnd_40m);
    setupBandBtn(ui->bnd_30m);
    setupBandBtn(ui->bnd_20m);
    setupBandBtn(ui->bnd_17m);
    setupBandBtn(ui->bnd_15m);
    setupBandBtn(ui->bnd_12m);
    setupBandBtn(ui->bnd_10m);
    setupBandBtn(ui->bnd_6m);
    setupBandBtn(ui->bnd_2m);
    setupBandBtn(ui->bnd_125cm);
    setupBandBtn(ui->bnd_70cm);
    setupBandBtn(ui->bnd_33cm);
    setupBandBtn(ui->bnd_23cm);
    setupBandBtn(ui->bnd_13cm);
    setupBandBtn(ui->bnd_10cm);
    setupBandBtn(ui->bnd_5cm);
    setupBandBtn(ui->bnd_gen);
    adjustSize();
}

void RadioCtrl::updateFilterWidget()
{
    QStringList btn_text=set->getFilterBtnText(m_receiver);
        ui->filter_1->setText(btn_text[0]);
        ui->filter_2->setText(btn_text[1]);
        ui->filter_3->setText(btn_text[2]);
        ui->filter_4->setText(btn_text[3]);
        ui->filter_5->setText(btn_text[4]);
        ui->filter_6->setText(btn_text[5]);
        ui->filter_7->setText(btn_text[6]);
        ui->filter_8->setText(btn_text[7]);
        ui->filter_9->setText(btn_text[8]);
        ui->filter_10->setText(btn_text[9]);
     //   SetVarSlider(ui->Var1_Slider);
     //   SetVarSlider(ui->Var2_Slider);
}



qreal RadioCtrl::SetVarSlider(QAbstractSlider *slider)
{
    return  (qreal) ui->Var1_Slider->value();
}


void RadioCtrl::ctrFrequencyChanged(int mode, int rx, long frequency) {

    Q_UNUSED (mode)

    if (m_receiver != rx) return;
    m_ctrFrequency = frequency;

    HamBand band = getBandFromFrequency(set->getBandFrequencyList(), frequency);
    m_lastCtrFrequencyList[(int) band] = m_ctrFrequency;
}

void RadioCtrl::vfoFrequencyChanged(int mode, int rx, long frequency) {

    Q_UNUSED (mode)

    if (m_receiver != rx) return;
    m_vfoFrequency = frequency;

    HamBand band = getBandFromFrequency(set->getBandFrequencyList(), frequency);
    m_lastVfoFrequencyList[(int) band] = m_vfoFrequency;
}


void RadioCtrl::bandChanged(int rx, bool byButton, HamBand band) {

    Q_UNUSED (byButton)

        if (m_receiver != rx) return;
    m_hamBand = band;

    foreach(AeroButton *btn, m_band_btnList) {

        btn->setBtnState(AeroButton::OFF);
        btn->update();
    }

    m_band_btnList.at(band)->setBtnState(AeroButton::ON);
    m_band_btnList.at(band)->update();
}


void RadioCtrl::BandbtnCallback() {
    AeroButton *button = qobject_cast<AeroButton *>(sender());
    int btnIndex = m_band_btnList.indexOf(button);
    if (btnIndex == -1) return;

    for(AeroButton *btn : m_band_btnList) {
        btn->setBtnState(AeroButton::OFF);
        btn->update();
    }

    button->setBtnState(AeroButton::ON);
    button->update();

    HamBand band = static_cast<HamBand>(btnIndex);
    set->setHamBand(m_receiver, true, band);

    if (btnIndex >= 0 && btnIndex < m_lastVfoFrequencyList.size()) {
        set->setVFOFrequency(2, m_receiver, m_lastVfoFrequencyList.at(btnIndex));
    }
}



void RadioCtrl::ModebtnCallback(){
    AeroButton *button = qobject_cast<AeroButton *>(sender());
    int btnIndex = m_mode_btnList.indexOf(button);
    if (btnIndex == -1) return;

    for(AeroButton *btn : m_mode_btnList) {
        btn->setBtnState(AeroButton::OFF);
        btn->update();
    }

    DSPMode mode = static_cast<DSPMode>(btnIndex);
    set->setDSPMode(m_receiver, mode);

    button->setBtnState(AeroButton::ON);
    button->update();
}

void RadioCtrl::FilterbtnCallback() {
    AeroButton *button = qobject_cast<AeroButton *>(sender());
    int btnIndex = m_filter_btnList.indexOf(button);
    if (btnIndex == -1) return;

    foreach(AeroButton *btn, m_filter_btnList) {
        btn->setBtnState(AeroButton::OFF);
        btn->update();
    }
    
    ui->Var1_Slider->setDisabled(true);
    ui->Var2_Slider->setDisabled(true);

    button->setBtnState(AeroButton::ON);
    
    if (btnIndex == 10) ui->Var1_Slider->setEnabled(true);
    else if (btnIndex == 11) ui->Var2_Slider->setEnabled(true);

    button->update();
    updateFilterWidget();
}



void RadioCtrl::filterChangedByBtn() {
    AeroButton *button = qobject_cast<AeroButton *>(sender());
    int btnIndex = m_filter_btnList.indexOf(button);
    if (btnIndex == -1) return;

    foreach(AeroButton *btn, m_filter_btnList) {
        btn->setBtnState(AeroButton::OFF);
        btn->update();
    }
    ui->Var1_Slider->setDisabled(true);
    ui->Var2_Slider->setDisabled(true);

    button->setBtnState(AeroButton::ON);
    
    qreal filterWidth = 0.0f;
    if (btnIndex == 10) { // Var1
        ui->Var1_Slider->setEnabled(true);
        filterWidth = (qreal)ui->Var1_Slider->value();
    } else if (btnIndex == 11) { // Var2
        ui->Var2_Slider->setEnabled(true);
        filterWidth = (qreal)ui->Var2_Slider->value();
    } else {
        DSPMode mode = m_dspModeList.at(m_hamBand);
        int groupIdx = -1;
        if (mode == LSB || mode == USB || mode == DIGU || mode == DIGL) groupIdx = 1; // Mid
        else if (mode == DSB || mode == FMN || mode == AM || mode == SAM) groupIdx = 2; // Wide
        else groupIdx = 0; // Narrow (CW)
        
        float widths[] = {25, 50, 100, 250, 400, 500, 600, 750, 800, 1000,   // Narrow
                          1000, 1800, 2100, 2400, 2700, 2900, 3300, 3800, 4400, 5000, // Mid
                          2400, 2900, 3100, 4000, 5200, 6600, 8000, 10000, 12000, 16000}; // Wide
        
        filterWidth = widths[groupIdx * 10 + btnIndex];
        
        int filterModeIdx = 2; // M_DSB
        if (mode == LSB || mode == DIGL || mode == CWL) filterModeIdx = 0; // M_LSB
        else if (mode == USB || mode == DIGU || mode == CWU) filterModeIdx = 1; // M_USB
        
        switch (filterModeIdx) {
            case 0: m_filterLo = -150.0f; m_filterHi = -filterWidth; break; // M_LSB
            case 1: m_filterLo = 150.0f; m_filterHi = filterWidth; break; // M_USB
            case 2: m_filterHi = filterWidth; m_filterLo = -filterWidth; break; // M_DSB
        }
    }
    
    button->update();
    set->setRXFilter(m_receiver, m_filterLo, m_filterHi);
}





void RadioCtrl::filterChanged(int rx, qreal low, qreal high) {

    if (m_receiver != rx) return;
    m_filterLo = low;
    m_filterHi = high;
}


void RadioCtrl::dspModeChanged(int rx, DSPMode mode)
{
    m_dspMode = mode;
    updateFilterWidget();
    if (m_receiver != rx) return;
    m_dspModeList[m_hamBand] = mode;

    foreach(AeroButton *btn, m_mode_btnList) {

        btn->setBtnState(AeroButton::OFF);
        btn->update();
    }

    m_mode_btnList.at(mode)->setBtnState(AeroButton::ON);
    m_mode_btnList.at(mode)->update();

}





RadioCtrl::~RadioCtrl()
{
    delete ui;
    disconnect(set, 0, this, 0);
    disconnect(0, 0, 0);
}

QSize RadioCtrl::sizeHint() const {

    return QSize(m_minimumWidgetWidth, height());
}

QSize RadioCtrl::minimumSizeHint() const {

    return QSize(m_minimumWidgetWidth, height());
}


void RadioCtrl::setupConnections() {

}




void RadioCtrl::systemStateChanged(
        QSDR::_Error err,
        QSDR::_HWInterfaceMode hwmode,
        QSDR::_ServerMode mode,
        QSDR::_DataEngineState state)
{

    Q_UNUSED (err)

    m_receiverDataList = set->getReceiverDataList();

    m_hamBand = m_receiverDataList.at(m_receiver).hamBand;
    //m_dspMode = m_receiverDataList.at(m_receiver).dspMode;
    m_dspModeList = m_receiverDataList.at(m_receiver).dspModeList;
    m_lastCtrFrequencyList = m_receiverDataList.at(m_receiver).lastCenterFrequencyList;
    m_lastVfoFrequencyList = m_receiverDataList.at(m_receiver).lastVfoFrequencyList;


    if (m_hwInterface != hwmode)
        m_hwInterface = hwmode;


    if (m_serverMode != mode)
        m_serverMode = mode;

    if (m_dataEngineState != state)
        m_dataEngineState = state;

    update();
}


void RadioCtrl::modeChange(){
    qDebug() << "Mode change";

}


