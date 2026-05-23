#ifndef CUSDR_GLSHADERS_H
#define CUSDR_GLSHADERS_H

#include <QOpenGLContext>
#include <QString>

namespace GlShaders {

inline bool isOpenGLES()
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    return ctx && ctx->isOpenGLES();
}

inline const char *coloredVertexSource()
{
    if (isOpenGLES()) {
        return
            "attribute vec3 position;\n"
            "attribute vec4 color;\n"
            "varying vec4 vertColor;\n"
            "uniform mat4 matrix;\n"
            "void main() {\n"
            "   vertColor = color;\n"
            "   gl_Position = matrix * vec4(position, 1.0);\n"
            "}\n";
    }
    return
        "#version 150\n"
        "in vec3 position;\n"
        "in vec4 color;\n"
        "out vec4 vertColor;\n"
        "uniform mat4 matrix;\n"
        "void main() {\n"
        "   vertColor = color;\n"
        "   gl_Position = matrix * vec4(position, 1.0);\n"
        "}\n";
}

inline const char *coloredFragmentSource()
{
    if (isOpenGLES()) {
        return
            "precision mediump float;\n"
            "varying vec4 vertColor;\n"
            "void main() {\n"
            "   gl_FragColor = vertColor;\n"
            "}\n";
    }
    return
        "#version 150\n"
        "in vec4 vertColor;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "   fragColor = vertColor;\n"
        "}\n";
}

inline const char *texturedVertexSource()
{
    if (isOpenGLES()) {
        return
            "attribute vec3 position;\n"
            "attribute vec2 texCoord;\n"
            "varying vec2 v_texCoord;\n"
            "uniform mat4 matrix;\n"
            "void main() {\n"
            "   v_texCoord = texCoord;\n"
            "   gl_Position = matrix * vec4(position, 1.0);\n"
            "}\n";
    }
    return
        "#version 150\n"
        "in vec3 position;\n"
        "in vec2 texCoord;\n"
        "out vec2 v_texCoord;\n"
        "uniform mat4 matrix;\n"
        "void main() {\n"
        "   v_texCoord = texCoord;\n"
        "   gl_Position = matrix * vec4(position, 1.0);\n"
        "}\n";
}

inline const char *coloredVertexSourceVec3()
{
    if (isOpenGLES()) {
        return
            "attribute vec3 position;\n"
            "attribute vec3 color;\n"
            "varying vec3 vertColor;\n"
            "uniform mat4 matrix;\n"
            "void main() {\n"
            "   vertColor = color;\n"
            "   gl_Position = matrix * vec4(position, 1.0);\n"
            "}\n";
    }
    return
        "#version 150\n"
        "in vec3 position;\n"
        "in vec3 color;\n"
        "out vec3 vertColor;\n"
        "uniform mat4 matrix;\n"
        "void main() {\n"
        "   vertColor = color;\n"
        "   gl_Position = matrix * vec4(position, 1.0);\n"
        "}\n";
}

inline const char *coloredFragmentSourceVec3()
{
    if (isOpenGLES()) {
        return
            "precision mediump float;\n"
            "varying vec3 vertColor;\n"
            "void main() {\n"
            "   gl_FragColor = vec4(vertColor, 1.0);\n"
            "}\n";
    }
    return
        "#version 150\n"
        "in vec3 vertColor;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "   fragColor = vec4(vertColor, 1.0);\n"
        "}\n";
}

inline const char *texturedQuadVertexSource()
{
    if (isOpenGLES()) {
        return
            "attribute vec2 position;\n"
            "attribute vec2 texCoord;\n"
            "varying vec2 v_texCoord;\n"
            "uniform mat4 matrix;\n"
            "void main() {\n"
            "   v_texCoord = texCoord;\n"
            "   gl_Position = matrix * vec4(position, 0.0, 1.0);\n"
            "}\n";
    }
    return
        "#version 150\n"
        "in vec2 position;\n"
        "in vec2 texCoord;\n"
        "out vec2 v_texCoord;\n"
        "uniform mat4 matrix;\n"
        "void main() {\n"
        "   v_texCoord = texCoord;\n"
        "   gl_Position = matrix * vec4(position, 0.0, 1.0);\n"
        "}\n";
}

// Wideband panel (a_pos / a_color / u_mvp)
inline const char *widebandVertexSource()
{
    if (isOpenGLES()) {
        return
            "attribute vec3 a_pos;\n"
            "attribute vec3 a_color;\n"
            "uniform mat4 u_mvp;\n"
            "varying vec3 v_color;\n"
            "void main() {\n"
            "   gl_Position = u_mvp * vec4(a_pos, 1.0);\n"
            "   v_color = a_color;\n"
            "}\n";
    }
    return
        "#version 130\n"
        "in vec3 a_pos;\n"
        "in vec3 a_color;\n"
        "uniform mat4 u_mvp;\n"
        "out vec3 v_color;\n"
        "void main() {\n"
        "   gl_Position = u_mvp * vec4(a_pos, 1.0);\n"
        "   v_color = a_color;\n"
        "}\n";
}

inline const char *widebandFragmentSource()
{
    if (isOpenGLES()) {
        return
            "precision mediump float;\n"
            "varying vec3 v_color;\n"
            "void main() {\n"
            "   gl_FragColor = vec4(v_color, 1.0);\n"
            "}\n";
    }
    return
        "#version 130\n"
        "in vec3 v_color;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "   fragColor = vec4(v_color, 1.0);\n"
        "}\n";
}

inline QString waterfallFragmentSource(const char *samplerName)
{
    if (isOpenGLES()) {
        return QString(
            "precision mediump float;\n"
            "varying vec2 v_texCoord;\n"
            "uniform sampler2D %1;\n"
            "void main() {\n"
            "   gl_FragColor = texture2D(%1, v_texCoord);\n"
            "}\n").arg(samplerName);
    }
    return QString(
        "#version 150\n"
        "in vec2 v_texCoord;\n"
        "out vec4 fragColor;\n"
        "uniform sampler2D %1;\n"
        "void main() {\n"
        "   fragColor = texture(%1, v_texCoord);\n"
        "}\n").arg(samplerName);
}

inline QString texturedFragmentSource(const char *samplerName)
{
    if (isOpenGLES()) {
        return QString(
            "precision mediump float;\n"
            "varying vec2 v_texCoord;\n"
            "uniform sampler2D %1;\n"
            "uniform vec4 textColor;\n"
            "void main() {\n"
            "   gl_FragColor = texture2D(%1, v_texCoord) * textColor;\n"
            "}\n").arg(samplerName);
    }
    return QString(
        "#version 150\n"
        "in vec2 v_texCoord;\n"
        "out vec4 fragColor;\n"
        "uniform sampler2D %1;\n"
        "uniform vec4 textColor;\n"
        "void main() {\n"
        "   fragColor = texture(%1, v_texCoord) * textColor;\n"
        "}\n").arg(samplerName);
}

} // namespace GlShaders

#endif // CUSDR_GLSHADERS_H
