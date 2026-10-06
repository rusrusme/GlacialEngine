#version 450 core

layout (location = 0) in vec3 a_position;
layout (location = 4) in mat4 a_instance_model;

// The vertex shader only computes the world position. The actual projection
// into each of the six cubemap faces happens in the geometry shader, which
// transforms gl_in[i].gl_Position by each face's light-space matrix.

void main() {
    gl_Position = a_instance_model * vec4(a_position, 1.0);
}