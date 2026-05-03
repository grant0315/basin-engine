#version 330 core

in vec3 vFragPos;
in vec3 vNormal;
in vec2 vTexCoords;
in mat3 vTBN;

// Directional light
vec3 lightDir = normalize(vec3(0.3, 1.0, 0.2));

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

// Simple parallax mapping — offsets UVs based on height map and view direction
vec2 parallaxOffset(vec2 texCoords, vec3 viewDirTangent) {
  float height = texture(texture_height, texCoords).r;
  // Offset along view direction in tangent space, scaled by height
  vec2 offset = viewDirTangent.xy / viewDirTangent.z * (height * heightScale);
  return texCoords - offset;
}

void main() {
  // View direction in tangent space (for parallax)
  vec3 viewDir = normalize(viewPos - vFragPos);
  vec3 viewDirTangent = normalize(transpose(vTBN) * viewDir);

  // UV coordinates (possibly offset by parallax)
  vec2 texCoords = vTexCoords;
  if (hasHeightMap) {
    texCoords = parallaxOffset(texCoords, viewDirTangent);
  }

  // Base color
  vec3 baseColor = hasTexture ? texture(texture_diffuse, texCoords).rgb : objectColor;

  // Normal — either from normal map or vertex normal
  vec3 normal;
  if (hasNormalMap) {
    // Sample normal map (stored as RGB [0,1], remap to [-1,1])
    normal = texture(texture_normal, texCoords).rgb * 2.0 - 1.0;
    normal = normalize(vTBN * normal);
  } else {
    normal = normalize(vNormal);
  }

  // Diffuse lighting
  float diff = max(dot(normal, lightDir), 0.0);
  float ambient = 0.2;
  float diffuse = (1.0 - ambient) * diff;

  // Specular (Blinn-Phong)
  vec3 halfDir = normalize(lightDir + viewDir);
  float spec = pow(max(dot(normal, halfDir), 0.0), 32.0);

  // Roughness attenuates specular (roughness 1.0 = no specular, 0.0 = full)
  float roughness = hasRoughnessMap ? texture(texture_roughness, texCoords).r : 0.5;
  float specStrength = 0.5 * (1.0 - roughness);

  float finalLight = ambient + diffuse + specStrength * spec;

  fragColor = vec4(baseColor * finalLight, objectAlpha);
}
