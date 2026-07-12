/**
 * author: Marcus Choo (m.choo@digipen.edu) (13/7/26)
 * Implements the 3D fan model using one indexed cube that is reused to draw
 * the base, pole, hub, and four blades. The file handles shader creation,
 * vertex and index buffer setup, hierarchical transforms, animation, part
 * colouring, resizing, tap input, and swipe-based speed control.
 */

#include "Fan.h"
#include "ShaderHelper.h"
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cmath>

namespace {

// 8 corners of a unit cube, centered at the origin.
const float kPositions[8 * 3] = {
    -0.5f, -0.5f,  0.5f,   //0 front-bottom-left
     0.5f, -0.5f,  0.5f,   //1 front-bottom-right
     0.5f,  0.5f,  0.5f,   //2 front-top-right
    -0.5f,  0.5f,  0.5f,   //3 front-top-left
    -0.5f, -0.5f, -0.5f,   //4 back-bottom-left
     0.5f, -0.5f, -0.5f,   //5 back-bottom-right
     0.5f,  0.5f, -0.5f,   //6 back-top-right
    -0.5f,  0.5f, -0.5f,   //7 back-top-left
};

// Per-corner grayscale factor: front (z>0) corners brighter than back.
const float kShade[8] = {
    1.0f, 1.0f, 1.0f, 1.0f,   // front corners
    0.6f, 0.6f, 0.6f, 0.6f,   // back corners
};

const GLushort kIndices[36] = {
    0,1,2,  2,3,0,      // front
    1,5,6,  6,2,1,      // right
    5,4,7,  7,6,5,      // back
    4,0,3,  3,7,4,      // left
    3,2,6,  6,7,3,      // top
    4,5,1,  1,0,4,      // bottom
};

const char* kVertSrc = R"GLSL(#version 300 es
layout(location = 0) in vec3 aPosition;
layout(location = 1) in float aShade;

uniform mat4 MODELVIEWPROJECTIONMATRIX;

out float vShade;

void main() {
    vShade = aShade;
    gl_Position = MODELVIEWPROJECTIONMATRIX * vec4(aPosition, 1.0);
}
)GLSL";

const char* kFragSrc = R"GLSL(#version 300 es
precision mediump float;

uniform vec3 PARTCOLOR;
in float vShade;
out vec4 fragColor;

void main() {
    fragColor = vec4(PARTCOLOR * vShade, 1.0);
}
)GLSL";

} // namespace

Fan::~Fan()
{
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (ibo) glDeleteBuffers(1, &ibo);
    if (program) glDeleteProgram(program);
}

void Fan::buildCubeGeometry()
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    const GLsizeiptr posBytes   = sizeof(kPositions);
    const GLsizeiptr shadeBytes = sizeof(kShade);
    glBufferData(GL_ARRAY_BUFFER, posBytes + shadeBytes, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,        posBytes,   kPositions);
    glBufferSubData(GL_ARRAY_BUFFER, posBytes, shadeBytes, kShade);

    // layout(location = 0) aPosition : 3 floats, offset 0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // layout(location = 1) aShade : 1 float, offset = posBytes
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, (void*)(uintptr_t)posBytes);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices), kIndices, GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void Fan::InitModel()
{
    buildCubeGeometry();

#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(assetMgr, "shader/FanVertex.glsl", "shader/FanFragment.glsl");
    if (!program) program = ShaderHelper::buildProgram(kVertSrc, kFragSrc); // fallback safety net
#else
    program = ShaderHelper::buildProgramFromFile("FanVertex.glsl", "FanFragment.glsl");
    if (!program) program = ShaderHelper::buildProgram(kVertSrc, kFragSrc); // fallback safety net
#endif
    uMVP        = glGetUniformLocation(program, "MODELVIEWPROJECTIONMATRIX");
    uPartColor  = glGetUniformLocation(program, "PARTCOLOR");

    transform.TransformInit();

    lastMoveTime = std::chrono::steady_clock::now();
}

