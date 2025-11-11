/**
* @file  cusdr_ogl3DPanel.cpp
* @brief 3D Panadapter panel implementation for cuSDR
* @author Simon Eatough, ZL2BRG (based on cuSDR framework by Hermann von Hasseln, DL3HVH)
* @version 0.1
* @date 2025-10-12
*/

#include "cusdr_ogl3DPanel.h"
#include <QDebug>
#include <QtMath>
#include <QDateTime>

#ifndef M_PI_2
#define M_PI_2 (M_PI / 2.0)
#endif

// Static constants definition
const int QGL3DPanel::MAX_TIME_SLICES;
const int QGL3DPanel::MAX_FREQ_BINS;

QGL3DPanel::QGL3DPanel(QWidget *parent, int rx)
    : QOpenGLWidget(parent)
    , set(Settings::instance())
    , m_receiver(rx)
    , m_centerFrequency(set->getCtrFrequency(m_receiver))
    , m_vfoFrequency(set->getVfoFrequency(m_receiver))
    , m_sampleRate(set->getSampleRate())
    , m_shaderProgram(nullptr)
    , m_gridVertexBuffer(nullptr)
    , m_gridVAO(nullptr)
    , m_gridVertexCount(0)
    , m_cameraDistance(300.0f)  // Zoomed out to show full spectrum
    , m_cameraPitch(30.0f)      // Elevated view from above (positive pitch)
    , m_cameraYaw(45.0f)        // Angled view for better perspective
    , m_cameraOffsetX(0.0f)     // Centered horizontally
    , m_cameraOffsetY(10.0f)    // Slightly elevated center point
    , m_mousePressed(false)
    , m_spectrumWidth(512)
    , m_currentTimeSlice(0)
    , m_newSliceAvailable(false)
    , m_heightScale(10.0f)
    , m_frequencyScale(1.0f)
    , m_timeScale(1.0f)
    , m_waterfallOffset(0.0f)   // Default offset
    , m_backgroundColor(Qt::black)
    , m_gridColor(Qt::darkGray)
    , m_axesColor(Qt::white)
    , m_updateInterval(50)  // 20 FPS
    , m_oglTextSmall(nullptr)
    , m_showGrid(true)
    , m_showAxes(true)
    , m_showWireframe(false)
    , m_aspectRatio(1.0f)
    , m_updateTimer(nullptr)
    , m_frameTimer(nullptr)
    , m_lastUpdateTime(0)
    , m_lastFrameTime(0)
    , m_frameCount(0)
    , m_currentFPS(0.0f)
    , m_targetFPS(30)
    , m_updateFrequencyMs(33)  // ~30 Hz (30 FPS) spectrum updates
    , m_isVisible(true)
    , m_dataUpdateCount(0)
    , m_meshUpdateCount(0)
    , m_lastDebugTime(0)
    , m_meshWorker(nullptr)
{
    // Initialize camera target and up vector
    m_cameraTarget = QVector3D(0, 0, 0);
    m_cameraUp = QVector3D(0, 1, 0);
    
    // Create and start mesh generation worker thread with high priority
    m_meshWorker = new MeshGeneratorWorker(this);
    connect(m_meshWorker, &MeshGeneratorWorker::meshReady, 
            this, &QGL3DPanel::onMeshReady, Qt::QueuedConnection);
    m_meshWorker->start(QThread::HighPriority);  // Run at higher priority for responsive mesh updates

    // Get dBm scale from receiver panadapter settings (same as 2D panel)
    QList<TReceiver> m_rxDataList = set->getReceiverDataList();
    HamBand band = m_rxDataList.at(m_receiver).hamBand;
    m_dBmPanMin = m_rxDataList.at(m_receiver).dBmPanScaleMinList.at(band);
    m_dBmPanMax = m_rxDataList.at(m_receiver).dBmPanScaleMaxList.at(band);

    // Connect to panadapter scale change signals
    connect(set, &Settings::dBmScaleMaxChanged,
        this, &QGL3DPanel::dBmScaleMaxChanged);
    connect(set, &Settings::dBmScaleMinChanged,
        this, &QGL3DPanel::dBmScaleMinChanged);


    // Initialize spectrum history
    m_spectrumHistory.reserve(MAX_TIME_SLICES);
    m_currentSpectrum.resize(m_spectrumWidth);
    
    // Initialize camera position from spherical coordinates
    updateCamera();

    // Try to bypass Qt's synchronized update throttling for docked widgets with shared contexts
    // Set the widget to not share its context even though AA_ShareOpenGLContexts is set
    // This may help with refresh rate while still allowing reparenting
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setSwapInterval(0);  // Disable VSync for this widget specifically
    setFormat(format);
    
    // Match the 2D receiver panel settings exactly
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setUpdateBehavior(QOpenGLWidget::PartialUpdate);
    setAutoFillBackground(false);

    // Don't use a timer - make it fully data-driven like the 2D receiver panel
    // The 2D panel doesn't use any recurring timer, only updates when data arrives
    // This prevents Qt from throttling updates with shared OpenGL contexts
    
    // Setup simple FPS monitoring
    m_frameTimer = new QTimer(this);
    connect(m_frameTimer, &QTimer::timeout, this, &QGL3DPanel::calculateFPS);
    m_frameTimer->start(1000); // Calculate FPS every second
    
    m_lastUpdateTime = QDateTime::currentMSecsSinceEpoch();

    // Enable mouse tracking
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

QGL3DPanel::~QGL3DPanel() {
    // Stop worker thread first
    if (m_meshWorker) {
        m_meshWorker->stop();
        m_meshWorker->wait();
        delete m_meshWorker;
    }
    
    makeCurrent();
    
    // Clean up all time slice meshes
    for (TimeSliceMesh& slice : m_timeSliceMeshes) {
        if (slice.vao) {
            slice.vao->destroy();
            delete slice.vao;
        }
        if (slice.vertexBuffer) {
            slice.vertexBuffer->destroy();
            delete slice.vertexBuffer;
        }
        if (slice.indexBuffer) {
            slice.indexBuffer->destroy();
            delete slice.indexBuffer;
        }
    }
    m_timeSliceMeshes.clear();
    
    // Clean up grid resources
    if (m_gridVAO) {
        m_gridVAO->destroy();
        delete m_gridVAO;
    }
    
    if (m_gridVertexBuffer) {
        m_gridVertexBuffer->destroy();
        delete m_gridVertexBuffer;
    }
    
    delete m_shaderProgram;
    delete m_oglTextSmall;
    
    doneCurrent();
}

void QGL3DPanel::dBmScaleMaxChanged(int rx, qreal val){
    if (rx != m_receiver) return;  // Only update if it's for this receiver
    m_dBmPanMax = val;
    update();  // Trigger redraw with new scale
}

void QGL3DPanel::dBmScaleMinChanged(int rx, qreal val){
    if (rx != m_receiver) return;  // Only update if it's for this receiver
    m_dBmPanMin = val;  // Fixed: was incorrectly setting m_dBmPanMax
    update();  // Trigger redraw with new scale
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
    
    // Setup grid rendering
    setupGrid();
    
    // Initialize text rendering
    QFont smallFont = QFont("Arial", 8);
    qreal dpr = devicePixelRatio();
    m_oglTextSmall = new OGLText(smallFont, dpr);
    
    // Initialize camera view matrix with our spherical coordinates
    updateCamera();
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
}

void QGL3DPanel::setupMesh() {
    // Per-slice rendering - meshes created dynamically as new spectra arrive
    // No initial setup needed
}

void QGL3DPanel::setupGrid() {
    // Create grid VAO and vertex buffer
    m_gridVAO = new QOpenGLVertexArrayObject();
    m_gridVAO->create();
    m_gridVAO->bind();
    
    m_gridVertexBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_gridVertexBuffer->create();
    m_gridVertexBuffer->bind();
    
    // Generate grid vertices
    QVector<float> gridVertices;
    
    // Frequency grid lines (X-axis lines at different Z positions)
    for (int f = -200; f <= 200; f += 40) {  // Every 40 units for cleaner look
        // Vertical line through Z-axis - make more visible at higher Y
        gridVertices << f << 5.0f << -100.0f << 0.6f << 0.6f << 0.6f;  // Position + Gray color, elevated
        gridVertices << f << 5.0f << 100.0f << 0.6f << 0.6f << 0.6f;
    }
    
    // Time grid lines (Z-axis lines at different X positions)  
    for (int t = -100; t <= 100; t += 20) {  // Every 20 units
        // Horizontal line through X-axis - make more visible at higher Y
        gridVertices << -200.0f << 5.0f << t << 0.6f << 0.6f << 0.6f;  // Position + Gray color, elevated
        gridVertices << 200.0f << 5.0f << t << 0.6f << 0.6f << 0.6f;
    }
    
    // Vertical amplitude grid lines on the sides - extend to full spectrum height
    // Create vertical lines from bottom to well above the spectrum (0 to 60)
    for (int i = 0; i <= 8; i++) {  // 9 lines total (every 25 units from -100 to +100)
        float zPos = -100.0f + (i * 25.0f);  // Z positions: -100, -75, -50, -25, 0, 25, 50, 75, 100
        
        // Left side vertical lines (at X = -200) - full height to cover all spectrum
        gridVertices << -200.0f << 0.0f << zPos << 0.75f << 0.75f << 0.75f;  // Light grey, bottom
        gridVertices << -200.0f << 60.0f << zPos << 0.75f << 0.75f << 0.75f; // Light grey, well above spectrum
        
        // Right side vertical lines (at X = +200) - full height to cover all spectrum
        gridVertices << 200.0f << 0.0f << zPos << 0.75f << 0.75f << 0.75f;   // Light grey, bottom
        gridVertices << 200.0f << 60.0f << zPos << 0.75f << 0.75f << 0.75f;  // Light grey, well above spectrum
    }
    
    // Horizontal dBm reference lines on the sides (every 10 dBm)
    // These show the amplitude scale: -120, -110, -100, -90, -80, -70, -60, -50, -40 dBm
    for (int dbm = -120; dbm <= -40; dbm += 10) {  // 10 dBm steps
        float height = (dbm + 120.0f) / 80.0f * 40.0f;  // Convert dBm to visual height (0-40 range)
        
        // Left side horizontal dBm lines (at X = -200) - span front to back at specific height
        gridVertices << -200.0f << height << -100.0f << 0.9f << 0.9f << 0.9f;  // Very light grey for dBm marks
        gridVertices << -200.0f << height << 100.0f << 0.9f << 0.9f << 0.9f;
        
        // Right side horizontal dBm lines (at X = +200) - span front to back at specific height
        gridVertices << 200.0f << height << -100.0f << 0.9f << 0.9f << 0.9f;   // Very light grey for dBm marks
        gridVertices << 200.0f << height << 100.0f << 0.9f << 0.9f << 0.9f;
    }
    
    m_gridVertexCount = gridVertices.size() / 6;  // 6 values per vertex (pos + color)
    
    // Upload grid data
    m_gridVertexBuffer->allocate(gridVertices.data(), gridVertices.size() * sizeof(float));
    
    // Setup vertex attributes for grid
    glEnableVertexAttribArray(0);  // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
    
    glEnableVertexAttribArray(1);  // Color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    
    m_gridVertexBuffer->release();
    m_gridVAO->release();
}

void QGL3DPanel::updateMesh() {
    if (!m_newSliceAvailable || m_spectrumHistory.isEmpty() || !m_meshWorker) return;
    
    // Skip if worker is still busy processing previous mesh
    if (m_meshWorker->isBusy()) {
        return;
    }
    
    // Calculate LOD level based on camera distance
    int lodLevel = calculateLODLevel();
    
    // Get the newest spectrum slice (index 0 is newest)
    const QVector<float>& newestSlice = m_spectrumHistory.first();
    
    // Request mesh generation for just this one slice
    m_meshWorker->generateSingleSliceMesh(newestSlice,
                                          m_timeSliceMeshes.size(),  // Time index
                                          m_spectrumWidth,
                                          lodLevel,
                                          m_heightScale,
                                          m_frequencyScale,
                                          m_timeScale,
                                          m_waterfallOffset,
                                          m_dBmPanMin,
                                          m_dBmPanMax);
    
    m_newSliceAvailable = false;
}

void QGL3DPanel::onMeshReady(const MeshGeneratorWorker::MeshData& meshData) {
    // This slot is called on the main thread when mesh generation is complete
    
    // Clean up old slices BEFORE adding new one if we're at capacity
    if (m_timeSliceMeshes.size() >= MAX_TIME_SLICES) {
        cleanupOldSlices();
    }
    
    // Add the new slice to our collection
    addNewSliceMesh(meshData);
    
    // Trigger repaint with new mesh
    update();
}

void QGL3DPanel::paintGL() {
    // Frame counting for FPS calculation
    m_frameCount++;
    
    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Update camera matrix (this should always work for mouse interaction)
    updateCamera();
    
    // Update mesh when new slice available
    if (m_newSliceAvailable) {
        updateMesh();
    }
    
    // Render 3D spectrum
    renderSpectrum3D();
    
    // Render grid and axes
    if (m_showGrid) renderGrid();
    if (m_showAxes) renderAxes();
    
    // Render vertical dBm scale
    renderVerticalScale();
}

void QGL3DPanel::renderSpectrum3D() {
    if (!m_shaderProgram || m_timeSliceMeshes.isEmpty()) return;
    
    // Bind shader program once
    m_shaderProgram->bind();
    
    // Set base uniforms
    m_shaderProgram->setUniformValue("heightScale", m_heightScale);
    
    // Set polygon mode
    if (m_showWireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    // Render each time slice with individual translation
    // Newer slices (index 0) are at front, older slices recede backward
    // Center the entire waterfall display around Z=0
    int numSlices = m_timeSliceMeshes.size();
    float totalDepth = (numSlices - 1) * 4.0f * m_timeScale;
    float centerOffset = totalDepth / 2.0f;  // Offset to center around Z=0
    
    for (int i = 0; i < numSlices; i++) {
        // Simple visibility culling - skip slices that are definitely off-screen
        if (!isSliceVisible(i)) {
            continue;
        }
        
        const TimeSliceMesh& slice = m_timeSliceMeshes[i];
        
        // Create model matrix with Z offset for this slice
        // Index 0 is newest (at centerOffset), higher indices recede backward
        QMatrix4x4 sliceModel = m_modelMatrix;
        float zOffset = centerOffset - (float)i * 4.0f * m_timeScale;
        sliceModel.translate(0.0f, 0.0f, zOffset);
        
        // Set MVP for this slice
        QMatrix4x4 mvpMatrix = m_projectionMatrix * m_viewMatrix * sliceModel;
        m_shaderProgram->setUniformValue("mvpMatrix", mvpMatrix);
        
        // Bind this slice's VAO and render
        slice.vao->bind();
        glDrawElements(GL_TRIANGLES, slice.indexCount, GL_UNSIGNED_INT, 0);
        slice.vao->release();
    }
    
    // Reset polygon mode
    if (m_showWireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    m_shaderProgram->release();
}

void QGL3DPanel::renderGrid() {
    if (!m_gridVAO || !m_shaderProgram) return;
    
    // Save current height scale
    float currentHeightScale = m_heightScale;
    
    // Use the same shader program as the spectrum
    m_shaderProgram->bind();
    
    // Set matrices
    m_shaderProgram->setUniformValue("mvpMatrix", m_projectionMatrix * m_viewMatrix * m_modelMatrix);
    m_shaderProgram->setUniformValue("heightScale", 1.0f);  // No height scaling for grid
    
    // Bind grid VAO and render as lines
    m_gridVAO->bind();
    
    // Disable depth test briefly for grid lines to show through
    glDisable(GL_DEPTH_TEST);
    glLineWidth(1.0f);
    
    glDrawArrays(GL_LINES, 0, m_gridVertexCount);
    
    glEnable(GL_DEPTH_TEST);
    
    m_gridVAO->release();
    
    // Restore height scale for subsequent rendering
    m_shaderProgram->setUniformValue("heightScale", currentHeightScale);
    
    m_shaderProgram->release();
}

void QGL3DPanel::renderAxes() {
    glDisable(GL_DEPTH_TEST);
    glLineWidth(2.0f);
    
    glBegin(GL_LINES);
    
    // X axis (frequency) - Red
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-250, 0, 0);
    glVertex3f(250, 0, 0);
    
    // Y axis (amplitude) - Green
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 60, 0);
    
    // Z axis (time) - Blue
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0, 0, -60);
    glVertex3f(0, 0, 60);
    
    glEnd();
    
    glLineWidth(1.0f);
    glEnable(GL_DEPTH_TEST);
}

