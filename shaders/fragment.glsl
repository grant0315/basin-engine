#version 330 core

in vec3 vNormal;
vec3 lightDir = vec3(0.0f, 0.0f, 1.0f);

out vec4 fragColor;
void main() {
  float lightIntensity = max(dot(vNormal, lightDir), 0.0f);
  fragColor = lightIntensity * vec4(1.0f, 0.5f, 0.2f, 1.0f);
}
