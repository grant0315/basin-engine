#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 vNormal;
out vec2 vTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
  vNormal = mat3(model) * aNormal;
  vTexCoords = aTexCoords;
  gl_Position = projection * view * model * vec4(aPos, 1.0f);
}
