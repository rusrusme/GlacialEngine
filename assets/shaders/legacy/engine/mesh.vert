#version 450 core

layout (location = 0) in vec3 attribute_position;
layout (location = 1) in vec3 attribute_normal;
layout (location = 2) in vec2 attribute_uv_coordinates;
layout (location = 3) in vec3 attribute_tangent;

layout (location = 4) in mat4 attribute_instance_model_matrix;
layout (location = 8) in vec4 attribute_instance_color;
layout (location = 9) in vec2 attribute_instance_pbr_properties;

#include "legacy/common/camera.glsl"

uniform mat4 u_light_space_matrix;

out vec3 varying_position;
out vec3 varying_normal;
out vec2 varying_uv_coordinates;
out vec3 varying_tangent;
out vec4 varying_color;
out vec2 varying_pbr_properties;
out vec4 varying_light_space_position;

void main() {
    vec4 world_position = attribute_instance_model_matrix * vec4(attribute_position, 1.0);
    varying_position = world_position.xyz;

    mat3 normal_matrix = mat3(transpose(inverse(attribute_instance_model_matrix)));
    varying_normal = normal_matrix * attribute_normal;
    varying_tangent = normal_matrix * attribute_tangent;

    varying_uv_coordinates = attribute_uv_coordinates;
    varying_color = attribute_instance_color;
    varying_pbr_properties = attribute_instance_pbr_properties;

    varying_light_space_position = u_light_space_matrix * world_position;

    gl_Position = u_projection * u_view * world_position;
}