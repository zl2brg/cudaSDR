#include "WaterfallRenderer.h"
#include <QDebug>

WaterfallRenderer::WaterfallRenderer()
    : m_textureId(0)
    , m_currentLine(0)
    , m_lineCnt(0)
    , m_oldWidth(0)
    , m_oldHeight(0)
    , m_updatePending(false)
{
}

WaterfallRenderer::~WaterfallRenderer() {
    if (m_textureId != 0) {
        // Assume context is current if we are being destroyed in proper order, 
        // otherwise this might leak or crash. In a robust impl, we'd handle context.
        // glDeleteTextures(1, &m_textureId);
    }
}

void WaterfallRenderer::initialize() {
    initializeOpenGLFunctions();
}

void WaterfallRenderer::reset() {
    m_updatePending = true;
}

void WaterfallRenderer::setupTexture(int width, int height) {
    if (m_textureId != 0) {
        glDeleteTextures(1, &m_textureId);
    }
    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    QVector<TGL_ubyteRGBA> blackBuffer(width * height);
    TGL_ubyteRGBA black; black.red = 0; black.green = 0; black.blue = 0; black.alpha = 255;
    blackBuffer.fill(black);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, blackBuffer.data());
    
    m_currentLine = 0;
    m_lineCnt = 0;
    m_oldWidth = width;
    m_oldHeight = height;
    m_updatePending = false;
}

void WaterfallRenderer::render(const QRect& rect, const QVarLengthArray<TGL_ubyteRGBA>& pixelData, QSDR::_DataEngineState dataEngineState) {
    if (rect.isEmpty()) return;

    int width = rect.width();
    int height = rect.height();
    int top = rect.top();
    int left = rect.left();

    if (m_textureId == 0 || m_oldWidth != width || m_oldHeight != height || m_updatePending) {
        setupTexture(width, height);
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    if (dataEngineState == QSDR::DataEngineUp) {
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        
        if (!pixelData.isEmpty()) {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, m_currentLine, width, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixelData.data());
        }

        glEnable(GL_TEXTURE_2D);
        
        float v0 = (float)m_currentLine / height;
        float h1 = (float)(m_currentLine + 1) / height;

        glBegin(GL_QUADS);
            glTexCoord2f(0, v0); glVertex2i(left,         top);
            glTexCoord2f(1, v0); glVertex2i(left + width, top);
            glTexCoord2f(1, 0);  glVertex2i(left + width, top + (m_currentLine + 1));
            glTexCoord2f(0, 0);  glVertex2i(left,         top + (m_currentLine + 1));
        glEnd();

        if (m_currentLine < height - 1) {
            glBegin(GL_QUADS);
                glTexCoord2f(0, 1);  glVertex2i(left,         top + (m_currentLine + 1));
                glTexCoord2f(1, 1);  glVertex2i(left + width, top + (m_currentLine + 1));
                glTexCoord2f(1, h1); glVertex2i(left + width, top + height);
                glTexCoord2f(0, h1); glVertex2i(left,         top + height);
            glEnd();
        }

        glDisable(GL_TEXTURE_2D);

        m_currentLine = (m_currentLine + 1) % height;
        if (m_lineCnt < height) m_lineCnt++;
    } else {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        
        float v0 = (float)((m_currentLine - 1 + height) % height) / height;
        
        glBegin(GL_QUADS);
            glTexCoord2f(0, v0); glVertex2i(left,         top);
            glTexCoord2f(1, v0); glVertex2i(left + width, top);
            glTexCoord2f(1, 0);  glVertex2i(left + width, top + m_currentLine);
            glTexCoord2f(0, 0);  glVertex2i(left,         top + m_currentLine);
        glEnd();

        if (m_currentLine < height) {
            glBegin(GL_QUADS);
                glTexCoord2f(0, 1);  glVertex2i(left,         top + m_currentLine);
                glTexCoord2f(1, 1);  glVertex2i(left + width, top + m_currentLine);
                glTexCoord2f(1, v0 + 1.0f/height); glVertex2i(left + width, top + height);
                glTexCoord2f(0, v0 + 1.0f/height); glVertex2i(left,         top + height);
            glEnd();
        }
        glDisable(GL_TEXTURE_2D);
    }
}
