/**
* @file  cusdr_oglText.cpp
* @brief OpenGL Text generation class for cuSDR
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2012-02-18
*/

/*
 *	 adapted from the MIFit project: http://code.google.com/p/mifit
 *
 *   Copyright 2012 adapted for cuSDR by Hermann von Hasseln, DL3HVH
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "cusdr_oglText.h"
#include "cusdr_glShaders.h"

#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

const int TEXTURE_SIZE = 1024;
	
struct CharData {
    GLuint textureId = 0;
    uint pixWidth = 0;
    uint pixHeight = 0;
    uint advance = 0;
    GLfloat s[2] = { 0, 0 };
    GLfloat t[2] = { 0, 0 };
};

struct OGLTextPrivate {

    OGLTextPrivate(const QFont &f, qreal devicePixelRatio);
    ~OGLTextPrivate();

    void allocateTexture();
    CharData &createCharacter(QChar c);

    QFont font;
    QFontMetrics fontMetrics;
    qreal dpr;

    QHash<ushort, CharData> characters;
    QList<GLuint> textures;

    GLint xOffset;
    GLint yOffset;

    QOpenGLShaderProgram *textProgram = nullptr;
    QOpenGLBuffer textVbo;

    bool ensureTextProgram();
    void renderTextProjected(const QMatrix4x4 &projection, float x, float y, float z,
                             const QString &text, const QColor &color);
    QMatrix4x4 orthoForCurrentViewport() const;
};

OGLTextPrivate::OGLTextPrivate(const QFont &f, qreal devicePixelRatio)
    : font(f), fontMetrics(f), dpr(devicePixelRatio), xOffset(0), yOffset(0) {
    // Note: DPR is stored for potential future use
}

OGLTextPrivate::~OGLTextPrivate() {
	
	foreach (GLuint texture, textures)
		glDeleteTextures(1, &texture);

    delete textProgram;
    if (textVbo.isCreated())
        textVbo.destroy();
}

bool OGLTextPrivate::ensureTextProgram()
{
    if (textProgram && textProgram->isLinked())
        return true;

    if (!textProgram)
        textProgram = new QOpenGLShaderProgram();

    textProgram->removeAllShaders();
    textProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, GlShaders::texturedQuadVertexSource());
    textProgram->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                       GlShaders::texturedFragmentSource("tex").toUtf8());
    textProgram->bindAttributeLocation("position", 0);
    textProgram->bindAttributeLocation("texCoord", 1);
    if (!textProgram->link())
        return false;

    if (!textVbo.isCreated()) {
        textVbo.create();
        textVbo.setUsagePattern(QOpenGLBuffer::StreamDraw);
    }
    return true;
}

namespace {

void orthoExtents(const QMatrix4x4 &projection, float &width, float &height)
{
    const float m00 = projection(0, 0);
    const float m11 = projection(1, 1);
    if (qFuzzyIsNull(m00) || qFuzzyIsNull(m11)) {
        width = 1.0f;
        height = 1.0f;
        return;
    }
    width = 2.0f / m00;
    height = 2.0f / qAbs(m11);
}

} // namespace

QMatrix4x4 OGLTextPrivate::orthoForCurrentViewport() const
{
    GLint vp[4] = { 0, 0, 0, 0 };
    glGetIntegerv(GL_VIEWPORT, vp);
    const qreal w = (dpr > 0) ? qreal(vp[2]) / dpr : qreal(vp[2]);
    const qreal h = (dpr > 0) ? qreal(vp[3]) / dpr : qreal(vp[3]);
    QMatrix4x4 projection;
    projection.ortho(0.0f, float(w), float(h), 0.0f, -5.0f, 5.0f);
    return projection;
}

void OGLTextPrivate::renderTextProjected(const QMatrix4x4 &projection,
                                         float x,
                                         float y,
                                         float /*z*/,
                                         const QString &text,
                                         const QColor &color)
{
    if (!ensureTextProgram())
        return;

    QOpenGLFunctions *gl = QOpenGLContext::currentContext()->functions();
    if (!gl)
        return;

    GLint vp[4] = { 0, 0, 0, 0 };
    gl->glGetIntegerv(GL_VIEWPORT, vp);

    float logicalW = 1.0f;
    float logicalH = 1.0f;
    orthoExtents(projection, logicalW, logicalH);

    const float scaleX = (logicalW > 0.0f) ? float(vp[2]) / logicalW : 1.0f;
    const float scaleY = (logicalH > 0.0f) ? float(vp[3]) / logicalH : 1.0f;

    QMatrix4x4 pixelProjection;
    pixelProjection.ortho(0.0f, float(vp[2]), float(vp[3]), 0.0f, -1.0f, 1.0f);

    struct GlyphVertex {
        float px, py;
        float u, v;
    };

    gl->glEnable(GL_BLEND);
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl->glDisable(GL_DEPTH_TEST);

    textProgram->bind();
    const int matrixLoc = textProgram->uniformLocation("matrix");
    if (matrixLoc >= 0)
        textProgram->setUniformValue(matrixLoc, pixelProjection);
    const int texLoc = textProgram->uniformLocation("tex");
    if (texLoc >= 0)
        textProgram->setUniformValue(texLoc, 0);
    const int colorLoc = textProgram->uniformLocation("textColor");
    if (colorLoc >= 0)
        textProgram->setUniformValue(colorLoc, color);

    GLuint texture = 0;
    float penX = x * scaleX;
    const float topY = y * scaleY;

    for (int i = 0; i < text.length(); ++i) {
        CharData &c = createCharacter(text.at(i));
        if (texture != c.textureId) {
            texture = c.textureId;
            gl->glActiveTexture(GL_TEXTURE0);
            gl->glBindTexture(GL_TEXTURE_2D, texture);
        }

        const float glyphW = float(c.pixWidth);
        const float glyphH = float(c.pixHeight);
        const float bottomY = topY + glyphH;

        const GlyphVertex quad[6] = {
            { penX, bottomY, c.s[0], c.t[0] },
            { penX + glyphW, bottomY, c.s[1], c.t[0] },
            { penX + glyphW, topY, c.s[1], c.t[1] },
            { penX, bottomY, c.s[0], c.t[0] },
            { penX + glyphW, topY, c.s[1], c.t[1] },
            { penX, topY, c.s[0], c.t[1] },
        };

        textVbo.bind();
        textVbo.allocate(quad, int(sizeof(quad)));

        const int posAttr = textProgram->attributeLocation("position");
        const int texAttr = textProgram->attributeLocation("texCoord");
        const GLsizei stride = GLsizei(sizeof(GlyphVertex));
        if (posAttr >= 0) {
            gl->glEnableVertexAttribArray(GLuint(posAttr));
            gl->glVertexAttribPointer(GLuint(posAttr), 2, GL_FLOAT, GL_FALSE, stride,
                                      reinterpret_cast<const void *>(0));
        }
        if (texAttr >= 0) {
            gl->glEnableVertexAttribArray(GLuint(texAttr));
            gl->glVertexAttribPointer(GLuint(texAttr), 2, GL_FLOAT, GL_FALSE, stride,
                                      reinterpret_cast<const void *>(2 * sizeof(float)));
        }

        gl->glDrawArrays(GL_TRIANGLES, 0, 6);

        if (posAttr >= 0)
            gl->glDisableVertexAttribArray(GLuint(posAttr));
        if (texAttr >= 0)
            gl->glDisableVertexAttribArray(GLuint(texAttr));
        textVbo.release();

        penX += float(c.advance) * scaleX;
    }

    gl->glBindTexture(GL_TEXTURE_2D, 0);
    textProgram->release();
}

