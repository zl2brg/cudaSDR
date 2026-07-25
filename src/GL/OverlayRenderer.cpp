#include "OverlayRenderer.h"
#include "cusdr_glShaders.h"
#include <QVarLengthArray>
#include <cmath>

namespace {

struct LineVert {
	float x, y, z;
	float r, g, b, a;
};

void beginOverlayLines(QOpenGLFunctions *gl)
{
    gl->glDisable(GL_DEPTH_TEST);
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl->glEnable(GL_BLEND);
    gl->glLineWidth(1.0f);
}

void endOverlayLines(QOpenGLFunctions *gl)
{
    Q_UNUSED(gl);
}

// Core-profile substitute for glLineStipple / Qt::DotLine.
void appendDashedSegment(QList<LineVert> *out,
                         float x1, float y1, float x2, float y2,
                         float z, float r, float g, float b, float a,
                         float dash = 3.0f, float gap = 3.0f)
{
	const float dx = x2 - x1;
	const float dy = y2 - y1;
	const float len = std::sqrt(dx * dx + dy * dy);
	if (len < 0.5f || !out)
		return;

	const float ux = dx / len;
	const float uy = dy / len;
	float pos = 0.0f;
	bool draw = true;
	while (pos < len) {
		const float seg = draw ? dash : gap;
		const float next = std::min(pos + seg, len);
		if (draw) {
			out->append({ x1 + ux * pos,  y1 + uy * pos,  z, r, g, b, a });
			out->append({ x1 + ux * next, y1 + uy * next, z, r, g, b, a });
		}
		pos = next;
		draw = !draw;
	}
}

} // namespace

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
        m_shader->addShaderFromSourceCode(QOpenGLShader::Vertex, GlShaders::coloredVertexSource());
        m_shader->addShaderFromSourceCode(QOpenGLShader::Fragment, GlShaders::coloredFragmentSource());
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
                               const QRect& freqScalePanRect,
                               const TScale& freqScale,
                               const TScale& dBmScale,
                               int freqPlotLeft,
                               float r, float g, float b, float alpha,
                               bool panGridEnabled) {
    if (!panGridEnabled) return;
    if (!m_shader || !m_shader->isLinked()) return;
    if (panRect.isEmpty()) return;

    beginOverlayLines(this);

    m_shader->bind();
    m_shader->setUniformValue("matrix", projection);

    m_vao.bind();
    m_vbo.bind();

    QList<LineVert> gridData;
    gridData.reserve((freqScale.mainPointPositions.length()
                      + dBmScale.mainPointPositions.length()) * 64);

    const int len = freqScale.mainPointPositions.length();
    for (int i = 0; i < len; i++) {
        const float x = float(panRect.left() + freqPlotLeft + freqScale.mainPointPositions.at(i));
        if (x < float(panRect.left() + freqPlotLeft) || x > float(panRect.right()))
            continue;
        appendDashedSegment(&gridData,
                            x, float(panRect.top()),
                            x, float(panRect.bottom()),
                            3.0f, r, g, b, alpha);
    }

    const int dBmLen = dBmScale.mainPointPositions.length();
    for (int i = 0; i < dBmLen; i++) {
        const float y = float(panRect.top() + dBmScale.mainPointPositions.at(i));
        appendDashedSegment(&gridData,
                            float(panRect.left()),  y,
                            float(panRect.right()), y,
                            3.0f, r, g, b, alpha);
    }

    if (!gridData.isEmpty()) {
        static_assert(sizeof(LineVert) == sizeof(VertexData), "grid vertex layout mismatch");
        m_vbo.allocate(gridData.data(), gridData.size() * (int)sizeof(LineVert));
        m_shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 7);
        m_shader->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 4, sizeof(float) * 7);
        glDrawArrays(GL_LINES, 0, gridData.size());
    }

    m_vao.release();
    m_shader->release();
    endOverlayLines(this);

    Q_UNUSED(freqScalePanRect);
}

