#include "PanadapterRenderer.h"
#include <QMatrix4x4>
#include <QVarLengthArray>

PanadapterRenderer::PanadapterRenderer()
    : m_shader(nullptr)
    , m_ownsShader(false)
    , m_vbo(QOpenGLBuffer::VertexBuffer)
    , m_vboSize(0)
{
}

PanadapterRenderer::~PanadapterRenderer() {
    if (m_vao.isCreated()) m_vao.destroy();
    if (m_vbo.isCreated()) m_vbo.destroy();
    if (m_ownsShader && m_shader) delete m_shader;
}

void PanadapterRenderer::initialize(QOpenGLShaderProgram* sharedShader) {
    initializeOpenGLFunctions();

    if (sharedShader) {
        m_shader = sharedShader;
        m_ownsShader = false;
    } else {
        m_shader = new QOpenGLShaderProgram();
        m_ownsShader = true;
        
        const char *vsrc =
            "#version 150\n"
            "in vec3 position;\n"
            "in vec4 color;\n"
            "out vec4 vertColor;\n"
            "uniform mat4 matrix;\n"
            "void main() {\n"
            "   vertColor = color;\n"
            "   gl_Position = matrix * vec4(position, 1.0);\n"
            "}\n";

        const char *fsrc =
            "#version 150\n"
            "in vec4 vertColor;\n"
            "out vec4 fragColor;\n"
            "void main() {\n"
            "   fragColor = vertColor;\n"
            "}\n";

        m_shader->addShaderFromSourceCode(QOpenGLShader::Vertex, vsrc);
        m_shader->addShaderFromSourceCode(QOpenGLShader::Fragment, fsrc);
        m_shader->bindAttributeLocation("position", 0);
        m_shader->bindAttributeLocation("color", 1);
        m_shader->link();
    }

    m_vao.create();
    m_vao.bind();
    m_vbo.create();
    m_vbo.bind();
    m_vbo.setUsagePattern(QOpenGLBuffer::StreamDraw);
    
    m_shader->bind();
    m_shader->enableAttributeArray(0);
    m_shader->enableAttributeArray(1);
    m_shader->release();
    
    m_vao.release();
    m_vbo.release();
}

