#pragma once

#common

layout(std140, binding = 0) uniform FrameBuffer {
    mat4 u_ViewProjection;
    mat4 u_View;
    mat4 u_Projection;
    vec4 u_CameraPosition;
    vec2 u_ViewportSize;
    float u_Time;
    float u_DeltaTime;
};

struct InstanceData {
    mat4 model;
    vec4 color;
    vec4 material_params; // x = Metallic, y = Roughness, z = AO, w = NormalScale
};

layout(std430, binding = 1) readonly buffer InstanceBuffer {
    InstanceData instances[];
};
