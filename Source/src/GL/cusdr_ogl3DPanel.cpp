/**
* @file  cusdr_ogl3DPanel.cpp
* @brief 3D Panadapter panel implementation for cuSDR
* @author Simon Brown, ZL2BRG (based on cuSDR framework by Hermann von Hasseln, DL3HVH)
* @version 0.1
* @date 2025-10-12
*/

#include "cusdr_ogl3DPanel.h"
#include <QDebug>
#include <QtMath>

// Static constants definition
const int QGL3DPanel::MAX_TIME_SLICES;
const int QGL3DPanel::MAX_FREQ_BINS;

QGL3DPanel::QGL3DPanel(QWidget *parent, Settings* settings)
    : QOpenGLWidget(parent)
    , set(settings)
    , m_shaderProgram(nullptr)
    , m_vertexBuffer(nullptr)
    , m_indexBuffer(nullptr)
    , m_vao(nullptr)
    , m_cameraDistance(50.0f)
    , m_cameraPitch(-30.0f)
    , m_cameraYaw(45.0f)
    , m_mousePressed(false)
    , m_spectrumWidth(512)
    , m_currentTimeSlice(0)
    , m_vertexCount(0)
    , m_indexCount(0)
    , m_meshNeedsUpdate(true)
    , m_heightScale(10.0f)
    , m_frequencyScale(1.0f)
    , m_timeScale(1.0f)
    , m_backgroundColor(Qt::black)
    , m_gridColor(Qt::darkGray)
    , m_axesColor(Qt::white)
    , m_updateInterval(50)  // 20 FPS
    , m_oglTextSmall(nullptr)
    , m_showGrid(true)
    , m_showAxes(true)
    , m_showWireframe(false)
    , m_aspectRatio(1.0f)
{
    // Initialize camera
    m_cameraPosition = QVector3D(0, 20, 30);
    m_cameraTarget = QVector3D(0, 0, 0);
    m_cameraUp = QVector3D(0, 1, 0);

    // Initialize spectrum history
    m_spectrumHistory.reserve(MAX_TIME_SLICES);
    m_currentSpectrum.resize(m_spectrumWidth);
    for (int i = 0; i < m_spectrumWidth; i++) {
        m_currentSpectrum[i] = 0.0f;
    }

    // Setup update timer
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &QGL3DPanel::onUpdateTimer);
    m_updateTimer->start(m_updateInterval);

    // Enable mouse tracking
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

QGL3DPanel::~QGL3DPanel() {
    makeCurrent();
    
    if (m_vao) {
        m_vao->destroy();
        delete m_vao;
    }
    
    if (m_vertexBuffer) {
        m_vertexBuffer->destroy();
        delete m_vertexBuffer;
    }
    
    if (m_indexBuffer) {
        m_indexBuffer->destroy();
        delete m_indexBuffer;
    }
    
    delete m_shaderProgram;
    delete m_oglTextSmall;
    
    doneCurrent();
}

void QGL3DPanel::initializeGL() {
    initializeOpenGLFunctions();
    
    // Enable depth testing and back-face culling
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Set clear color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // Setup shaders
    setupShaders();
    
    // Setup mesh buffers
    setupMesh();
    
    // Initialize text rendering
    QFont smallFont = QFont("Arial", 8);
    qreal dpr = devicePixelRatio();
    m_oglTextSmall = new OGLText(smallFont, dpr);
    
    qDebug() << "3D Panel initialized successfully";
}

void QGL3DPanel::setupShaders() {
    m_shaderProgram = new QOpenGLShaderProgram();
    
    // Vertex shader
    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 color;
        
        uniform mat4 mvpMatrix;
        uniform float heightScale;
        
        out vec3 fragColor;
        out vec3 worldPos;
        
        void main() {
            vec3 scaledPos = vec3(position.x, position.y * heightScale, position.z);
            worldPos = scaledPos;
            gl_Position = mvpMatrix * vec4(scaledPos, 1.0);
            fragColor = color;
        }
    )";
    
    // Fragment shader
    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec3 fragColor;
        in vec3 worldPos;
        
        out vec4 finalColor;
        
        void main() {
            // Simple lighting based on height
            float lightIntensity = 0.3 + 0.7 * clamp(worldPos.y / 20.0, 0.0, 1.0);
            finalColor = vec4(fragColor * lightIntensity, 1.0);
        }
    )";
    
    if (!m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource)) {
        qDebug() << "Failed to compile vertex shader:" << m_shaderProgram->log();
        return;
    }
    
    if (!m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource)) {
        qDebug() << "Failed to compile fragment shader:" << m_shaderProgram->log();
        return;
    }
    
    if (!m_shaderProgram->link()) {
        qDebug() << "Failed to link shader program:" << m_shaderProgram->log();
        return;
    }
    
    qDebug() << "Shaders compiled and linked successfully";
}

