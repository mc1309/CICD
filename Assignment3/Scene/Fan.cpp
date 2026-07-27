/**
 * author: Marcus Choo (m.choo@digipen.edu) (27/7/26)
 * Fan.cpp
 * Android Programming Quiz 3
 *
 * Implements the 3D fan model using one indexed cube, reused to draw the
 * base, pole, hub, and four blades,
 *
 * Changes from Quiz 2:
 *  - The cube now carries 24 verts (4 per face) with a real per-face normal
 *    instead of 8 shared verts with a baked grayscale "shade" value.
 *  - Camera transform is gone from here -- Scene3D now owns it and pushes a
 *    shared view/projection matrix into this class every frame.
 *  - Lighting is done in EYE SPACE and the fragment shader runs the same
 *    ambient+diffuse+specular formula against a light position that's been
 *    pre-transformed into eye space (View * worldLightPos) before upload.
 *  - Every drawPart() call still caches its WORLD-space matrix into `parts`
 *    for Scene3D's 3D-picking ray tests -- picking intentionally stays in
 *    world space; only the lighting moved to eye space.
 *  - All Quiz 2 tap-to-toggle / drag-to-boost gesture code has  been removed.
 */

#include "Fan.h"
#include "ShaderHelper.h"
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cmath>

namespace {

// 24 corners (4 per face) of a unit cube, centered at the origin -- duplicated
// per face so each face can carry its own flat normal (Quiz 3 Part 2 gotcha).
    const float kPositions[24 * 3] = {
            // front (+Z)
            -0.5f,-0.5f, 0.5f,   0.5f,-0.5f, 0.5f,   0.5f, 0.5f, 0.5f,  -0.5f, 0.5f, 0.5f,
            // back (-Z)
            0.5f,-0.5f,-0.5f,  -0.5f,-0.5f,-0.5f,  -0.5f, 0.5f,-0.5f,   0.5f, 0.5f,-0.5f,
            // right (+X)
            0.5f,-0.5f, 0.5f,   0.5f,-0.5f,-0.5f,   0.5f, 0.5f,-0.5f,   0.5f, 0.5f, 0.5f,
            // left (-X)
            -0.5f,-0.5f,-0.5f,  -0.5f,-0.5f, 0.5f,  -0.5f, 0.5f, 0.5f,  -0.5f, 0.5f,-0.5f,
            // top (+Y)
            -0.5f, 0.5f, 0.5f,   0.5f, 0.5f, 0.5f,   0.5f, 0.5f,-0.5f,  -0.5f, 0.5f,-0.5f,
            // bottom (-Y)
            -0.5f,-0.5f,-0.5f,   0.5f,-0.5f,-0.5f,   0.5f,-0.5f, 0.5f,  -0.5f,-0.5f, 0.5f,
    };

    const float kNormals[24 * 3] = {
            0,0, 1,   0,0, 1,   0,0, 1,   0,0, 1,     // front
            0,0,-1,   0,0,-1,   0,0,-1,   0,0,-1,     // back
            1,0, 0,   1,0, 0,   1,0, 0,   1,0, 0,     // right
            -1,0, 0,  -1,0, 0,  -1,0, 0,  -1,0, 0,     // left
            0,1, 0,   0,1, 0,   0,1, 0,   0,1, 0,     // top
            0,-1,0,   0,-1,0,   0,-1,0,   0,-1,0,     // bottom
    };

    const GLushort kIndices[36] = {
            0, 1, 2,   2, 3, 0,         // front
            4, 5, 6,   6, 7, 4,        // back
            8, 9,10,  10,11, 8,    // right
            12,13,14,  14,15,12,   // left
            16,17,18,  18,19,16,  // top
            20,21,22,  22,23,20,  // bottom
    };

// Scene-wide light + material constants (mirrors the Light*/Material* /
// ShininessFactor uniforms from 6-PhongShader-Modern)
    const glm::vec3 kLightAmbient   (0.28f, 0.28f, 0.30f);
    const glm::vec3 kLightDiffuse   (0.90f, 0.90f, 0.85f);
    const glm::vec3 kLightSpecular  (1.00f, 1.00f, 1.00f);
    const glm::vec3 kMaterialSpecular(0.85f, 0.85f, 0.85f); // fan parts are glossy plastic-ish
    const float      kShininess = 28.0f;

// Fallback source, only used if the on-disk / asset shader can't be found.
    const char* kVertSrc = R"GLSL(#version 300 es
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
uniform mat4 MVPMATRIX;
uniform mat4 MODELVIEWMATRIX;
uniform mat3 NORMALMATRIX;
out vec3 normalCoord;
out vec3 eyeCoord;
void main() {
    vec4 eyePos = MODELVIEWMATRIX * vec4(aPosition, 1.0);
    eyeCoord = eyePos.xyz;
    normalCoord = normalize(NORMALMATRIX * aNormal);
    gl_Position = MVPMATRIX * vec4(aPosition, 1.0);
}
)GLSL";

    const char* kFragSrc = R"GLSL(#version 300 es
