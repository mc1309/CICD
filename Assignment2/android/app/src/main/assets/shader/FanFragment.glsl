#version 300 es
// author: Marcus Choo (m.choo@digipen.edu) (13/7/26)
// Fragment shader for the 3D Fan model. Applies the uploaded part colour
// together with the interpolated grayscale shading factor to produce the
// final colour of each rendered fragment.
precision mediump float;

uniform vec3 PARTCOLOR;
in float vShade;
out vec4 fragColor;

void main() {
    fragColor = vec4(PARTCOLOR * vShade, 1.0);
}
