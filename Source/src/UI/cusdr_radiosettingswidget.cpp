#include "cusdr_radiosettingswidget.h"
#include "ui_cusdr_radiosettingswidget.h"

cusdr_radioSettingsWidget::cusdr_radioSettingsWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::cusdr_radioSettingsWidget)
{
    ui->setupUi(this);
}

cusdr_radioSettingsWidget::~cusdr_radioSettingsWidget()
{
    delete ui;
}
