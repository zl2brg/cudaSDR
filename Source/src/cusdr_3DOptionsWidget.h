/**
* @file  cusdr_3DOptionsWidget.h
* @brief 3D display options widget header file for cuSDR
* @author Simon Brown, ZL2BRG
* @version 0.1
* @date 2024-12-28
*/

#ifndef _CUSDR_3DOPTIONSWIDGET_H
#define _CUSDR_3DOPTIONSWIDGET_H

#include <QWidget>
#include <QGroupBox>
#include <QCheckBox>
#include <QSlider>
#include <QLabel>
#include <QComboBox>

#include "cusdr_settings.h"
#include "Util/cusdr_buttons.h"

class Options3DWidget : public QWidget {
    Q_OBJECT

public:
    explicit Options3DWidget(QWidget *parent = 0);
    ~Options3DWidget();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

private slots:
    void enable3DChanged();
    void heightScaleChanged(int value);
    void frequencyScaleChanged(int value);
    void timeScaleChanged(int value);
    void updateIntervalChanged(int value);
    void showGridChanged();
    void showAxesChanged();
    void wireframeModeChanged();
    void showContoursChanged();
    void contourIntervalChanged(int value);
    void contourMinLevelChanged(int value);
    void waterfallOffsetChanged(int value);

private:
    void setupConnections();
    void createGeneralGroup();
    void createRenderingGroup();
    void createPerformanceGroup();

    Settings* set;
    
    // Groups
    QGroupBox* m_generalGroup;
    QGroupBox* m_renderingGroup;
    QGroupBox* m_performanceGroup;
    
    // General options
    QCheckBox* m_enable3DCheckBox;
    
    // Rendering options
    QSlider* m_heightScaleSlider;
    QLabel* m_heightScaleLabel;
    QSlider* m_frequencyScaleSlider;
    QLabel* m_frequencyScaleLabel;
    QSlider* m_timeScaleSlider;
    QLabel* m_timeScaleLabel;
    QCheckBox* m_showGridCheckBox;
    QCheckBox* m_showAxesCheckBox;
    QCheckBox* m_wireframeModeCheckBox;
    QCheckBox* m_showContoursCheckBox;
    QSlider* m_contourIntervalSlider;
    QLabel* m_contourIntervalLabel;
    QSlider* m_contourMinLevelSlider;
    QLabel* m_contourMinLevelLabel;
    QSlider* m_waterfallOffsetSlider;
    QLabel* m_waterfallOffsetLabel;
    
    // Performance options
    QSlider* m_updateIntervalSlider;
    QLabel* m_updateIntervalLabel;
    QComboBox* m_qualityComboBox;
    
    int m_minimumWidgetWidth;
    int m_minimumGroupBoxWidth;

signals:
    void show3DPanadapterChanged(bool enabled);
    void heightScaleValueChanged(float scale);
    void frequencyScaleValueChanged(float scale);
    void timeScaleValueChanged(float scale);
    void updateIntervalValueChanged(int interval);
    void showGridValueChanged(bool show);
    void showAxesValueChanged(bool show);
    void wireframeModeValueChanged(bool wireframe);
    void showContoursValueChanged(bool show);
    void contourIntervalValueChanged(float interval);
    void contourMinLevelValueChanged(float minLevel);
    void waterfallOffsetValueChanged(float offset);
};

#endif // _CUSDR_3DOPTIONSWIDGET_H