#ifndef CUSDR_DISPLAY_H
#define CUSDR_DISPLAY_H

#include <QWidget>

namespace Ui {
class cusdr_Display;
}

class cusdr_Display : public QWidget
{
    Q_OBJECT

public:
    explicit cusdr_Display(QWidget *parent = nullptr);
    ~cusdr_Display();

private:
    Ui::cusdr_Display *ui;
};

#endif // CUSDR_DISPLAY_H
