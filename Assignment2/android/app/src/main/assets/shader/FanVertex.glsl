#version 300 es
// author: Marcus Choo (m.choo@digipen.edu) (13/7/26)
// Vertex shader for the 3D Fan model. Transforms each cube vertex using the
// Model-View-Projection matrix and passes a grayscale shading value to the
// fragment shader for simple faceted lighting.
layout(location = 0) in vec3 aPosition;
layout(location = 1) in float aShade;

uniform mat4 MODELVIEWPROJECTIONMATRIX;

out float vShade;

void main() {
    vShade = aShade;
    gl_Position = MODELVIEWPROJECTIONMATRIX * vec4(aPosition, 1.0);
}
