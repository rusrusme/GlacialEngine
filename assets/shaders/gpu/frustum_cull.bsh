#comp
#pragma once

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

struct GPUDrawCommand {
    uint count;
    uint instance_count;
    uint first_index;
    int base_vertex;
    uint base_instance;
};

struct GPUInstanceData {
    mat4 model_matrix;
    vec4 bounding_center_radius; // xyz: World Center, w: Radius
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

bool is_sphere_in_frustum(vec3 center, float radius) {
    for (int i = 0; i < 6; ++i) {
        float dist = dot(camera.frustum_planes[i].xyz, center) + camera.frustum_planes[i].w;
        if (dist < -radius) {
            return false; // Sphere is completely outside this frustum plane
        }
    }
    return true; // Visible or intersecting frustum
}

void main() {
    uint g_id = gl_GlobalInvocationID.x;

    // Fetch instance data
    GPUInstanceData instance = instances[g_id];

    // Sphere frustum culling test
    vec3 center = instance.bounding_center_radius.xyz;
    float radius = instance.bounding_center_radius.w;

    bool visible = is_sphere_in_frustum(center, radius);

    // Toggle instance count for indirect rendering (0 = culled, 1 = visible)
    commands[g_id].instance_count = visible ? 1u : 0u;
}