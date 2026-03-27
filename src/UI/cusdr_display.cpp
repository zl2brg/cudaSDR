#include "cusdr_display.h"
#include "ui_cusdr_display.h"

cusdr_Display::cusdr_Display(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::cusdr_Display)
{
    ui->setupUi(this);
}

cusdr_Display::~cusdr_Display()
{
    delete ui;
}
