#pragma once

/**
 * author: Marcus Choo (m.choo@digipen.edu) (13/7/26)
 * Renderer.h
 *
 * Singleton controller — the only thing main.cpp / NativeTemplate.cpp
 * talk to. Owns all Model instances through the Model interface and
 * drives their lifecycle (init / resize / render / touch) every frame.
 *
 */

#include "Model.h"
#include "Platform.h"
#include <vector>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

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

    // Forwarded to every Model — see Model::TouchDown/TouchMove/TouchUp.
    void touchDown(float x, float y);
    void touchMove(float x, float y);
    void touchUp();

private:
    Renderer()  = default;
    ~Renderer();

    void createModels();
    void initializeModels();
    void clearModels();

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif

    std::vector<Model*> models;   // owns Fan (and any other registered Models)
};
