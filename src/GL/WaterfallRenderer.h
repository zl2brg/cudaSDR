#ifndef WATERFALLRENDERER_H
#define WATERFALLRENDERER_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <QVector>
#include <QRect>
#include "../cusdr_settings.h"
#include "cusdr_oglUtils.h"

class WaterfallRenderer : protected QOpenGLFunctions {
public:
    explicit WaterfallRenderer();
    ~WaterfallRenderer();

    void initialize();
    void render(const QMatrix4x4& projection, const QRect& rect, const QVarLengthArray<TGL_ubyteRGBA>& pixelData, QSDR::_DataEngineState dataEngineState, float dpr);
    void reset();

private:
    struct VertexData {
        float x, y, z;
        float u, v;
    };

    void setupTexture(int width, int height);

    GLuint m_textureId;
    int m_currentLine;
    int m_lineCnt;
    int m_oldWidth;
    int m_oldHeight;
    float m_oldDpr;
    bool m_updatePending;
    QVector<TGL_ubyteRGBA> m_textureBuffer;

    QOpenGLShaderProgram* m_shader;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;

    GLuint m_pboIds[2];
    int m_pboIndex;
    int m_prevLine;
    bool m_pboActive;
};

#endif // WATERFALLRENDERER_H
