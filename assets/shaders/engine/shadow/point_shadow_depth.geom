#version 450 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

// One perspective light-space matrix per cubemap face (6 faces, 90 deg FOV each).
uniform mat4 u_shadow_matrices[6];

out vec4 FragPos;

void main() {
    for (int face = 0; face < 6; ++face) {
        // Route this emitted primitive to the correct cubemap face.
        gl_Layer = face;

        for (int i = 0; i < 3; ++i) {
            // The vertex shader already placed the world position in gl_in[i].gl_Position.
            FragPos = gl_in[i].gl_Position;
            gl_Position = u_shadow_matrices[face] * FragPos;
            EmitVertex();
        }
        EndPrimitive();
    }
}