precision mediump float;
uniform vec3  MaterialAmbient;
uniform vec3  MaterialDiffuse;
uniform vec3  MaterialSpecular;
uniform vec3  LightAmbient;
uniform vec3  LightDiffuse;
uniform vec3  LightSpecular;
uniform vec3  LightPosition;   // already in eye space
uniform float ShininessFactor;
in vec3 normalCoord;
in vec3 eyeCoord;
layout(location = 0) out vec4 FinalColor;
void main() {
    vec3 N = normalize(normalCoord);
    vec3 L = normalize(LightPosition - eyeCoord);
    vec3 V = normalize(-eyeCoord);
    vec3 R = reflect(-L, N);

    float cosAngle   = max(dot(N, L), 0.0);
    float sIntensity = pow(max(dot(R, V), 0.0), ShininessFactor);

    vec3 ambient  = MaterialAmbient  * LightAmbient;
    vec3 diffuse  = MaterialDiffuse  * LightDiffuse;
    vec3 specular = MaterialSpecular * LightSpecular;

    FinalColor = vec4(ambient + (cosAngle * diffuse) + (sIntensity * specular), 1.0);
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

    const GLsizeiptr posBytes = sizeof(kPositions);
    const GLsizeiptr nrmBytes = sizeof(kNormals);
    glBufferData(GL_ARRAY_BUFFER, posBytes + nrmBytes, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,        posBytes, kPositions);
    glBufferSubData(GL_ARRAY_BUFFER, posBytes, nrmBytes, kNormals);

    // layout(location = 0) aPosition : 3 floats, offset 0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // layout(location = 1) aNormal : 3 floats, offset = posBytes
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(uintptr_t)posBytes);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices), kIndices, GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void Fan::InitModel()
{
    LOGI("Fan::InitModel");
    buildCubeGeometry();

#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(assetMgr, "shader/FanVertex.glsl", "shader/FanFragment.glsl");
    if (!program) program = ShaderHelper::buildProgram(kVertSrc, kFragSrc); // fallback safety net
#else
    program = ShaderHelper::buildProgramFromFile("FanVertex.glsl", "FanFragment.glsl");
    if (!program) program = ShaderHelper::buildProgram(kVertSrc, kFragSrc); // fallback safety net
#endif
    uMVP              = glGetUniformLocation(program, "MVPMATRIX");
    uModelViewMatrix  = glGetUniformLocation(program, "MODELVIEWMATRIX");
    uNormalMatrix     = glGetUniformLocation(program, "NORMALMATRIX");
    uMaterialAmbient  = glGetUniformLocation(program, "MaterialAmbient");
    uMaterialDiffuse  = glGetUniformLocation(program, "MaterialDiffuse");
    uMaterialSpecular = glGetUniformLocation(program, "MaterialSpecular");
    uLightAmbient     = glGetUniformLocation(program, "LightAmbient");
    uLightDiffuse     = glGetUniformLocation(program, "LightDiffuse");
    uLightSpecular    = glGetUniformLocation(program, "LightSpecular");
    uLightPosition    = glGetUniformLocation(program, "LightPosition");
    uShininess        = glGetUniformLocation(program, "ShininessFactor");

    transform.TransformInit();

}

void Fan::Resize(int w, int h)
{
    viewportW = w > 0 ? w : 1;
    viewportH = h > 0 ? h : 1;
    // Projection now owned by Scene3D and pushed in via SetViewProjection().
}

void Fan::SetViewProjection(const glm::mat4& view, const glm::mat4& proj)
{
    glm::mat4 v = view, p = proj;
    transform.TransformSetMatrixMode(VIEW_MATRIX);
    transform.TransformLoadMatrix(&v);
    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformLoadMatrix(&p);
}

void Fan::SetLight(const glm::vec3& lightPosWorld)
{
    lightPos = lightPosWorld;
}

void Fan::SetSpeed(float speed0to20)
{
    speed = std::clamp(speed0to20, 0.0f, 20.0f);
}

