/**
* @file  cusdr_ogl3DPanel.h
* @brief 3D Panadapter panel header file for cuSDR
* @author Simon Brown, ZL2BRG (based on cuSDR framework by Hermann von Hasseln, DL3HVH)
* @version 0.1
* @date 2025-10-12
*/

/*
 *   Copyright 2025 Simon Brown, ZL2BRG
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
    void setSpectrumBuffer(int rx, const QVector<float>& buffer);
    void setCtrFrequency(QObject* sender, int mode, int rx, long freq);
    void setVFOFrequency(QObject* sender, int mode, int rx, long freq);

protected:
    // OpenGL overrides
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

    // Event handlers
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    // Core rendering
    void setupShaders();
    void setupMesh();
    void updateMesh();
    void renderSpectrum3D();
    void renderGrid();
    void renderAxes();
    
    // Camera system
    void updateCamera();
    void resetCamera();
    QMatrix4x4 getCameraMatrix();
    
    // Utility functions
    QVector3D spectrumToWorld(int freqBin, float amplitude, int timeSlice);
    QColor amplitudeToColor(float amplitude);
    void qglColor(QColor color);

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
    
    // Camera state
    QVector3D m_cameraPosition;
    QVector3D m_cameraTarget;
    QVector3D m_cameraUp;
    float m_cameraDistance;
    float m_cameraPitch;
    float m_cameraYaw;
    QMatrix4x4 m_viewMatrix;
    QMatrix4x4 m_projectionMatrix;
    QMatrix4x4 m_modelMatrix;
    
    // Mouse interaction
    QPoint m_lastMousePos;
    bool m_mousePressed;
    Qt::MouseButton m_mouseButton;
    
    // Spectrum data management
    static const int MAX_TIME_SLICES = 100;
    static const int MAX_FREQ_BINS = 2048;
    QVector<QVector<float>> m_spectrumHistory;
    QVector<float> m_currentSpectrum;
    int m_spectrumWidth;
    int m_currentTimeSlice;
    
    // Mesh data
    QVector<float> m_vertices;
    QVector<unsigned int> m_indices;
    int m_vertexCount;
    int m_indexCount;
    bool m_meshNeedsUpdate;
    
    // Rendering parameters
    float m_heightScale;
    float m_frequencyScale;
    float m_timeScale;
    QColor m_backgroundColor;
    QColor m_gridColor;
    QColor m_axesColor;
    
    // Performance
    QTimer* m_updateTimer;
    int m_updateInterval;  // ms
    
    // Fonts and text
    TFonts m_fonts;
    OGLText* m_oglTextSmall;
    
    // Display state
    bool m_showGrid;
    bool m_showAxes;
    bool m_showWireframe;
    float m_aspectRatio;

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