void QGL3DPanel::renderVerticalScale() {
    if (!m_oglTextSmall) return;
    
    // Create scale lines using VAO/VBO like the grid does
    // Position the scale at the front left of the 3D scene
    // Scale positioning should respond to time scale changes but stay reasonable
    float scaleX = -200.0f * m_frequencyScale;  // Left side, scaled with frequency
    // Move closer to front - increase Z value to get to front corner
    float scaleZ = 150.0f * m_timeScale;  // Higher value to move toward front corner
    
    // Use dynamic dBm range from 2D panadapter for scale labels
    // Even though mesh height is fixed, scale labels reflect current dBm range
    float minDb = m_dBmPanMin;
    float maxDb = m_dBmPanMax;
    float dbRange = maxDb - minDb;
    float scaleHeight = 40.0f * m_heightScale;  // Match mesh height (40 units base)
    
    // Create vertices for the scale lines
    QVector<float> scaleVertices;
    
    // Main vertical line (position + color)
    scaleVertices << scaleX << 0.0f << scaleZ << 0.9f << 0.9f << 0.9f;  // Bottom
    scaleVertices << scaleX << scaleHeight << scaleZ << 0.9f << 0.9f << 0.9f;  // Top
    
    // Tick marks every 10 dB
    // Note: maxDb (e.g., -40) is GREATER than minDb (e.g., -140), so iterate downwards
    for (int db = (int)maxDb; db >= (int)minDb; db -= 10) {
        float normalizedPos = (float)(db - minDb) / dbRange;
        float yPos = normalizedPos * scaleHeight;
        
        // Major tick every 20 dB
        float tickLength = (db % 20 == 0) ? 15.0f : 8.0f;
        
        // Tick mark line
        scaleVertices << scaleX << yPos << scaleZ << 0.9f << 0.9f << 0.9f;
        scaleVertices << (scaleX + tickLength) << yPos << scaleZ << 0.9f << 0.9f << 0.9f;
    }
    
    // Render using OpenGL similar to how grid is rendered
    if (!scaleVertices.isEmpty() && m_shaderProgram) {
        // Use the same shader program as the spectrum
        m_shaderProgram->bind();
        
        // Set matrices
        m_shaderProgram->setUniformValue("mvpMatrix", m_projectionMatrix * m_viewMatrix * m_modelMatrix);
        m_shaderProgram->setUniformValue("heightScale", 1.0f);  // No height scaling for scale
        
        // Create temporary VAO and VBO for the scale
        QOpenGLVertexArrayObject scaleVAO;
        scaleVAO.create();
        scaleVAO.bind();
        
        QOpenGLBuffer scaleVBO(QOpenGLBuffer::VertexBuffer);
        scaleVBO.create();
        scaleVBO.bind();
        scaleVBO.allocate(scaleVertices.constData(), scaleVertices.size() * sizeof(float));
        
        // Setup vertex attributes (position + color)
        glEnableVertexAttribArray(0);  // Position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
        
        glEnableVertexAttribArray(1);  // Color
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        
        // Disable depth test and render as lines
        glDisable(GL_DEPTH_TEST);
        glLineWidth(2.0f);
        
        // Draw the scale lines
        int vertexCount = scaleVertices.size() / 6;  // 6 floats per vertex (pos + color)
        glDrawArrays(GL_LINES, 0, vertexCount);
        
        // Restore state
        glEnable(GL_DEPTH_TEST);
        glLineWidth(1.0f);
        
        scaleVBO.release();
        scaleVAO.release();
        
        m_shaderProgram->release();
    }
    
    // Render text labels using 2D overlay (this part is working)
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width(), height(), 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Calculate MVP matrix for 3D to screen conversion
    QMatrix4x4 mvpMatrix = m_projectionMatrix * m_viewMatrix * m_modelMatrix;
    
    // Render labels for major ticks only
    // Note: m_dBmPanMax (e.g., -40) is GREATER than m_dBmPanMin (e.g., -140)
    // So we need to iterate from max down to min (stepping by -20)
    for (int db = (int)m_dBmPanMax; db >= (int)m_dBmPanMin; db -= 20) {
        float normalizedPos = (float)(db - minDb) / dbRange;
        float yPos = normalizedPos * scaleHeight;
        
        // 3D position of label - now scales with frequency scale
        QVector3D worldPos(scaleX - 25.0f * m_frequencyScale, yPos, scaleZ);
        
        // Convert to screen coordinates
        QVector4D clipPos = mvpMatrix * QVector4D(worldPos, 1.0f);
        if (clipPos.w() > 0.001f) {
            QVector3D ndcPos = clipPos.toVector3D() / clipPos.w();
            
            float screenX = (ndcPos.x() + 1.0f) * 0.5f * width();
            float screenY = (1.0f - ndcPos.y()) * 0.5f * height();
            
            // Only render if on screen
            if (screenX >= -100 && screenX < width() + 100 && screenY >= -100 && screenY < height() + 100) {
                QString label = QString("%1").arg(db);
                glColor3f(1.0f, 1.0f, 1.0f);
                m_oglTextSmall->renderText(screenX, screenY, 1.0f, label);
            }
        }
    }
    
    // Add "dBm" label at the top - now scales with frequency scale
    QVector3D topWorldPos(scaleX - 20.0f * m_frequencyScale, scaleHeight + 10.0f * m_heightScale, scaleZ);
    QVector4D topClipPos = mvpMatrix * QVector4D(topWorldPos, 1.0f);
    if (topClipPos.w() > 0.001f) {
        QVector3D topNdcPos = topClipPos.toVector3D() / topClipPos.w();
        float topScreenX = (topNdcPos.x() + 1.0f) * 0.5f * width();
        float topScreenY = (1.0f - topNdcPos.y()) * 0.5f * height();
        
        if (topScreenX >= -100 && topScreenX < width() + 100 && topScreenY >= -100 && topScreenY < height() + 100) {
            glColor3f(1.0f, 1.0f, 1.0f);
            m_oglTextSmall->renderText(topScreenX, topScreenY, 1.0f, "dBm");
        }
    }
    
    // Restore matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void QGL3DPanel::resizeGL(int width, int height) {
    m_aspectRatio = (float)width / (float)height;
    
    glViewport(0, 0, width, height);
    
    // Update projection matrix
    m_projectionMatrix.setToIdentity();
    m_projectionMatrix.perspective(45.0f, m_aspectRatio, 0.1f, 1000.0f);
}