void OverlayRenderer::drawFrequencyScaleTicks(const QMatrix4x4& projection,
                                              const QRect& freqScalePanRect,
                                              const TScale& freqScale,
                                              float deltaF,
                                              float zoomFactor,
                                              float r, float g, float b, float alpha) {
    if (!m_shader || !m_shader->isLinked()) return;
    if (freqScalePanRect.isEmpty() || zoomFactor <= 0.0f) return;

    const float panOffset = deltaF * float(freqScalePanRect.width()) / zoomFactor;
    const float y1 = float(freqScalePanRect.top() + 1);
    const float y2 = float(freqScalePanRect.top() + 4);
    const float ySub = float(freqScalePanRect.top() + 3);

    beginOverlayLines(this);
    m_shader->bind();
    m_shader->setUniformValue("matrix", projection);
    m_vao.bind();
    m_vbo.bind();

    QList<VertexData> tickData;
    tickData.reserve((freqScale.mainPointPositions.length() + freqScale.subPointPositions.length()) * 2);

    for (int i = 0; i < freqScale.mainPointPositions.length(); i++) {
        const float x = float(freqScalePanRect.left() + freqScale.mainPointPositions.at(i))
                        - panOffset;
        tickData.append({ x, y1, 2.0f, r, g, b, alpha });
        tickData.append({ x, y2, 2.0f, r, g, b, alpha });
    }

    const float sr = r * 0.85f;
    const float sg = g * 0.85f;
    const float sb = b * 0.85f;
    for (int i = 0; i < freqScale.subPointPositions.length(); i++) {
        const float x = float(freqScalePanRect.left() + freqScale.subPointPositions.at(i))
                        - panOffset;
        tickData.append({ x, y1, 2.0f, sr, sg, sb, alpha * 0.85f });
        tickData.append({ x, ySub, 2.0f, sr, sg, sb, alpha * 0.85f });
    }

    if (!tickData.isEmpty()) {
        m_vbo.allocate(tickData.data(), tickData.size() * (int)sizeof(VertexData));
        m_shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 7);
        m_shader->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 4, sizeof(float) * 7);
        glDrawArrays(GL_LINES, 0, tickData.size());
    }

    m_vao.release();
    m_shader->release();
    endOverlayLines(this);
}

void OverlayRenderer::drawDBmScaleTicks(const QMatrix4x4& projection,
                                        const QRect& dBmScalePanRect,
                                        const TScale& dBmScale,
                                        float r, float g, float b, float alpha) {
    if (!m_shader || !m_shader->isLinked()) return;
    if (dBmScalePanRect.isEmpty()) return;

    const float x1 = float(dBmScalePanRect.right() - 4);
    const float x2 = float(dBmScalePanRect.right());

    beginOverlayLines(this);
    m_shader->bind();
    m_shader->setUniformValue("matrix", projection);
    m_vao.bind();
    m_vbo.bind();

    QList<VertexData> tickData;
    tickData.reserve(dBmScale.mainPointPositions.length() * 2);

    for (int i = 0; i < dBmScale.mainPointPositions.length(); i++) {
        const float y = float(dBmScalePanRect.top() + dBmScale.mainPointPositions.at(i));
        tickData.append({ x1, y, 2.0f, r, g, b, alpha });
        tickData.append({ x2, y, 2.0f, r, g, b, alpha });
    }

    if (!tickData.isEmpty()) {
        m_vbo.allocate(tickData.data(), tickData.size() * (int)sizeof(VertexData));
        m_shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 7);
        m_shader->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 4, sizeof(float) * 7);
        glDrawArrays(GL_LINES, 0, tickData.size());
    }

    m_vao.release();
    m_shader->release();
    endOverlayLines(this);
}

void OverlayRenderer::drawCenterLine(const QMatrix4x4& projection,
                                     const QRect& panRect,
                                     const QRect& freqScalePanRect,
                                     const QRect& waterfallRect,
                                     int centerlineHeight,
                                     float deltaF,
                                     float zoomFactor,
                                     const QColor& centerColor,
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

        float centerX = (float)panRect.left() + (float)panRect.width() / 2.0f;
        float centerY = (float)(panRect.top() + panRect.height() - 1);
		const QColor centerCol = centerColor.isValid() ? centerColor : QColor(246, 7, 19);

        glLineWidth(3.0f);

        QVarLengthArray<VertexData, 8> lines;
        float cr = centerCol.redF(); float cg = centerCol.greenF(); float cb = centerCol.blueF(); float ca = centerCol.alphaF();

        lines.append({ centerX, y1 + 1.0f, 3.5f, cr, cg, cb, ca });
        lines.append({ centerX, centerY - 1.0f,  3.5f, cr, cg, cb, ca });

        if (waterfallRect.isValid() && waterfallRect.height() > 2) {
            const float wfTop = (float)(freqScalePanRect.bottom() + 1);
            const float wfBottom = (float)(freqScalePanRect.bottom() + waterfallRect.height() - 1);
            lines.append({ centerX, wfTop, 3.5f, cr, cg, cb, ca });
            lines.append({ centerX, wfBottom, 3.5f, cr, cg, cb, ca });
        }

		float vfoX = (float)(panRect.left() + qRound((qreal)(panRect.width()/2.0f)  - deltaF * panRect.width() / zoomFactor));
        float vr = vfoColor.redF(); float vg = vfoColor.greenF(); float vb = vfoColor.blueF(); float va = 1.0f;

        if (!qIsNaN(vfoX) && !qIsInf(vfoX)) {
            lines.append({ vfoX, y1 + 1.0f, 4.0f, vr, vg, vb, va });
            lines.append({ vfoX, centerY - 1.0f,  4.0f, vr, vg, vb, va });
            if (waterfallRect.isValid() && waterfallRect.height() > 2) {
                const float wfTop = (float)(freqScalePanRect.bottom() + 1);
                const float wfBottom = (float)(freqScalePanRect.bottom() + waterfallRect.height() - 1);
                const float vfoZ = (dragMouse && !panLocked) ? 3.0f : 4.0f;
                lines.append({ vfoX, wfTop, vfoZ, vr, vg, vb, va });
                lines.append({ vfoX, wfBottom, vfoZ, vr, vg, vb, va });
            }
        }

        if (lines.size() >= 2) {
            m_vbo.allocate(lines.data(), (int)(lines.size() * sizeof(VertexData)));
            m_shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 7);
            m_shader->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 4, sizeof(float) * 7);
            glDrawArrays(GL_LINES, 0, lines.size());
        }

        m_vao.release();
        m_shader->release();
	}
}

