#include "OverlayRenderer.h"
#include <QVarLengthArray>

OverlayRenderer::OverlayRenderer()
    : m_shader(nullptr)
    , m_ownsShader(false)
    , m_vbo(QOpenGLBuffer::VertexBuffer)
{
}

OverlayRenderer::~OverlayRenderer() {
    if (m_vao.isCreated()) m_vao.destroy();
    if (m_vbo.isCreated()) m_vbo.destroy();
    if (m_ownsShader && m_shader) delete m_shader;
}

void OverlayRenderer::initialize(QOpenGLShaderProgram* sharedShader) {
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

void OverlayRenderer::drawGrid(const QMatrix4x4& projection,
                               const QRect& panRect,
                               const QRect& dBmScalePanRect,
                               const TScale& freqScale,
                               const TScale& dBmScale,
                               float r, float g, float b, float alpha,
                               bool panGridEnabled) {
    if (!panGridEnabled) return;
    if (!m_shader || !m_shader->isLinked()) return;

    glDisable(GL_MULTISAMPLE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0x5555);
    glLineWidth(1.0f);
    glDisable(GL_DEPTH_TEST);

    m_shader->bind();
    m_shader->setUniformValue("matrix", projection);

    m_vao.bind();
    m_vbo.bind();

    QList<VertexData> gridData;
    
    int len = freqScale.mainPointPositions.length();
    for (int i = 0; i < len; i++) {
        float x = (float)freqScale.mainPointPositions.at(i);
        gridData.append({ x, (float)panRect.top(),    3.0f, r, g, b, alpha });
        gridData.append({ x, (float)panRect.bottom(), 3.0f, r, g, b, alpha });
    }
    
    len = dBmScale.mainPointPositions.length();
    for (int i = 0; i < len; i++) {
        float y = (float)dBmScale.mainPointPositions.at(i);
        gridData.append({ (float)panRect.left(),  y, 3.0f, r, g, b, alpha });
        gridData.append({ (float)panRect.right(), y, 3.0f, r, g, b, alpha });
    }
    
    if (!gridData.isEmpty()) {
        m_vbo.allocate(gridData.data(), gridData.size() * (int)sizeof(VertexData));
        m_shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 7);
        m_shader->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 4, sizeof(float) * 7);
        glDrawArrays(GL_LINES, 0, gridData.size());
    }

    m_vao.release();
    m_shader->release();
    glDisable(GL_LINE_STIPPLE);
    glDisable(GL_BLEND);
	glEnable(GL_MULTISAMPLE);
}

void OverlayRenderer::drawCenterLine(const QMatrix4x4& projection,
                                     const QRect& panRect,
                                     const QRect& freqScalePanRect,
                                     const QRect& waterfallRect,
                                     int centerlineHeight,
                                     float deltaF,
                                     float zoomFactor,
                                     const QColor& vfoColor,
                                     bool dragMouse,
                                     bool panLocked) {

	float y1 = (float)panRect.top();
    float y2 = (float)centerlineHeight;
	
	if (y2 > y1 + 3) {
        if (!m_shader || !m_shader->isLinked()) return;

        glDisable(GL_DEPTH_TEST);
        m_shader->bind();
        m_shader->setUniformValue("matrix", projection);

        m_vao.bind();
        m_vbo.bind();

        float centerX = (float)panRect.width()/2.0f;
        float centerY = (float)(panRect.top() + panRect.height() - 1);
		QColor centerCol = QColor(80, 180, 240, 180);

		glDisable(GL_MULTISAMPLE);
        glLineWidth(3.0f);

        QVarLengthArray<VertexData, 8> lines;
        float cr = centerCol.redF(); float cg = centerCol.greenF(); float cb = centerCol.blueF(); float ca = centerCol.alphaF();

        lines.append({ centerX, y1 + 1.0f, 3.5f, cr, cg, cb, ca });
        lines.append({ centerX, centerY - 1.0f,  3.5f, cr, cg, cb, ca });
			
		float vfoX = (float)(panRect.left() + qRound((qreal)(panRect.width()/2.0f)  - deltaF * panRect.width() / zoomFactor));
        float vr = vfoColor.redF(); float vg = vfoColor.greenF(); float vb = vfoColor.blueF(); float va = 1.0f;

        if (!qIsNaN(vfoX) && !qIsInf(vfoX)) {
            if (dragMouse && !panLocked) {
                lines.append({ vfoX, (float)freqScalePanRect.bottom() + 1.0f, 3.0f, vr, vg, vb, va });
                lines.append({ vfoX, (float)(freqScalePanRect.bottom() + waterfallRect.height() - 1), 3.0f, vr, vg, vb, va });
            }
            lines.append({ vfoX, y1 + 1.0f, 4.0f, vr, vg, vb, va });
            lines.append({ vfoX, centerY - 1.0f,  4.0f, vr, vg, vb, va });
        }

        if (lines.size() >= 2) {
            m_vbo.allocate(lines.data(), (int)(lines.size() * sizeof(VertexData)));
            m_shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 7);
            m_shader->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 4, sizeof(float) * 7);
            glDrawArrays(GL_LINES, 0, lines.size());
        }

        m_vao.release();
        m_shader->release();
		glEnable(GL_MULTISAMPLE);
	}
}

