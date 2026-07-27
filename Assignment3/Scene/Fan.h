#pragma once
/**
 * author: Marcus Choo (m.choo@digipen.edu) (27/7/26)
 * Fan.h
 * Android Programming Quiz 3
 *
 * Declares the Fan model used across Android, Desktop, and WebGL.
 *
 * Changes from Quiz 2:
 *  - TouchDown/TouchMove/TouchUp tap-to-toggle & drag-to-boost gesture code
 *    is REMOVED. Speed now arrives every frame from SceneHUD via SetSpeed().
 *  - The camera is no longer baked into the part hierarchy; Scene3D supplies
 *    a shared view/projection matrix every frame via SetViewProjection().
 *  - Per-fragment Phong shading replaces the old baked per-vertex "shade"
 *    gradient. Lighting is computed in EYE SPACE, the vertex
 *    shader outputs eyeCoord (View*Model*position) and normalCoord
 *    (NormalMatrix*normal, where NormalMatrix = inverse-transpose of
 *    View*Model's upper 3x3 -- built in via Transform::TransformGetNormalMatrix),
 *    and the fragment shader does ambient + diffuse + specular against a
 *    light position that's pre-transformed into eye space every draw call.
 *  - Every part drawn during Render() is cached (name + WORLD-space matrix +
 *    local half extents) in `parts` so Scene3D::PickAt() can ray-test each
 *    part of the fan for 3D picking (Quiz 3 Part 8). Picking stays in world
 *    space -- only the *lighting* moved to eye space.
 */

#include "Model.h"
#include "Transform.h"
#include <vector>
#include <string>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Fan : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Fan(AAssetManager* mgr) : assetMgr(mgr) {}
#else
    Fan() = default;
#endif
    ~Fan() override;

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;

    // ---- Driven by Scene3D every frame (Quiz 3 Parts 6 / 7) ----
    void SetViewProjection(const glm::mat4& view, const glm::mat4& proj);
    void SetLight(const glm::vec3& lightPosWorld);   // world-space; converted to eye space per draw
    void SetSpeed(float speed0to20);

    // ---- 3D picking support (Quiz 3 Part 8) ----
    struct PickablePart {
        std::string name;
        glm::mat4   modelMatrix;   // WORLD-space transform of this part (pre-view)
        glm::vec3   halfExtents;   // local unit-cube half extents (post local-scale)
    };
    const std::vector<PickablePart>& GetPickableParts() const { return parts; }

    void SetHighlightedPart(int index) { highlightedPart = index; }
    int  GetHighlightedPart() const    { return highlightedPart; }

private:
    void buildCubeGeometry();
    void drawPart(const glm::vec3& color, const char* name);

    // ---- GL objects (one shared cube, now with per-face normals) ----
    GLuint vao = 0;
    GLuint vbo = 0;   // positions sub-region + normal sub-region
    GLuint ibo = 0;
    GLuint program = 0;

    // ---- uniform locations (names match 6-PhongShader-Modern) ----
    GLint uMVP              = -1;   // MVPMATRIX
    GLint uModelViewMatrix  = -1;   // MODELVIEWMATRIX
    GLint uNormalMatrix     = -1;   // NORMALMATRIX
    GLint uMaterialAmbient  = -1;
    GLint uMaterialDiffuse  = -1;
    GLint uMaterialSpecular = -1;
    GLint uLightAmbient     = -1;
    GLint uLightDiffuse     = -1;
    GLint uLightSpecular    = -1;
    GLint uLightPosition    = -1;   // uploaded in EYE space
    GLint uShininess        = -1;

    // ---- matrix stack (provided) ----
    Transform transform;

    // ---- animation state ----
    float spinAngle = 0.0f;     // current blade angle (degrees)
    float speed     = 8.0f;     // 0..20, set every frame by SceneHUD via SetSpeed()

    // ---- lighting (world-space light position; eye-space math done per-draw) ----
    glm::vec3 lightPos = glm::vec3(5.0f, 8.0f, 4.0f);

    // ---- picking state ----
    int highlightedPart = -1;
    std::vector<PickablePart> parts;

    // ---- viewport ----
    int viewportW = 1, viewportH = 1;

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif
};
