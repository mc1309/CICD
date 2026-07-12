#pragma once
/**
 * author: Marcus Choo (m.choo@digipen.edu) (13/7/26)
 * Declares the Fan model used across Android, Desktop, and WebGL. The class
 * stores the shared cube geometry, shader program, transform stack, animation
 * state, viewport dimensions, and gesture data required to render and control
 * the 3D fan.
 */

#include "Model.h"
#include "Transform.h"
#include <chrono>

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

    void TouchDown(float x, float y) override;
    void TouchMove(float x, float y) override;
    void TouchUp() override;

private:
    void buildCubeGeometry();
    void drawPart(const glm::vec3& color);

    // ---- GL objects (one shared cube) ----
    GLuint vao = 0;
    GLuint vbo = 0;   // positions sub-region + shade sub-region
    GLuint ibo = 0;
    GLuint program = 0;
    GLint  uMVP = -1;
    GLint  uPartColor = -1;

    // ---- matrix stack (provided) ----
    Transform transform;

    // ---- animation / gesture state (per assignment spec) ----
    float spinAngle = 0.0f;     // current blade angle (degrees)
    bool  fanOn     = true;     // toggled by a tap
    float dragBoost = 0.0f;     // extra speed while dragging, 0 otherwise

    static constexpr float kBaseSpeed    = 1.5f;   // deg/frame when ON
    static constexpr float kMaxBoost     = 20.0f;  // clamp for the drag boost
    static constexpr float kBoostScale   = 8.0f;   // px/ms -> deg/frame
    static constexpr float kTapThreshold = 12.0f;  // px: below this it's a tap

    float lastX = 0.0f, lastY = 0.0f;  // previous touch position
    float movedDistance = 0.0f;        // total movement since touch-down
    bool  dragging = false;
    std::chrono::steady_clock::time_point lastMoveTime;

    // ---- viewport ----
    int viewportW = 1, viewportH = 1;

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif
};