void PanadapterRenderer::render(const QMatrix4x4& projection,
                                const QRect& panRect, 
                                const QVector<qreal>& bins, 
                                qreal dBmMax, qreal dBmMin, 
                                PanGraphicsMode mode, 
                                float scaleMult, float dpr,
                                int parentHeight,
                                const Colors& colors,
                                QSDR::_DataEngineState dataEngineState,
                                bool isCurrentReceiver) {
    
    int vertexArrayLength = (int)bins.size();
    if (vertexArrayLength == 0) return;

    if (!m_shader || !m_shader->isLinked()) return;

    int x1 = panRect.left();
    int y1 = panRect.top();
    int x2 = x1 + panRect.width();
    int y2 = y1 + panRect.height();
    int height = panRect.height();

    // Set scissor box for spectrum clipping
    glScissor((int)(x1 * dpr), (int)((parentHeight - y2) * dpr), (int)((x2 - x1) * dpr), (int)(height * dpr));
    glEnable(GL_SCISSOR_TEST);

    qreal dBmRange = qAbs(dBmMax - dBmMin);
    float yScale = panRect.height() / (float)dBmRange;
    float yTop = (float)y2;

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    m_shader->bind();
    m_shader->setUniformValue("matrix", projection);

    m_vao.bind();
    m_vbo.bind();

    auto updateVBO = [&](int size, const void* data) {
        if (size > m_vboSize) {
            m_vbo.allocate(data, size);
            m_vboSize = size;
        } else {
            m_vbo.write(0, data, size);
        }
        m_shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 7);
        m_shader->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 4, sizeof(float) * 7);
    };

    // Modern Background Rendering
    float r1, g1, b1, r2, g2, b2, r3, g3, b3, r4, g4, b4, a = 1.0f;
    if (dataEngineState == QSDR::_DataEngineState::DataEngineUp) {
        r1 = 0.8f * colors.bkgR; g1 = 0.8f * colors.bkgG; b1 = 0.8f * colors.bkgB;
        r2 = 0.6f * colors.bkgR; g2 = 0.6f * colors.bkgG; b2 = 0.6f * colors.bkgB;
        r3 = 0.4f * colors.bkgR; g3 = 0.4f * colors.bkgG; b3 = 0.4f * colors.bkgB;
        r4 = 0.2f * colors.bkgR; g4 = 0.2f * colors.bkgG; b4 = 0.2f * colors.bkgB;
        
        if (!isCurrentReceiver) {
            r1 = r2 = r3 = r4 = 0.4f * colors.bkgR;
            g1 = g2 = g3 = g4 = 0.4f * colors.bkgG;
            b1 = b2 = b3 = b4 = 0.4f * colors.bkgB;
        }
    } else {
        r1 = r2 = r3 = r4 = 0.15f * colors.bkgR;
        g1 = g2 = g3 = g4 = 0.15f * colors.bkgG;
        b1 = b2 = b3 = b4 = 0.15f * colors.bkgB;
    }

    VertexData bkgData[4] = {
        { (float)x1, (float)y1, -4.0f, r1, g1, b1, a },
        { (float)x2, (float)y1, -4.0f, r2, g2, b2, a },
        { (float)x1, (float)y2, -4.0f, r3, g3, b3, a },
        { (float)x2, (float)y2, -4.0f, r4, g4, b4, a }
    };
    updateVBO(4 * sizeof(VertexData), bkgData);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    switch (mode) {
        case (PanGraphicsMode) FilledLine: {
            m_vertexCache.resize(vertexArrayLength * 2);
            for (int i = 0; i < vertexArrayLength; i++) {
                float vx = (float)(x1 + (i/scaleMult));
                float vy = (float)(yTop - yScale * (float)bins.at(i));
                m_vertexCache[2*i] = { vx, vy, -1.5f, 0.7f * colors.rf, 0.7f * colors.gf, 0.7f * colors.bf, 0.4f };
                m_vertexCache[2*i+1] = { vx, yTop, -1.5f, 0.3f * colors.rf, 0.3f * colors.gf, 0.3f * colors.bf, 0.2f };
            }
            updateVBO((int)(m_vertexCache.size() * sizeof(VertexData)), m_vertexCache.data());
            glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexArrayLength * 2);

            m_vertexCache.resize(vertexArrayLength);
            for (int i = 0; i < vertexArrayLength; i++) {
                float vx = (float)(x1 + (i/scaleMult));
                m_vertexCache[i] = { vx, (float)(yTop - yScale * (float)bins.at(i)), -1.0f, colors.r, colors.g, colors.b, 1.0f };
            }
            updateVBO((int)(m_vertexCache.size() * sizeof(VertexData)), m_vertexCache.data());
            glDrawArrays(GL_LINE_STRIP, 0, vertexArrayLength);
            break;
        }

        case (PanGraphicsMode) Line: {
            m_vertexCache.resize(vertexArrayLength);
            for (int i = 0; i < vertexArrayLength; i++) {
                float vx = (float)(x1 + (i/scaleMult));
                m_vertexCache[i] = { vx, (float)(yTop - yScale * (float)bins.at(i)), -1.0f, colors.r, colors.g, colors.b, 1.0f };
            }
            updateVBO((int)(m_vertexCache.size() * sizeof(VertexData)), m_vertexCache.data());
            glDrawArrays(GL_LINE_STRIP, 0, vertexArrayLength);
            break;
        }

        case (PanGraphicsMode) Solid: {
            glDisable(GL_MULTISAMPLE);
            m_vertexCache.resize(vertexArrayLength * 2);
            for (int i = 0; i < vertexArrayLength; i++) {
                float vx = (float)(x1 + (i/scaleMult));
                float vy = (float)(yTop - yScale * (float)bins.at(i));
                m_vertexCache[2*i]   = { vx, vy,   -1.0f, colors.rst, colors.gst, colors.bst, 1.0f };
                m_vertexCache[2*i+1] = { vx, yTop, -1.0f, colors.rsb, colors.gsb, colors.bsb, 1.0f };
            }
            updateVBO((int)(m_vertexCache.size() * sizeof(VertexData)), m_vertexCache.data());
            glDrawArrays(GL_LINES, 0, vertexArrayLength * 2);
            glEnable(GL_MULTISAMPLE);
            break;
        }
    }

    m_vao.release();
    m_vbo.release();
    m_shader->release();
    glDisable(GL_SCISSOR_TEST);
}