void OverlayRenderer::drawFilter(const QMatrix4x4& projection,
                                 const QRect& panRect,
                                 const QRect& waterfallRect,
                                 float filterLo, float filterHi,
                                 float deltaF, float zoomFactor,
                                 const QColor& filterColor,
                                 bool highlightFilter,
                                 bool dragPanning,
                                 bool showLeftBoundary, bool showRightBoundary,
                                 int& filterLeft, int& filterRight,
                                 int& filterTop, int& filterBottom) {

    if (!m_shader || !m_shader->isLinked()) return;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

	QColor color = filterColor;
	if (highlightFilter)
		color.setAlpha(qMin(255, filterColor.alpha() + 40));

	const QColor boundaryColor(filterColor.red(), filterColor.green(), filterColor.blue(),
	                           qMax(230, filterColor.alpha()));
	const QColor edgeColor(filterColor.red(), filterColor.green(), filterColor.blue(),
	                       qMax(180, filterColor.alpha()));

	filterLeft = panRect.left() + qRound((qreal)(panRect.width()/2.0f) + (filterLo - deltaF) * panRect.width() / zoomFactor);
	filterRight = panRect.left() + qRound((qreal)(panRect.width()/2.0f) + (filterHi - deltaF) * panRect.width() / zoomFactor);
	filterTop = panRect.top() + 1;
	filterBottom = panRect.top() + panRect.height() - 1;
	
    QRect filterRect(filterLeft, filterTop, filterRight - filterLeft, filterBottom - filterTop);

    m_shader->bind();
    m_shader->setUniformValue("matrix", projection);

    m_vao.bind();
    m_vbo.bind();

	const bool filterVisible = (filterLeft >= panRect.left() && filterLeft <= panRect.right())
	    || (filterRight >= panRect.left() && filterRight <= panRect.right())
	    || (filterLeft < panRect.left() && filterRight > panRect.right());

	if (filterVisible) {
		if (filterRect.height() > 5) {
            const float fillAlphaScale = dragPanning ? 0.25f : 0.4f;
            float fr = color.redF(), fg = color.greenF(), fb = color.blueF(), fa = color.alphaF() * fillAlphaScale;
            QVarLengthArray<VertexData, 4> rectData;
            const float rx1 = (float)filterRect.left();
            const float ry1 = (float)filterRect.top();
            const float rx2 = (float)filterRect.right();
            const float ry2 = (float)filterRect.bottom();

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

    if (waterfallRect.isValid() && waterfallRect.height() > 5 && filterVisible) {
        const int wfTop = waterfallRect.top() + 1;
        const int wfBottom = waterfallRect.top() + waterfallRect.height() - 1;
        const float fillAlphaScale = dragPanning ? 0.25f : 0.4f;
        float fr = color.redF(), fg = color.greenF(), fb = color.blueF(), fa = color.alphaF() * fillAlphaScale;
        QVarLengthArray<VertexData, 4> wfRectData;
        const float rx1 = (float)filterLeft;
        const float rx2 = (float)filterRight;
        const float ry1 = (float)wfTop;
        const float ry2 = (float)wfBottom;

        wfRectData.append({ rx1, ry1, 4.0f, fr, fg, fb, fa });
        wfRectData.append({ rx2, ry1, 4.0f, fr, fg, fb, fa });
        wfRectData.append({ rx1, ry2, 4.0f, fr, fg, fb, fa });
        wfRectData.append({ rx2, ry2, 4.0f, fr, fg, fb, fa });

        m_vbo.allocate(wfRectData.data(), (int)(wfRectData.size() * sizeof(VertexData)));
        m_shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 7);
        m_shader->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 4, sizeof(float) * 7);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    QVarLengthArray<VertexData, 8> lines;
    if (dragPanning && filterVisible) {
        const float r = edgeColor.redF(), g = edgeColor.greenF(), b = edgeColor.blueF(), a = edgeColor.alphaF();
        lines.append({ (float)filterLeft, (float)filterTop,    5.0f, r, g, b, a });
        lines.append({ (float)filterLeft, (float)filterBottom, 5.0f, r, g, b, a });
        lines.append({ (float)filterRight, (float)filterTop,    5.0f, r, g, b, a });
        lines.append({ (float)filterRight, (float)filterBottom, 5.0f, r, g, b, a });
        if (waterfallRect.isValid() && waterfallRect.height() > 5) {
            const float wfTop = (float)(waterfallRect.top() + 1);
            const float wfBottom = (float)(waterfallRect.top() + waterfallRect.height() - 1);
            lines.append({ (float)filterLeft, wfTop,    5.0f, r, g, b, a });
            lines.append({ (float)filterLeft, wfBottom, 5.0f, r, g, b, a });
            lines.append({ (float)filterRight, wfTop,    5.0f, r, g, b, a });
            lines.append({ (float)filterRight, wfBottom, 5.0f, r, g, b, a });
        }
    }

	if (showLeftBoundary) {
        const float r = boundaryColor.redF(), g = boundaryColor.greenF(), b = boundaryColor.blueF(), a = boundaryColor.alphaF();
        lines.append({ (float)filterLeft, (float)filterTop,    5.0f, r, g, b, a });
        lines.append({ (float)filterLeft, (float)filterBottom, 5.0f, r, g, b, a });
        if (waterfallRect.isValid() && waterfallRect.height() > 5) {
            const float wfTop = (float)(waterfallRect.top() + 1);
            const float wfBottom = (float)(waterfallRect.top() + waterfallRect.height() - 1);
            lines.append({ (float)filterLeft, wfTop,    5.0f, r, g, b, a });
            lines.append({ (float)filterLeft, wfBottom, 5.0f, r, g, b, a });
        }
	}

	if (showRightBoundary) {
        const float r = boundaryColor.redF(), g = boundaryColor.greenF(), b = boundaryColor.blueF(), a = boundaryColor.alphaF();
        lines.append({ (float)filterRight, (float)filterTop,    5.0f, r, g, b, a });
        lines.append({ (float)filterRight, (float)filterBottom, 5.0f, r, g, b, a });
        if (waterfallRect.isValid() && waterfallRect.height() > 5) {
            const float wfTop = (float)(waterfallRect.top() + 1);
            const float wfBottom = (float)(waterfallRect.top() + waterfallRect.height() - 1);
            lines.append({ (float)filterRight, wfTop,    5.0f, r, g, b, a });
            lines.append({ (float)filterRight, wfBottom, 5.0f, r, g, b, a });
        }
	}

    if (!lines.isEmpty()) {
        glLineWidth(1);
        m_vbo.allocate(lines.data(), (int)(lines.size() * sizeof(VertexData)));
        m_shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 7);
        m_shader->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 4, sizeof(float) * 7);
        glDrawArrays(GL_LINES, 0, lines.size());
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

    beginOverlayLines(this);

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
    endOverlayLines(this);
    m_shader->release();
}

