#version 450 core

// Quad in NDC (Normalized Device Coordinates)
vec3 positions[6] = vec3[](
        vec3(-1.0, -1.0, 0.0),
        vec3( 1.0, -1.0, 0.0),
        vec3( 1.0,  1.0, 0.0),
        vec3(-1.0, -1.0, 0.0),
        vec3( 1.0,  1.0, 0.0),
        vec3(-1.0,  1.0, 0.0)
);

layout(std140, binding = 0) uniform CameraData {
    mat4 u_view;
    mat4 u_projection;
};

out vec3 v_near_point;
out vec3 v_far_point;

vec3 unproject_point(float x, float y, float z, mat4 view_proj_inv) {
    vec4 unprojected = view_proj_inv * vec4(x, y, z, 1.0);
    return unprojected.xyz / unprojected.w;
}

void main() {
    vec3 pos = positions[gl_VertexID];
    mat4 inv_view_proj = inverse(u_projection * u_view);

    v_near_point = unproject_point(pos.x, pos.y, 0.0, inv_view_proj);
    v_far_point  = unproject_point(pos.x, pos.y, 1.0, inv_view_proj);

    gl_Position = vec4(pos, 1.0);
}