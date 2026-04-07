#include "tx_settings_dialog.h"
#include "ui_tx_settings_dialog.h"
#include "QtWDSP/qtwdsp_dspEngine.h"
#include "AudioEngine/cusdr_audio_input.h"

namespace {
int findDeviceComboIndex(const QList<QAudioDevice> &devices, const QString &name, int offset)
{
    for (int i = 0; i < devices.size(); ++i) {
        if (devices.at(i).description() == name)
            return i + offset;
    }
    return -1;
}
}

tx_settings_dialog::tx_settings_dialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tx_settings_dialog),
    set(Settings::instance()),
    m_codec2ModeCombo(nullptr)

{

    m_amCarrierLevel = set->getAMCarrierLevel();
    m_audioCompressionLevel = set->getAudioCompression();
    qDebug() << "Am carrier level load" << m_amCarrierLevel;
    qDebug() << "compressionl evel load" << m_audioCompressionLevel;



    setContentsMargins(4, 0, 4, 0);
    ui->setupUi(this);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    int temp = m_amCarrierLevel * 100;
    ui->amCarrierLevel->setSliderPosition(0.5);
    ui->audioCompression->setSliderPosition(m_audioCompressionLevel);
    ui->fm_deviation->setValue(int(set->getFMDeveation() / 1000.0));

    // Populate audio input device list from the shared TX audio input helper
    ui->audiodevlist->clear();
    ui->audiodevlist->addItem("HPSDR Mic Input");
    
    const QList<QAudioDevice> micInputs = TransmitAudioInput::availableAudioInputDevices();
    for (const QAudioDevice &deviceInfo : micInputs) {
        ui->audiodevlist->addItem(deviceInfo.description());
        qDebug() << "Audio input device:" << deviceInfo.description();
    }

    {
        int micIndex = -1;
        const QString savedMicName = set->getMicInputSourceName();
        if (savedMicName == "hpsdr-local") {
            micIndex = 0;
        } else {
            micIndex = findDeviceComboIndex(micInputs, savedMicName, 1);
            if (micIndex < 0) {
                const QString defaultName = QMediaDevices::defaultAudioInput().description();
                micIndex = findDeviceComboIndex(micInputs, defaultName, 1);
            }
            if (micIndex < 0)
                micIndex = 0;
        }
        ui->audiodevlist->setCurrentIndex(micIndex);
        set->setMicInputDev(micIndex);
        if (micIndex == 0)
            set->setMicInputSourceName("hpsdr-local");
        else
            set->setMicInputSourceName(ui->audiodevlist->currentText());
    }

    // Populate digital audio input device list (for FT8 / digi modes)
    ui->digitalAudioDevList->clear();
    ui->digitalAudioDevList->addItem("None");
    const QList<QAudioDevice> digitalInputs = TransmitAudioInput::availableAudioInputDevices();
    for (const QAudioDevice &deviceInfo : digitalInputs) {
        ui->digitalAudioDevList->addItem(deviceInfo.description());
    }
    {
        int digitalIndex = -1;
        const QString savedDigitalName = set->getDigitalInputSourceName();
        if (savedDigitalName == "none") {
            digitalIndex = 0;
        } else {
            digitalIndex = findDeviceComboIndex(digitalInputs, savedDigitalName, 1);
            if (digitalIndex < 0) {
                const QString defaultName = QMediaDevices::defaultAudioInput().description();
                digitalIndex = findDeviceComboIndex(digitalInputs, defaultName, 1);
            }
            if (digitalIndex < 0)
                digitalIndex = 0;
        }
        ui->digitalAudioDevList->setCurrentIndex(digitalIndex);
        set->setDigitalAudioInputDev(digitalIndex);
        if (digitalIndex == 0)
            set->setDigitalInputSourceName("none");
        else
            set->setDigitalInputSourceName(ui->digitalAudioDevList->currentText());
    }
    
    // Populate Codec2/FreeDV mode selector
    m_codec2ModeCombo = new QComboBox(this);
    m_codec2ModeCombo->setObjectName("codec2ModeCombo");
    QList<int> availableModes = set->availableCodec2Modes();
    for (int mode : availableModes) {
        m_codec2ModeCombo->addItem(set->getCodec2ModeString(mode), mode);
    }
    // Set current mode
    int currentMode = set->getFreeDVMode(0); // Use receiver 0 for now
    int modeIndex = m_codec2ModeCombo->findData(currentMode);
    if (modeIndex >= 0) {
        m_codec2ModeCombo->setCurrentIndex(modeIndex);
    } else {
        m_codec2ModeCombo->setCurrentIndex(0); // Default to mode 0
    }
    
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

 connect(ui->audiodevlist, &QComboBox::currentIndexChanged, this, [this](int index) {
     if (index == 0)
         set->setMicInputSourceName("hpsdr-local");
     else
         set->setMicInputSourceName(ui->audiodevlist->itemText(index));
 });

 CHECKED_CONNECT(ui->digitalAudioDevList,
                 SIGNAL(currentIndexChanged(int)),
                 set,
                 SLOT(setDigitalAudioInputDev(int)));

 connect(ui->digitalAudioDevList, &QComboBox::currentIndexChanged, this, [this](int index) {
     if (index == 0)
         set->setDigitalInputSourceName("none");
     else
         set->setDigitalInputSourceName(ui->digitalAudioDevList->itemText(index));
 });

 // Codec2 mode selector
 connect(m_codec2ModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
     if (index >= 0) {
         int mode = m_codec2ModeCombo->itemData(index).toInt();
         set->setFreeDVMode(this, 0, mode); // Set mode for receiver 0
         qDebug() << "Codec2 mode changed to:" << mode << set->getCodec2ModeString(mode);
     }
 });

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

