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
    ui->lnaSpinBox->blockSignals(true);
    ui->tiaCombo->blockSignals(true);
    ui->pgaSpinBox->blockSignals(true);
    ui->overallGainSlider->blockSignals(true);
    ui->overallGainSpinBox->blockSignals(true);

    ui->lnaSpinBox->setValue(set->getSoapyLnaGain());
    int tiaGain = set->getSoapyTiaGain();
    ui->tiaCombo->setCurrentIndex(tiaGain >= 12 ? 2 : tiaGain >= 9 ? 1 : 0);
    ui->pgaSpinBox->setValue(set->getSoapyPgaGain());
    ui->overallGainSlider->setValue(set->getSoapyOverallGain());
    ui->overallGainSpinBox->setValue(set->getSoapyOverallGain());

    ui->lnaSpinBox->blockSignals(false);
    ui->tiaCombo->blockSignals(false);
    ui->pgaSpinBox->blockSignals(false);
    ui->overallGainSlider->blockSignals(false);
    ui->overallGainSpinBox->blockSignals(false);

    QStringList antennas = set->getSoapyAntennaList();
    if (!antennas.isEmpty())
        onSoapyAntennaListChanged(antennas);

    updateGainGroupVisibility();

    connect(set, &Settings::soapyAntennaListChanged,
            this, &cusdr_radioSettingsWidget::onSoapyAntennaListChanged);
    connect(set, &Settings::soapyHardwareKeyChanged,
            this, &cusdr_radioSettingsWidget::onSoapyHardwareKeyChanged);
    connect(ui->antennaCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &cusdr_radioSettingsWidget::onAntennaComboChanged);
    connect(ui->lnaSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &cusdr_radioSettingsWidget::onLnaSpinBoxChanged);
    connect(ui->tiaCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &cusdr_radioSettingsWidget::onTiaComboChanged);
    connect(ui->pgaSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &cusdr_radioSettingsWidget::onPgaSpinBoxChanged);
    connect(ui->overallGainSlider, &QSlider::valueChanged,
            this, &cusdr_radioSettingsWidget::onOverallGainSliderChanged);
    connect(ui->overallGainSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &cusdr_radioSettingsWidget::onOverallGainSpinBoxChanged);
#endif
}

void cusdr_radioSettingsWidget::updateGainGroupVisibility()
{
#ifdef HAVE_SOAPYSDR
    bool isLime = set->getSoapyHardwareKey().contains("LimeSDR", Qt::CaseInsensitive);
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

void cusdr_radioSettingsWidget::onSoapyHardwareKeyChanged(QString /*key*/)
{
    updateGainGroupVisibility();
}

void cusdr_radioSettingsWidget::onAntennaComboChanged(int index)
{
    set->setSoapyRxAntenna(ui->antennaCombo->itemText(index));
}

void cusdr_radioSettingsWidget::onLnaSpinBoxChanged(int value)
{
    set->setSoapyLnaGain(value);
}

void cusdr_radioSettingsWidget::onTiaComboChanged(int index)
{
    static const int tiaValues[] = {0, 9, 12};
    set->setSoapyTiaGain((index >= 0 && index < 3) ? tiaValues[index] : 0);
}

void cusdr_radioSettingsWidget::onPgaSpinBoxChanged(int value)
{
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
