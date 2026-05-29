#include "cusdr_radiosettingswidget.h"
#include "ui_cusdr_radiosettingswidget.h"

cusdr_radioSettingsWidget::cusdr_radioSettingsWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::cusdr_radioSettingsWidget),
    set(Settings::instance())
{
    ui->setupUi(this);
    setupRadioTab();
}

cusdr_radioSettingsWidget::~cusdr_radioSettingsWidget()
{
    delete ui;
}

void cusdr_radioSettingsWidget::setupRadioTab()
{
#ifdef HAVE_SOAPYSDR
    // Populate from current settings
    ui->fullDuplexCheck->blockSignals(true);
    ui->autoCalCheck->blockSignals(true);
    ui->lnaSlider->blockSignals(true);
    ui->lnaSpinBox->blockSignals(true);
    ui->tiaSlider->blockSignals(true);
    ui->pgaSlider->blockSignals(true);
    ui->pgaSpinBox->blockSignals(true);
    ui->overallGainSlider->blockSignals(true);
    ui->overallGainSpinBox->blockSignals(true);

    ui->fullDuplexCheck->setChecked(set->getTxFullDuplex());
    ui->autoCalCheck->setChecked(set->getSoapyAutoCalibrate());
    ui->lnaSlider->setValue(set->getSoapyLnaGain());
    ui->lnaSpinBox->setValue(set->getSoapyLnaGain());
    const int tiaGain = set->getSoapyTiaGain();
    const int tiaIndex = tiaGain >= 12 ? 2 : tiaGain >= 9 ? 1 : 0;
    ui->tiaSlider->setValue(tiaIndex);
    ui->tiaValueLabel->setText(QStringLiteral("%1 dB").arg(tiaGain >= 12 ? 12 : tiaGain >= 9 ? 9 : 0));
    ui->pgaSlider->setValue(set->getSoapyPgaGain());
    ui->pgaSpinBox->setValue(set->getSoapyPgaGain());
    ui->overallGainSlider->setValue(set->getSoapyOverallGain());
    ui->overallGainSpinBox->setValue(set->getSoapyOverallGain());

    ui->fullDuplexCheck->blockSignals(false);
    ui->autoCalCheck->blockSignals(false);
    ui->lnaSlider->blockSignals(false);
    ui->lnaSpinBox->blockSignals(false);
    ui->tiaSlider->blockSignals(false);
    ui->pgaSlider->blockSignals(false);
    ui->pgaSpinBox->blockSignals(false);
    ui->overallGainSlider->blockSignals(false);
    ui->overallGainSpinBox->blockSignals(false);

    QStringList antennas = set->getSoapyAntennaList();
    if (!antennas.isEmpty())
        onSoapyAntennaListChanged(antennas);
    QStringList txAntennas = set->getSoapyTxAntennaList();
    if (!txAntennas.isEmpty())
        onSoapyTxAntennaListChanged(txAntennas);

    updateGainGroupVisibility();

    connect(set, &Settings::soapyAntennaListChanged,
            this, &cusdr_radioSettingsWidget::onSoapyAntennaListChanged);
    connect(set, &Settings::soapyTxAntennaListChanged,
            this, &cusdr_radioSettingsWidget::onSoapyTxAntennaListChanged);
    connect(set, &Settings::soapyHardwareKeyChanged,
            this, &cusdr_radioSettingsWidget::onSoapyHardwareKeyChanged);
    connect(set, &Settings::soapyAutoCalibrateChanged,
            this, &cusdr_radioSettingsWidget::onSoapyAutoCalibrateChanged);
    connect(set, &Settings::txFullDuplexChanged,
            this, &cusdr_radioSettingsWidget::onTxFullDuplexChanged);
    connect(ui->fullDuplexCheck, &QCheckBox::toggled,
            this, &cusdr_radioSettingsWidget::onFullDuplexToggled);
    connect(ui->autoCalCheck, &QCheckBox::toggled,
            this, &cusdr_radioSettingsWidget::onAutoCalToggled);
    connect(ui->antennaCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &cusdr_radioSettingsWidget::onAntennaComboChanged);
    connect(ui->txAntennaCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &cusdr_radioSettingsWidget::onTxAntennaComboChanged);
    connect(ui->lnaSlider, &QSlider::valueChanged,
            this, &cusdr_radioSettingsWidget::onLnaSliderChanged);
    connect(ui->lnaSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &cusdr_radioSettingsWidget::onLnaSpinBoxChanged);
    connect(ui->tiaSlider, &QSlider::valueChanged,
            this, &cusdr_radioSettingsWidget::onTiaSliderChanged);
    connect(ui->pgaSlider, &QSlider::valueChanged,
            this, &cusdr_radioSettingsWidget::onPgaSliderChanged);
    connect(ui->pgaSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &cusdr_radioSettingsWidget::onPgaSpinBoxChanged);
    connect(ui->overallGainSlider, &QSlider::valueChanged,
            this, &cusdr_radioSettingsWidget::onOverallGainSliderChanged);
    connect(ui->overallGainSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &cusdr_radioSettingsWidget::onOverallGainSpinBoxChanged);
#endif
}

