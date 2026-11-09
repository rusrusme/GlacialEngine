#pragma once

#vert

layout(std140, binding = 0) uniform GridUniformData {
    mat4 u_View;
    mat4 u_Projection;
    mat4 u_InvView;
    mat4 u_InvProjection;
    vec4 u_CameraPosition;
    float u_NearClip;
    float u_FarClip;
    float u_GridScale;
    float u_FadeDistance;
};

layout(location = 0) out vec3 v_NearPoint;
layout(location = 1) out vec3 v_FarPoint;

const vec3 kPlaneVertices[6] = vec3[6](
vec3(-1.0, -1.0, 0.0),
vec3( 1.0, -1.0, 0.0),
vec3( 1.0,  1.0, 0.0),
vec3(-1.0, -1.0, 0.0),
vec3( 1.0,  1.0, 0.0),
vec3(-1.0,  1.0, 0.0)
);

vec3 unproject(vec3 ndcPoint, mat4 invView, mat4 invProj) {
    vec4 unprojected = invView * invProj * vec4(ndcPoint, 1.0);
    return unprojected.xyz / unprojected.w;
}

void main() {
    vec3 p = kPlaneVertices[gl_VertexID];
    v_NearPoint = unproject(vec3(p.xy, -1.0), u_InvView, u_InvProjection);
    v_FarPoint  = unproject(vec3(p.xy,  1.0), u_InvView, u_InvProjection);
    gl_Position = vec4(p.xy, 0.0, 1.0);
}

#frag

layout(location = 0) in vec3 v_NearPoint;
layout(location = 1) in vec3 v_FarPoint;

layout(location = 0) out vec4 o_Color;

layout(std140, binding = 0) uniform GridUniformData {
    mat4 u_View;
    mat4 u_Projection;
    mat4 u_InvView;
    mat4 u_InvProjection;
    vec4 u_CameraPosition;
    float u_NearClip;
    float u_FarClip;
    float u_GridScale;
    float u_FadeDistance;
};

// Analytical anti-aliasing with automatic high-frequency (Moiré) suppression
float drawGrid(vec2 coord, float spacing, float thicknessPixels) {
    vec2 gridCoord = coord / spacing;
    vec2 dudv = max(fwidth(gridCoord), vec2(1e-4));

    // Distance to nearest line center
    vec2 dist = abs(fract(gridCoord - 0.5) - 0.5);

    // Box-filter coverage in screen pixels
    vec2 lineWeight = clamp((dudv * (thicknessPixels * 0.5) - dist) / dudv + 0.5, 0.0, 1.0);
    float coverage = max(lineWeight.x, lineWeight.y);

    // Smoothly dissolve lines when their density exceeds screen resolution (eliminates Moiré noise)
    float maxDeriv = max(dudv.x, dudv.y);
    float nyquistFade = clamp(2.0 - maxDeriv * 2.0, 0.0, 1.0);

    return coverage * nyquistFade;
}

void main() {
    float dy = v_FarPoint.y - v_NearPoint.y;
    if (abs(dy) < 1e-6) discard;

    float t = -v_NearPoint.y / dy;
    if (t <= 0.0) discard;

    vec3 fragPos3D = v_NearPoint + t * (v_FarPoint - v_NearPoint);

    vec4 clipSpace = u_Projection * u_View * vec4(fragPos3D, 1.0);
    if (clipSpace.w <= 0.0) discard;

    float ndcDepth = clipSpace.z / clipSpace.w;
    if (ndcDepth < -1.0 || ndcDepth > 1.0) discard;

    gl_FragDepth = (ndcDepth * 0.5) + 0.5;

    // --- Uniform Global Camera LOD ---
    const float kSubdivision = 5.0;
    float baseScale = max(u_GridScale, 1e-4);

    float camDist = max(abs(u_CameraPosition.y), 1.0);
    float logScale = log(camDist * 0.25 / baseScale) / log(kSubdivision);
    float lod = floor(logScale);
    float fade = fract(logScale);

    float minorCell = pow(kSubdivision, lod) * baseScale;
    float majorCell = minorCell * kSubdivision;

    // Draw both grid layers
    float gMinor = drawGrid(fragPos3D.xz, minorCell, 1.25);
    float gMajor = drawGrid(fragPos3D.xz, majorCell, 1.85);

    float minorAlpha = 0.28 * (1.0 - smoothstep(0.0, 1.0, fade));
    float majorAlpha = 0.55;

    float finalAlpha = max(gMajor * majorAlpha, gMinor * minorAlpha);
    vec4 color = vec4(0.48, 0.48, 0.52, finalAlpha);

    // --- Coordinate Axes (Red = X, Blue = Z) ---
    vec2 axisDeriv = max(fwidth(fragPos3D.xz), vec2(1e-4));
    const float axisThicknessPixels = 2.0;

    // X-axis (Z = 0)
    float zDist = abs(fragPos3D.z);
    float zAlpha = clamp((axisDeriv.y * (axisThicknessPixels * 0.5) - zDist) / axisDeriv.y + 0.5, 0.0, 1.0);
    if (zAlpha > 0.0) {
        color = mix(color, vec4(0.92, 0.25, 0.25, 0.95), zAlpha);
    }

    // Z-axis (X = 0)
    float xDist = abs(fragPos3D.x);
    float xAlpha = clamp((axisDeriv.x * (axisThicknessPixels * 0.5) - xDist) / axisDeriv.x + 0.5, 0.0, 1.0);
    if (xAlpha > 0.0) {
        color = mix(color, vec4(0.25, 0.52, 0.95, 0.95), xAlpha);
    }

    // --- Distance & Grazing Horizon Falloff ---
    vec3 viewVec = fragPos3D - u_CameraPosition.xyz;
    float viewDist = length(viewVec);

    // 1. Distance fogging: full brightness in front, dissolving gently into the horizon
    float fadeStart = u_FadeDistance * 0.40;
    float fadeEnd   = u_FadeDistance * 0.95;
    float distanceFade = 1.0 - smoothstep(fadeStart, fadeEnd, viewDist);

    // 2. Horizon angle fade: gently tapers off as the view vector flattens against the ground plane
    float sinViewAngle = abs(viewVec.y) / max(viewDist, 1e-4);
    float angleFade = smoothstep(0.01, 0.12, sinViewAngle);

    color.a *= (distanceFade * angleFade);

    if (color.a <= 0.002) discard;

    o_Color = color;
}