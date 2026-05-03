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

uniform float dotSize;
uniform float maxRadius;
uniform float softness;
uniform float gridGap;

uniform vec3 objectColor;
uniform vec3 backgroundColor;

out vec4 FragColor;

void main()
{
    vec3 N = normalize(vNormal);

    float totalBrightness = 0.0;
    for (int i = 0; i < uLightCount && i < MAX_LIGHTS; ++i) {
        vec3 L;
        float attenuation = 1.0;
        if (uLights[i].type == 0) {
            L = normalize(-uLights[i].direction);
        } else {
            L = normalize(uLights[i].position - vFragPos);
            float dist = length(uLights[i].position - vFragPos);
            attenuation = 1.0 / (uLights[i].constant + uLights[i].linear * dist + uLights[i].quadratic * dist * dist);
        }
        float diffuse = max(dot(N, L), 0.0);
        totalBrightness += diffuse * uLights[i].intensity * attenuation;
    }

    // Clamp and add base ambient
    float brightness = clamp(totalBrightness * 0.85 + 0.15, 0.15, 1.0);

    // Dot grid
    vec2 grid = gl_FragCoord.xy / dotSize;
    vec2 local = fract(grid) - 0.5;
    float dist = length(local);

    float radius = brightness * maxRadius;
    float dotMask = 1.0 - smoothstep(radius, radius + softness, dist);

    // Retro grid gap
    vec2 cellEdge = abs(local) * 2.0;
    float cellMax = max(cellEdge.x, cellEdge.y);
    float gridMask = 1.0 - smoothstep(gridGap, 1.0, cellMax);

    // Scanline effect
    float scanline = sin(gl_FragCoord.y * 3.14159265 / dotSize) * 0.5 + 0.5;
    float scanlineMask = mix(0.92, 1.0, scanline);

    vec3 litColor = objectColor * (brightness * 1.1);
    vec3 finalColor = mix(backgroundColor, litColor, dotMask * gridMask);
    finalColor *= scanlineMask;

    FragColor = vec4(finalColor, 1.0);
}
