/**
 * author: Marcus Choo (m.choo@digipen.edu) (2/7/26)
 * Square.cpp
 *
 * Implements the square model.
 * Creates the shader program and vertex buffers,
 * and renders a static square independently of
 * the triangle.
 */
#define LOG_TAG "Square"

#include "Square.h"
#include "ShaderHelper.h"

// ---------------------------------------------------------------------------
// Vertex data — offset to the right side of clip space so it does not
// overlap the triangle, drawn as a GL_TRIANGLE_FAN (4 vertices = 1 quad).
// ---------------------------------------------------------------------------

static const GLfloat kPositions[] = {
        0.40f,  0.95f,
        0.40f,  0.75f,
        0.75f,  0.75f,
        0.75f,  0.95f
};

#ifdef PLATFORM_ANDROID
Square::Square(AAssetManager* assetMgr) : mgr(assetMgr) {}
#else
Square::Square() = default;
#endif

Square::~Square()
{
    glDeleteBuffers(1, &vboPos);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(programID);
}

void Square::InitModel()
{
    LOGI("Square::InitModel");

#ifdef PLATFORM_ANDROID
    programID = ShaderHelper::buildProgramFromAssets(mgr, "shader/YellowSquareVertex.glsl",
                                                     "shader/YellowSquareFragment.glsl");
#else
    programID = ShaderHelper::buildProgramFromFile("YellowSquareVertex.glsl",
                                                     "YellowSquareFragment.glsl");
#endif
    if (!programID) { LOGE("Square: could not create program"); return; }

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kPositions), kPositions, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    LOGI("Square::InitModel done");
}

void Square::Render()
{
    glUseProgram(programID);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

void Square::Resize(int w, int h)
{
    // Square does not need to react to resize; default Model::Resize()
    // behavior would be sufficient, but this override is kept explicit
    // to mirror Triangle's lifecycle for symmetry.
    glViewport(0, 0, w, h);
}
