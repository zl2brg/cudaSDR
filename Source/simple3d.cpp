/**
* @file  simple3d.cpp
* @brief Standalone 3D Panadapter test - minimal dependencies
* @author Simon Brown, ZL2BRG
* @date 2025-10-12
*/

#include <QApplication>
#include <QMainWindow>
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
#include <QVBoxLayout>
#include <QLabel>
#include <QtMath>
#include <QDebug>

class Simple3DPanel : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    explicit Simple3DPanel(QWidget *parent = nullptr) 
        : QOpenGLWidget(parent)
        , m_shaderProgram(nullptr)
        , m_vertexBuffer(nullptr)
        , m_indexBuffer(nullptr)
        , m_vao(nullptr)
        , m_cameraDistance(80.0f)
        , m_cameraPitch(-45.0f)  // Better angle for waterfall view
        , m_cameraYaw(0.0f)      // Start looking straight down the frequency axis
        , m_mousePressed(false)
        , m_heightScale(3.0f)    // Reduced height scale for better proportion
        , m_showWireframe(false)
    {
        m_cameraPosition = QVector3D(0, 40, 50);
        m_cameraTarget = QVector3D(0, 0, 25);  // Look toward middle of time axis
        m_cameraUp = QVector3D(0, 1, 0);
        
        // Initialize spectrum data
        m_spectrumHistory.reserve(50);  // 50 time slices
        
        setMouseTracking(true);
        setFocusPolicy(Qt::StrongFocus);
    }

    ~Simple3DPanel() {
        makeCurrent();
        cleanup();
        doneCurrent();
    }

    void addSpectrumData(const QVector<float>& data) {
        m_spectrumHistory.prepend(data);
        if (m_spectrumHistory.size() > 50) {
            m_spectrumHistory.removeLast();
        }
        m_meshNeedsUpdate = true;
        update();
    }

protected:
    void initializeGL() override {
        initializeOpenGLFunctions();
        
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        
        setupShaders();
        setupMesh();
        
        qDebug() << "Simple 3D Panel initialized";
    }

    void paintGL() override {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        updateCamera();
        
        if (m_meshNeedsUpdate && !m_spectrumHistory.isEmpty()) {
            updateMesh();
        }
        
        renderSpectrum();
        renderGrid();
    }

    void resizeGL(int width, int height) override {
        glViewport(0, 0, width, height);
        
        m_projectionMatrix.setToIdentity();
        m_projectionMatrix.perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
    }

    void mousePressEvent(QMouseEvent *event) override {
        m_lastMousePos = event->pos();
        m_mousePressed = true;
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        if (!m_mousePressed) return;
        
        QPoint delta = event->pos() - m_lastMousePos;
        
        m_cameraYaw += delta.x() * 0.5f;
        m_cameraPitch += delta.y() * 0.5f;
        m_cameraPitch = qBound(-89.0f, m_cameraPitch, 89.0f);
        
        update();
        m_lastMousePos = event->pos();
    }

    void mouseReleaseEvent(QMouseEvent *event) override {
        Q_UNUSED(event)
        m_mousePressed = false;
    }

    void wheelEvent(QWheelEvent *event) override {
        float delta = event->angleDelta().y() / 120.0f;
        m_cameraDistance -= delta * 2.0f;
        m_cameraDistance = qBound(10.0f, m_cameraDistance, 200.0f);
        update();
    }

    void keyPressEvent(QKeyEvent *event) override {
        switch (event->key()) {
        case Qt::Key_R:
            m_cameraDistance = 50.0f;
            m_cameraPitch = -30.0f;
            m_cameraYaw = 45.0f;
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
        }
        update();
    }

