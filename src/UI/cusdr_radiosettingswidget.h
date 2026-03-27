#ifndef CUSDR_RADIOSETTINGSWIDGET_H
#define CUSDR_RADIOSETTINGSWIDGET_H

#include <QDialog>

namespace Ui {
class cusdr_radioSettingsWidget;
}

class cusdr_radioSettingsWidget : public QDialog
{
    Q_OBJECT

public:
    explicit cusdr_radioSettingsWidget(QWidget *parent = nullptr);
    ~cusdr_radioSettingsWidget();

private:
    Ui::cusdr_radioSettingsWidget *ui;
};

#endif // CUSDR_RADIOSETTINGSWIDGET_H