void OverlayRenderer::drawFilter(const QMatrix4x4& projection,
                                 const QRect& panRect,
                                 float filterLo, float filterHi,
                                 float deltaF, float zoomFactor,
                                 bool highlightFilter,
                                 bool showLeftBoundary, bool showRightBoundary,
                                 int& filterLeft, int& filterRight,
                                 int& filterTop, int& filterBottom) {

    if (!m_shader || !m_shader->isLinked()) return;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

	QColor color;
	if (highlightFilter)
		color = QColor(150, 150, 150, 140);
	else
		color = QColor(150, 150, 150, 100);

	filterLeft = panRect.left() + qRound((qreal)(panRect.width()/2.0f) + (filterLo - deltaF) * panRect.width() / zoomFactor);
	filterRight = panRect.left() + qRound((qreal)(panRect.width()/2.0f) + (filterHi - deltaF) * panRect.width() / zoomFactor);
	filterTop = panRect.top() + 1;
	filterBottom = panRect.top() + panRect.height() - 1;
	
    QRect filterRect(filterLeft, filterTop, filterRight - filterLeft, filterBottom - filterTop);

    m_shader->bind();
    m_shader->setUniformValue("matrix", projection);

    m_vao.bind();
    m_vbo.bind();

	if ((filterLeft >= panRect.left() && filterLeft <= panRect.right()) ||
		(filterRight >= panRect.left() && filterRight <= panRect.right()) ||
		(filterLeft < panRect.left() && filterRight > panRect.right()))
	{
		if (filterRect.height() > 5) {
            float fr = color.redF(), fg = color.greenF(), fb = color.blueF(), fa = color.alphaF() * 0.4f;
            QVarLengthArray<VertexData, 4> rectData;
            float rx1 = (float)filterRect.left(), ry1 = (float)filterRect.top(), rx2 = (float)filterRect.right(), ry2 = (float)filterRect.bottom();
            
            rectData.append({ rx1, ry1, 4.0f, fr, fg, fb, fa });
            rectData.append({ rx2, ry1, 4.0f, fr, fg, fb, fa });
            rectData.append({ rx1, ry2, 4.0f, fr, fg, fb, fa });
            rectData.append({ rx2, ry2, 4.0f, fr, fg, fb, fa });

            m_vbo.allocate(rectData.data(), (int)(rectData.size() * sizeof(VertexData)));
            m_shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 7);
            m_shader->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 4, sizeof(float) * 7);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
	}

    QVarLengthArray<VertexData, 4> lines;
	if (showLeftBoundary) {
		color = QColor(150, 150, 150, 230);
        float r = color.redF(), g = color.greenF(), b = color.blueF(), a = color.alphaF();
        lines.append({ (float)filterLeft, (float)filterTop,    5.0f, r, g, b, a });
        lines.append({ (float)filterLeft, (float)filterBottom, 5.0f, r, g, b, a });
	}

	if (showRightBoundary) {
		color = QColor(150, 150, 150, 230);
        float r = color.redF(), g = color.greenF(), b = color.blueF(), a = color.alphaF();
        lines.append({ (float)filterRight, (float)filterTop,    5.0f, r, g, b, a });
        lines.append({ (float)filterRight, (float)filterBottom, 5.0f, r, g, b, a });
	}

    if (!lines.isEmpty()) {
        glDisable(GL_MULTISAMPLE);
        glLineWidth(1);
        m_vbo.allocate(lines.data(), (int)(lines.size() * sizeof(VertexData)));
        m_shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 7);
        m_shader->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 4, sizeof(float) * 7);
        glDrawArrays(GL_LINES, 0, lines.size());
        glEnable(GL_MULTISAMPLE);
    }
    
    m_vao.release();
    m_shader->release();
}

