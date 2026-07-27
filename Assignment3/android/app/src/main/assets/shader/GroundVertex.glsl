#version 300 es
// author: Marcus Choo (m.choo@digipen.edu) (27/7/26)
// Android Programming Quiz 3
// Vertex shader for the procedural checkerboard Ground plane. Outputs
// eye-space position/normal for lighting (same technique as FanVertex.glsl)
// PLUS a stable world-space XZ position, used only by the fragment shader's
// checkerboard pattern so the tiling doesn't slide around as the camera moves.
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

uniform mat4 MVPMATRIX;
uniform mat4 MODELMATRIX;
uniform mat4 MODELVIEWMATRIX;
uniform mat3 NORMALMATRIX;

out vec3 normalCoord;
out vec3 eyeCoord;
out vec2 vWorldXZ;

void main() {
    vec4 eyePos = MODELVIEWMATRIX * vec4(aPosition, 1.0);
    eyeCoord = eyePos.xyz;
    normalCoord = normalize(NORMALMATRIX * aNormal);
    vWorldXZ = (MODELMATRIX * vec4(aPosition, 1.0)).xz;
    gl_Position = MVPMATRIX * vec4(aPosition, 1.0);
}
