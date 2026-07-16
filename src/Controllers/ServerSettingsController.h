#ifndef SERVERSETTINGSCONTROLLER_H
#define SERVERSETTINGSCONTROLLER_H

#include <QObject>

class ServerWidget;
class Settings;

class ServerSettingsController : public QObject
{
    Q_OBJECT
public:
    explicit ServerSettingsController(QObject* parent = nullptr);

    void bind(ServerWidget* view, Settings* model);

private:
    ServerWidget* m_view = nullptr;
    Settings*     m_model = nullptr;
};

#endif // SERVERSETTINGSCONTROLLER_H
