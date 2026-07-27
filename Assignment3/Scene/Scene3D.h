#pragma once
/**
 * author: Marcus Choo (m.choo@digipen.edu) (27/7/26)
 * Scene3D.h
 * Android Programming Quiz 3
 *
 * Scene3D owns everything perspective-projected and depth-tested: the Fan
 * (from Quiz 2) plus the new procedural Ground. It applies a single shared
 * Phong light and an adjustable camera distance to both before drawing them,
 * and owns 3D picking (PickAt).
 */

#include "Model.h"
#include "Fan.h"
#include "Ground.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Scene3D : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Scene3D(AAssetManager* mgr) : assetMgr(mgr) {}
#else
    Scene3D() = default;
#endif
    ~Scene3D() override;

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;

    void SetCameraDistance(float d);   // called by Renderer using SceneHUD's zoom value
    void SetFanSpeed(float s);         // called by Renderer using SceneHUD's speed value

    bool PickAt(float screenX, float screenY);   // 3D picking -- Part 8

private:
    Fan*    fan    = nullptr;
    Ground* ground = nullptr;

    float cameraDistance = 8.0f;   // clamped to [3, 20]
    int   viewportW = 1, viewportH = 1;

    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 projMatrix = glm::mat4(1.0f);

    // World-space light POSITION (a point light, not directional). Fan/Ground
    // each convert this into eye space (View * lightPos) themselves right
    // before uploading it, since their fragment shaders do all Phong math in
    // eye space (see Fan.cpp / Ground.cpp). Placed up, to the side, and on
    // the SAME general side as the camera (positive Z) -- with the light
    // behind the object instead, every camera-facing surface points away
    // from it and reads almost pure ambient (black).
    glm::vec3 lightPos = glm::vec3(4.5f, 9.0f, 5.5f);

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif
};
