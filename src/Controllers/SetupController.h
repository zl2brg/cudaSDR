#ifndef SETUPCONTROLLER_H
#define SETUPCONTROLLER_H

#include <QObject>

class cusdr_SetupWidget;
class Settings;

class SetupController : public QObject
{
    Q_OBJECT
public:
    explicit SetupController(QObject* parent = nullptr);

    void bind(cusdr_SetupWidget* view, Settings* model);

private:
    cusdr_SetupWidget* m_view = nullptr;
    Settings*          m_model = nullptr;
};

#endif // SETUPCONTROLLER_H
