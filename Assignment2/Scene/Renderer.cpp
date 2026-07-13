/**
 * author: Marcus Choo (m.choo@digipen.edu) (13/7/26)
 * Renderer.cpp
 *
 * Implementation of the Renderer singleton. This is the only file in the
 * project (besides Fan itself) that #includes Fan.h — Renderer.h stays
 * ignorant of concrete Models.
 */

#include "Renderer.h"
#include "Fan.h"

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
    models.push_back(new Fan(assetMgr));
#else
    models.push_back(new Fan());
#endif
}

void Renderer::initializeModels()
{
    for (Model* m : models) {
        m->InitModel();
    }
}

void Renderer::clearModels()
{
    for (Model* m : models) {
        delete m;
    }
    models.clear();
}

bool Renderer::initializeRenderer()
{
    LOGI("Renderer::initializeRenderer");
    createModels();
    initializeModels();
    return true;
}

void Renderer::resize(int w, int h)
{
    LOGI("Renderer::resize %d x %d", w, h);
    for (Model* m : models) {
        m->Resize(w, h);
    }
}

void Renderer::render()
{
    for (Model* m : models) {
        m->Render();
    }
}

void Renderer::touchDown(float x, float y)
{
    for (Model* m : models) {
        m->TouchDown(x, y);
    }
}

void Renderer::touchMove(float x, float y)
{
    for (Model* m : models) {
        m->TouchMove(x, y);
    }
}

void Renderer::touchUp()
{
    for (Model* m : models) {
        m->TouchUp();
    }
}
