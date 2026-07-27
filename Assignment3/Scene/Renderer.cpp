/**
 * author: Marcus Choo (m.choo@digipen.edu) (27/7/26)
 * Renderer.cpp
 * Android Programming Quiz 3.
 *
 * Renderer.cpp
 *
 * Implementation of the Renderer singleton.
 *
 */

#include "Renderer.h"
#include "Scene3D.h"
#include "SceneHUD.h"

Renderer& Renderer::Instance()
{
    static Renderer instance;
    return instance;
}

Renderer::~Renderer()
{
    clearModels();
}

#ifdef PLATFORM_ANDROID
void Renderer::setAssetManager(AAssetManager* mgr)
{
    assetMgr = mgr;
}
#endif

void Renderer::createModels()
{
    clearModels();
#ifdef PLATFORM_ANDROID
    scene3D  = new Scene3D(assetMgr);
    sceneHUD = new SceneHUD(assetMgr);
#else
    scene3D  = new Scene3D();
    sceneHUD = new SceneHUD();
#endif
}

void Renderer::clearModels()
{
    delete scene3D;  scene3D  = nullptr;
    delete sceneHUD; sceneHUD = nullptr;
}

bool Renderer::initializeRenderer()
{
    LOGI("Renderer::initializeRenderer");
    createModels();
    scene3D->InitModel();
    sceneHUD->InitModel();
    return true;
}

void Renderer::resize(int w, int h)
{
    LOGI("Renderer::resize %d x %d", w, h);
    scene3D->Resize(w, h);
    sceneHUD->Resize(w, h);
}

void Renderer::render()
{
    scene3D->SetFanSpeed(sceneHUD->CurrentSpeed());
    scene3D->SetCameraDistance(sceneHUD->CurrentZoom());

    // 1. Clear framebuffer.
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.05f, 0.06f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 2. Render perspective 3D scene.
    scene3D->Render();

    // 3. Render orthographic HUD over the 3D scene.
    glDisable(GL_DEPTH_TEST);
    sceneHUD->Render();

    // 4. Restore state for the next frame.
    glEnable(GL_DEPTH_TEST);
}

void Renderer::touchDown(float x, float y)
{
    // SceneHUD gets first refusal. A button press must be fully consumed and
    // must never also trigger a 3D pick underneath it.
    if (sceneHUD->HandleTouchDown(x, y)) {
        return;
    }
    scene3D->PickAt(x, y);
}

void Renderer::touchMove(float x, float y)
{
    // Reserved for the press-and-hold bonus; no-op for the core quiz.
    (void)x; (void)y;
}

void Renderer::touchUp()
{
    sceneHUD->TouchRelease();
}
