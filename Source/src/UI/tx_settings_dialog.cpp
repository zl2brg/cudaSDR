#include "tx_settings_dialog.h"
#include "ui_tx_settings_dialog.h"
#include "QtWDSP/qtwdsp_dspEngine.h"

tx_settings_dialog::tx_settings_dialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tx_settings_dialog),
    set(Settings::instance())

{
    m_amCarrierLevel = set->getAMCarrierLevel();
    m_audioCompressionLevel = set->getAudioCompression();
    qDebug() << "Am carrier level load" << m_amCarrierLevel;
    qDebug() << "compression l evel load" << m_audioCompressionLevel;



    setContentsMargins(4, 0, 4, 0);
    ui->setupUi(this);
 //   this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->setStyleSheet(set->getWidgetStyle());
    this->ui->amCarrierLevel->setStyleSheet(set->getVolSliderStyle());
    this->ui->audioCompression->setStyleSheet(set->getVolSliderStyle());

    int temp = m_amCarrierLevel * 100;
    ui->amCarrierLevel->setSliderPosition(0.5);
    ui->audioCompression->setSliderPosition(m_audioCompressionLevel);
    ui->fm_deviation->setValue(int(set->getFMDeveation() / 1000.0));



    ui->audiodevlist->clear();
    ui->audiodevlist->addItem("HPSDR Mic Input");
    for( int i=0; i<set->numDevices; i++ )
    {
        set->deviceInfo = Pa_GetDeviceInfo( i );
        paDeviceList.append(QString(set->deviceInfo->name));
        ui->audiodevlist->addItem(QString(set->deviceInfo->name));
        qDebug() << "PA device " << set->deviceInfo->name;

    }
    ui->audiodevlist->setCurrentIndex(set->getMicInputDev());


    setContentsMargins(4, 4, 4, 4);
    setWindowOpacity(0.9);

 CHECKED_CONNECT(ui->audiodevlist,
                 SIGNAL(currentIndexChanged(int)),
                 this,
                 SLOT(audio_input_changed(int)));

 CHECKED_CONNECT(ui->audioCompression,
                 SIGNAL(valueChanged(int)),
                 this,
                 SLOT(audioCompressionChanged(int)));

 CHECKED_CONNECT(ui->amCarrierLevel,
                 SIGNAL(valueChanged(int)),
                 this,
                 SLOT(amCarrierLevelChanged(int)));

 CHECKED_CONNECT(ui->fm_deviation,
                  SIGNAL(valueChanged(int)),
                   this,
                  SLOT(fmDeviationChanged(int)));
}




tx_settings_dialog::~tx_settings_dialog()
{
    delete ui;
    disconnect(set, 0, this, 0);
    disconnect(this, 0, 0, 0);
}


void tx_settings_dialog::audio_input_changed(int index){

qDebug() << "selected" << index;
set->setMicInputDev(index);

}


void tx_settings_dialog::amCarrierLevelChanged(int level)
{
double temp = level/100.0;
qDebug() << "Am Carrier Level" << temp;
set->setAMCarrierLevel(temp);
}

void tx_settings_dialog::audioCompressionChanged(int level)
{
if (level < 0 || level > 20) level = 0;
qDebug() << "Audio Compression Level" << level;

//set->setAudioCompression(level + 0.0);
}


void tx_settings_dialog::fmDeviationChanged(int level) {
    if (level > 15) level=15;
    set->setFmDeveation( level * 1000.0 );
}
