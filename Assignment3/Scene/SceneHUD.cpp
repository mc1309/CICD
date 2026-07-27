/**
 * author: Marcus Choo (m.choo@digipen.edu) (27/7/26)
 * SceneHUD.cpp
 * Android Programming Quiz 3
 * Implements SceneHUD: screen-space button/bar geometry, hit-testing, and
 * the live 20-block speed indicator. See SceneHUD.h.
 *
 * BONUS: smooth zoom/speed easing (line 223-232)
 */

#include "SceneHUD.h"
#include "ShaderHelper.h"
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cmath>
#include <vector>

namespace {

const char* kVertSrc = R"GLSL(#version 300 es
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec3 aColor;
uniform mat4 ORTHOMATRIX;
out vec3 vColor;
void main() {
    vColor = aColor;
    gl_Position = ORTHOMATRIX * vec4(aPosition, 0.0, 1.0);
}
)GLSL";

const char* kFragSrc = R"GLSL(#version 300 es
precision mediump float;
in vec3 vColor;
out vec4 fragColor;
void main() {
    fragColor = vec4(vColor, 1.0);
}
)GLSL";

void AddQuad(std::vector<float>& verts, std::vector<GLushort>& idx,
             float x, float y, float w, float h, const glm::vec3& color)
{
    GLushort base = static_cast<GLushort>(verts.size() / 5);
    float positions[4][2] = { {x,y}, {x+w,y}, {x+w,y+h}, {x,y+h} };
    for (auto& p : positions) {
        verts.push_back(p[0]); verts.push_back(p[1]);
        verts.push_back(color.r); verts.push_back(color.g); verts.push_back(color.b);
    }
    idx.push_back(base + 0);
    idx.push_back(base + 2);
    idx.push_back(base + 1);

    idx.push_back(base + 0);
    idx.push_back(base + 3);
    idx.push_back(base + 2);
}

} // namespace

bool SceneHUD::PointInRect(float px, float py, const Rect& r)
{
    return px >= r.x && px <= r.x + r.w && py >= r.y && py <= r.y + r.h;
}

SceneHUD::~SceneHUD()
{
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (ibo) glDeleteBuffers(1, &ibo);
    if (program) glDeleteProgram(program);
}

void SceneHUD::InitModel()
{
    LOGI("SceneHUD::InitModel");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // Sized generously up front (buttons + glyphs + 20 bar blocks); re-uploaded
    // with glBufferData each frame since it's a genuinely dynamic HUD element.
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 5 * 512, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 768, nullptr, GL_DYNAMIC_DRAW);

    glBindVertexArray(0);

#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(assetMgr, "shader/HUDVertex.glsl", "shader/HUDFragment.glsl");
    if (!program) program = ShaderHelper::buildProgram(kVertSrc, kFragSrc);
#else
    program = ShaderHelper::buildProgramFromFile("HUDVertex.glsl", "HUDFragment.glsl");
    if (!program) program = ShaderHelper::buildProgram(kVertSrc, kFragSrc);
#endif
    uOrtho = glGetUniformLocation(program, "ORTHOMATRIX");

    transform.TransformInit();
    layoutButtons(viewportW, viewportH);
}

void SceneHUD::layoutButtons(int w, int h)
{
    const float btnW = 200.0f, btnH = 72.0f, margin = 28.0f, gap = 26.0f;

    btnSpeedUp   = { margin,                     margin,               btnW, btnH };
    btnSpeedDown = { margin,                     margin + btnH + gap,  btnW, btnH };
    btnZoomIn    = { w - margin - btnW,           margin,               btnW, btnH };
    btnZoomOut   = { w - margin - btnW,           margin + btnH + gap,  btnW, btnH };
}

void SceneHUD::Resize(int w, int h)
{
    viewportW = w > 0 ? w : 1;
    viewportH = h > 0 ? h : 1;

    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformLoadIdentity();
    transform.TransformOrtho(0.0f, static_cast<float>(viewportW),
                              static_cast<float>(viewportH), 0.0f, -1.0f, 1.0f);

    layoutButtons(viewportW, viewportH);
}

