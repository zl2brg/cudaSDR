#ifndef CUSDR_GLDRAW_H
#define CUSDR_GLDRAW_H

#include "cusdr_glShaders.h"

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QRect>
#include <QColor>

namespace GlDraw {

struct Vec3Rgb {
    float x = 0, y = 0, z = 0;
    float r = 0, g = 0, b = 0;
};

struct Vec3Rgba {
    float x = 0, y = 0, z = 0;
    float r = 0, g = 0, b = 0, a = 1.0f;
};

inline void bindVec3ColorAttribs(QOpenGLFunctions *gl, QOpenGLShaderProgram *prog, GLsizei stride)
{
    int pos = prog->attributeLocation("position");
    if (pos < 0) pos = prog->attributeLocation("a_pos");
    int col = prog->attributeLocation("color");
    if (col < 0) col = prog->attributeLocation("a_color");

    if (pos >= 0) {
        gl->glEnableVertexAttribArray(GLuint(pos));
        gl->glVertexAttribPointer(GLuint(pos), 3, GL_FLOAT, GL_FALSE, stride,
                                  reinterpret_cast<const void *>(0));
    }
    if (col >= 0) {
        gl->glEnableVertexAttribArray(GLuint(col));
        gl->glVertexAttribPointer(GLuint(col), 3, GL_FLOAT, GL_FALSE, stride,
                                  reinterpret_cast<const void *>(3 * sizeof(float)));
    }
}

inline void bindVec3ColorRgbaAttribs(QOpenGLFunctions *gl, QOpenGLShaderProgram *prog, GLsizei stride)
{
    int pos = prog->attributeLocation("position");
    if (pos < 0) pos = prog->attributeLocation("a_pos");
    int col = prog->attributeLocation("color");
    if (col < 0) col = prog->attributeLocation("a_color");

    if (pos >= 0) {
        gl->glEnableVertexAttribArray(GLuint(pos));
        gl->glVertexAttribPointer(GLuint(pos), 3, GL_FLOAT, GL_FALSE, stride,
                                  reinterpret_cast<const void *>(0));
    }
    if (col >= 0) {
        gl->glEnableVertexAttribArray(GLuint(col));
        gl->glVertexAttribPointer(GLuint(col), 4, GL_FLOAT, GL_FALSE, stride,
                                  reinterpret_cast<const void *>(3 * sizeof(float)));
    }
}

inline void unbindVec3ColorAttribs(QOpenGLFunctions *gl, QOpenGLShaderProgram *prog)
{
    int pos = prog->attributeLocation("position");
    if (pos < 0) pos = prog->attributeLocation("a_pos");
    int col = prog->attributeLocation("color");
    if (col < 0) col = prog->attributeLocation("a_color");

    if (pos >= 0)
        gl->glDisableVertexAttribArray(GLuint(pos));
    if (col >= 0)
        gl->glDisableVertexAttribArray(GLuint(col));
}

inline void drawColoredLines(QOpenGLFunctions *gl,
                             QOpenGLShaderProgram *prog,
                             QOpenGLBuffer &vbo,
                             const QMatrix4x4 &mvp,
                             const Vec3Rgb *verts,
                             int vertexCount)
{
    if (!gl || !prog || !prog->isLinked() || vertexCount < 2)
        return;

    prog->bind();
    int matrixLoc = prog->uniformLocation("matrix");
    if (matrixLoc < 0) matrixLoc = prog->uniformLocation("u_mvp");
    if (matrixLoc >= 0)
        prog->setUniformValue(matrixLoc, mvp);

    vbo.bind();
    vbo.allocate(verts, vertexCount * int(sizeof(Vec3Rgb)));

    const GLsizei stride = GLsizei(sizeof(Vec3Rgb));
    bindVec3ColorAttribs(gl, prog, stride);
    gl->glDrawArrays(GL_LINES, 0, vertexCount);
    unbindVec3ColorAttribs(gl, prog);
    vbo.release();
    prog->release();
}

inline void drawColoredRgbaLines(QOpenGLFunctions *gl,
                                 QOpenGLShaderProgram *prog,
                                 QOpenGLBuffer &vbo,
                                 const QMatrix4x4 &mvp,
                                 const Vec3Rgba *verts,
                                 int vertexCount)
{
    if (!gl || !prog || !prog->isLinked() || vertexCount < 2)
        return;

    prog->bind();
    int matrixLoc = prog->uniformLocation("matrix");
    if (matrixLoc < 0) matrixLoc = prog->uniformLocation("u_mvp");
    if (matrixLoc >= 0)
        prog->setUniformValue(matrixLoc, mvp);

    vbo.bind();
    vbo.allocate(verts, vertexCount * int(sizeof(Vec3Rgba)));

    const GLsizei stride = GLsizei(sizeof(Vec3Rgba));
    bindVec3ColorRgbaAttribs(gl, prog, stride);
    gl->glDrawArrays(GL_LINES, 0, vertexCount);
    unbindVec3ColorAttribs(gl, prog);
    vbo.release();
    prog->release();
}

inline void drawSolidRect(QOpenGLFunctions *gl,
                          QOpenGLShaderProgram *prog,
                          QOpenGLBuffer &vbo,
                          const QMatrix4x4 &mvp,
                          const QRect &rect,
                          const QColor &color,
                          float z = 0.0f)
{
    if (rect.isEmpty())
        return;

    const float x1 = float(rect.left());
    const float y1 = float(rect.top());
    const float x2 = float(rect.left() + rect.width());
    const float y2 = float(rect.top() + rect.height());
    const float r = color.redF(), g = color.greenF(), b = color.blueF();

    const Vec3Rgb quad[4] = {
        { x1, y1, z, r, g, b },
        { x2, y1, z, r, g, b },
        { x1, y2, z, r, g, b },
        { x2, y2, z, r, g, b },
    };

    if (!gl || !prog || !prog->isLinked())
        return;

    prog->bind();
    int matrixLoc = prog->uniformLocation("matrix");
    if (matrixLoc < 0) matrixLoc = prog->uniformLocation("u_mvp");
    if (matrixLoc >= 0)
        prog->setUniformValue(matrixLoc, mvp);

    vbo.bind();
    vbo.allocate(quad, int(sizeof(quad)));

    const GLsizei stride = GLsizei(sizeof(Vec3Rgb));
    bindVec3ColorAttribs(gl, prog, stride);
    gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    unbindVec3ColorAttribs(gl, prog);
    vbo.release();
    prog->release();
}

inline void drawGradientRect(QOpenGLFunctions *gl,
                             QOpenGLShaderProgram *prog,
                             QOpenGLBuffer &vbo,
                             const QMatrix4x4 &mvp,
                             const QRect &rect,
                             const QColor &c1,
                             const QColor &c2,
                             bool leftToRight,
                             float z = 0.0f)
{
    if (rect.isEmpty())
        return;

    const float x1 = float(rect.left());
    const float y1 = float(rect.top());
    const float x2 = float(rect.right() + 1);
    const float y2 = float(rect.bottom() + 1);

    const float r1 = c1.redF(), g1 = c1.greenF(), b1 = c1.blueF();
    const float r2 = c2.redF(), g2 = c2.greenF(), b2 = c2.blueF();

    Vec3Rgb quad[4];
    if (leftToRight) {
        quad[0] = { x1, y1, z, r1, g1, b1 };
        quad[1] = { x2, y1, z, r2, g2, b2 };
        quad[2] = { x1, y2, z, r1, g1, b1 };
        quad[3] = { x2, y2, z, r2, g2, b2 };
    } else {
        quad[0] = { x1, y1, z, r1, g1, b1 };
        quad[1] = { x2, y1, z, r1, g1, b1 };
        quad[2] = { x1, y2, z, r2, g2, b2 };
        quad[3] = { x2, y2, z, r2, g2, b2 };
    }

    if (!gl || !prog || !prog->isLinked())
        return;

    prog->bind();
    int matrixLoc = prog->uniformLocation("matrix");
    if (matrixLoc < 0) matrixLoc = prog->uniformLocation("u_mvp");
    if (matrixLoc >= 0)
        prog->setUniformValue(matrixLoc, mvp);

    vbo.bind();
    vbo.allocate(quad, int(sizeof(quad)));

    const GLsizei stride = GLsizei(sizeof(Vec3Rgb));
    bindVec3ColorAttribs(gl, prog, stride);
    gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    unbindVec3ColorAttribs(gl, prog);
    vbo.release();
    prog->release();
}

inline void renderTexturedQuad(QOpenGLFunctions *gl,
                               QOpenGLShaderProgram *prog,
                               QOpenGLBuffer &vbo,
                               const QMatrix4x4 &mvp,
                               const QRect &rect,
                               GLuint texId,
                               float z = 0.0f)
{
    if (!gl || !prog || !prog->isLinked() || rect.isEmpty() || !texId)
        return;

    gl->glEnable(GL_BLEND);
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    struct Vertex {
        float x, y;
        float u, v;
    };

    const float x1 = float(rect.left());
    const float y1 = float(rect.top());
    const float x2 = float(rect.left() + rect.width());
    const float y2 = float(rect.top() + rect.height());

    const Vertex quad[4] = {
        { x1, y1, 0.0f, 1.0f },
        { x2, y1, 1.0f, 1.0f },
        { x1, y2, 0.0f, 0.0f },
        { x2, y2, 1.0f, 0.0f },
    };

    prog->bind();
    int matrixLoc = prog->uniformLocation("matrix");
    if (matrixLoc < 0) matrixLoc = prog->uniformLocation("u_mvp");
    if (matrixLoc >= 0)
        prog->setUniformValue(matrixLoc, mvp);

    vbo.bind();
    vbo.allocate(quad, int(sizeof(quad)));

    int posLoc = prog->attributeLocation("position");
    if (posLoc < 0) posLoc = prog->attributeLocation("a_pos");
    int texLoc = prog->attributeLocation("texCoord");
    if (texLoc < 0) texLoc = prog->attributeLocation("a_texCoord");
    if (texLoc < 0) texLoc = prog->attributeLocation("texCoord");

    const GLsizei stride = GLsizei(sizeof(Vertex));
    if (posLoc >= 0) {
        gl->glEnableVertexAttribArray(GLuint(posLoc));
        gl->glVertexAttribPointer(GLuint(posLoc), 2, GL_FLOAT, GL_FALSE, stride,
                                  reinterpret_cast<const void *>(0));
    }
    if (texLoc >= 0) {
        gl->glEnableVertexAttribArray(GLuint(texLoc));
        gl->glVertexAttribPointer(GLuint(texLoc), 2, GL_FLOAT, GL_FALSE, stride,
                                  reinterpret_cast<const void *>(2 * sizeof(float)));
    }

    gl->glActiveTexture(GL_TEXTURE0);
    gl->glBindTexture(GL_TEXTURE_2D, texId);
    const int texUniform = prog->uniformLocation("tex");
    if (texUniform >= 0)
        prog->setUniformValue(texUniform, 0);
    const int colorLoc = prog->uniformLocation("textColor");
    if (colorLoc >= 0)
        prog->setUniformValue(colorLoc, QColor(255, 255, 255));

    gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if (posLoc >= 0)
        gl->glDisableVertexAttribArray(GLuint(posLoc));
    if (texLoc >= 0)
        gl->glDisableVertexAttribArray(GLuint(texLoc));
    gl->glBindTexture(GL_TEXTURE_2D, 0);
    vbo.release();
    prog->release();
}

} // namespace GlDraw

#endif // CUSDR_GLDRAW_H