void OverlayRenderer::drawCrossHair(const QMatrix4x4& projection,
                                    const QRect& panRect,
                                    const QRect& dBmScalePanRect,
                                    const QPoint& mousePos,
                                    float dpr,
                                    int parentHeight) {
    
    if (!m_shader || !m_shader->isLinked()) return;

	QRect rect(0, panRect.top(), panRect.width(), parentHeight - panRect.top());
	int x = mousePos.x();
	int y = mousePos.y();
	int crossHairSize = (int)(20 * dpr);

	glDisable(GL_MULTISAMPLE);
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
	glLineWidth(1.0f * dpr);
    glDisable(GL_DEPTH_TEST);

    m_shader->bind();
    m_shader->setUniformValue("matrix", projection);

	glScissor((int)(rect.left() * dpr), (int)((parentHeight - rect.bottom()) * dpr), (int)(rect.width() * dpr - 1), (int)(rect.height() * dpr));
	glEnable(GL_SCISSOR_TEST);

    m_vao.bind();
    m_vbo.bind();

    QVarLengthArray<VertexData, 8> lines;
    float r = 95/255.0f, g = 95/255.0f, b = 95/255.0f, a = 1.0f;

	// horizontal line
    lines.append({ (float)dBmScalePanRect.right() - 2.0f, (float)y, 4.0f, r, g, b, a });
    lines.append({ (float)rect.right() - 1.0f, (float)y, 4.0f, r, g, b, a });

	// vertical line
    lines.append({ (float)x, (float)rect.top() + 1.0f, 4.0f, r, g, b, a });
    lines.append({ (float)x, (float)rect.bottom() - 1.0f, 4.0f, r, g, b, a });

	// cross hair
    r = 180/255.0f; g = 180/255.0f; b = 180/255.0f;
    lines.append({ (float)x     , (float)y - (float)crossHairSize, 5.0f, r, g, b, a });
    lines.append({ (float)x     , (float)y + (float)crossHairSize, 5.0f, r, g, b, a });
    lines.append({ (float)x - (float)crossHairSize, (float)y, 5.0f, r, g, b, a });
    lines.append({ (float)x + (float)crossHairSize, (float)y, 5.0f, r, g, b, a });

    if (!lines.isEmpty()) {
        m_vbo.allocate(lines.data(), (int)(lines.size() * sizeof(VertexData)));
        m_shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 7);
        m_shader->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 4, sizeof(float) * 7);
        glDrawArrays(GL_LINES, 0, lines.size());
    }

    m_vao.release();
	glDisable(GL_SCISSOR_TEST);
    glEnable(GL_MULTISAMPLE);
    m_shader->release();
}

