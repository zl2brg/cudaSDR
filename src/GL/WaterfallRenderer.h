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
    void render(const QMatrix4x4& projection, const QRect& rect, const QVarLengthArray<TGL_ubyteRGBA>& pixelData, QSDR::_DataEngineState dataEngineState);
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
    bool m_updatePending;

    QOpenGLShaderProgram* m_shader;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
};

#endif // WATERFALLRENDERER_H
