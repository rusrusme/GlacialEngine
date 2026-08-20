#version 450 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_uv;

layout(std140, binding = 0) uniform CameraData {
    mat4 u_view;
    mat4 u_projection;
};

uniform mat4 u_model;

out vec3 v_position;
out vec3 v_normal;
out vec2 v_uv;

void main() {
    vec4 world_pos = u_model * vec4(a_position, 1.0);
    v_position = world_pos.xyz;

    v_normal = mat3(transpose(inverse(u_model))) * a_normal;
    v_uv = a_uv;

    gl_Position = u_projection * u_view * world_pos;
}