#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec3 vFragPos;
out vec3 vNormal;
out vec2 vTexCoords;
out mat3 vTBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

void main()
{
  vec4 worldPos = model * vec4(aPos, 1.0);
  vFragPos = worldPos.xyz;
  vTexCoords = aTexCoords;

  vNormal = normalize(normalMatrix * aNormal);

  // Build TBN matrix for normal mapping
  vec3 T = normalize(normalMatrix * aTangent);
  vec3 N = vNormal;
  // Re-orthogonalize T with respect to N (Gram-Schmidt)
  T = normalize(T - dot(T, N) * N);
  vec3 B = cross(N, T);
  vTBN = mat3(T, B, N);

  gl_Position = projection * view * worldPos;
}
