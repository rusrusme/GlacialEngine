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

vec4 calculateGrid(vec3 fragPos3D, float scale, bool drawAxes) {
    vec2 coord = fragPos3D.xz * scale;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / max(derivative, vec2(1e-5));
    float line = min(grid.x, grid.y);
    float alpha = 1.0 - min(line, 1.0);

    // Fine grid: subtle gray lines; Coarse grid: more visible lines
    vec4 color = drawAxes ? vec4(0.45, 0.45, 0.48, alpha * 0.75)
    : vec4(0.30, 0.30, 0.33, alpha * 0.35);

    if (drawAxes) {
        // X-axis (Z = 0) -> Red
        float zDist = abs(fragPos3D.z) / max(fwidth(fragPos3D.z), 1e-5);
        if (zDist < 1.0) {
            color = mix(color, vec4(0.90, 0.25, 0.25, 0.95), 1.0 - zDist);
        }
        // Z-axis (X = 0) -> Blue
        float xDist = abs(fragPos3D.x) / max(fwidth(fragPos3D.x), 1e-5);
        if (xDist < 1.0) {
            color = mix(color, vec4(0.25, 0.50, 0.95, 0.95), 1.0 - xDist);
        }
    }
    return color;
}

void main() {
    float dy = v_FarPoint.y - v_NearPoint.y;
    if (abs(dy) < 1e-6) {
        discard;
    }

    float t = -v_NearPoint.y / dy;
    if (t <= 0.0) {
        discard;
    }

    vec3 fragPos3D = v_NearPoint + t * (v_FarPoint - v_NearPoint);

    vec4 clipSpace = u_Projection * u_View * vec4(fragPos3D, 1.0);
    if (clipSpace.w <= 0.0) {
        discard;
    }

    float ndcDepth = clipSpace.z / clipSpace.w;
    if (ndcDepth < -1.0 || ndcDepth > 1.0) {
        discard;
    }

    // Write depth for correct occlusion with 3D scene objects
    gl_FragDepth = (ndcDepth * 0.5) + 0.5;

    // Dual-scale grid
    vec4 fineGrid   = calculateGrid(fragPos3D, u_GridScale, false);
    vec4 coarseGrid = calculateGrid(fragPos3D, u_GridScale * 0.1, true);

    vec4 finalColor = max(fineGrid, coarseGrid);

    // Fade with distance from camera
    float dist = length(fragPos3D.xz - u_CameraPosition.xz);
    float fade = clamp(1.0 - (dist / u_FadeDistance), 0.0, 1.0);
    finalColor.a *= (fade * fade);

    if (finalColor.a <= 0.01) {
        discard;
    }

    o_Color = finalColor;
}