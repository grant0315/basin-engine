#version 330 core

in vec3 vFragPos;
in vec3 vNormal;
in vec2 vTexCoords;
in mat3 vTBN;

uniform vec3 lightPos;
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
    vec3 L = normalize(lightPos - vFragPos);

    float diffuse = max(dot(N, L), 0.0);
    float brightness = diffuse * 0.85 + 0.15;

    // Dot grid
    vec2 grid = gl_FragCoord.xy / dotSize;
    vec2 local = fract(grid) - 0.5;
    float dist = length(local);

    float radius = brightness * maxRadius;
    float dotMask = 1.0 - smoothstep(radius, radius + softness, dist);

    // Retro grid gap — darken space between dot cells
    vec2 cellEdge = abs(local) * 2.0;
    float cellMax = max(cellEdge.x, cellEdge.y);
    float gridMask = 1.0 - smoothstep(gridGap, 1.0, cellMax);

    // Scanline effect (subtle horizontal banding)
    float scanline = sin(gl_FragCoord.y * 3.14159265 / dotSize) * 0.5 + 0.5;
    float scanlineMask = mix(0.92, 1.0, scanline);

    // Use the object's own color as the basis
    vec3 litColor = objectColor * (brightness * 1.1);
    vec3 finalColor = mix(backgroundColor, litColor, dotMask * gridMask);
    finalColor *= scanlineMask;

    FragColor = vec4(finalColor, 1.0);
}
