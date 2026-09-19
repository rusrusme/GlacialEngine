layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_uv;
layout (location = 3) in vec3 a_tangent;

layout (location = 4) in mat4 a_instance_model;
layout (location = 8) in vec4 a_instance_color;
layout (location = 9) in vec2 a_instance_pbr;

#include "common/camera.glsl"

uniform mat4 u_light_space_matrix;

out vec3 v_position;
out vec3 v_normal;
out vec2 v_uv;
out vec3 v_tangent;
out vec4 v_color;
out vec2 v_pbr;
out vec4 v_light_space_pos;

void main() {
    vec4 world_pos = a_instance_model * vec4(a_position, 1.0);
    v_position = world_pos.xyz;

    mat3 normal_matrix = mat3(transpose(inverse(a_instance_model)));
    v_normal = normal_matrix * a_normal;
    v_tangent = normal_matrix * a_tangent;

    v_uv = a_uv;
    v_color = a_instance_color;
    v_pbr = a_instance_pbr;

    v_light_space_pos = u_light_space_matrix * world_pos;

    gl_Position = u_projection * u_view * world_pos;
}