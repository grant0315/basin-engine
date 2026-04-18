#version 330 core

in vec3 vNormal;
in vec2 vTexCoords;
vec3 lightDir = vec3(0.0f, 1.0f, 0.0f);

uniform sampler2D texture_diffuse;
uniform vec3 objectColor;
uniform bool hasTexture;

out vec4 fragColor;
void main() {
  vec3 normalizedLight = normalize(lightDir);
  float lightIntensity = max(dot(normalize(vNormal), normalizedLight), 0.0f);
  float ambient = 0.2f;
  float finalLight = ambient + (1.0f - ambient) * lightIntensity;
  
  vec3 baseColor = hasTexture ? texture(texture_diffuse, vTexCoords).rgb : objectColor;
  fragColor = vec4(baseColor * finalLight, 1.0f);
}