void Fan::drawPart(const glm::vec3& color, const char* name)
{
    // World-space model matrix -- cached for picking
    glm::mat4 model = *transform.TransformGetModelMatrix();

    // Eye-space quantities for lighting
    glm::mat4 modelView = *transform.TransformGetModelViewMatrix();
    glm::mat3 normalMatrix;
    transform.TransformGetNormalMatrix(&normalMatrix);

    // Light position must be uploaded in eye space: View * worldLightPos.
    glm::vec3 lightPosEye = glm::vec3(*transform.TransformGetViewMatrix() * glm::vec4(lightPos, 1.0f));

    int partIndex = static_cast<int>(parts.size());
    glm::vec3 useColor = (partIndex == highlightedPart) ? glm::vec3(1.0f, 0.75f, 0.10f) : color;

    glUniformMatrix4fv(uMVP, 1, GL_FALSE,
                       glm::value_ptr(*transform.TransformGetModelViewProjectionMatrix()));
    glUniformMatrix4fv(uModelViewMatrix, 1, GL_FALSE, glm::value_ptr(modelView));
    glUniformMatrix3fv(uNormalMatrix, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    glUniform3fv(uMaterialAmbient,  1, glm::value_ptr(useColor));          // PARTCOLOR doubles as ambient...
    glUniform3fv(uMaterialDiffuse,  1, glm::value_ptr(useColor));          // ...and diffuse material colour
    glUniform3fv(uMaterialSpecular, 1, glm::value_ptr(kMaterialSpecular));
    glUniform3fv(uLightAmbient,     1, glm::value_ptr(kLightAmbient));
    glUniform3fv(uLightDiffuse,     1, glm::value_ptr(kLightDiffuse));
    glUniform3fv(uLightSpecular,    1, glm::value_ptr(kLightSpecular));
    glUniform3fv(uLightPosition,    1, glm::value_ptr(lightPosEye));
    glUniform1f(uShininess, kShininess);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, nullptr);

    // Cache this part's WORLD transform for Scene3D's picking ray tests.
    parts.push_back(PickablePart{ name, model, glm::vec3(0.5f, 0.5f, 0.5f) });
}

void Fan::Render()
{
    // ---- advance blade animation from the HUD-driven speed value ----
    spinAngle += speed * 0.2f;
    if (spinAngle >= 360.0f) spinAngle -= 360.0f;

    parts.clear();

    glUseProgram(program);
    glBindVertexArray(vao);

    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();

    // ---- base ----
    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, -2.6f, 0.0f);
    transform.TransformScale(1.6f, 0.25f, 0.8f);
    drawPart(glm::vec3(0.45f, 0.28f, 0.12f), "Base");
    transform.TransformPopMatrix();

    // ---- pole ----
    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, -1.21f, 0.0f);
    transform.TransformScale(0.15f, 2.53f, 0.15f);
    drawPart(glm::vec3(0.55f, 0.55f, 0.58f), "Pole");
    transform.TransformPopMatrix();

    // ---- hub ----
    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, 0.2f, 0.0f);
    transform.TransformScale(0.3f, 0.3f, 0.3f);
    drawPart(glm::vec3(0.20f, 0.20f, 0.22f), "Hub");
    transform.TransformPopMatrix();

    // The pitch gives the face normal an X/Z component, which does
    // get swept around by the Z-axis spin, so each blade's brightness now
    // genuinely rises and falls with its orientation relative to the light
    // as it rotates -- matching real per-fragment Phong on a moving object.
    static const float kBladePitchDeg = 18.0f;

    static const glm::vec3 kBladeColors[4] = {
            glm::vec3(0.85f, 0.10f, 0.10f), // red
            glm::vec3(0.10f, 0.35f, 0.85f), // blue
            glm::vec3(0.90f, 0.55f, 0.10f), // orange
            glm::vec3(0.15f, 0.70f, 0.20f), // green
    };
    static const char* kBladeNames[4] = { "Blade0", "Blade1", "Blade2", "Blade3" };

    for (int i = 0; i < 4; ++i) {
        transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, 0.2f, 0.15f);
        transform.TransformRotate(glm::radians(spinAngle + i * 90.0f), 0.0f, 0.0f, 1.0f);
        transform.TransformTranslate(0.0f, 0.55f, 0.0f);
        transform.TransformRotate(glm::radians(kBladePitchDeg), 0.0f, 1.0f, 0.0f);
        transform.TransformScale(0.22f, 0.8f, 0.05f);
        drawPart(kBladeColors[i], kBladeNames[i]);
        transform.TransformPopMatrix();
    }

    glBindVertexArray(0);
}
