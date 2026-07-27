/**
 * author: Marcus Choo (m.choo@digipen.edu) (27/7/26)
 * Ground.cpp
 * Android Programming Quiz 3
 *
 * Implements the procedural checkerboard ground plane. See Ground.h.
 */

#include "Ground.h"
#include "ShaderHelper.h"
#include <glm/gtc/type_ptr.hpp>

namespace {

// One big quad in the XZ plane, facing up (+Y). World-space XZ position
// (interpolated per-fragment) drives the procedural checkerboard.
const float kPositions[4 * 3] = {
    -40.0f, 0.0f, -40.0f,
     40.0f, 0.0f, -40.0f,
     40.0f, 0.0f,  40.0f,
    -40.0f, 0.0f,  40.0f,
};
const float kNormals[4 * 3] = {
    0,1,0,  0,1,0,  0,1,0,  0,1,0,
};
const GLushort kIndices[6] = { 0, 2, 1, 0, 3, 2};

const glm::vec3 kLightAmbient (0.28f, 0.28f, 0.30f);
const glm::vec3 kLightDiffuse (0.90f, 0.90f, 0.85f);
const glm::vec3 kLightSpecular(1.00f, 1.00f, 1.00f);
const float kShininess = 10.0f; // matte floor -> broad, faint highlight

    const char* kVertSrc = R"GLSL(#version 300 es
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
uniform mat4 MVPMATRIX;
uniform mat4 MODELMATRIX;
uniform mat4 MODELVIEWMATRIX;
uniform mat3 NORMALMATRIX;
out vec3 normalCoord;
out vec3 eyeCoord;
out vec2 vWorldXZ;
void main() {
    vec4 eyePos = MODELVIEWMATRIX * vec4(aPosition, 1.0);
    eyeCoord = eyePos.xyz;
    normalCoord = normalize(NORMALMATRIX * aNormal);
    vWorldXZ = (MODELMATRIX * vec4(aPosition, 1.0)).xz;
    gl_Position = MVPMATRIX * vec4(aPosition, 1.0);
}
)GLSL";

    const char* kFragSrc = R"GLSL(#version 300 es
precision mediump float;
uniform vec3  LightAmbient;
uniform vec3  LightDiffuse;
uniform vec3  LightSpecular;
uniform vec3  LightPosition;   // already in eye space
uniform float ShininessFactor;
in vec3 normalCoord;
in vec3 eyeCoord;
in vec2 vWorldXZ;
layout(location = 0) out vec4 FinalColor;

float checker(vec2 worldXZ, float tileSize) {
    vec2 cell = floor(worldXZ / tileSize);
    return mod(cell.x + cell.y, 2.0); // 0.0 or 1.0 -> mix() between two colours
}

void main() {
    float parity = checker(vWorldXZ, 2.0);
    vec3 colorA = vec3(0.85, 0.85, 0.88);
    vec3 colorB = vec3(0.08, 0.09, 0.11);
    vec3 MaterialAmbient  = mix(colorA, colorB, parity);
    vec3 MaterialDiffuse  = MaterialAmbient;
    vec3 MaterialSpecular = vec3(0.25);

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

Ground::~Ground()
{
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (ibo) glDeleteBuffers(1, &ibo);
    if (program) glDeleteProgram(program);
}

void Ground::buildQuadGeometry()
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

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(uintptr_t)posBytes);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices), kIndices, GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void Ground::InitModel()
{
    LOGI("Ground::InitModel");
    buildQuadGeometry();

#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(assetMgr, "shader/GroundVertex.glsl", "shader/GroundFragment.glsl");
    if (!program) program = ShaderHelper::buildProgram(kVertSrc, kFragSrc);
#else
    program = ShaderHelper::buildProgramFromFile("GroundVertex.glsl", "GroundFragment.glsl");
    if (!program) program = ShaderHelper::buildProgram(kVertSrc, kFragSrc);
#endif

    uMVP             = glGetUniformLocation(program, "MVPMATRIX");
    uModelMatrix     = glGetUniformLocation(program, "MODELMATRIX");
    uModelViewMatrix = glGetUniformLocation(program, "MODELVIEWMATRIX");
    uNormalMatrix    = glGetUniformLocation(program, "NORMALMATRIX");
    uLightAmbient    = glGetUniformLocation(program, "LightAmbient");
    uLightDiffuse    = glGetUniformLocation(program, "LightDiffuse");
    uLightSpecular   = glGetUniformLocation(program, "LightSpecular");
    uLightPosition   = glGetUniformLocation(program, "LightPosition");
    uShininess       = glGetUniformLocation(program, "ShininessFactor");

    transform.TransformInit();
}

void Ground::Resize(int w, int h)
{
    viewportW = w > 0 ? w : 1;
    viewportH = h > 0 ? h : 1;
}

void Ground::SetViewProjection(const glm::mat4& view, const glm::mat4& proj)
{
    glm::mat4 v = view, p = proj;
    transform.TransformSetMatrixMode(VIEW_MATRIX);
    transform.TransformLoadMatrix(&v);
    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformLoadMatrix(&p);
}

void Ground::SetLight(const glm::vec3& lightPosWorld)
{
    lightPos = lightPosWorld;
}

void Ground::Render()
{
    glUseProgram(program);
    glBindVertexArray(vao);

    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();
    transform.TransformTranslate(0.0f, -2.85f, 0.0f); // sits just below the fan's base

    glm::mat4 model     = *transform.TransformGetModelMatrix();
    glm::mat4 modelView = *transform.TransformGetModelViewMatrix();
    glm::mat3 normalMatrix;
    transform.TransformGetNormalMatrix(&normalMatrix);

    glm::vec3 lightPosEye = glm::vec3(*transform.TransformGetViewMatrix() * glm::vec4(lightPos, 1.0f));

    glUniformMatrix4fv(uMVP, 1, GL_FALSE,
                       glm::value_ptr(*transform.TransformGetModelViewProjectionMatrix()));
    glUniformMatrix4fv(uModelMatrix,     1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(uModelViewMatrix, 1, GL_FALSE, glm::value_ptr(modelView));
    glUniformMatrix3fv(uNormalMatrix,    1, GL_FALSE, glm::value_ptr(normalMatrix));
    glUniform3fv(uLightAmbient,  1, glm::value_ptr(kLightAmbient));
    glUniform3fv(uLightDiffuse,  1, glm::value_ptr(kLightDiffuse));
    glUniform3fv(uLightSpecular, 1, glm::value_ptr(kLightSpecular));
    glUniform3fv(uLightPosition, 1, glm::value_ptr(lightPosEye));
    glUniform1f(uShininess, kShininess);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

    glBindVertexArray(0);
}
