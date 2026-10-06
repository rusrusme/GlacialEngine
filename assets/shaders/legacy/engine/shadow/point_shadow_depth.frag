#version 450 core

in vec4 FragPos;

uniform vec3 u_light_position;
uniform float u_far_plane; // Point light radius

void main() {
    // Calculate true 3D Euclidean distance to the light source
    float light_distance = length(FragPos.xyz - u_light_position);

    // Normalize and clamp to valid depth range [0.0, 1.0]
    gl_FragDepth = clamp(light_distance / u_far_plane, 0.0, 1.0);
}