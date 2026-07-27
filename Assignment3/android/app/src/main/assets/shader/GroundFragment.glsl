#version 300 es
// author: Marcus Choo (m.choo@digipen.edu) (27/7/26)
// Android Programming Quiz 3
// Fragment shader for the Ground plane. Computes a checkerboard purely
// procedurally from world-space XZ position (no texture), then lights the
// result with the same eye-space ADS Phong formula as the Fan.
precision mediump float;

uniform vec3  LightAmbient;
uniform vec3  LightDiffuse;
uniform vec3  LightSpecular;
uniform vec3  LightPosition;   // already in eye space
uniform float ShininessFactor;

in vec3 normalCoord;
in vec3 eyeCoord;
in vec2 vWorldXZ;

layout(location = 0) out vec4 FinalColor;

float checker(vec2 worldXZ, float tileSize) {
    vec2 cell = floor(worldXZ / tileSize);
    return mod(cell.x + cell.y, 2.0); // 0.0 or 1.0 -> mix() between two colours
}

void main() {
    float parity = checker(vWorldXZ, 2.0);
    vec3 colorA = vec3(0.85, 0.85, 0.88);
    vec3 colorB = vec3(0.08, 0.09, 0.11);
    vec3 MaterialAmbient  = mix(colorA, colorB, parity);
    vec3 MaterialDiffuse  = MaterialAmbient;
    vec3 MaterialSpecular = vec3(0.25);

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