#ifdef HAVE_SOAPYSDR
QWidget *cusdr_radioSettingsWidget::detachRadioConfigPage()
{
    const int idx = ui->tabWidget->indexOf(ui->radio);
    if (idx < 0)
        return nullptr;

    QWidget *page = ui->tabWidget->widget(idx);
    ui->tabWidget->removeTab(idx);
    page->setParent(nullptr);
    return page;
}
#endif

void cusdr_radioSettingsWidget::updateGainGroupVisibility()
{
#ifdef HAVE_SOAPYSDR
    bool isLime = set->getSoapyHardwareKey().contains("LimeSDR", Qt::CaseInsensitive);
    ui->calibrationGroup->setVisible(isLime);
    ui->limeGainGroup->setVisible(isLime);
    ui->overallGainGroup->setVisible(!isLime);
#endif
}

#ifdef HAVE_SOAPYSDR
void cusdr_radioSettingsWidget::onSoapyAntennaListChanged(QStringList list)
{
    ui->antennaCombo->blockSignals(true);
    ui->antennaCombo->clear();
    ui->antennaCombo->addItems(list);
    int idx = list.indexOf(set->getSoapyRxAntenna());
    if (idx >= 0) ui->antennaCombo->setCurrentIndex(idx);
    ui->antennaCombo->blockSignals(false);
}

void cusdr_radioSettingsWidget::onSoapyTxAntennaListChanged(QStringList list)
{
    ui->txAntennaCombo->blockSignals(true);
    ui->txAntennaCombo->clear();
    ui->txAntennaCombo->addItems(list);
    int idx = list.indexOf(set->getSoapyTxAntenna());
    if (idx < 0)
        idx = list.indexOf(set->getSoapyRxAntenna());
    if (idx < 0 && !list.isEmpty())
        idx = 0;
    if (idx >= 0)
        ui->txAntennaCombo->setCurrentIndex(idx);
    ui->txAntennaCombo->blockSignals(false);
}

void cusdr_radioSettingsWidget::onSoapyHardwareKeyChanged(QString /*key*/)
{
    updateGainGroupVisibility();
}

void cusdr_radioSettingsWidget::onSoapyAutoCalibrateChanged(bool enabled)
{
    ui->autoCalCheck->blockSignals(true);
    ui->autoCalCheck->setChecked(enabled);
    ui->autoCalCheck->blockSignals(false);
}

void cusdr_radioSettingsWidget::onTxFullDuplexChanged(bool fullDuplex)
{
    ui->fullDuplexCheck->blockSignals(true);
    ui->fullDuplexCheck->setChecked(fullDuplex);
    ui->fullDuplexCheck->blockSignals(false);
}

void cusdr_radioSettingsWidget::onFullDuplexToggled(bool enabled)
{
    set->setTxFullDuplex(enabled);
}

void cusdr_radioSettingsWidget::onAutoCalToggled(bool enabled)
{
    set->setSoapyAutoCalibrate(enabled);
}

void cusdr_radioSettingsWidget::onAntennaComboChanged(int index)
{
    set->setSoapyRxAntenna(ui->antennaCombo->itemText(index));
}

void cusdr_radioSettingsWidget::onTxAntennaComboChanged(int index)
{
    set->setSoapyTxAntenna(ui->txAntennaCombo->itemText(index));
}

void cusdr_radioSettingsWidget::onLnaSliderChanged(int value)
{
    ui->lnaSpinBox->blockSignals(true);
    ui->lnaSpinBox->setValue(value);
    ui->lnaSpinBox->blockSignals(false);
    set->setSoapyLnaGain(value);
}

void cusdr_radioSettingsWidget::onLnaSpinBoxChanged(int value)
{
    ui->lnaSlider->blockSignals(true);
    ui->lnaSlider->setValue(value);
    ui->lnaSlider->blockSignals(false);
    set->setSoapyLnaGain(value);
}

void cusdr_radioSettingsWidget::onTiaSliderChanged(int index)
{
    static const int tiaValues[] = {0, 9, 12};
    const int gain = (index >= 0 && index < 3) ? tiaValues[index] : 0;
    ui->tiaValueLabel->setText(QStringLiteral("%1 dB").arg(gain));
    set->setSoapyTiaGain(gain);
}

void cusdr_radioSettingsWidget::onPgaSliderChanged(int value)
{
    ui->pgaSpinBox->blockSignals(true);
    ui->pgaSpinBox->setValue(value);
    ui->pgaSpinBox->blockSignals(false);
    set->setSoapyPgaGain(value);
}

void cusdr_radioSettingsWidget::onPgaSpinBoxChanged(int value)
{
    ui->pgaSlider->blockSignals(true);
    ui->pgaSlider->setValue(value);
    ui->pgaSlider->blockSignals(false);
    set->setSoapyPgaGain(value);
}

void cusdr_radioSettingsWidget::onOverallGainSliderChanged(int value)
{
    ui->overallGainSpinBox->blockSignals(true);
    ui->overallGainSpinBox->setValue(value);
    ui->overallGainSpinBox->blockSignals(false);
    set->setSoapyOverallGain(value);
}

void cusdr_radioSettingsWidget::onOverallGainSpinBoxChanged(int value)
{
    ui->overallGainSlider->blockSignals(true);
    ui->overallGainSlider->setValue(value);
    ui->overallGainSlider->blockSignals(false);
    set->setSoapyOverallGain(value);
}
#endif
