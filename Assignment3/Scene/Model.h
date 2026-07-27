#pragma once
/**
 * author: Marcus Choo (m.choo@digipen.edu) (27/7/26)
 * Model.h
 *
 * Abstract base class representing a renderable object.
 * All renderable models inherit from this class and implement
 * initialization, rendering, and viewport resize behaviour.
 *
 */
class Model {
public:
    Model() {}
    virtual ~Model() {}
    virtual void InitModel() = 0; // called once, before the first frame
    virtual void Render() = 0; // called once per frame
    virtual void Resize(int w, int h) {} // called whenever the surface/window resizes

    // Touch / mouse gesture pipeline (Chapter 2 tutorial 4 pattern).
    // x, y are in surface pixel coordinates.
    virtual void TouchDown(float x, float y) {}
    virtual void TouchMove(float x, float y) {}
    virtual void TouchUp() {}
};
