#version 450 core

layout (location = 0) in vec3 a_position;
layout (location = 4) in mat4 a_instance_model;

uniform mat4 u_light_space_matrix; // Current cubemap face matrix

out vec3 v_world_position;

void main() {
    vec4 world_pos = a_instance_model * vec4(a_position, 1.0);
    v_world_position = world_pos.xyz;
    gl_Position = u_light_space_matrix * world_pos;
}