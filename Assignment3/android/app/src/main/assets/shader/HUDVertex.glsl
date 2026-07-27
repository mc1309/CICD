#version 300 es
// author: Marcus Choo (m.choo@digipen.edu) (27/7/26)
// Android Programming Quiz 3
// Vertex shader for screen-space HUD quads/triangles (buttons, glyphs, the
// speed bar). No texture, no font rendering -- just flat vertex colour.
layout(location = 0) in vec2 aPosition; // pixel coordinates, top-left origin
layout(location = 1) in vec3 aColor;

uniform mat4 ORTHOMATRIX;

out vec3 vColor;

void main() {
    vColor = aColor;
    gl_Position = ORTHOMATRIX * vec4(aPosition, 0.0, 1.0);
}
