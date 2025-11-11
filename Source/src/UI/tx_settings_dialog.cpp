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
    qDebug() << "compressionl evel load" << m_audioCompressionLevel;



    setContentsMargins(4, 0, 4, 0);
    ui->setupUi(this);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->setStyleSheet(set->getTabWidgetStyle());
    this->ui->amCarrierLevel->setStyleSheet(set->getVolSliderStyle());
    this->ui->audioCompression->setStyleSheet(set->getVolSliderStyle());

    int temp = m_amCarrierLevel * 100;
    ui->amCarrierLevel->setSliderPosition(0.5);
    ui->audioCompression->setSliderPosition(m_audioCompressionLevel);
    ui->fm_deviation->setValue(int(set->getFMDeveation() / 1000.0));

    // Populate audio input device list using Qt6 API
    ui->audiodevlist->clear();
    ui->audiodevlist->addItem("HPSDR Mic Input");
    
    // Get available audio input devices
    QList<QAudioDevice> audioInputs = QMediaDevices::audioInputs();
    for (const QAudioDevice &deviceInfo : audioInputs) {
        ui->audiodevlist->addItem(deviceInfo.description());
        qDebug() << "Audio input device:" << deviceInfo.description();
    }

    ui->audiodevlist->setCurrentIndex(set->getMicInputDev());
    ui->sidetone_freq->setValue(set->getCwSidetoneFreq());
    ui->sidetone_volume->setValue(set->getCwSidetoneVolume());
    ui->cw_hangtime->setValue(set->getCwHangTime());
    ui->KeyerMode->setCurrentIndex(set->getCwKeyerMode());
    ui->internal_keyer->setChecked(set->isInternalCw());
    ui->keyer_reverse->setChecked(set->isCwKeyReversed());
    ui->keyer_spacing->setChecked(set->getCwKeyerSpacing());

    ui->weight->setValue(set->getCwKeyerWeight());
    ui->groupBox->setContentsMargins(2,2,2,2);


    setContentsMargins(4, 4, 4, 4);
    setWindowOpacity(0.9);

 CHECKED_CONNECT(ui->audiodevlist,
                 SIGNAL(currentIndexChanged(int)),
                 set,
                 SLOT(setMicInputDev(int)));

 CHECKED_CONNECT(ui->audioCompression,
                 SIGNAL(valueChanged(int)),
                 set,
                 SLOT(setAudioCompression(int)));

 CHECKED_CONNECT(ui->amCarrierLevel,
                 SIGNAL(valueChanged(int)),
                 set,
                 SLOT(setAMCarrierLevel(int)));

 CHECKED_CONNECT(ui->fm_deviation,
                  SIGNAL(valueChanged(int)),
                   set,
                 SLOT(setFmDeveation(int)));


 CHECKED_CONNECT(ui->KeyerMode,
                  SIGNAL(currentIndexChanged(int)),
                   set,
                  SLOT(setCwKeyerMode(int)));

 CHECKED_CONNECT(ui->internal_keyer,
                  SIGNAL(stateChanged(int)),
                   set,
                  SLOT(setInternalCw(int)));

 CHECKED_CONNECT(ui->keyer_reverse,
                  SIGNAL(stateChanged(int)),
                   set,
                  SLOT(setCwKeyReversed(int)));

 CHECKED_CONNECT(ui->keyer_speed,
                  SIGNAL(valueChanged(int)),
                   set,
                  SLOT(setCwKeyerSpeed(int)));

 CHECKED_CONNECT(ui->ptt_delay,
                  SIGNAL(valueChanged(int)),
                   set,
                  SLOT(setCwPttDelay(int)));

 CHECKED_CONNECT(ui->sidetone_freq,
                  SIGNAL(valueChanged(int)),
                   set,
                  SLOT(setCwSidetoneFreq(int)));


    CHECKED_CONNECT(ui->sidetone_volume,
                    SIGNAL(valueChanged(int)),
                    set,
                    SLOT(setCwSidetoneVolume(int)));

 CHECKED_CONNECT(ui->cw_hangtime,
                  SIGNAL(valueChanged(int)),
                   set,
                  SLOT(setCwHangTime(int)));

    CHECKED_CONNECT(ui->weight,
                    SIGNAL(valueChanged(int)),
                    set,
                    SLOT(setCwKeyerWeight(int)));

    CHECKED_CONNECT(ui->keyer_spacing,
                    SIGNAL(stateChanged(int)),
                    set,
                    SLOT(setCwKeyerSpacing(int)));

}




tx_settings_dialog::~tx_settings_dialog()
{
    delete ui;
    disconnect(set, 0, this, 0);
    disconnect(this, 0, 0, 0);
}