void QGL3DPanel::setupMesh() {
    // Create vertex array object
    m_vao = new QOpenGLVertexArrayObject();
    m_vao->create();
    
    // Create vertex buffer
    m_vertexBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_vertexBuffer->create();
    
    // Create index buffer
    m_indexBuffer = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    m_indexBuffer->create();
    
    qDebug() << "Mesh buffers created";
}

void QGL3DPanel::updateMesh() {
    if (m_spectrumHistory.isEmpty()) return;
    
    m_vertices.clear();
    m_indices.clear();
    
    int timeSlices = qMin(m_spectrumHistory.size(), MAX_TIME_SLICES);
    int freqBins = qMin(m_spectrumWidth, MAX_FREQ_BINS);
    
    // Generate vertices
    for (int t = 0; t < timeSlices; t++) {
        const QVector<float>& spectrum = m_spectrumHistory[t];
        for (int f = 0; f < freqBins; f++) {
            float amplitude = (f < spectrum.size()) ? spectrum[f] : 0.0f;
            
            // Position (frequency, amplitude, time)
            float x = (float)f - freqBins / 2.0f;  // Center frequency
            float y = amplitude;
            float z = (float)t - timeSlices / 2.0f;  // Center time
            
            m_vertices.append(x);
            m_vertices.append(y);
            m_vertices.append(z);
            
            // Color based on amplitude
            QColor color = amplitudeToColor(amplitude);
            m_vertices.append(color.redF());
            m_vertices.append(color.greenF());
            m_vertices.append(color.blueF());
        }
    }
    
    // Generate indices for triangle strips
    for (int t = 0; t < timeSlices - 1; t++) {
        for (int f = 0; f < freqBins - 1; f++) {
            int i0 = t * freqBins + f;
            int i1 = t * freqBins + (f + 1);
            int i2 = (t + 1) * freqBins + f;
            int i3 = (t + 1) * freqBins + (f + 1);
            
            // First triangle
            m_indices.append(i0);
            m_indices.append(i1);
            m_indices.append(i2);
            
            // Second triangle
            m_indices.append(i1);
            m_indices.append(i3);
            m_indices.append(i2);
        }
    }
    
    m_vertexCount = m_vertices.size() / 6;  // 6 floats per vertex (pos + color)
    m_indexCount = m_indices.size();
    
    // Update buffers
    m_vao->bind();
    
    m_vertexBuffer->bind();
    m_vertexBuffer->allocate(m_vertices.constData(), m_vertices.size() * sizeof(float));
    
    m_indexBuffer->bind();
    m_indexBuffer->allocate(m_indices.constData(), m_indices.size() * sizeof(unsigned int));
    
    // Setup vertex attributes
    m_shaderProgram->enableAttributeArray(0);  // position
    m_shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, 6 * sizeof(float));
    
    m_shaderProgram->enableAttributeArray(1);  // color
    m_shaderProgram->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 3, 6 * sizeof(float));
    
    m_vao->release();
    
    m_meshNeedsUpdate = false;
    
    qDebug() << "Mesh updated: " << m_vertexCount << " vertices, " << m_indexCount << " indices";
}

void QGL3DPanel::paintGL() {
    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Update camera
    updateCamera();
    
    // Update mesh if needed
    if (m_meshNeedsUpdate) {
        updateMesh();
    }
    
    // Render 3D spectrum
    renderSpectrum3D();
    
    // Render grid and axes
    if (m_showGrid) renderGrid();
    if (m_showAxes) renderAxes();
}

void QGL3DPanel::renderSpectrum3D() {
    if (!m_shaderProgram || m_vertexCount == 0) return;
    
    m_shaderProgram->bind();
    
    // Set uniforms
    QMatrix4x4 mvpMatrix = m_projectionMatrix * m_viewMatrix * m_modelMatrix;
    m_shaderProgram->setUniformValue("mvpMatrix", mvpMatrix);
    m_shaderProgram->setUniformValue("heightScale", m_heightScale);
    
    // Render mesh
    m_vao->bind();
    
    if (m_showWireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
    
    m_vao->release();
    m_shaderProgram->release();
}

void QGL3DPanel::renderGrid() {
    // Simple grid rendering using immediate mode (for proof of concept)
    glDisable(GL_DEPTH_TEST);
    
    QMatrix4x4 mvpMatrix = m_projectionMatrix * m_viewMatrix * m_modelMatrix;
    
    // This would be better implemented with shaders in a production version
    glBegin(GL_LINES);
    qglColor(m_gridColor);
    
    // Frequency grid lines
    for (int f = -256; f <= 256; f += 64) {
        glVertex3f(f, 0, -50);
        glVertex3f(f, 0, 50);
    }
    
    // Time grid lines  
    for (int t = -50; t <= 50; t += 10) {
        glVertex3f(-256, 0, t);
        glVertex3f(256, 0, t);
    }
    
    glEnd();
    
    glEnable(GL_DEPTH_TEST);
}

void QGL3DPanel::renderAxes() {
    glDisable(GL_DEPTH_TEST);
    glLineWidth(2.0f);
    
    glBegin(GL_LINES);
    
    // X axis (frequency) - Red
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-300, 0, 0);
    glVertex3f(300, 0, 0);
    
    // Y axis (amplitude) - Green
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 30, 0);
    
    // Z axis (time) - Blue
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0, 0, -60);
    glVertex3f(0, 0, 60);
    
    glEnd();
    
    glLineWidth(1.0f);
    glEnable(GL_DEPTH_TEST);
}

