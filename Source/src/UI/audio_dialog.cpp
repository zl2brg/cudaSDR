#include "audio_dialog.h"
#include "ui_audio_dialog.h"
#include "QtWDSP/qtwdsp_dspEngine.h"

audio_dialog::audio_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::audio_dialog),
    set(Settings::instance()),
    m_availableAudioInputDevices(
    QAudioDeviceInfo::availableDevices(QAudio::AudioInput)),
    m_audioInputDevice(QAudioDeviceInfo::defaultInputDevice())

{
    setContentsMargins(4, 0, 4, 0);
    ui->setupUi(this);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->setStyleSheet(set->getSDRStyle());
    ui->drivelevelSlider->setStyleSheet(set->getVolSliderStyle());
    ui->inputLevelSlider->setStyleSheet(set->getVolSliderStyle());
    ui->audiodevlist->setStyleSheet(set->getComboBoxStyle());
    ui->groupBox->setStyleSheet(set->getWidgetStyle());
    ui->groupBox_2->setStyleSheet(set->getWidgetStyle());
    ui->groupBox_3->setStyleSheet(set->getWidgetStyle());




    ui->audiodevlist->clear();
    ui->audiodevlist->setCurrentIndex(set->getMicInputDev());
    for (QAudioDeviceInfo d : m_availableAudioInputDevices) {
           ui->audiodevlist->addItem(d.deviceName(),QVariant::fromValue(d));
       }
    ui->inputLevelSlider->setValue(set->getMicInputLevel());
    ui->drivelevelSlider->setValue(set->getDriveLevel());


 CHECKED_CONNECT(
            ui->OkButton ,
            SIGNAL(clicked()),
            this,
            SLOT(ok_pressed()));

 CHECKED_CONNECT(
            ui->CancelButton ,
            SIGNAL(clicked()),
            this,
            SLOT(cancel_pressed()));

 CHECKED_CONNECT(ui->audiodevlist,
                 SIGNAL(currentIndexChanged(int)),
                 this,
                 SLOT(audio_input_changed(int)));

 CHECKED_CONNECT(ui->inputLevelSlider,
                    SIGNAL(valueChanged(int)),
                    this,
                    SLOT(audio_input_changed(int)));

 }

audio_dialog::~audio_dialog()
{
    delete ui;
}


void audio_dialog::ok_pressed(){
    QVariant v = ui->audiodevlist->currentData();
    QAudioDeviceInfo dev = v.value<QAudioDeviceInfo>();
    qDebug() << "input device " << dev.deviceName();
    set->setMicInputDev(this,ui->audiodevlist->currentIndex());
    set->setMicInputLevel(this,ui->inputLevelSlider->value());
    set->setDriveLevel(this,ui->drivelevelSlider->value());
close();
}



void audio_dialog::cancel_pressed(){
    this->close();
}

void audio_dialog::audio_input_changed(int index){

qDebug() << "selected" << index;
}


void audio_dialog::mic_level_changed(int level) {

    qDebug() << "Mic Level Changed" << level;
    set->setMicInputLevel(this,ui->inputLevelSlider->value());
    double mic_gain = level;

    SetTXAPanelGain1(TX_ID,pow(10.0, mic_gain/20.0));


}
