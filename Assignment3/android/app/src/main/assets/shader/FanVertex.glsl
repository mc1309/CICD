#version 300 es
// author: Marcus Choo (m.choo@digipen.edu) (27/7/26)
// Android Programming Quiz 3
// Vertex shader for the 3D Fan model. Computes EYE-space position and normal
// matching 6-PhongShader-Modern / 7-PhongDirectionalLight-Modern - using
// the framework-provided Transform::TransformGetModelViewMatrix() /
// TransformGetNormalMatrix() results uploaded as MODELVIEWMATRIX / NORMALMATRIX.
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

uniform mat4 MVPMATRIX;
uniform mat4 MODELVIEWMATRIX;
uniform mat3 NORMALMATRIX;

out vec3 normalCoord;
out vec3 eyeCoord;

void main() {
    vec4 eyePos = MODELVIEWMATRIX * vec4(aPosition, 1.0);
    eyeCoord = eyePos.xyz;
    normalCoord = normalize(NORMALMATRIX * aNormal);
    gl_Position = MVPMATRIX * vec4(aPosition, 1.0);
}
