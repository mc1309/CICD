#pragma once
/**
 * author: Marcus Choo (m.choo@digipen.edu) (27/7/26)
 * SceneHUD.h
 * Android Programming Quiz 3
 *
 * Four buttons (+SPD, -SPD, +ZOOM, -ZOOM) and a 20-block speed bar, all drawn
 * in one orthographic screen-space pass. SceneHUD owns no 3D state itself --
 * it just reads/writes the shared speed & zoom values that Renderer forwards
 * to Scene3D every frame (CurrentSpeed() / CurrentZoom()).
 *
 * BONUS: smooth zoom/speed easing declarations (line 61, 62, 64)
 */

#include "Model.h"
#include "Transform.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class SceneHUD : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit SceneHUD(AAssetManager* mgr) : assetMgr(mgr) {}
#else
    SceneHUD() = default;
#endif
    ~SceneHUD() override;

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;

    // HUD hit-testing. Named HandleTouchDown (not TouchDown) because
    // Model already declares a virtual `void TouchDown(float,float)
    // Returns true if a button was hit
    bool HandleTouchDown(float x, float y);
    void TouchRelease();

    float CurrentSpeed() const { return speed; }
    float CurrentZoom()  const { return zoomDistance; }

private:
    struct Rect { float x, y, w, h; };
    static bool PointInRect(float px, float py, const Rect& r);

    void layoutButtons(int w, int h);
    void rebuildGeometry();

    GLuint vao = 0, vbo = 0, ibo = 0, program = 0;
    GLint  uOrtho = -1;
    GLsizei lastIndexCount = 0;

    Transform transform;

    Rect btnSpeedUp{}, btnSpeedDown{}, btnZoomIn{}, btnZoomOut{};

    float speed        = 8.0f;   // 0..20 -- Part 6
    float zoomDistance  = 8.0f;   // 3..15 -- Part 7 (mirrors Scene3D's clamp range)
    float targetSpeed   = 8.0f;   // 0..20  -- set instantly by button presses
    float targetZoom    = 8.0f;   // 3..15  -- set instantly by button presses

    static constexpr float kEaseFactor = 0.12f; // fraction closed per frame (bonus: smooth easing)

    int activeButton = -1;    // which button (if any) is currently held: 0..3, -1 = none
    int viewportW = 1, viewportH = 1;

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif
};