void OverlayRenderer::drawAGCControl(const QMatrix4x4& projection,
                                     const QRect& panRect,
                                     const QRect& dBmScalePanRect,
                                     AGCMode mode,
                                     bool hangEnabled,
                                     float agcThreshold,
                                     float agcHangLevel,
                                     float agcFixedGain,
                                     qreal dBmMax, qreal dBmMin,
                                     float dpr,
                                     int parentHeight,
                                     float& threshPixel, float& hangPixel, float& fixedPixel) {

    if (!m_shader || !m_shader->isLinked()) return;

	glDisable(GL_MULTISAMPLE);
	glLineStipple(1, 0x0C0C);
	glEnable(GL_LINE_STIPPLE);
	glLineWidth(1.0f);
    glDisable(GL_DEPTH_TEST);

    m_shader->bind();
    m_shader->setUniformValue("matrix", projection);

	glScissor((int)(panRect.left() * dpr), (int)((parentHeight - panRect.bottom()) * dpr), (int)(panRect.width() * dpr), (int)(panRect.height() * dpr));
	glEnable(GL_SCISSOR_TEST);

    m_vao.bind();
    m_vbo.bind();

    QVarLengthArray<VertexData, 10> lines;
    
	if (mode == (AGCMode) agcOFF) {
		fixedPixel = (float)dBmToGLPixel(panRect, dBmMax, dBmMin, -agcFixedGain);
        float r = 225/255.0f, g = 125/255.0f, b = 225/255.0f;
        lines.append({ (float)dBmScalePanRect.right() - 1.0f, fixedPixel + 2.0f, 4.0f, 0, 0, 0, 1.0f });
        lines.append({ (float)panRect.right() - 1.0f, fixedPixel, 4.0f, 0, 0, 0, 1.0f });
        lines.append({ (float)dBmScalePanRect.right() - 3.0f, fixedPixel, 5.0f, r, g, b, 1.0f });
        lines.append({ (float)panRect.right() - 1.0f, fixedPixel, 4.0f, r, g, b, 1.0f });
	} else {
		threshPixel = (float)dBmToGLPixel(panRect, dBmMax, dBmMin, agcThreshold);
        float r = 225/255.0f, g = 125/255.0f, b = 125/255.0f;
        lines.append({ (float)dBmScalePanRect.right() - 1.0f, threshPixel + 2.0f, 4.0f, 0, 0, 0, 1.0f });
        lines.append({ (float)panRect.right() - 1.0f, threshPixel, 4.0f, 0, 0, 0, 1.0f });
        lines.append({ (float)dBmScalePanRect.right() - 3.0f, threshPixel, 5.0f, r, g, b, 1.0f });
        lines.append({ (float)panRect.right() - 1.0f, threshPixel, 4.0f, r, g, b, 1.0f });

		if (hangEnabled) {
			hangPixel = (float)dBmToGLPixel(panRect, dBmMax, dBmMin, agcHangLevel);
            r = 125/255.0f; g = 225/255.0f; b = 125/255.0f;
            lines.append({ (float)dBmScalePanRect.right() - 1.0f, hangPixel + 2.0f, 4.0f, 0, 0, 0, 1.0f });
            lines.append({ (float)panRect.right() - 1.0f, hangPixel, 4.0f, 0, 0, 0, 1.0f });
            lines.append({ (float)dBmScalePanRect.right() - 3.0f, hangPixel, 5.0f, r, g, b, 1.0f });
            lines.append({ (float)panRect.right() - 1.0f, hangPixel, 4.0f, r, g, b, 1.0f });
		}
	}

    if (!lines.isEmpty()) {
        m_vbo.allocate(lines.data(), (int)(lines.size() * sizeof(VertexData)));
        m_shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 7);
        m_shader->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 4, sizeof(float) * 7);
        glDrawArrays(GL_LINES, 0, lines.size());
    }

    m_vao.release();
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_LINE_STIPPLE);
	glEnable(GL_MULTISAMPLE);
    m_shader->release();
}
