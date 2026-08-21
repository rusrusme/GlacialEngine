#version 450 core

layout (location = 0) in vec2 a_position;
layout (location = 1) in vec4 a_color;
layout (location = 2) in vec2 a_uv;

uniform mat4 u_projection;
uniform vec2 u_translation;

out vec4 v_color;
out vec2 v_uv;

void main() {
    v_color = a_color;
    v_uv = a_uv;
    vec2 pos = a_position + u_translation;
    gl_Position = u_projection * vec4(pos, 0.0, 1.0);
}