void QGL3DPanel::updateCamera() {
    // Convert degrees to radians
    float radPitch = qDegreesToRadians(m_cameraPitch);
    float radYaw = qDegreesToRadians(m_cameraYaw);
    
    // Calculate camera position using proper spherical coordinates
    float cosYaw = cos(radYaw);
    float sinYaw = sin(radYaw);
    float cosPitch = cos(radPitch);
    float sinPitch = sin(radPitch);
    
    // Position camera using spherical coordinates
    m_cameraPosition.setX(m_cameraDistance * cosPitch * cosYaw + m_cameraOffsetX);
    m_cameraPosition.setY(m_cameraDistance * sinPitch + m_cameraOffsetY);
    m_cameraPosition.setZ(m_cameraDistance * cosPitch * sinYaw);
    
    // Look at the center of the spectrum (with offsets)
    m_cameraTarget = QVector3D(m_cameraOffsetX, 20.0f + m_cameraOffsetY, 0.0f);
    
    // Update view matrix
    m_viewMatrix.setToIdentity();
    m_viewMatrix.lookAt(m_cameraPosition, m_cameraTarget, m_cameraUp);
}

void QGL3DPanel::calculateVisibleRange(int& minTimeSlice, int& maxTimeSlice, int& minFreqBin, int& maxFreqBin) {
    // Calculate view-projection matrix
    QMatrix4x4 mvpMatrix = m_projectionMatrix * m_viewMatrix * m_modelMatrix;
    
    int totalTimeSlices = qMin(m_spectrumHistory.size(), MAX_TIME_SLICES);
    
    // Mesh bounds in world space:
    // X: -200*freqScale to +200*freqScale
    // Z: -totalTimeSlices*2*timeScale to +totalTimeSlices*2*timeScale
    
    // Instead of trying to unproject the frustum, we'll test a grid of points
    // on the mesh and see which ones are visible
    
    // Sample grid resolution for testing visibility
    const int testSamplesTime = 10;
    const int testSamplesFreq = 10;
    
    minTimeSlice = totalTimeSlices;
    maxTimeSlice = 0;
    minFreqBin = m_spectrumWidth;
    maxFreqBin = 0;
    
    bool anyVisible = false;
    
    // Test points across the mesh at Y=0 (the base plane)
    for (int ts = 0; ts < testSamplesTime; ts++) {
        for (int fs = 0; fs < testSamplesFreq; fs++) {
            // Calculate world position for this test point
            float t = (float)ts / (testSamplesTime - 1) * totalTimeSlices;
            float f = (float)fs / (testSamplesFreq - 1) * m_spectrumWidth;
            
            float x = (f / (float)m_spectrumWidth) * 400.0f * m_frequencyScale - 200.0f * m_frequencyScale;
            float y = 0.0f; // Test at base of mesh
            float z = ((float)totalTimeSlices - t) * 4.0f * m_timeScale - (float)totalTimeSlices * 2.0f * m_timeScale;
            
            // Transform to clip space
            QVector4D worldPos(x, y, z, 1.0f);
            QVector4D clipPos = mvpMatrix * worldPos;
            
            // Perspective divide to get NDC
            if (clipPos.w() > 0.0001f) {
                float ndcX = clipPos.x() / clipPos.w();
                float ndcY = clipPos.y() / clipPos.w();
                float ndcZ = clipPos.z() / clipPos.w();
                
                // Check if in frustum (with margin)
                const float margin = 0.3f; // 30% margin outside screen
                if (ndcX >= -1.0f - margin && ndcX <= 1.0f + margin &&
                    ndcY >= -1.0f - margin && ndcY <= 1.0f + margin &&
                    ndcZ >= -1.0f && ndcZ <= 1.0f) {
                    
                    anyVisible = true;
                    int timeSlice = (int)t;
                    int freqBin = (int)f;
                    
                    minTimeSlice = qMin(minTimeSlice, timeSlice);
                    maxTimeSlice = qMax(maxTimeSlice, timeSlice + 1);
                    minFreqBin = qMin(minFreqBin, freqBin);
                    maxFreqBin = qMax(maxFreqBin, freqBin + 1);
                }
            }
        }
    }
    
    if (!anyVisible) {
        // Nothing visible in our samples, use full range as fallback
        minTimeSlice = 0;
        maxTimeSlice = totalTimeSlices;
        minFreqBin = 0;
        maxFreqBin = m_spectrumWidth;
        return;
    }
    
    // Expand range slightly to avoid edge artifacts
    int timeMargin = qMax(1, (maxTimeSlice - minTimeSlice) / 5);
    int freqMargin = qMax(1, (maxFreqBin - minFreqBin) / 5);
    
    minTimeSlice = qMax(0, minTimeSlice - timeMargin);
    maxTimeSlice = qMin(totalTimeSlices, maxTimeSlice + timeMargin);
    minFreqBin = qMax(0, minFreqBin - freqMargin);
    maxFreqBin = qMin(m_spectrumWidth, maxFreqBin + freqMargin);
}

