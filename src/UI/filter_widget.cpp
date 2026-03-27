#include "filter_widget.h"
#include "ui_filter_widget.h"

namespace Ui {
    class  FilterWidget;
}



FilterWidget::FilterWidget(QWidget *parent)
    : baseWidget(parent)
    , set(Settings::instance())
    , m_serverMode(set->getCurrentServerMode())
    , m_hwInterface(set->getHWInterface())
    , m_dataEngineState(set->getDataEngineState())
    //, m_panadapterMode(set->getPanadapterMode())
    //, m_waterColorScheme(set->getWaterfallColorScheme())

    , ui(new Ui::FilterWidget)

{
    CHECKED_CONNECT(
        set,
        SIGNAL(dspModeChanged(QObject *, int, DSPMode)),
        this,
        SLOT(dspModeChanged(QObject *, int, DSPMode)));


    setContentsMargins(4, 0, 4, 0);
    m_FilterData = Mid_FilterGroup;
    this->setStyleSheet(set->getSDRStyle());
    //this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    ui->setupUi(this);
    CHECKED_CONNECT(this->ui->Var1_Slider, SIGNAL(valueChanged(int)), this, SLOT(slider_changed(int)));
    CHECKED_CONNECT(this->ui->Var2_Slider, SIGNAL(valueChanged(int)), this, SLOT(slider_changed(int)));
    this->setStyleSheet(set->getWidgetStyle());
    ui->Var1_Slider->setStyleSheet(set->getVolSliderStyle());
    ui->Var2_Slider->setStyleSheet(set->getVolSliderStyle());

    setupModeBtn(ui->filter_1);
    setupModeBtn(ui->filter_2);
    setupModeBtn(ui->filter_3);
    setupModeBtn(ui->filter_4);
    setupModeBtn(ui->filter_5);
    setupModeBtn(ui->filter_6);
    setupModeBtn(ui->filter_7);
    setupModeBtn(ui->filter_8);
    setupModeBtn(ui->filter_9);
    setupModeBtn(ui->filter_10);
    setupModeBtn(ui->filter_var1);
    setupModeBtn(ui->filter_var2);
    updateFilterWidget();
}


void FilterWidget::updateFilterWidget()
{

    m_FilterMode = setFilterMode(m_dspMode);
    m_FilterGroup= setFilterGroup(m_dspMode);

        ui->filter_1->setText(m_FilterData->txt);
        m_FilterData++;
        ui->filter_2->setText(m_FilterData->txt);
        m_FilterData++;
        ui->filter_3->setText(m_FilterData->txt);
        m_FilterData++;
        ui->filter_4->setText(m_FilterData->txt);
        m_FilterData++;
        ui->filter_5->setText(m_FilterData->txt);
        m_FilterData++;
        ui->filter_6->setText(m_FilterData->txt);
        m_FilterData++;
        ui->filter_7->setText(m_FilterData->txt);
        m_FilterData++;
        ui->filter_8->setText(m_FilterData->txt);
        m_FilterData++;
        ui->filter_9->setText(m_FilterData->txt);
        m_FilterData++;
        ui->filter_10->setText(m_FilterData->txt);
        SetVarSlider(ui->Var1_Slider);
        SetVarSlider(ui->Var2_Slider);
}


FilterWidget::~FilterWidget()
{
    delete ui;
}

filterMode FilterWidget::setFilterMode(DSPMode mode){

    switch (mode) {

    case (DSPMode) DSB:
    case (DSPMode) AM:
    case (DSPMode) SAM:
    case (DSPMode) FMN:
    case (DSPMode) DRM:
    case (DSPMode) SPEC:

        return (filterMode) M_DSB;
        break;

    case (DSPMode) LSB:
    case (DSPMode) DIGL:
    case (DSPMode) CWL:

        return (filterMode) M_LSB;


    default:
        return (filterMode) M_USB;

    }
}

