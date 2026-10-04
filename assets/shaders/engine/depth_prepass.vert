#version 450 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;
layout(location = 3) in vec3 a_tangent;

// Instanced attributes
layout(location = 4) in mat4 a_instance_transform;

layout(std140, binding = 0) uniform CameraData {
    mat4 u_view;
    mat4 u_projection;
};

void main() {
    gl_Position = u_projection * u_view * a_instance_transform * vec4(a_position, 1.0);
}