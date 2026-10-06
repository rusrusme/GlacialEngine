#version 450 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_color;

layout(std140, binding = 0) uniform CameraData {
    mat4 u_view;
    mat4 u_projection;
};

out vec3 v_color;

void main() {
    v_color = a_color;
    gl_Position = u_projection * u_view * vec4(a_position, 1.0);
}