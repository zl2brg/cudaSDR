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
    this->setStyleSheet(set->getSDRStyle());
    setFilterWidget();
    setBandWidget();
    setModeWidget();
    ui->groupBox->setStyleSheet(set->getWidgetStyle());
    ui->groupBox_2->setStyleSheet(set->getWidgetStyle());
    ui->groupBox_4->setStyleSheet(set->getWidgetStyle());
    CHECKED_CONNECT(
        set,
        SIGNAL(systemStateChanged(
                    QObject *,
                    QSDR::_Error,
                    QSDR::_HWInterfaceMode,
                    QSDR::_ServerMode,
                    QSDR::_DataEngineState)),
        this,
        SLOT(systemStateChanged(
                    QObject *,
                    QSDR::_Error,
                    QSDR::_HWInterfaceMode,
                    QSDR::_ServerMode,
                    QSDR::_DataEngineState)));


     CHECKED_CONNECT(
     set,
     SIGNAL(dspModeChanged(
                QObject *,
                int,
                DSPMode)),
    this,
    SLOT(dspModeChanged(
             QObject*,
             int,
             DSPMode)));

     CHECKED_CONNECT(
         set,
         SIGNAL(vfoFrequencyChanged(QObject *, int, int, long)),
         this,
         SLOT(vfoFrequencyChanged(QObject *, int, int, long)));

     CHECKED_CONNECT(
         set,
         SIGNAL(hamBandChanged(QObject *, int, bool, HamBand)),
         this,
         SLOT(bandChanged(QObject *, int, bool, HamBand)));


setFilterWidget();
setModeWidget();
setBandWidget();
set->getFilterGroup(m_receiver);
set->getFilterMode(m_receiver);
set->setRxFilterByIndex(this,m_receiver,set->getFilterbtnIndex(m_receiver));

dspModeChanged(this, 0, m_dspModeList.at(m_hamBand));



}

void RadioCtrl::setFilterWidget(){

    ui->Var1_Slider->setStyleSheet(set->getVolSliderStyle());
    ui->Var2_Slider->setStyleSheet(set->getVolSliderStyle());
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
    if (m_FilterData->filterWidth != slider->maximum())
    {
//         slider->setMaximum(m_FilterData->filterWidth);
         qDebug() << "set slider max" << slider->maximum();
//         return m_FilterData->filterWidth/2;
    }
    return  (qreal) ui->Var1_Slider->value();
}


void RadioCtrl::ctrFrequencyChanged(QObject* sender, int mode, int rx, long frequency) {

    Q_UNUSED (sender)
    Q_UNUSED (mode)

    if (m_receiver != rx) return;
    m_ctrFrequency = frequency;

    HamBand band = getBandFromFrequency(set->getBandFrequencyList(), frequency);
    m_lastCtrFrequencyList[(int) band] = m_ctrFrequency;
}

void RadioCtrl::vfoFrequencyChanged(QObject* sender, int mode, int rx, long frequency) {

    Q_UNUSED (sender)
    Q_UNUSED (mode)

    if (m_receiver != rx) return;
    m_vfoFrequency = frequency;

    HamBand band = getBandFromFrequency(set->getBandFrequencyList(), frequency);
    m_lastVfoFrequencyList[(int) band] = m_vfoFrequency;
}


void RadioCtrl::bandChanged(QObject *sender, int rx, bool byButton, HamBand band) {

    Q_UNUSED (byButton)

    if (sender == this) return;

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
    int btn = m_band_btnList.indexOf(button);
    qDebug() << "Button" << btn;
    foreach(AeroButton *btn, m_band_btnList) {

        btn->setBtnState(AeroButton::OFF);
        btn->update();
    }

    button->setBtnState(AeroButton::ON);
    button->update();
    qDebug() << "size" << m_band_btnList.size();
    set->setHamBand(this, m_receiver, true, (HamBand) btn);

    QString str = button->text();
    if (str == "2200 m")
        set->setVFOFrequency(this, 2, m_receiver, m_lastVfoFrequencyList.at(m2200));
    else
    if (str == "630 m")
        set->setVFOFrequency(this, 2, m_receiver, m_lastVfoFrequencyList.at(m630));
    else
    if (str == "160 m")
        set->setVFOFrequency(this, 2, m_receiver, m_lastVfoFrequencyList.at(m160));
    else
    if (str == "80 m")
        set->setVFOFrequency(this, 2, m_receiver, m_lastVfoFrequencyList.at(m80));
    else
    if (str == "60 m")
        set->setVFOFrequency(this, 2, m_receiver, m_lastVfoFrequencyList.at(m60));
    else
    if (str == "40 m")
        set->setVFOFrequency(this, 2, m_receiver, m_lastVfoFrequencyList.at(m40));
    else
    if (str == "30 m")
        set->setVFOFrequency(this, 2, m_receiver, m_lastVfoFrequencyList.at(m30));
    else
    if (str == "20 m")
        set->setVFOFrequency(this, 2, m_receiver, m_lastVfoFrequencyList.at(m20));
    else
    if (str == "17 m")
        set->setVFOFrequency(this, 2, m_receiver, m_lastVfoFrequencyList.at(m17));
    else
    if (str == "15 m")
        set->setVFOFrequency(this, 2, m_receiver, m_lastVfoFrequencyList.at(m15));
    else
    if (str == "12 m")
        set->setVFOFrequency(this, 2, m_receiver, m_lastVfoFrequencyList.at(m12));
    else
    if (str == "10 m")
        set->setVFOFrequency(this, 2, m_receiver, m_lastVfoFrequencyList.at(m10));
    else
    if (str == "6 m")
        set->setVFOFrequency(this, 2, m_receiver, m_lastVfoFrequencyList.at(m6));
    else
    if (str == "2 m")
        set->setVFOFrequency(this, 2, m_receiver, m_lastVfoFrequencyList.at(m2));

    else
        set->setVFOFrequency(this, 2, m_receiver, m_lastVfoFrequencyList.at(gen));
}



