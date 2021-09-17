#include "audio_dialog.h"
#include "ui_audio_dialog.h"

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

 CHECKED_CONNECT(ui->audiodevlist,
                 SIGNAL(currentIndexChanged(int)),
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
