/**
* @file  cusdr_ogl3DPanel.h
* @brief 3D Panadapter panel header file for cuSDR
* @author Simon Eatough, ZL2BRG (based on cuSDR framework by Hermann von Hasseln, DL3HVH)
* @version 0.1
* @date 2025-10-12
*/

/*
 *   Copyright 2025 Simon Eatough, ZL2BRG
 *   Based on cuSDR framework Copyright 2011 Hermann von Hasseln, DL3HVH
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef _CUSDR_OGL3D_PANEL_H
#define _CUSDR_OGL3D_PANEL_H

#include "cusdr_oglUtils.h"
#include "cusdr_settings.h"
#include "cusdr_fonts.h"
#include "cusdr_oglText.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <QVector3D>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>

class QGL3DPanel : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    explicit QGL3DPanel(QWidget *parent, int rx = 0);
    ~QGL3DPanel();

    // Public interface
    void setSpectrumData(const QVector<float>& spectrumData);
    void setPanadapterColors();
    void setDisplaySize(QSize size);

public slots:
    // SDR integration slots
    void setSpectrumBuffer(int rx, const qVectorFloat& buffer);
    void setCtrFrequency(QObject* sender, int mode, int rx, long freq);
    void setVFOFrequency(QObject* sender, int mode, int rx, long freq);
    
    // 3D Display control slots
    void setHeightScale(float scale);
    void setFrequencyScale(float scale);
    void setTimeScale(float scale);
    void setUpdateRate(int intervalMs);
    void setShowGrid(bool show);
    void setShowAxes(bool show);
    void setWireframeMode(bool wireframe);
    void setWaterfallOffset(float offset);

protected:
    // OpenGL overrides
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

    // Event handlers
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    // Core rendering
    void setupShaders();
    void setupMesh();
    void setupGrid();
    void updateMesh();
    void renderSpectrum3D();
    void renderGrid();
    void renderAxes();
    void renderVerticalScale();
    
    // Camera system
    void updateCamera();
    void resetCamera();
    QMatrix4x4 getCameraMatrix();
    
    // Utility functions
    QVector3D spectrumToWorld(int freqBin, float amplitude, int timeSlice);
    QColor amplitudeToColor(float amplitude);
    QColor amplitudeToColorWithOffset(float amplitude, float offset);
    void qglColor(QColor color);

private slots:
    void performUpdate();
    void calculateFPS();

private:
    Settings* set;
    
    // SDR integration
    int m_receiver;
    long m_centerFrequency;
    long m_vfoFrequency;
    float m_sampleRate;
    
    // 3D Rendering infrastructure
    QOpenGLShaderProgram* m_shaderProgram;
    QOpenGLBuffer* m_vertexBuffer;
    QOpenGLBuffer* m_indexBuffer;
    QOpenGLVertexArrayObject* m_vao;
    
    // Grid rendering
    QOpenGLBuffer* m_gridVertexBuffer;
    QOpenGLVertexArrayObject* m_gridVAO;
    int m_gridVertexCount;
    
    // Camera state
    QVector3D m_cameraPosition;
    QVector3D m_cameraTarget;
    QVector3D m_cameraUp;
    float m_cameraDistance;
    float m_cameraPitch;
    float m_cameraYaw;
    float m_cameraOffsetX;      // Horizontal pan offset
    float m_cameraOffsetY;      // Vertical pan offset
    QMatrix4x4 m_viewMatrix;
    QMatrix4x4 m_projectionMatrix;
    QMatrix4x4 m_modelMatrix;
    
    // Mouse interaction
    QPoint m_lastMousePos;
    bool m_mousePressed;
    Qt::MouseButton m_mouseButton;
    
    // Spectrum data management
    static const int MAX_TIME_SLICES = 300;  // Increased for deeper waterfall effect
    static const int MAX_FREQ_BINS = 8192;  // Increased to handle 4096+ samples
    QVector<QVector<float>> m_spectrumHistory;
    QVector<float> m_currentSpectrum;
    QVector<QVector<float>> m_meshSpectrumSnapshot;  // Stable snapshot for mesh generation
    float m_meshWaterfallOffset;  // Waterfall offset snapshot for consistent colors
    int m_spectrumWidth;
    int m_currentTimeSlice;
    
    // Mesh data
    QVector<float> m_vertices;
    QVector<unsigned int> m_indices;
    int m_vertexCount;
    int m_indexCount;
    bool m_meshNeedsUpdate;
    
    // Performance optimization
    QTimer* m_updateTimer;
    QTimer* m_frameTimer;
    qint64 m_lastUpdateTime;
    qint64 m_lastFrameTime;
    
    int m_frameCount;
    float m_currentFPS;
    int m_targetFPS;
    int m_updateFrequencyMs;
    bool m_isVisible;
    
    // Rendering parameters
    float m_heightScale;
    float m_frequencyScale;
    float m_timeScale;
    float m_waterfallOffset;  // dBm offset for color mapping
    QColor m_backgroundColor;
    QColor m_gridColor;
    QColor m_axesColor;
    
    // Legacy performance (keep for compatibility)
    int m_updateInterval;  // ms
    
    // Fonts and text
    TFonts m_fonts;
    OGLText* m_oglTextSmall;
    
    // Display state
    bool m_showGrid;
    bool m_showAxes;
    bool m_showWireframe;
    float m_aspectRatio;
    
    // Debug and monitoring
    int m_dataUpdateCount;
    int m_meshUpdateCount;
    qint64 m_lastDebugTime;

public slots:
    void spectrumDataChanged(const QVector<float>& data);
    void updateDisplay();

signals:
    void mousePositionChanged(QPoint position);
    void frequencySelected(float frequency);

private slots:
    void onUpdateTimer();
};

#endif // _CUSDR_OGL3D_PANEL_H