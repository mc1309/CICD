#version 300 es
// author: Marcus Choo (m.choo@digipen.edu) (27/7/26)
// Android Programming Quiz 3
// Fragment shader for screen-space HUD quads/triangles.
precision mediump float;

in vec3 vColor;
out vec4 fragColor;

void main() {
    fragColor = vec4(vColor, 1.0);
}