void QGL3DPanel::mousePressEvent(QMouseEvent *event) {
    m_lastMousePos = event->pos();
    m_mousePressed = true;
    m_mouseButton = event->button();
    setFocus(); // Ensure widget has focus for keyboard events
}

void QGL3DPanel::mouseMoveEvent(QMouseEvent *event) {
    if (!m_mousePressed) return;
    
    QPoint delta = event->pos() - m_lastMousePos;
    float sensitivity = 0.5f;
    float panSensitivity = 0.2f;
    
    if (m_mouseButton == Qt::LeftButton) {
        // Rotate camera
        m_cameraYaw += delta.x() * sensitivity;
        m_cameraPitch += delta.y() * sensitivity;
        
        // Allow nearly full range for top-down and bottom-up views
        m_cameraPitch = qBound(-89.9f, m_cameraPitch, 89.9f);
        
        updateCamera();
        update();
    }
    else if (m_mouseButton == Qt::RightButton) {
        // Pan camera (horizontal and vertical movement)
        float panX = delta.x() * panSensitivity;
        float panY = -delta.y() * panSensitivity; // Invert Y for natural feel
        
        // Apply pan relative to camera orientation
        float radYaw = qDegreesToRadians(m_cameraYaw);
        m_cameraOffsetX += panX * cos(radYaw + M_PI_2); // Perpendicular to view direction
        m_cameraOffsetY += panY;
        
        updateCamera();
        update();
    }
    else if (m_mouseButton == Qt::MiddleButton) {
        // Vertical movement only
        m_cameraOffsetY -= delta.y() * panSensitivity;
        
        updateCamera();
        update();
    }
    
    m_lastMousePos = event->pos();
}

