#version 450 core

in vec3 v_world_position;

uniform vec3 u_light_position;
uniform float u_far_plane; // Point light radius

void main() {
    // Calculate true 3D Euclidean distance to the light source
    float light_distance = length(v_world_position - u_light_position);

    // Normalize to [0, 1] range relative to light radius
    light_distance = light_distance / u_far_plane;

    // Write explicitly into depth buffer
    gl_FragDepth = light_distance;
}