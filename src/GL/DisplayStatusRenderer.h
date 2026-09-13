/**
 * @file  DisplayStatusRenderer.h
 * @brief Renderer for status badges and hardware indicator regions in OGLDisplayPanel.
 * @author Simon Eatough <simon.eatough@gmail.com>
 * @date 2026-09-08
 */

#ifndef DISPLAY_STATUS_RENDERER_H
#define DISPLAY_STATUS_RENDERER_H

class OGLDisplayPanel;

class DisplayStatusRenderer {
public:
    explicit DisplayStatusRenderer(OGLDisplayPanel *panel);
    ~DisplayStatusRenderer() = default;

    void paintUpperRegion();
    void paintLowerRegion();

private:
    OGLDisplayPanel *m_panel;
};

#endif // DISPLAY_STATUS_RENDERER_H