void OGLTextPrivate::allocateTexture() {
	

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // the texture ends at the edges (clamp)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // select modulate to mix texture with color for shading
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    QImage image(TEXTURE_SIZE, TEXTURE_SIZE, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXTURE_SIZE, TEXTURE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());

    textures += texture;
}

CharData &OGLTextPrivate::createCharacter(QChar c) {
	
    ushort unicodeC = c.unicode();
        if (characters.contains(unicodeC))
            return characters[unicodeC];

        if (textures.empty())
            allocateTexture();

        GLuint texture = textures.last();

        QFont renderFont = font;
        const qreal scale = (dpr > 1.0) ? dpr : 1.0;
        if (scale > 1.0) {
            if (renderFont.pointSizeF() > 0)
                renderFont.setPointSizeF(renderFont.pointSizeF() * scale);
            else
                renderFont.setPixelSize(qMax(1, qRound(renderFont.pixelSize() * scale)));
        }
        const QFontMetrics renderFm(renderFont);

        const int texWidth = qMax(1, renderFm.horizontalAdvance(c));
        const int texHeight = qMax(1, renderFm.height());

        // Save OpenGL state before QPainter operations
        GLint oldTexture;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTexture);
        const bool textureEnabled = glIsEnabled(GL_TEXTURE_2D);

        // QImage (not QPixmap) avoids HiDPI devicePixelRatio stretching the upload.
        QImage image(texWidth, texHeight, QImage::Format_ARGB32);
        image.fill(Qt::transparent);

        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setFont(renderFont);
        painter.setPen(Qt::white);
        painter.drawText(image.rect(), Qt::TextSingleLine | Qt::TextDontClip | Qt::AlignCenter, c);
        painter.end();

        if (textureEnabled)
            glEnable(GL_TEXTURE_2D);
        else
            glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture);

        QImage glImage = image.convertToFormat(QImage::Format_RGBA8888).flipped();
        const int uploadW = glImage.width();
        const int uploadH = glImage.height();
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glTexSubImage2D(GL_TEXTURE_2D, 0, xOffset, yOffset, uploadW, uploadH,
                        GL_RGBA, GL_UNSIGNED_BYTE, glImage.constBits());

        glBindTexture(GL_TEXTURE_2D, oldTexture);

        CharData& character = characters[unicodeC];
        character.textureId = texture;
        character.pixWidth = uint(uploadW);
        character.pixHeight = uint(uploadH);
        character.advance = uint(qMax(1, qRound(renderFm.horizontalAdvance(c) / scale)));
        character.s[0] = static_cast<GLfloat>(xOffset) / TEXTURE_SIZE;
        character.t[0] = static_cast<GLfloat>(yOffset) / TEXTURE_SIZE;
        character.s[1] = static_cast<GLfloat>(xOffset + uploadW) / TEXTURE_SIZE;
        character.t[1] = static_cast<GLfloat>(yOffset + uploadH) / TEXTURE_SIZE;

        xOffset += uploadW;
        if (xOffset + renderFm.maxWidth() >= TEXTURE_SIZE) {
            xOffset = 1;
            yOffset += uploadH;
        }
        if (yOffset + renderFm.height() >= TEXTURE_SIZE) {
            allocateTexture();
            yOffset = 1;
        }
        return character;
}



