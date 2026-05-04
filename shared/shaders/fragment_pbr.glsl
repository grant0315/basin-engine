#version 330 core

in vec3 vFragPos;
in vec3 vNormal;
in vec2 vTexCoords;
in mat3 vTBN;

#define MAX_LIGHTS 8

struct Light {
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
    int type;
};

uniform Light uLights[MAX_LIGHTS];
uniform int uLightCount;

// 8 texture slots
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

const float PI = 3.14159265359;

vec2 parallaxOffset(vec2 texCoords, vec3 viewDirTangent) {
    float height = texture(texture_height, texCoords).r;
    vec2 offset = viewDirTangent.xy / viewDirTangent.z * (height * 0.05);
    return texCoords - offset;
}

float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom + 0.0001);
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return geometrySchlickGGX(NdotV, roughness) * geometrySchlickGGX(NdotL, roughness);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    vec3 viewDir = normalize(viewPos - vFragPos);
    vec3 viewDirTangent = normalize(transpose(vTBN) * viewDir);

    vec2 texCoords = vTexCoords;
    if (hasHeightMap) {
        texCoords = parallaxOffset(texCoords, viewDirTangent);
    }

    // Base color
    vec3 albedo = hasTexture ? texture(texture_diffuse, texCoords).rgb : objectColor;

    // Normal
    vec3 normal;
    if (hasNormalMap) {
        normal = texture(texture_normal, texCoords).rgb * 2.0 - 1.0;
        normal = normalize(vTBN * normal);
    } else {
        normal = normalize(vNormal);
    }

    // PBR parameters
    float roughness = hasRoughnessMap ? texture(texture_roughness, texCoords).r : uRoughness;
    float metallic = hasMetallicMap ? texture(texture_metallic, texCoords).r : uMetallic;
    float ao = hasAOMap ? texture(texture_ao, texCoords).r : 1.0;
    ao *= uAOStrength;
    vec3 emissive = hasEmissiveMap ? texture(texture_emissive, texCoords).rgb : uEmissive;
    float alpha = hasOpacityMap ? texture(texture_opacity, texCoords).r : objectAlpha;

    // Dielectric F0 = 0.04, metallic surfaces use albedo
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 result = vec3(0.0);

    for (int i = 0; i < uLightCount && i < MAX_LIGHTS; ++i) {
        vec3 lightDir;
        float attenuation = 1.0;

        if (uLights[i].type == 0) {
            // Directional
            lightDir = normalize(-uLights[i].direction);
        } else {
            // Point
            lightDir = normalize(uLights[i].position - vFragPos);
            float dist = length(uLights[i].position - vFragPos);
            attenuation = 1.0 / (uLights[i].constant + uLights[i].linear * dist + uLights[i].quadratic * dist * dist);
        }

        vec3 halfwayDir = normalize(lightDir + viewDir);
        float NDF = distributionGGX(normal, halfwayDir, roughness);
        float G = geometrySmith(normal, viewDir, lightDir, roughness);
        vec3 F = fresnelSchlick(max(dot(halfwayDir, normal), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

        float NdotL = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = kD * albedo / PI;
        result += (diffuse + specular) * uLights[i].color * uLights[i].intensity * NdotL * attenuation;
    }

    // Ambient
    vec3 ambient = vec3(0.03) * albedo * ao;
    result += ambient + emissive;

    // HDR tonemapping (simple)
    result = result / (result + vec3(1.0));
    // Gamma correction
    result = pow(result, vec3(1.0 / 2.2));

    fragColor = vec4(result, alpha);
}