void QGL3DPanel::resizeGL(int width, int height) {
    m_aspectRatio = (float)width / (float)height;
    
    glViewport(0, 0, width, height);
    
    // Update projection matrix
    m_projectionMatrix.setToIdentity();
    m_projectionMatrix.perspective(45.0f, m_aspectRatio, 0.1f, 1000.0f);
}

void QGL3DPanel::updateCamera() {
    // Convert spherical coordinates to cartesian
    float radPitch = qDegreesToRadians(m_cameraPitch);
    float radYaw = qDegreesToRadians(m_cameraYaw);
    
    m_cameraPosition.setX(m_cameraDistance * cos(radPitch) * cos(radYaw));
    m_cameraPosition.setY(m_cameraDistance * sin(radPitch));
    m_cameraPosition.setZ(m_cameraDistance * cos(radPitch) * sin(radYaw));
    
    // Update view matrix
    m_viewMatrix.setToIdentity();
    m_viewMatrix.lookAt(m_cameraPosition, m_cameraTarget, m_cameraUp);
}

void QGL3DPanel::mousePressEvent(QMouseEvent *event) {
    m_lastMousePos = event->pos();
    m_mousePressed = true;
    m_mouseButton = event->button();
}

void QGL3DPanel::mouseMoveEvent(QMouseEvent *event) {
    if (!m_mousePressed) return;
    
    QPoint delta = event->pos() - m_lastMousePos;
    
    if (m_mouseButton == Qt::LeftButton) {
        // Rotate camera
        m_cameraYaw += delta.x() * 0.5f;
        m_cameraPitch += delta.y() * 0.5f;
        
        // Clamp pitch
        m_cameraPitch = qBound(-89.0f, m_cameraPitch, 89.0f);
        
        update();
    }
    
    m_lastMousePos = event->pos();
}

void QGL3DPanel::wheelEvent(QWheelEvent *event) {
    float delta = event->angleDelta().y() / 120.0f;
    m_cameraDistance -= delta * 2.0f;
    m_cameraDistance = qBound(5.0f, m_cameraDistance, 200.0f);
    
    update();
}

void QGL3DPanel::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_R:
        resetCamera();
        break;
    case Qt::Key_G:
        m_showGrid = !m_showGrid;
        break;
    case Qt::Key_A:
        m_showAxes = !m_showAxes;
        break;
    case Qt::Key_W:
        m_showWireframe = !m_showWireframe;
        break;
    case Qt::Key_Plus:
        m_heightScale *= 1.2f;
        break;
    case Qt::Key_Minus:
        m_heightScale /= 1.2f;
        break;
    }
    update();
}

void QGL3DPanel::resetCamera() {
    m_cameraDistance = 50.0f;
    m_cameraPitch = -30.0f;
    m_cameraYaw = 45.0f;
    update();
}

void QGL3DPanel::setSpectrumData(const QVector<float>& spectrumData) {
    m_currentSpectrum = spectrumData;
    m_spectrumWidth = spectrumData.size();
    
    // Add to history
    m_spectrumHistory.prepend(spectrumData);
    if (m_spectrumHistory.size() > MAX_TIME_SLICES) {
        m_spectrumHistory.removeLast();
    }
    
    m_meshNeedsUpdate = true;
}

QColor QGL3DPanel::amplitudeToColor(float amplitude) {
    // Simple height-to-color mapping
    float normalized = qBound(0.0f, amplitude / 20.0f, 1.0f);  // Assuming max amplitude ~20
    
    if (normalized < 0.25f) {
        // Blue to cyan
        float t = normalized * 4.0f;
        return QColor::fromRgbF(0, t, 1.0f);
    } else if (normalized < 0.5f) {
        // Cyan to green
        float t = (normalized - 0.25f) * 4.0f;
        return QColor::fromRgbF(0, 1.0f, 1.0f - t);
    } else if (normalized < 0.75f) {
        // Green to yellow
        float t = (normalized - 0.5f) * 4.0f;
        return QColor::fromRgbF(t, 1.0f, 0);
    } else {
        // Yellow to red
        float t = (normalized - 0.75f) * 4.0f;
        return QColor::fromRgbF(1.0f, 1.0f - t, 0);
    }
}

void QGL3DPanel::qglColor(QColor color) {
    glColor4f(color.redF(), color.greenF(), color.blueF(), color.alphaF());
}

void QGL3DPanel::onUpdateTimer() {
    // Trigger a repaint
    update();
}

// Slots
void QGL3DPanel::spectrumDataChanged(const QVector<float>& data) {
    setSpectrumData(data);
}

void QGL3DPanel::updateDisplay() {
    update();
}