#ifndef WINDOWCONFIG_H
#define WINDOWCONFIG_H

#include <QObject>
#include <QJsonObject>

class QSettings;

class WindowConfig : public QObject {
    Q_OBJECT
    Q_PROPERTY(int minimumWidgetWidth READ minimumWidgetWidth WRITE setMinimumWidgetWidth NOTIFY minimumWidgetWidthChanged)
    Q_PROPERTY(int minimumGroupBoxWidth READ minimumGroupBoxWidth WRITE setMinimumGroupBoxWidth NOTIFY minimumGroupBoxWidthChanged)
    Q_PROPERTY(int multiRxView READ multiRxView WRITE setMultiRxView NOTIFY multiRxViewChanged)

public:
    explicit WindowConfig(QObject *parent = nullptr);

    int minimumWidgetWidth() const { return m_minimumWidgetWidth; }
    void setMinimumWidgetWidth(int value);

    int minimumGroupBoxWidth() const { return m_minimumGroupBoxWidth; }
    void setMinimumGroupBoxWidth(int value);

    int multiRxView() const { return m_multiRxView; }
    void setMultiRxView(int view);

    void load(const QJsonObject &json);
    void save(QJsonObject &json) const;

    void loadIni(QSettings *settings);
    void saveIni(QSettings *settings) const;

signals:
    void minimumWidgetWidthChanged(int value);
    void minimumGroupBoxWidthChanged(int value);
    void multiRxViewChanged(int view);

private:
    int m_minimumWidgetWidth;
    int m_minimumGroupBoxWidth;
    int m_multiRxView;
};

#endif // WINDOWCONFIG_H