private:
    void cleanup() {
        if (m_vao) { m_vao->destroy(); delete m_vao; m_vao = nullptr; }
        if (m_vertexBuffer) { m_vertexBuffer->destroy(); delete m_vertexBuffer; m_vertexBuffer = nullptr; }
        if (m_indexBuffer) { m_indexBuffer->destroy(); delete m_indexBuffer; m_indexBuffer = nullptr; }
        delete m_shaderProgram; m_shaderProgram = nullptr;
    }

    void setupShaders() {
        m_shaderProgram = new QOpenGLShaderProgram();
        
        const char* vertexSource = R"(
            #version 330 core
            layout(location = 0) in vec3 position;
            layout(location = 1) in vec3 color;
            
            uniform mat4 mvpMatrix;
            uniform float heightScale;
            
            out vec3 fragColor;
            
            void main() {
                vec3 scaledPos = vec3(position.x, position.y * heightScale, position.z);
                gl_Position = mvpMatrix * vec4(scaledPos, 1.0);
                fragColor = color;
            }
        )";
        
        const char* fragmentSource = R"(
            #version 330 core
            in vec3 fragColor;
            out vec4 finalColor;
            
            void main() {
                finalColor = vec4(fragColor, 1.0);
            }
        )";
        
        m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexSource);
        m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentSource);
        m_shaderProgram->link();
    }

    void setupMesh() {
        m_vao = new QOpenGLVertexArrayObject();
        m_vao->create();
        
        m_vertexBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        m_vertexBuffer->create();
        
        // Index buffer will be created in updateMesh when needed
        m_indexBuffer = nullptr;
    }

    void updateMesh() {
        if (m_spectrumHistory.isEmpty()) return;
        
        m_vertices.clear();
        m_indices.clear();
        
        int timeSlices = qMin(m_spectrumHistory.size(), 50);
        int freqBins = 256;  // Fixed for simplicity
        
        // Generate vertices for surface mesh
        for (int t = 0; t < timeSlices; t++) {
            const QVector<float>& spectrum = m_spectrumHistory[t];
            for (int f = 0; f < freqBins; f++) {
                float amplitude = (f < spectrum.size()) ? spectrum[f] : 0.0f;
                
                // Position - NEW DATA AT FRONT (z=0), OLD DATA RECEDES TO BACK
                float x = (float)f - freqBins / 2.0f;           // Frequency axis
                float y = amplitude;                            // Amplitude (height)
                float z = (float)t * 2.0f;                     // Time axis - newer = closer to viewer
                
                m_vertices.append(x);
                m_vertices.append(y);
                m_vertices.append(z);
                
                // Color based on amplitude and age
                QColor color = amplitudeToColor(amplitude);
                float ageFactor = 1.0f - (float)t / timeSlices;  // Fade older data
                m_vertices.append(color.redF() * ageFactor);
                m_vertices.append(color.greenF() * ageFactor);
                m_vertices.append(color.blueF() * ageFactor);
            }
        }
        
        // Generate indices for triangle strips to create surface
        m_indices.clear();
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
        
        // Update buffer
        m_vao->bind();
        m_vertexBuffer->bind();
        m_vertexBuffer->allocate(m_vertices.constData(), m_vertices.size() * sizeof(float));
        
        // Create index buffer for surface rendering
        if (!m_indexBuffer) {
            m_indexBuffer = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
            m_indexBuffer->create();
        }
        
        m_indexBuffer->bind();
        m_indexBuffer->allocate(m_indices.constData(), m_indices.size() * sizeof(unsigned int));
        
        m_shaderProgram->enableAttributeArray(0);
        m_shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, 6 * sizeof(float));
        
        m_shaderProgram->enableAttributeArray(1);
        m_shaderProgram->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 3, 6 * sizeof(float));
        
        m_vao->release();
        m_meshNeedsUpdate = false;
    }

    void renderSpectrum() {
        if (!m_shaderProgram || m_vertices.isEmpty()) return;
        
        m_shaderProgram->bind();
        
        QMatrix4x4 mvpMatrix = m_projectionMatrix * m_viewMatrix;
        m_shaderProgram->setUniformValue("mvpMatrix", mvpMatrix);
        m_shaderProgram->setUniformValue("heightScale", m_heightScale);
        
        m_vao->bind();
        
        if (m_showWireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(1.0f);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        
        // Render as connected surface (triangles) instead of points
        if (!m_indices.isEmpty()) {
            glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
        } else {
            // Fallback to points if no indices
            glPointSize(2.0f);
            glDrawArrays(GL_POINTS, 0, m_vertices.size() / 6);
        }
        
        m_vao->release();
        m_shaderProgram->release();
    }

    void renderGrid() {
        glDisable(GL_DEPTH_TEST);
        glColor3f(0.3f, 0.3f, 0.3f);
        glBegin(GL_LINES);
        
        // Grid lines
        for (int i = -100; i <= 100; i += 20) {
            glVertex3f(i, 0, -25);
            glVertex3f(i, 0, 25);
            glVertex3f(-128, 0, i/4);
            glVertex3f(128, 0, i/4);
        }
        
        glEnd();
        glEnable(GL_DEPTH_TEST);
    }

    void updateCamera() {
        float radPitch = qDegreesToRadians(m_cameraPitch);
        float radYaw = qDegreesToRadians(m_cameraYaw);
        
        m_cameraPosition.setX(m_cameraDistance * cos(radPitch) * cos(radYaw));
        m_cameraPosition.setY(m_cameraDistance * sin(radPitch));
        m_cameraPosition.setZ(m_cameraDistance * cos(radPitch) * sin(radYaw));
        
        m_viewMatrix.setToIdentity();
        m_viewMatrix.lookAt(m_cameraPosition, m_cameraTarget, m_cameraUp);
    }

    QColor amplitudeToColor(float amplitude) {
        float normalized = qBound(0.0f, amplitude / 10.0f, 1.0f);
        
        if (normalized < 0.25f) {
            float t = normalized * 4.0f;
            return QColor::fromRgbF(0, t, 1.0f);
        } else if (normalized < 0.5f) {
            float t = (normalized - 0.25f) * 4.0f;
            return QColor::fromRgbF(0, 1.0f, 1.0f - t);
        } else if (normalized < 0.75f) {
            float t = (normalized - 0.5f) * 4.0f;
            return QColor::fromRgbF(t, 1.0f, 0);
        } else {
            float t = (normalized - 0.75f) * 4.0f;
            return QColor::fromRgbF(1.0f, 1.0f - t, 0);
        }
    }

private:
    QOpenGLShaderProgram* m_shaderProgram;
    QOpenGLBuffer* m_vertexBuffer;
    QOpenGLBuffer* m_indexBuffer;
    QOpenGLVertexArrayObject* m_vao;
    
    QVector3D m_cameraPosition, m_cameraTarget, m_cameraUp;
    float m_cameraDistance, m_cameraPitch, m_cameraYaw;
    QMatrix4x4 m_viewMatrix, m_projectionMatrix;
    
    QPoint m_lastMousePos;
    bool m_mousePressed;
    float m_heightScale;
    bool m_showWireframe;
    
    QVector<QVector<float>> m_spectrumHistory;
    QVector<float> m_vertices;
    QVector<unsigned int> m_indices;
    bool m_meshNeedsUpdate = true;
};