filterGroup FilterWidget::setFilterGroup(DSPMode mode){

    switch (mode) {

    case (DSPMode) USB:
    case (DSPMode) LSB:
    case (DSPMode) DIGL:
    case (DSPMode) DIGU:
        m_FilterData = Mid_FilterGroup;
        return (filterGroup) MID_FILTER;


    case (DSPMode) DSB:
    case (DSPMode) AM:
    case (DSPMode) SAM:
    case (DSPMode) FMN:
    case (DSPMode) DRM:
    case (DSPMode) SPEC:
        m_FilterData = Wide_FilterGroup;

        return (filterGroup) WIDEBAND_FILTER;
        break;

      default:
          m_FilterData = Narrow_FilterGroup;
        return (filterGroup) NARROW_FILTER;

    }
}



void FilterWidget::setupConnections() {

}




void FilterWidget::systemStateChanged(
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
    m_dspMode = m_receiverDataList.at(m_receiver).dspMode;
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

void FilterWidget::getSettings() {
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


void FilterWidget::setFilterbyMode(QString str, DSPMode dspMode)
{


    }


qreal FilterWidget::SetVarSlider(QAbstractSlider *slider)
{
    if (m_FilterData->filterWidth != slider->maximum())
    {
         slider->setMaximum(m_FilterData->filterWidth);
         qDebug() << "set slider max" << slider->maximum();
         return m_FilterData->filterWidth/2;
    }
    return  (qreal) ui->Var1_Slider->value();
}


void FilterWidget::btnCallback() {

    AeroButton *button = qobject_cast<AeroButton *>(sender());

    qreal filter = 0.0f;

    foreach(AeroButton *btn, btnList) {

        btn->setBtnState(AeroButton::OFF);
        btn->update();

    }
    ui->Var1_Slider->setDisabled(true);
    ui->Var2_Slider->setDisabled(true);

    button->setBtnState(AeroButton::ON);
    qDebug() << "button" << button->text();
    if (button->text() == "Var1") ui->Var1_Slider->setDisabled(true);
    if (button->text() == "Var2") ui->Var2_Slider->setDisabled(false);

    switch(m_FilterGroup){
    case NARROW_FILTER:
        m_FilterData = Narrow_FilterGroup;
        break;
    case    MID_FILTER:
        m_FilterData = Mid_FilterGroup;

        break;
    default:
        m_FilterData = Wide_FilterGroup;
           break;

    }



    for (int count = 0;count < 12;count++) {

        if (m_FilterData->txt == button->text() )
        {
            if (button->text() == "Var1" )
            {
                filter =  SetVarSlider(ui->Var1_Slider);
            }
            else   if (button->text() == "Var2" )
            {
                filter =  SetVarSlider(ui->Var2_Slider);
            }
            else
            filter = m_FilterData->filterWidth;
            break;
        }
        m_FilterData++;

    }
    button->update();
    updateFilterWidget();

    switch (m_FilterMode) {
    case M_DSB:
      m_filterHi = filter;
      m_filterLo = -filter;
    break;
    case M_LSB:
        m_filterLo = -150.0f;
        m_filterHi = -filter;
    break;
    case M_USB:
        m_filterLo = 150.0f;
        m_filterHi = filter;

    break;

    }



}
void FilterWidget::filterChangedByBtn()	 {

    AeroButton *button = qobject_cast<AeroButton *>(sender());


    foreach(AeroButton *btn, btnList) {

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





void FilterWidget::filterChanged(QObject *sender, int rx, qreal low, qreal high) {

    Q_UNUSED(sender)

    if (m_receiver != rx) return;
    m_filterLo = low;
    m_filterHi = high;



}


void FilterWidget::dspModeChanged(QObject *sender,int rx, DSPMode mode)
{
    m_dspMode = mode;
    updateFilterWidget();
}


void FilterWidget::slider_changed(int value){
    qreal filter = 0.0f;

    switch(m_FilterGroup){
    case NARROW_FILTER:
        m_FilterData = Narrow_FilterGroup;
        break;
    case    MID_FILTER:
        m_FilterData = Mid_FilterGroup;
        break;
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
        m_filterLo = -150.0f;
        m_filterHi = -filter;
    break;
    case M_USB:
        m_filterLo = 150.0f;
        m_filterHi = filter;

    break;
    }

    set->setRXFilter(this, m_receiver, m_filterLo, m_filterHi);


};