void QGL3DPanel::wheelEvent(QWheelEvent *event) {
    float delta = event->angleDelta().y() / 120.0f; // Normalize wheel delta
    float zoomFactor = 1.1f;
    
    if (delta > 0) {
        // Zoom in
        m_cameraDistance /= zoomFactor;
    } else {
        // Zoom out  
        m_cameraDistance *= zoomFactor;
    }
    
    // Clamp zoom distance to reasonable limits (increased max zoom out by 25%)
    m_cameraDistance = qBound(10.0f, m_cameraDistance, 625.0f);
    
    updateCamera();
    update();
    event->accept();
}

void QGL3DPanel::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event)
    m_mousePressed = false;
}

void QGL3DPanel::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_R:
        resetCamera();
        break;
    case Qt::Key_T:
        // Top-down view
        m_cameraPitch = -85.0f;  // Nearly straight down
        m_cameraYaw = 0.0f;      // Face forward
        m_cameraDistance = 150.0f; // Good overview distance
        m_cameraOffsetX = 0.0f;
        m_cameraOffsetY = 0.0f;
        updateCamera();
        break;
    case Qt::Key_O:
        // Overview - nice angled perspective showing full spectrum
        m_cameraDistance = 300.0f;  // Zoomed out
        m_cameraPitch = 30.0f;      // Elevated view from above (positive pitch)
        m_cameraYaw = 45.0f;        // Angled view for perspective
        m_cameraOffsetX = 0.0f;     // Centered
        m_cameraOffsetY = 10.0f;    // Slightly elevated center
        updateCamera();
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
    case Qt::Key_Equal:
        m_heightScale *= 1.2f;
        break;
    case Qt::Key_Minus:
        m_heightScale /= 1.2f;
        break;
    // Camera movement with arrow keys
    case Qt::Key_Up:
        if (event->modifiers() & Qt::ShiftModifier) {
            // Shift+Up: Move camera position up
            m_cameraOffsetY += 2.0f;
        } else {
            // Up: Tilt camera up (decrease pitch)
            m_cameraPitch = qMax(-89.9f, m_cameraPitch - 5.0f);
        }
        updateCamera();
        break;
    case Qt::Key_Down:
        if (event->modifiers() & Qt::ShiftModifier) {
            // Shift+Down: Move camera position down
            m_cameraOffsetY -= 2.0f;
        } else {
            // Down: Tilt camera down (increase pitch)
            m_cameraPitch = qMin(89.9f, m_cameraPitch + 5.0f);
        }
        updateCamera();
        break;
    case Qt::Key_Left:
        if (event->modifiers() & Qt::ShiftModifier) {
            // Shift+Left: Move camera position left
            m_cameraOffsetX -= 2.0f;
        } else {
            // Left: Rotate camera left
            m_cameraYaw -= 5.0f;
        }
        updateCamera();
        break;
    case Qt::Key_Right:
        if (event->modifiers() & Qt::ShiftModifier) {
            // Shift+Right: Move camera position right
            m_cameraOffsetX += 2.0f;
        } else {
            // Right: Rotate camera right
            m_cameraYaw += 5.0f;
        }
        updateCamera();
        break;
    // Zoom with Page Up/Down
    case Qt::Key_PageUp:
        m_cameraDistance /= 1.1f;
        m_cameraDistance = qMax(10.0f, m_cameraDistance);
        updateCamera();
        break;
    case Qt::Key_PageDown:
        m_cameraDistance *= 1.1f;
        m_cameraDistance = qMin(500.0f, m_cameraDistance);
        updateCamera();
        break;
    }
    update();
}

