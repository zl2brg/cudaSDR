#ifndef NETWORKSETTINGSCONTROLLER_H
#define NETWORKSETTINGSCONTROLLER_H

#include <QObject>

class Settings;
class NetworkWidget;

class NetworkSettingsController : public QObject {
    Q_OBJECT

public:
    explicit NetworkSettingsController(QObject* parent = nullptr);

    void bind(NetworkWidget* view, Settings* model);

private:
    NetworkWidget* m_view = nullptr;
    Settings* m_model = nullptr;
};

#endif // NETWORKSETTINGSCONTROLLER_H