OGLText::OGLText(const QFont &f, qreal devicePixelRatio) : d(new OGLTextPrivate(f, devicePixelRatio)) {}


OGLText::~OGLText() {

    delete d;
}

void OGLText::invalidateCache()
{
    d->characters.clear();
    d->xOffset = 0;
    d->yOffset = 0;
}

void OGLText::setDevicePixelRatio(qreal devicePixelRatio)
{
    if (qFuzzyCompare(d->dpr, devicePixelRatio))
        return;
    d->dpr = devicePixelRatio;
    invalidateCache();
}

QFont OGLText::font() const
{
    return d->font;
}

QFontMetrics OGLText::fontMetrics() const {

    return d->fontMetrics;
}

void OGLText::renderText(const QMatrix4x4 &projection, float x, float y, const QString &text,
                         const QColor &color)
{
    d->renderTextProjected(projection, x, y, 0.0f, text, color);
}

void OGLText::renderText(const QMatrix4x4 &projection, float x, float y, float z, const QString &text,
                         const QColor &color)
{
    d->renderTextProjected(projection, x, y, z, text, color);
}

void OGLText::renderText(float x, float y, const QString &text, const QColor &color) {
    d->renderTextProjected(d->orthoForCurrentViewport(), x, y, 0.0f, text, color);
}

void OGLText::renderText(float x, float y, float z, const QString &text, const QColor &color) {
    d->renderTextProjected(d->orthoForCurrentViewport(), x, y, z, text, color);
}