void QGL3DPanel::resetCamera() {
    m_cameraDistance = 300.0f;  // Zoomed out to show full spectrum
    m_cameraPitch = 30.0f;      // Elevated view from above (positive pitch)
    m_cameraYaw = 45.0f;        // Angled view for better perspective
    m_cameraOffsetX = 0.0f;     // Centered horizontally
    m_cameraOffsetY = 10.0f;    // Slightly elevated center point
    updateCamera();
    update();
}

void QGL3DPanel::setSpectrumData(const QVector<float>& spectrumData) {
    // Always accept and store the data immediately
    m_currentSpectrum = spectrumData;
    m_spectrumWidth = spectrumData.size();
    m_dataUpdateCount++;
    
    // Add incoming spectrum to history
    m_spectrumHistory.prepend(spectrumData);
    if (m_spectrumHistory.size() > MAX_TIME_SLICES) {
        m_spectrumHistory.removeLast();
    }
    
    // Throttle mesh generation for new slices
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    if (currentTime - m_lastUpdateTime >= m_updateFrequencyMs) {
        m_newSliceAvailable = true;
        m_lastUpdateTime = currentTime;
        m_meshUpdateCount++;
    }
    
    // Update display immediately (rendering is fast now)
    if (m_isVisible) {
        update();
    }
}

QColor QGL3DPanel::amplitudeToColor(float amplitude) {
    // Use the same Enhanced waterfall color algorithm as the RX panel
    // This matches the getWaterfallColorAtPixel() Enhanced mode
    
    // Adjust the dB range and apply user offset for color mapping
    float lowerThreshold = -160.0f + m_waterfallOffset;  // User-adjustable minimum color level
    float upperThreshold = -80.0f + m_waterfallOffset;   // Upper end adjusts with offset
    
    QColor color;
    int r, g, b;
    
    if (amplitude <= lowerThreshold) {
        // Use black/dark blue for lowest signals
        color = QColor(0, 0, 20);
    }
    else if (amplitude >= upperThreshold) {
        // White for highest signals  
        color = QColor(255, 255, 255);
    }
    else {
        // Enhanced waterfall color algorithm - exact copy from RX panel
        float offset = amplitude - lowerThreshold;
        float globalRange = offset / (upperThreshold - lowerThreshold);
        
        if (globalRange < 2.0f/9.0f) { // background to blue
            float localRange = globalRange / (2.0f/9.0f);
            r = (int)((1.0f - localRange) * 0);
            g = (int)((1.0f - localRange) * 0);
            b = (int)(20 + localRange * (255 - 20));
        }
        else if (globalRange < 3.0f/9.0f) { // blue to blue-green
            float localRange = (globalRange - 2.0f/9.0f) / (1.0f/9.0f);
            r = 0;
            g = (int)(localRange * 255);
            b = 255;
        }
        else if (globalRange < 4.0f/9.0f) { // blue-green to green
            float localRange = (globalRange - 3.0f/9.0f) / (1.0f/9.0f);
            r = 0;
            g = 255;
            b = (int)((1.0f - localRange) * 255);
        }
        else if (globalRange < 5.0f/9.0f) { // green to red-green
            float localRange = (globalRange - 4.0f/9.0f) / (1.0f/9.0f);
            r = (int)(localRange * 255);
            g = 255;
            b = 0;
        }
        else if (globalRange < 7.0f/9.0f) { // red-green to red
            float localRange = (globalRange - 5.0f/9.0f) / (2.0f/9.0f);
            r = 255;
            g = (int)((1.0f - localRange) * 255);
            b = 0;
        }
        else if (globalRange < 8.0f/9.0f) { // red to red-blue
            float localRange = (globalRange - 7.0f/9.0f) / (1.0f/9.0f);
            r = 255;
            g = 0;
            b = (int)(localRange * 255);
        }
        else { // red-blue to purple end
            float localRange = (globalRange - 8.0f/9.0f) / (1.0f/9.0f);
            r = (int)((0.75f + 0.25f * (1.0f - localRange)) * 255);
            g = (int)(localRange * 255 * 0.5f);
            b = 255;
        }
        
        // Clamp values - no intensity reduction, keep original colors
        r = qBound(0, r, 255);
        g = qBound(0, g, 255);
        b = qBound(0, b, 255);
        
        color = QColor(r, g, b);
    }
    
    return color;
}