bool SceneHUD::HandleTouchDown(float x, float y)
{
    if (PointInRect(x, y, btnSpeedUp)) {
        targetSpeed = std::clamp(targetSpeed + 1.0f, 0.0f, 20.0f);
        activeButton = 0;
        LOGI("SceneHUD: +SPD pressed (speed=%.0f)", targetSpeed);
        return true;
    }
    if (PointInRect(x, y, btnSpeedDown)) {
        targetSpeed = std::clamp(targetSpeed - 1.0f, 0.0f, 20.0f);
        activeButton = 1;
        LOGI("SceneHUD: -SPD pressed (speed=%.0f)", targetSpeed);
        return true;
    }
    if (PointInRect(x, y, btnZoomIn)) {
        targetZoom = std::clamp(targetZoom - 1.0f, 3.0f, 15.0f);
        activeButton = 2;
        LOGI("SceneHUD: +ZOOM pressed (distance=%.0f)", targetZoom);
        return true;
    }
    if (PointInRect(x, y, btnZoomOut)) {
        targetZoom = std::clamp(targetZoom + 1.0f, 3.0f, 15.0f);
        activeButton = 3;
        LOGI("SceneHUD: -ZOOM pressed (distance=%.0f)", targetZoom);
        return true;
    }
    activeButton = -1;
    return false;
}

void SceneHUD::TouchRelease()
{
    activeButton = -1;
}

void SceneHUD::rebuildGeometry()
{
    std::vector<float>   verts;
    std::vector<GLushort> idx;
    verts.reserve(5 * 512);
    idx.reserve(768);

    const glm::vec3 kIdleColor(0.20f, 0.22f, 0.28f);
    const glm::vec3 kHeldColor(0.35f, 0.55f, 0.95f);
    const glm::vec3 kGlyphColor(0.92f, 0.92f, 0.95f);

    auto drawButtonWithGlyph = [&](const Rect& r, int myIndex, bool plus) {
        AddQuad(verts, idx, r.x, r.y, r.w, r.h, activeButton == myIndex ? kHeldColor : kIdleColor);

        // Simple +/- glyph made from triangles/quads at the button's center.
        float cx = r.x + r.w * 0.5f, cy = r.y + r.h * 0.5f;
        float barLen = 34.0f, barThick = 8.0f;
        AddQuad(verts, idx, cx - barLen*0.5f, cy - barThick*0.5f, barLen, barThick, kGlyphColor);
        if (plus) {
            AddQuad(verts, idx, cx - barThick*0.5f, cy - barLen*0.5f, barThick, barLen, kGlyphColor);
        }
    };

    drawButtonWithGlyph(btnSpeedUp,   0, true);
    drawButtonWithGlyph(btnSpeedDown, 1, false);
    drawButtonWithGlyph(btnZoomIn,    2, true);
    drawButtonWithGlyph(btnZoomOut,   3, false);

    // ---- 20-block speed bar: 10x20 px blocks, first `speed` green, rest grey ----
    const int   kBlockCount = 20;
    const float kBlockW = 10.0f, kBlockH = 20.0f, kBlockGap = 5.0f;
    const float barTotalW = kBlockCount * (kBlockW + kBlockGap) - kBlockGap;
    const float barX = (viewportW - barTotalW) * 0.5f;
    const float barY = viewportH - 90.0f;

    int litBlocks = static_cast<int>(std::round(speed));
    litBlocks = std::clamp(litBlocks, 0, kBlockCount);

    for (int i = 0; i < kBlockCount; ++i) {
        glm::vec3 color = (i < litBlocks) ? glm::vec3(0.20f, 0.85f, 0.30f)
                                           : glm::vec3(0.28f, 0.28f, 0.32f);
        AddQuad(verts, idx, barX + i * (kBlockW + kBlockGap), barY, kBlockW, kBlockH, color);
    }

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(verts.size() * sizeof(float)), verts.data());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(idx.size() * sizeof(GLushort)), idx.data());
    glBindVertexArray(0);

    lastIndexCount = static_cast<GLsizei>(idx.size());
}

void SceneHUD::Render()
{
    // ---- Bonus: smooth zoom/speed easing ----
    // Button presses only moved targetSpeed/targetZoom; chase them here by a
    // fixed fraction each frame so the displayed value (and therefore the
    // fan's actual speed / the camera's actual distance, since Renderer reads
    // CurrentSpeed()/CurrentZoom() straight from these) glides instead of
    // jumping. Snap once the gap is imperceptible to avoid an infinite tail.
    speed        += (targetSpeed - speed) * kEaseFactor;
    zoomDistance += (targetZoom  - zoomDistance) * kEaseFactor;
    if (std::fabs(targetSpeed - speed) < 0.02f) speed = targetSpeed;
    if (std::fabs(targetZoom  - zoomDistance) < 0.02f) zoomDistance = targetZoom;
    rebuildGeometry();

    glUseProgram(program);
    glUniformMatrix4fv(uOrtho, 1, GL_FALSE, glm::value_ptr(*transform.TransformGetProjectionMatrix()));

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, lastIndexCount, GL_UNSIGNED_SHORT, nullptr);
    glBindVertexArray(0);
}
