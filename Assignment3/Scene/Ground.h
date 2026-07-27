#pragma once
/**
 * author: Marcus Choo (m.choo@digipen.edu) (27/7/26)
 * Ground.h
 * Android Programming Quiz 3
 *
 * A single large quad, positioned under the Fan, that renders a procedural
 * black/white checkerboard from a stable WORLD-space XZ position in the
 * fragment shader. The checkerboard tiling itself must
 * stay put as the camera moves, so it's driven from MODELMATRIX*position
 * (world space) actual Phong lighting is computed in EYE SPACE,
 * exactly like the Fan.
 */

#include "Model.h"
#include "Transform.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Ground : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Ground(AAssetManager* mgr) : assetMgr(mgr) {}
#else
    Ground() = default;
#endif
    ~Ground() override;

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;

    void SetViewProjection(const glm::mat4& view, const glm::mat4& proj);
    void SetLight(const glm::vec3& lightPosWorld);   // world-space; converted to eye space per draw

private:
    void buildQuadGeometry();

    GLuint vao = 0, vbo = 0, ibo = 0, program = 0;

    GLint uMVP             = -1;   // MVPMATRIX
    GLint uModelMatrix     = -1;   // MODELMATRIX (world-space XZ for the checker pattern only)
    GLint uModelViewMatrix = -1;   // MODELVIEWMATRIX
    GLint uNormalMatrix    = -1;   // NORMALMATRIX
    GLint uLightAmbient    = -1;
    GLint uLightDiffuse    = -1;
    GLint uLightSpecular   = -1;
    GLint uLightPosition   = -1;   // uploaded in EYE space
    GLint uShininess       = -1;

    Transform transform;

    glm::vec3 lightPos = glm::vec3(5.0f, 8.0f, 4.0f);

    int viewportW = 1, viewportH = 1;

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif
};
