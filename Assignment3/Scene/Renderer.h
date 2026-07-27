#pragma once

/**
 * author: Marcus Choo (m.choo@digipen.edu) (27/7/26)
 * Renderer.h
 * Android Programming Quiz 3.
 *
 * Singleton controller -- the only thing main.cpp / NativeTemplate.cpp talk
 * to. Clear -> Scene3D -> disable depth -> SceneHUD ->
 * enable depth -> swap order
 */

#include "Model.h"
#include "Platform.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Scene3D;
class SceneHUD;

class Renderer {
public:
    static Renderer& Instance();

    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;

#ifdef PLATFORM_ANDROID
    void setAssetManager(AAssetManager* mgr);
#endif

    bool initializeRenderer();
    void resize(int w, int h);
    void render();

    // Forwarded touch pipeline. touchDown routes to SceneHUD first; only a
    // miss (no button hit) falls through to Scene3D's 3D picking.
    void touchDown(float x, float y);
    void touchMove(float x, float y);
    void touchUp();

private:
    Renderer()  = default;
    ~Renderer();

    void createModels();
    void clearModels();

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif

    Scene3D*  scene3D  = nullptr;   // owns the Fan + Ground
    SceneHUD* sceneHUD = nullptr;   // owns the 4 buttons + speed bar
};
