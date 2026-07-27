#version 300 es
// author: Marcus Choo (m.choo@digipen.edu) (27/7/26)
// Android Programming Quiz 3
// Fragment shader for the 3D Fan model. Same eye-space ADS Phong formula as
// 6-PhongShader-Modern, fed PARTCOLOR as both the ambient and diffuse
// material colour (MaterialAmbient/MaterialDiffuse), with a fixed glossy
// MaterialSpecular so every part shows a visible highlight.
precision mediump float;

uniform vec3  MaterialAmbient;
uniform vec3  MaterialDiffuse;
uniform vec3  MaterialSpecular;

uniform vec3  LightAmbient;
uniform vec3  LightDiffuse;
uniform vec3  LightSpecular;
uniform vec3  LightPosition;   // already in eye space
uniform float ShininessFactor;

in vec3 normalCoord;
in vec3 eyeCoord;

layout(location = 0) out vec4 FinalColor;

void main() {
    vec3 N = normalize(normalCoord);
    vec3 L = normalize(LightPosition - eyeCoord);
    vec3 V = normalize(-eyeCoord);
    vec3 R = reflect(-L, N);

    float cosAngle   = max(dot(N, L), 0.0);
    float sIntensity = pow(max(dot(R, V), 0.0), ShininessFactor);

    vec3 ambient  = MaterialAmbient  * LightAmbient;
    vec3 diffuse  = MaterialDiffuse  * LightDiffuse;
    vec3 specular = MaterialSpecular * LightSpecular;

    FinalColor = vec4(ambient + (cosAngle * diffuse) + (sIntensity * specular), 1.0);
}