void RadioCtrl::ModebtnCallback(){
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
//#ToDO emit button text change

}


void RadioCtrl::FilterbtnCallback() {

    AeroButton *button = qobject_cast<AeroButton *>(sender());


    foreach(AeroButton *btn, m_filter_btnList) {

        btn->setBtnState(AeroButton::OFF);
        btn->update();

    }
    ui->Var1_Slider->setDisabled(true);
    ui->Var2_Slider->setDisabled(true);

    button->setBtnState(AeroButton::ON);
    qDebug() << "button" << button->text();
    if (button->text() == "Var1") ui->Var1_Slider->setDisabled(true);
    if (button->text() == "Var2") ui->Var2_Slider->setDisabled(false);
    int index = m_filter_btnList.indexOf(button);

    button->update();
    updateFilterWidget();
    set->setRxFilterByIndex(this, m_receiver, index);


}



void RadioCtrl::filterChangedByBtn()	 {

    AeroButton *button = qobject_cast<AeroButton *>(sender());


    foreach(AeroButton *btn, m_filter_btnList) {

        btn->setBtnState(AeroButton::OFF);
        btn->update();
        ui->Var1_Slider->setDisabled(true);
        ui->Var2_Slider->setDisabled(true);
    }

    button->setBtnState(AeroButton::ON);
    button->update();


    m_dspMode = m_dspModeList.at(m_hamBand);

    set->setRXFilter(this, m_receiver, m_filterLo, m_filterHi);
}





void RadioCtrl::filterChanged(QObject *sender, int rx, qreal low, qreal high) {

    Q_UNUSED(sender)
    if (m_receiver != rx) return;
    m_filterLo = low;
    m_filterHi = high;
}


void RadioCtrl::dspModeChanged(QObject *sender,int rx, DSPMode mode)
{
    m_dspMode = mode;
    updateFilterWidget();
    Q_UNUSED(sender)

    if (m_receiver != rx) return;
    m_dspModeList[m_hamBand] = mode;

    foreach(AeroButton *btn, m_mode_btnList) {

        btn->setBtnState(AeroButton::OFF);
        btn->update();
    }

    m_mode_btnList.at(mode)->setBtnState(AeroButton::ON);
    m_mode_btnList.at(mode)->update();

}


void RadioCtrl::slider_changed(int value){
    qreal filter = 0.0f;

    switch(m_FilterGroup){
    case NARROW_FILTER:
        m_FilterData = Narrow_FilterGroup;
        break;
    case    MID_FILTER:
    default:
        m_FilterData = Wide_FilterGroup;
           break;
    }
    filter = value + 150.0f;

    switch (m_FilterMode) {
    case M_DSB:
      m_filterHi = filter;
      m_filterLo = -filter;
    break;
    case M_LSB:
        m_filterLo =  -filter;
        m_filterHi =  -150.0f;
    break;
    case M_USB:
        m_filterLo = 150.0f;
        m_filterHi = filter;

    break;
    }

    set->setRXFilter(this, m_receiver, m_filterLo, m_filterHi);


};




RadioCtrl::~RadioCtrl()
{
    delete ui;
    disconnect(set, 0, this, 0);
    disconnect(this, 0, 0, 0);
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
        QObject *sender,
        QSDR::_Error err,
        QSDR::_HWInterfaceMode hwmode,
        QSDR::_ServerMode mode,
        QSDR::_DataEngineState state)
{

    Q_UNUSED (sender)
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