void Fan::Resize(int w, int h)
{
    viewportW = w > 0 ? w : 1;
    viewportH = h > 0 ? h : 1;
    glViewport(0, 0, viewportW, viewportH);

    float aspect = static_cast<float>(viewportW) / static_cast<float>(viewportH);
    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformLoadIdentity();
    transform.TransformSetPerspective(glm::radians(60.0f), aspect, 0.01f, 1000.0f, 0.0f);
}

void Fan::drawPart(const glm::vec3& color)
{
    glUniformMatrix4fv(uMVP, 1, GL_FALSE,
                        glm::value_ptr(*transform.TransformGetModelViewProjectionMatrix()));
    glUniform3fv(uPartColor, 1, glm::value_ptr(color));
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, nullptr);
}

void Fan::Render()
{
    // ---- Part 2: advance blade animation ----
    if (fanOn) spinAngle += kBaseSpeed + dragBoost;
    if (spinAngle >= 360.0f) spinAngle -= 360.0f;

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);
    glBindVertexArray(vao);

    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();

    // Shared world transform: pull the fan back and give it a 3D tilt.
    transform.TransformTranslate(0.0f, 0.8f, -8.0f);
    transform.TransformRotate(glm::radians(20.0f), 0.0f, 1.0f, 0.0f);

    // ---- base ----
    transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, -2.6f, 0.0f);
        transform.TransformScale(1.6f, 0.25f, 0.8f);
        drawPart(glm::vec3(0.45f, 0.28f, 0.12f));
    transform.TransformPopMatrix();

    // ---- pole ----
    transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, -1.21f, 0.0f);
        transform.TransformScale(0.15f, 2.53f, 0.15f);
        drawPart(glm::vec3(0.55f, 0.55f, 0.58f));
    transform.TransformPopMatrix();

    // ---- hub ----
    transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, 0.2f, 0.0f);
        transform.TransformScale(0.3f, 0.3f, 0.3f);
        drawPart(glm::vec3(0.20f, 0.20f, 0.22f));
    transform.TransformPopMatrix();

    // ---- 4 blades, 90 degrees apart, chasing each other around the hub ----
    static const glm::vec3 kBladeColors[4] = {
        glm::vec3(0.85f, 0.10f, 0.10f), // red
        glm::vec3(0.10f, 0.35f, 0.85f), // blue
        glm::vec3(0.90f, 0.55f, 0.10f), // orange
        glm::vec3(0.15f, 0.70f, 0.20f), // green
    };
    for (int i = 0; i < 4; ++i) {
        transform.TransformPushMatrix();
            transform.TransformTranslate(0.0f, 0.2f, 0.15f);
            transform.TransformRotate(glm::radians(spinAngle + i * 90.0f), 0.0f, 0.0f, 1.0f);
            transform.TransformTranslate(0.0f, 0.55f, 0.0f);
            transform.TransformScale(0.22f, 0.8f, 0.05f);
            drawPart(kBladeColors[i]);
        transform.TransformPopMatrix();
    }

    glBindVertexArray(0);
}

// ---------------------------------------------------------------------------
// Part 3 / Part 4 -- tap toggles the fan, swipe velocity boosts spin speed.
// ---------------------------------------------------------------------------

void Fan::TouchDown(float x, float y)
{
    lastX = x;
    lastY = y;
    movedDistance = 0.0f;
    dragging = true;
    lastMoveTime = std::chrono::steady_clock::now();
}

void Fan::TouchMove(float x, float y)
{
    if (!dragging) return;

    float dx = x - lastX;
    float dy = y - lastY;
    float dist = std::sqrt(dx * dx + dy * dy);
    movedDistance += dist;

    auto now = std::chrono::steady_clock::now();
    float dtMs = std::chrono::duration<float, std::milli>(now - lastMoveTime).count();
    if (dtMs > 0.0f) {
        float velocity = dist / dtMs; // px per ms
        dragBoost = std::min(velocity * kBoostScale, kMaxBoost);
    }
    lastMoveTime = now;

    lastX = x;
    lastY = y;
}

void Fan::TouchUp()
{
    if (dragging && movedDistance < kTapThreshold) {
        fanOn = !fanOn;
        LOGI("Fan %s", fanOn ? "ON" : "OFF");
    }
    dragging = false;
    dragBoost = 0.0f; // boost applies only while dragging
}
