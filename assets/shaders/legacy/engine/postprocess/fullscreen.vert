#version 450 core

out vec2 v_uv;

void main() {
    // Generates UVs: (0,0), (2,0), (0,2) across 3 vertices
    v_uv = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);

    // Maps UV coordinates to Normalized Device Coordinates [-1, 1] for X/Y
    gl_Position = vec4(v_uv * 2.0 - 1.0, 0.0, 1.0);
}