void OverlayRenderer::drawFilledRect(const QMatrix4x4& projection,
                                     const QRect& rect,
                                     const QColor& topColor,
                                     const QColor& bottomColor,
                                     float z)
{
	if (!m_shader || !m_shader->isLinked() || rect.isEmpty())
		return;

	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	m_shader->bind();
	m_shader->setUniformValue("matrix", projection);

	m_vao.bind();
	m_vbo.bind();

	const float x1 = float(rect.left());
	const float y1 = float(rect.top());
	const float x2 = float(rect.left() + rect.width());
	const float y2 = float(rect.top() + rect.height());
	const float r1 = topColor.redF(), g1 = topColor.greenF(), b1 = topColor.blueF(), a1 = topColor.alphaF();
	const float r2 = bottomColor.redF(), g2 = bottomColor.greenF(), b2 = bottomColor.blueF(), a2 = bottomColor.alphaF();

	const VertexData quad[4] = {
		{ x1, y1, z, r1, g1, b1, a1 },
		{ x2, y1, z, r1, g1, b1, a1 },
		{ x1, y2, z, r2, g2, b2, a2 },
		{ x2, y2, z, r2, g2, b2, a2 },
	};

	m_vbo.allocate(quad, int(sizeof(quad)));
	m_shader->enableAttributeArray(0);
	m_shader->enableAttributeArray(1);
	m_shader->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 7);
	m_shader->setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 4, sizeof(float) * 7);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	m_vao.release();
	m_shader->release();
}
