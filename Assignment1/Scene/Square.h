#pragma once

/**
 * author: Marcus Choo (m.choo@digipen.edu) (2/7/26)
 * Square.h
 *
 * Concrete Model: a simple flat-colored square rendered alongside
 * Triangle. Owns its own shader program, VAO, and VBOs — it does not
 * reuse Triangle's GPU resources. Proves the Model/Renderer
 * architecture built in Task 1 scales to multiple shapes (Task 3).
 */

#include "Platform.h"
#include "Model.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Square : public Model {
public:
#ifdef PLATFORM_ANDROID
    explicit Square(AAssetManager* assetMgr);
#else
    Square();
#endif
    ~Square() override;

    void InitModel()         override;
    void Render()             override;
    void Resize(int w, int h) override;

private:
#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif
    GLuint programID = 0;
    GLuint vao       = 0;
    GLuint vboPos    = 0;
};
