#ifndef PANADAPTERRENDERER_H
#define PANADAPTERRENDERER_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QVector>
#include <QRect>
#include "../cusdr_settings.h"

class PanadapterRenderer : protected QOpenGLFunctions {
public:
    struct Colors {
        float r, g, b;       // Line color
        float rf, gf, bf;    // Filled color
        float rst, gst, bst; // Solid Top color
        float rsb, gsb, bsb; // Solid Bottom color
        float bkgR, bkgG, bkgB; // Background color
    };

    explicit PanadapterRenderer();
    ~PanadapterRenderer();

    void initialize(QOpenGLShaderProgram* sharedShader = nullptr);
    void render(const QMatrix4x4& projection,
                const QRect& panRect, 
                const QVector<qreal>& bins, 
                qreal dBmMax, qreal dBmMin, 
                PanGraphicsMode mode, 
                float scaleMult, float dpr,
                int parentHeight,
                const Colors& colors,
                QSDR::_DataEngineState dataEngineState,
                bool isCurrentReceiver);

private:
    struct VertexData {
        float x, y, z;
        float r, g, b, a;
    };

    QOpenGLShaderProgram* m_shader;
    bool m_ownsShader;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
    int m_vboSize; // current allocated size in bytes
    QVarLengthArray<VertexData> m_vertexCache;
};

#endif // PANADAPTERRENDERER_H
