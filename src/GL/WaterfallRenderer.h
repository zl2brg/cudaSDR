#ifndef WATERFALLRENDERER_H
#define WATERFALLRENDERER_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <QVector>
#include <QVarLengthArray>
#include <QRect>
#include <QColor>
#include "../cusdr_settings.h"
#include "cusdr_oglUtils.h"

struct WaterfallMapping {
    float lowerThreshold = -160.0f;
    float upperThreshold = 0.0f;
    float colorRange = 160.0f;
    WaterfallColorMode mode = Simple;
    QColor lo = QColor(0, 0, 0);
    QColor mid = QColor(192, 124, 255);
    QColor hi = QColor(192, 124, 255);
    float alpha = 1.0f;
};

class WaterfallRenderer : protected QOpenGLFunctions {
public:
    explicit WaterfallRenderer();
    ~WaterfallRenderer();

    void initialize();
    void render(const QMatrix4x4& projection, const QRect& rect, const QVarLengthArray<float>& intensity,
                QSDR::_DataEngineState dataEngineState, float dpr, bool newLine, const WaterfallMapping& mapping);
    void reset();

private:
    struct VertexData {
        float x, y, z;
        float u, v;
    };

    void setupTexture(int width, int height);
    void uploadNewRow(const float* logical, int logicalWidth, int texWidth, int texHeight, qreal dpr);
    void ensurePaletteLUT(const WaterfallMapping& mapping);
    void resolveTexStorage();
    void allocateTexture2D(GLenum internalFormat, int width, int height,
                           GLenum uploadFormat, GLenum uploadType, const void *pixels);

    using TexStorage2DFn = void (*)(GLenum target, GLsizei levels, GLenum internalformat,
                                    GLsizei width, GLsizei height);

    GLuint m_textureId;
    GLuint m_lutId;
    int m_headLine;
    int m_oldWidth;
    int m_oldHeight;
    float m_oldDpr;
    bool m_updatePending;
    bool m_hasTexStorage;
    TexStorage2DFn m_glTexStorage2D;

    QOpenGLShaderProgram* m_shader;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;

    QOpenGLBuffer m_pbo[2];
    int m_pboIndex;
    QVector<float> m_devRow;

    WaterfallColorMode m_lutMode;
    QColor m_lutLo;
    QColor m_lutMid;
    QColor m_lutHi;
};

#endif // WATERFALLRENDERER_H
