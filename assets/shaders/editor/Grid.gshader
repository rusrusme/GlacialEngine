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

// Analytical anti-aliased grid with grazing-angle compensation
float drawGrid(vec2 coord, float spacing) {
    vec2 gridCoord = coord / spacing;
    vec2 dudv = max(fwidth(gridCoord), vec2(1e-4));

    // Antialiased box-filter across line width
    vec2 lineWeight = clamp(0.5 - abs(fract(gridCoord - 0.5) - 0.5) / dudv, 0.0, 1.0);

    // Suppress lines when cell size in screen pixels becomes smaller than 2-3 pixels (prevents moiré fog)
    float lodFalloff = clamp(1.0 - max(dudv.x, dudv.y) * 0.5, 0.0, 1.0);

    return max(lineWeight.x, lineWeight.y) * lodFalloff;
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

    // --- Dynamic Zoom Scaling ---
    const float kSubdivision = 5.0;
    const float minPixelsPerCell = 60.0;

    vec2 coord = fragPos3D.xz;
    vec2 dxy = fwidth(coord);
    float pixelSize = max(dxy.x, dxy.y);

    float baseScale = max(u_GridScale, 1e-4);

    float logScale = log(pixelSize * minPixelsPerCell / baseScale) / log(kSubdivision);
    float lod = floor(logScale);
    float fade = fract(logScale);

    float minorCell = pow(kSubdivision, lod) * baseScale;
    float majorCell = minorCell * kSubdivision;

    float gMinor = drawGrid(coord, minorCell);
    float gMajor = drawGrid(coord, majorCell);

    float minorAlpha = 0.22 * (1.0 - smoothstep(0.0, 1.0, fade));
    float majorAlpha = 0.45;

    float finalAlpha = max(gMajor * majorAlpha, gMinor * minorAlpha);
    vec4 color = vec4(0.45, 0.45, 0.48, finalAlpha);

    // --- Coordinate Axes (Red = X, Blue = Z) ---
    // Antialiased 1px axis lines
    vec2 axisDeriv = max(fwidth(fragPos3D.xz), vec2(1e-4));

    // X-axis (Z = 0)
    float zDist = abs(fragPos3D.z) / axisDeriv.y;
    if (zDist < 1.0) {
        color = mix(color, vec4(0.90, 0.25, 0.25, 0.95), clamp(1.0 - zDist, 0.0, 1.0));
    }

    // Z-axis (X = 0)
    float xDist = abs(fragPos3D.x) / axisDeriv.x;
    if (xDist < 1.0) {
        color = mix(color, vec4(0.25, 0.50, 0.95, 0.95), clamp(1.0 - xDist, 0.0, 1.0));
    }

    // --- Natural View Distance Falloff ---
    // Use true Euclidean distance in 3D to eliminate elliptical/cylindrical warping
    float viewDist = length(fragPos3D - u_CameraPosition.xyz);

    // Smooth quadratic ease-out to the fade distance
    float distanceFade = clamp(1.0 - (viewDist / u_FadeDistance), 0.0, 1.0);
    distanceFade = distanceFade * distanceFade * (3.0 - 2.0 * distanceFade); // smoothstep curve

    color.a *= distanceFade;

    if (color.a <= 0.002) discard;

    o_Color = color;
}