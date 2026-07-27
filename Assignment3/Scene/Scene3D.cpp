/**
 * author: Marcus Choo (m.choo@digipen.edu) (27/7/26)
 * Scene3D.cpp
 * Android Programming Quiz 3
 *
 * Implements Scene3D: shared camera/light setup, draw order, and 3D picking.
 *
 * PickAt() follows the same three-step technique demonstrated in the Scene
 * Graph examples' PickAt() (1-SceneGraph-RobotArm-Modern /
 * 2-SceneGraph-Windmill-Modern):

 * Lighting is a single POINT light -- `lightPos` is a fixed world-space
 * position. Fan/Ground each convert it into an eye-space position (View *
 * worldLightPos) before uploading it, since their fragment shaders do all
 * Phong lighting math in eye space.
 */

#include "Scene3D.h"
#include "Transform.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <algorithm>
#include <cmath>

namespace {

// Ray-vs-oriented-bounding-box test, done in the box's own local space.
// modelMatrix carries the part's world transform (translate * rotate * scale);
// halfExtents is the LOCAL half-size of the box before that transform.
    bool RayIntersectsOBB(const glm::vec3& rayOriginWorld, const glm::vec3& rayDirWorld,
                          const glm::mat4& modelMatrix, const glm::vec3& halfExtents,
                          float& outWorldDistance)
    {
        glm::mat4 inv = glm::inverse(modelMatrix);
        glm::vec3 localOrigin = glm::vec3(inv * glm::vec4(rayOriginWorld, 1.0f));
        glm::vec3 localDir    = glm::vec3(inv * glm::vec4(rayDirWorld,    0.0f));

        float tmin = -1e30f, tmax = 1e30f;
        for (int axis = 0; axis < 3; ++axis) {
            float o = localOrigin[axis];
            float d = localDir[axis];
            float h = halfExtents[axis];
            if (std::fabs(d) < 1e-8f) {
                if (o < -h || o > h) return false;
            } else {
                float t1 = (-h - o) / d;
                float t2 = ( h - o) / d;
                if (t1 > t2) std::swap(t1, t2);
                tmin = std::max(tmin, t1);
                tmax = std::min(tmax, t2);
                if (tmin > tmax) return false;
            }
        }
        if (tmax < 0.0f) return false;

        float localT = (tmin >= 0.0f) ? tmin : tmax;
        glm::vec3 localHit = localOrigin + localDir * localT;
        glm::vec3 worldHit  = glm::vec3(modelMatrix * glm::vec4(localHit, 1.0f));
        outWorldDistance = glm::length(worldHit - rayOriginWorld);
        return true;
    }

} // namespace

Scene3D::~Scene3D()
{
    delete fan;
    delete ground;
}

void Scene3D::InitModel()
{
    LOGI("Scene3D::InitModel");
#ifdef PLATFORM_ANDROID
    fan    = new Fan(assetMgr);
    ground = new Ground(assetMgr);
#else
    fan    = new Fan();
    ground = new Ground();
#endif
    fan->InitModel();
    ground->InitModel();
}

void Scene3D::Resize(int w, int h)
{
    viewportW = w > 0 ? w : 1;
    viewportH = h > 0 ? h : 1;

    float aspect = static_cast<float>(viewportW) / static_cast<float>(viewportH);
    projMatrix = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 1000.0f);

    fan->Resize(viewportW, viewportH);
    ground->Resize(viewportW, viewportH);
}

void Scene3D::SetCameraDistance(float d)
{
    cameraDistance = std::clamp(d, 3.0f, 15.0f);
}

void Scene3D::SetFanSpeed(float s)
{
    if (fan) fan->SetSpeed(s);
}

void Scene3D::Render()
{
    glViewport(0, 0, viewportW, viewportH);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.05f, 0.06f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Camera: same fixed pull-back + tilt as Quiz 2, now driven by an
    // adjustable distance (SetCameraDistance) instead of a hard-coded -8.
    viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.8f, -cameraDistance));
    viewMatrix = glm::rotate(viewMatrix, glm::radians(20.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    ground->SetViewProjection(viewMatrix, projMatrix);
    ground->SetLight(lightPos);
    ground->Render();

    fan->SetViewProjection(viewMatrix, projMatrix);
    fan->SetLight(lightPos);
    fan->Render();
}

bool Scene3D::PickAt(float screenX, float screenY)
{
    if (!fan) return false;

    // ---- Step 1: unproject the click into a world-space ray ----
    int vp[4] = { 0, 0, viewportW, viewportH };
    float winY = static_cast<float>(viewportH) - screenY; // GL viewport is bottom-left origin

    Transform t; // scratch instance, used only for its unproject math
    glm::mat4 v = viewMatrix, p = projMatrix;
    t.TransformSetMatrixMode(VIEW_MATRIX);       t.TransformLoadMatrix(&v);
    t.TransformSetMatrixMode(PROJECTION_MATRIX); t.TransformLoadMatrix(&p);

    float nx, ny, nz, fx, fy, fz;
    t.TransformUnproject(screenX, winY, 0.0f,
                         t.TransformGetViewMatrix(), t.TransformGetProjectionMatrix(),
                         vp, &nx, &ny, &nz);
    t.TransformUnproject(screenX, winY, 1.0f,
                         t.TransformGetViewMatrix(), t.TransformGetProjectionMatrix(),
                         vp, &fx, &fy, &fz);

    glm::vec3 rayOrigin(nx, ny, nz);
    glm::vec3 rayDir = glm::normalize(glm::vec3(fx - nx, fy - ny, fz - nz));

    // ---- Step 2: test the ray against every pickable part ----
    const auto& parts = fan->GetPickableParts();
    int   bestIndex = -1;
    float bestDist  = 1e30f;
    for (size_t i = 0; i < parts.size(); ++i) {
        float worldDist;
        if (RayIntersectsOBB(rayOrigin, rayDir, parts[i].modelMatrix, parts[i].halfExtents, worldDist)) {
            // ---- Step 3: keep the nearest hit ----
            if (worldDist < bestDist) {
                bestDist  = worldDist;
                bestIndex = static_cast<int>(i);
            }
        }
    }

    if (bestIndex < 0) {
        fan->SetHighlightedPart(-1);   // clicked empty space -> clear highlight
        return false;
    }

    if (fan->GetHighlightedPart() == bestIndex) {
        LOGI("WindmillScene: %s picked - highlight off", parts[bestIndex].name.c_str());
        fan->SetHighlightedPart(-1);   // clicked the same part again -> clear
    } else {
        LOGI("WindmillScene: %s picked - highlight on", parts[bestIndex].name.c_str());
        fan->SetHighlightedPart(bestIndex);
    }
    return true;
}
