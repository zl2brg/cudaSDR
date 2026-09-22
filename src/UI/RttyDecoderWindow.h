/**
 * @file  RttyDecoderWindow.h
 * @brief Independent movable desktop window for the RTTY demodulator, oscilloscope, and terminal.
 * @author Simon Eatough <simon.eatough@gmail.com>
 * @date 2026-09-22
 */

#pragma once

#include <QWidget>
#include <QMenu>
#include <functional>

class SliceModel;
class Settings;
class QPushButton;
class QCheckBox;
class QPlainTextEdit;
class QScrollBar;
class QVBoxLayout;
class QHBoxLayout;
class QSplitter;

/**
 * @class RttyScopeWidget
 * @brief Compact real-time Lissajous XY oscilloscope and frequency nudge control for RTTY tuning.
 */
class RttyScopeWidget : public QWidget {
    Q_OBJECT

public:
    explicit RttyScopeWidget(SliceModel *slice, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    SliceModel *m_sliceModel = nullptr;
    float m_peak = 0.0001f;
    QPushButton *m_nudgeLeftBtn = nullptr;
    QPushButton *m_nudgeRightBtn = nullptr;
};

/**
 * @class RttyDecoderWindow
 * @brief Top-level desktop window for RTTY decoding, movable anywhere across monitors.
 */
class RttyDecoderWindow : public QWidget {
    Q_OBJECT

public:
    explicit RttyDecoderWindow(SliceModel *slice, Settings *settings, int rx, QWidget *parent = nullptr);
    ~RttyDecoderWindow() override = default;

    static QMenu* createConfigMenu(QWidget *parent, SliceModel *slice, Settings *settings, int rx, std::function<void()> updateCb = nullptr);

    void showConfigMenu(const QPoint &globalPos);

protected:
    void closeEvent(QCloseEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void updateStatusBadge();
    void onDecodedTextChanged(const QString &text);
    void onScrollValueChanged(int value);
    void scrollToBottom();

private:
    void setupUi();
    void updateScopeVisibility(bool visible);

    SliceModel *m_sliceModel = nullptr;
    Settings *m_settings = nullptr;
    int m_rx = 0;

    QPushButton *m_badgeBtn = nullptr;
    QPushButton *m_autoBtn = nullptr;
    QPushButton *m_scopeToggleBtn = nullptr;
    QPushButton *m_clearBtn = nullptr;
    QPushButton *m_dockBtn = nullptr;

    QWidget *m_scopeContainer = nullptr;
    RttyScopeWidget *m_scopeWidget = nullptr;
    QPlainTextEdit *m_terminal = nullptr;
    QPushButton *m_jumpLatestBtn = nullptr;

    QString m_lastRenderedText;
    bool m_followLatest = true;
    bool m_isInternalGeometryChange = false;
};