QColor QGL3DPanel::amplitudeToColorWithOffset(float amplitude, float offset) {
    // Use the same Enhanced waterfall color algorithm as the RX panel
    // This matches the getWaterfallColorAtPixel() Enhanced mode
    
    // Adjust the dB range and apply provided offset for color mapping
    float lowerThreshold = -160.0f + offset;  // User-adjustable minimum color level
    float upperThreshold = -80.0f + offset;   // Upper end adjusts with offset
    
    QColor color;
    int r, g, b;
    
    if (amplitude <= lowerThreshold) {
        // Use black/dark blue for lowest signals
        color = QColor(0, 0, 20);
    }
    else if (amplitude >= upperThreshold) {
        // White for highest signals  
        color = QColor(255, 255, 255);
    }
    else {
        // Enhanced waterfall color algorithm - exact copy from RX panel
        float offset_amp = amplitude - lowerThreshold;
        float globalRange = offset_amp / (upperThreshold - lowerThreshold);
        
        if (globalRange < 2.0f/9.0f) { // background to blue
            float localRange = globalRange / (2.0f/9.0f);
            r = (int)((1.0f - localRange) * 0);
            g = (int)((1.0f - localRange) * 0);
            b = (int)(20 + localRange * (255 - 20));
        }
        else if (globalRange < 3.0f/9.0f) { // blue to blue-green
            float localRange = (globalRange - 2.0f/9.0f) / (1.0f/9.0f);
            r = 0;
            g = (int)(localRange * 255);
            b = 255;
        }
        else if (globalRange < 4.0f/9.0f) { // blue-green to green
            float localRange = (globalRange - 3.0f/9.0f) / (1.0f/9.0f);
            r = 0;
            g = 255;
            b = (int)((1.0f - localRange) * 255);
        }
        else if (globalRange < 5.0f/9.0f) { // green to red-green
            float localRange = (globalRange - 4.0f/9.0f) / (1.0f/9.0f);
            r = (int)(localRange * 255);
            g = 255;
            b = 0;
        }
        else if (globalRange < 7.0f/9.0f) { // red-green to red
            float localRange = (globalRange - 5.0f/9.0f) / (2.0f/9.0f);
            r = 255;
            g = (int)((1.0f - localRange) * 255);
            b = 0;
        }
        else if (globalRange < 8.0f/9.0f) { // red to red-blue
            float localRange = (globalRange - 7.0f/9.0f) / (1.0f/9.0f);
            r = 255;
            g = 0;
            b = (int)(localRange * 255);
        }
        else { // red-blue to purple end
            float localRange = (globalRange - 8.0f/9.0f) / (1.0f/9.0f);
            r = (int)((0.75f + 0.25f * (1.0f - localRange)) * 255);
            g = (int)(localRange * 255 * 0.5f);
            b = 255;
        }
        
        // Clamp values - no intensity reduction, keep original colors
        r = qBound(0, r, 255);
        g = qBound(0, g, 255);
        b = qBound(0, b, 255);
        
        color = QColor(r, g, b);
    }
    
    return color;
}

void QGL3DPanel::qglColor(QColor color) {
    glColor4f(color.redF(), color.greenF(), color.blueF(), color.alphaF());
}

void QGL3DPanel::onUpdateTimer() {
    // With per-slice rendering, just trigger repaints for smooth animation
    if (m_isVisible) {
        update();
    }
}

// Slots
void QGL3DPanel::spectrumDataChanged(const QVector<float>& data) {
    setSpectrumData(data);
}

// SDR integration methods
void QGL3DPanel::setSpectrumBuffer(int rx, const qVectorFloat& buffer) {
    if (rx != m_receiver) return; // Only accept data for our receiver
    
    // Convert qVectorFloat to QVector<float> for our internal use
    QVector<float> spectrumData;
    spectrumData.reserve(buffer.size());
    for (const auto& value : buffer) {
        spectrumData.append(value);
    }
    
    setSpectrumData(spectrumData);
}

void QGL3DPanel::setCtrFrequency(QObject* sender, int mode, int rx, long freq) {
    Q_UNUSED(sender)
    Q_UNUSED(mode)
    
    if (rx != m_receiver) return;
    
    m_centerFrequency = freq;
    update(); // Trigger repaint to update frequency labels
}

void QGL3DPanel::setVFOFrequency(QObject* sender, int mode, int rx, long freq) {
    Q_UNUSED(sender)
    Q_UNUSED(mode)
    
    if (rx != m_receiver) return;
    
    m_vfoFrequency = freq;
    update(); // Trigger repaint to update frequency cursor
}

void QGL3DPanel::updateDisplay() {
    if (m_isVisible) {
        update();
    }
}

