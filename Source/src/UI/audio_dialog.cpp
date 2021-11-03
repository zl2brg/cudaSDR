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
    ui->audiodevlist->setStyleSheet(set->getComboBoxStyle());
    ui->groupBox->setStyleSheet(set->getWidgetStyle());
    ui->groupBox_2->setStyleSheet(set->getWidgetStyle());
    ui->groupBox_3->setStyleSheet(set->getWidgetStyle());
    ui->am_carrierlevel->setStyleSheet(set->getSpinBoxStyle());
    ui->checkBox->setStyleSheet(set->getCheckBoxStyle());
    ui->ctcss_tone->setStyleSheet(set->getCheckBoxStyle());
    ui->fm_deviation->setStyleSheet(set->getSpinBoxStyle());
    ui->am_carrierlevel->setStyleSheet(set->getSpinBoxStyle());
    ui->fm_deviation->setStyleSheet(set->getSpinBoxStyle());
    ui->audio_compression->setStyleSheet(set->getSpinBoxStyle());
    ui->checkBox_3->setStyleSheet(set->getCheckBoxStyle());
    ui->label->setStyleSheet(set->getLabelStyle());
    ui->label_2->setStyleSheet(set->getLabelStyle());
    ui->label_3->setStyleSheet(set->getLabelStyle());
    ui->label_4->setStyleSheet(set->getLabelStyle());
    ui->label->setFrameStyle(QFrame::Box | QFrame::Raised);
    ui->label_2->setFrameStyle(QFrame::Box | QFrame::Raised);
    ui->label_3->setFrameStyle(QFrame::Box | QFrame::Raised);
    ui->label_4->setFrameStyle(QFrame::Box | QFrame::Raised);


    ui->local_mic->setStyleSheet(set->getCheckBoxStyle());
    ui->repeater_offset->setStyleSheet(set->getSpinBoxStyle());
    setContentsMargins(4, 4, 4, 4);
    setWindowOpacity(0.9);






    ui->audiodevlist->clear();
    for (QAudioDeviceInfo d : m_availableAudioInputDevices) {
           ui->audiodevlist->addItem(d.deviceName(),QVariant::fromValue(d));
       }
    ui->audiodevlist->setCurrentIndex(set->getMicInputDev());


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
close();
}



void audio_dialog::cancel_pressed(){
    this->close();
}

void audio_dialog::audio_input_changed(int index){

qDebug() << "selected" << index;
}


