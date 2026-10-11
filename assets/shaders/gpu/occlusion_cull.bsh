#comp
#pragma once

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

struct GPUDrawCommand {
    uint count;
    uint instance_count;
    uint first_index;
    int  base_vertex;
    uint base_instance;
};

struct GPUInstanceData {
    mat4 model_matrix;
    vec4 bounding_center_radius;
    vec4 bounding_box_min;
    vec4 bounding_box_max;
    uint material_id;
    uint mesh_id;
    uint flags;
    uint padding;
};

layout(std140, binding = 0) uniform CameraBuffer {
    mat4 view;
    mat4 projection;
    mat4 view_projection;
    vec4 camera_position;
    vec4 frustum_planes[6];
    vec2 viewport_size;
    float near_plane;
    float far_plane;
} camera;

layout(std430, binding = 0) buffer InstanceBuffer {
    GPUInstanceData instances[];
};

layout(std430, binding = 1) buffer DrawCommandBuffer {
    GPUDrawCommand commands[];
};

layout(binding = 0) uniform sampler2D u_HZBTexture;

bool is_sphere_in_frustum(vec3 center, float radius) {
    for (int i = 0; i < 6; ++i) {
        float dist = dot(camera.frustum_planes[i].xyz, center) + camera.frustum_planes[i].w;
            if (dist < -radius) {
                return false;
            }
    }
    return true;
}

void main() {
    uint g_id = gl_GlobalInvocationID.x;

    GPUInstanceData instance = instances[g_id];

    // 1. Quick Frustum Culling
    vec3 center = instance.bounding_center_radius.xyz;
    float radius = instance.bounding_center_radius.w;

    if (!is_sphere_in_frustum(center, radius)) {
        commands[g_id].instance_count = 0u;
        return;
    }

    // 2. Project Local AABB Corners to Clip Space
    vec3 bbox_min = instance.bounding_box_min.xyz;
    vec3 bbox_max = instance.bounding_box_max.xyz;

    vec3 corners[8] = vec3[8](
    vec3(bbox_min.x, bbox_min.y, bbox_min.z),
    vec3(bbox_max.x, bbox_min.y, bbox_min.z),
    vec3(bbox_min.x, bbox_max.y, bbox_min.z),
    vec3(bbox_max.x, bbox_max.y, bbox_min.z),
    vec3(bbox_min.x, bbox_min.y, bbox_max.z),
    vec3(bbox_max.x, bbox_min.y, bbox_max.z),
    vec3(bbox_min.x, bbox_max.y, bbox_max.z),
    vec3(bbox_max.x, bbox_max.y, bbox_max.z)
    );

    vec2 min_ndc = vec2(1.0, 1.0);
    vec2 max_ndc = vec2(-1.0, -1.0);
    float min_z = 1.0;

    for (int i = 0; i < 8; ++i) {
    vec4 world_pos = instance.model_matrix * vec4(corners[i], 1.0);
    vec4 clip_pos = camera.view_projection * world_pos;

    if (clip_pos.w <= 0.0) {
        commands[g_id].instance_count = 1u;
        return;
    }

    vec3 ndc = clip_pos.xyz / clip_pos.w;
    min_ndc = min(min_ndc, ndc.xy);
    max_ndc = max(max_ndc, ndc.xy);
    min_z = min(min_z, ndc.z);
    }

    // Convert NDC [-1, 1] to UV [0, 1] and Depth [0, 1]
    vec2 uv_min = clamp(min_ndc * 0.5 + 0.5, 0.0, 1.0);
    vec2 uv_max = clamp(max_ndc * 0.5 + 0.5, 0.0, 1.0);
    float min_z_depth = clamp(min_z * 0.5 + 0.5, 0.0, 1.0);

    // Calculate HZB Mip Level
    vec2 bbox_size = (uv_max - uv_min) * camera.viewport_size;
    float max_dim = max(bbox_size.x, bbox_size.y);
    float mip_level = clamp(ceil(log2(max_dim)), 0.0, 9.0);

    // Sample 4 texels at calculated mip level
    float hzb_d0 = textureLod(u_HZBTexture, vec2(uv_min.x, uv_min.y), mip_level).r;
    float hzb_d1 = textureLod(u_HZBTexture, vec2(uv_max.x, uv_min.y), mip_level).r;
    float hzb_d2 = textureLod(u_HZBTexture, vec2(uv_min.x, uv_max.y), mip_level).r;
    float hzb_d3 = textureLod(u_HZBTexture, vec2(uv_max.x, uv_max.y), mip_level).r;

    float max_hzb_depth = max(max(hzb_d0, hzb_d1), max(hzb_d2, hzb_d3));

    // Visible if closest point of bounding box is closer than or equal to HZB max depth
    bool visible = min_z_depth <= max_hzb_depth;

    commands[g_id].instance_count = visible ? 1u : 0u;
}