// Performance optimization methods
void QGL3DPanel::performUpdate() {
    // With per-slice rendering, updates happen automatically when new data arrives
    // This timer just triggers redraws for smooth animation
    if (m_isVisible) {
        update();
    }
}

void QGL3DPanel::calculateFPS() {
    m_currentFPS = m_frameCount;
    m_frameCount = 0;
}

void QGL3DPanel::showEvent(QShowEvent* event) {
    QOpenGLWidget::showEvent(event);
    m_isVisible = true;
    if (m_updateTimer) {
        m_updateTimer->start(m_updateFrequencyMs);
    }
}

void QGL3DPanel::hideEvent(QHideEvent* event) {
    QOpenGLWidget::hideEvent(event);
    m_isVisible = false;
    if (m_updateTimer) {
        m_updateTimer->stop();
    }
}

// 3D Display control methods
void QGL3DPanel::setHeightScale(float scale) {
    m_heightScale = qBound(0.1f, scale, 100.0f); // Reasonable range
    update(); // Trigger redraw
}

void QGL3DPanel::setFrequencyScale(float scale) {
    m_frequencyScale = qBound(0.1f, scale, 10.0f); // Reasonable range
    // Scales are applied in shader, no mesh regeneration needed
    update(); // Trigger redraw
}

void QGL3DPanel::setTimeScale(float scale) {
    m_timeScale = qBound(0.1f, scale, 10.0f); // Reasonable range
    // Scales are applied in shader, no mesh regeneration needed
    update(); // Trigger redraw
}

void QGL3DPanel::setUpdateRate(int intervalMs) {
    m_updateFrequencyMs = qBound(16, intervalMs, 1000); // 16ms (60 FPS) to 1000ms (1 FPS)
    qDebug() << "3D Panel: Update rate changed to" << intervalMs << "ms (" << (1000.0f / intervalMs) << "FPS)";
    if (m_updateTimer) {
        m_updateTimer->setInterval(m_updateFrequencyMs);
    }
}

void QGL3DPanel::setShowGrid(bool show) {
    m_showGrid = show;
    update(); // Trigger redraw
}

void QGL3DPanel::setShowAxes(bool show) {
    m_showAxes = show;
    update(); // Trigger redraw
}

void QGL3DPanel::setWireframeMode(bool wireframe) {
    m_showWireframe = wireframe;
    update(); // Trigger redraw
}

void QGL3DPanel::setWaterfallOffset(float offset) {
    m_waterfallOffset = qBound(-50.0f, offset, 50.0f);  // Reasonable offset range
    // Trigger a color update for the waterfall
    update();
}

int QGL3DPanel::calculateLODLevel() const {
    // Calculate LOD based on camera distance
    // LOD 0 = full detail, LOD 1 = half, LOD 2 = quarter
    if (m_cameraDistance > 250.0f) {
        return 2;  // Quarter resolution when far away
    } else if (m_cameraDistance > 150.0f) {
        return 1;  // Half resolution at medium distance
    }
    return 0;  // Full resolution when close
}

bool QGL3DPanel::isSliceVisible(int sliceIndex) const {
    Q_UNUSED(sliceIndex)
    
    // Simple visibility check - could be enhanced with proper frustum culling
    // For now, just check if slice is within reasonable bounds
    
    // Always render if we have few slices
    if (m_timeSliceMeshes.size() < 20) {
        return true;
    }
    
    // When we have many slices, we could cull based on camera view
    // For now, render all slices (this is still much faster than regenerating)
    return true;
}

void QGL3DPanel::cleanupOldSlices() {
    // Remove oldest slices when we exceed maximum
    while (m_timeSliceMeshes.size() > MAX_TIME_SLICES) {
        TimeSliceMesh& oldSlice = m_timeSliceMeshes.last();
        
        // Ensure we have OpenGL context
        makeCurrent();
        
        // Destroy OpenGL resources
        if (oldSlice.vao) {
            oldSlice.vao->destroy();
            delete oldSlice.vao;
        }
        if (oldSlice.vertexBuffer) {
            oldSlice.vertexBuffer->destroy();
            delete oldSlice.vertexBuffer;
        }
        if (oldSlice.indexBuffer) {
            oldSlice.indexBuffer->destroy();
            delete oldSlice.indexBuffer;
        }
        
        doneCurrent();
        
        m_timeSliceMeshes.removeLast();
    }
}

void QGL3DPanel::addNewSliceMesh(const MeshGeneratorWorker::MeshData& meshData) {
    makeCurrent();  // Ensure we have OpenGL context
    
    TimeSliceMesh newSlice;
    
    // Create VAO
    newSlice.vao = new QOpenGLVertexArrayObject();
    newSlice.vao->create();
    newSlice.vao->bind();
    
    // Create and fill vertex buffer
    newSlice.vertexBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    newSlice.vertexBuffer->create();
    newSlice.vertexBuffer->bind();
    newSlice.vertexBuffer->allocate(meshData.vertices.constData(), 
                                     meshData.vertices.size() * sizeof(float));
    
    // Create and fill index buffer
    newSlice.indexBuffer = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    newSlice.indexBuffer->create();
    newSlice.indexBuffer->bind();
    newSlice.indexBuffer->allocate(meshData.indices.constData(), 
                                    meshData.indices.size() * sizeof(unsigned int));
    
    // Setup vertex attributes (must match shader)
    m_shaderProgram->bind();
    m_shaderProgram->enableAttributeArray(0);  // position
    m_shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, 6 * sizeof(float));
    
    m_shaderProgram->enableAttributeArray(1);  // color
    m_shaderProgram->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 3, 6 * sizeof(float));
    m_shaderProgram->release();
    
    newSlice.vao->release();
    
    // Store metadata
    newSlice.vertexCount = meshData.vertexCount;
    newSlice.indexCount = meshData.indexCount;
    newSlice.timeIndex = 0;  // Always at front
    newSlice.lodLevel = calculateLODLevel();
    
    // Add to FRONT of collection (index 0 = newest)
    // This pushes all old slices backward in the array
    m_timeSliceMeshes.prepend(newSlice);
    
    doneCurrent();
}