class TestWindow : public QMainWindow {
    Q_OBJECT

public:
    TestWindow() {
        QWidget *central = new QWidget(this);
        setCentralWidget(central);
        
        QVBoxLayout *layout = new QVBoxLayout(central);
        
        m_panel = new Simple3DPanel(this);
        m_panel->setMinimumSize(800, 600);
        layout->addWidget(m_panel);
        
        QLabel *label = new QLabel("3D Spectrum Test - Mouse: rotate, Wheel: zoom, R: reset, W: wireframe, +/-: height");
        layout->addWidget(label);
        
        setWindowTitle("Simple 3D Panadapter Test");
        resize(900, 700);
        
        // Generate test data
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &TestWindow::generateData);
        m_timer->start(100);
        
        generateData();
    }

private slots:
    void generateData() {
        QVector<float> spectrum(256);
        static float time = 0.0f;
        time += 0.1f;
        
        for (int i = 0; i < spectrum.size(); i++) {
            float freq = (float)i / spectrum.size();
            float noise = (rand() % 100) / 1000.0f;
            
            // Create some fake signals
            float signal1 = 3.0f * exp(-50.0f * pow(freq - 0.3f, 2));
            float signal2 = 2.0f * exp(-100.0f * pow(freq - 0.7f, 2));
            float signal3 = 1.5f * sin(time + freq * 8) * exp(-20.0f * pow(freq - 0.5f, 2));
            
            spectrum[i] = noise + signal1 + signal2 + signal3;
        }
        
        m_panel->addSpectrumData(spectrum);
    }

private:
    Simple3DPanel* m_panel;
    QTimer* m_timer;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    TestWindow window;
    window.show();
    
    return app.exec();
}

#include "simple3d.moc"