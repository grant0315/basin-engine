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

// Texture samplers
uniform sampler2D texture_diffuse;
uniform sampler2D texture_normal;
uniform sampler2D texture_height;
uniform sampler2D texture_roughness;

// Feature flags
uniform bool hasTexture;
uniform bool hasNormalMap;
uniform bool hasHeightMap;
uniform bool hasRoughnessMap;

// Material / scene
uniform vec3 objectColor;
uniform float objectAlpha;
uniform vec3 viewPos;

// Parallax strength
const float heightScale = 0.05;

out vec4 fragColor;

vec2 parallaxOffset(vec2 texCoords, vec3 viewDirTangent) {
  float height = texture(texture_height, texCoords).r;
  vec2 offset = viewDirTangent.xy / viewDirTangent.z * (height * heightScale);
  return texCoords - offset;
}

vec3 calcLight(Light light, vec3 normal, vec3 viewDir, vec3 baseColor, float roughness) {
  vec3 lightDir;
  float attenuation = 1.0;

  if (light.type == 0) {
    // Directional
    lightDir = normalize(-light.direction);
  } else {
    // Point (and Spot for now)
    lightDir = normalize(light.position - vFragPos);
    float dist = length(light.position - vFragPos);
    attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);
  }

  // Diffuse
  float diff = max(dot(normal, lightDir), 0.0);

  // Specular (Blinn-Phong)
  vec3 halfDir = normalize(lightDir + viewDir);
  float spec = pow(max(dot(normal, halfDir), 0.0), 32.0);
  float specStrength = 0.5 * (1.0 - roughness);

  vec3 ambient = baseColor * 0.1 * light.color * light.intensity;
  vec3 diffuse = baseColor * diff * light.color * light.intensity;
  vec3 specular = vec3(specStrength * spec) * light.color * light.intensity;

  return (ambient + diffuse + specular) * attenuation;
}

void main() {
  vec3 viewDir = normalize(viewPos - vFragPos);
  vec3 viewDirTangent = normalize(transpose(vTBN) * viewDir);

  vec2 texCoords = vTexCoords;
  if (hasHeightMap) {
    texCoords = parallaxOffset(texCoords, viewDirTangent);
  }

  vec3 baseColor = hasTexture ? texture(texture_diffuse, texCoords).rgb : objectColor;

  vec3 normal;
  if (hasNormalMap) {
    normal = texture(texture_normal, texCoords).rgb * 2.0 - 1.0;
    normal = normalize(vTBN * normal);
  } else {
    normal = normalize(vNormal);
  }

  float roughness = hasRoughnessMap ? texture(texture_roughness, texCoords).r : 0.5;

  vec3 result = vec3(0.0);
  for (int i = 0; i < uLightCount && i < MAX_LIGHTS; ++i) {
    result += calcLight(uLights[i], normal, viewDir, baseColor, roughness);
  }

  // Ambient base level so unlit areas aren't pure black
  result += baseColor * 0.05;

  fragColor = vec4(result, objectAlpha);
}
