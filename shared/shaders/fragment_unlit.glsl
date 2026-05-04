#version 330 core

in vec3 vFragPos;
in vec3 vNormal;
in vec2 vTexCoords;
in mat3 vTBN;

// 8 texture slots (only diffuse and opacity are used by unlit)
uniform sampler2D texture_diffuse;
uniform sampler2D texture_normal;
uniform sampler2D texture_height;
uniform sampler2D texture_roughness;
uniform sampler2D texture_metallic;
uniform sampler2D texture_ao;
uniform sampler2D texture_emissive;
uniform sampler2D texture_opacity;

// Feature flags
uniform bool hasTexture;
uniform bool hasNormalMap;
uniform bool hasHeightMap;
uniform bool hasRoughnessMap;
uniform bool hasMetallicMap;
uniform bool hasAOMap;
uniform bool hasEmissiveMap;
uniform bool hasOpacityMap;

// Material scalars
uniform vec3 objectColor;
uniform float objectAlpha;
uniform float uRoughness;
uniform float uMetallic;
uniform vec3 uEmissive;
uniform float uAOStrength;

uniform vec3 viewPos;

out vec4 fragColor;

void main() {
    vec3 color = hasTexture ? texture(texture_diffuse, vTexCoords).rgb : objectColor;
    float alpha = hasOpacityMap ? texture(texture_opacity, vTexCoords).r : objectAlpha;

    // Add emissive contribution
    vec3 emissive = hasEmissiveMap ? texture(texture_emissive, vTexCoords).rgb : uEmissive;
    color += emissive;

    fragColor = vec4(color, alpha);
}