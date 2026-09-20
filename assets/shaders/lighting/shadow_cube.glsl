#ifndef SHADOW_CUBE_GLSL
#define SHADOW_CUBE_GLSL

uniform samplerCube u_point_shadow_map;

const vec3 sample_directions[20] = vec3[](
        vec3( 1.0,  1.0,  1.0), vec3( 1.0, -1.0,  1.0), vec3(-1.0, -1.0,  1.0), vec3(-1.0,  1.0,  1.0),
        vec3( 1.0,  1.0, -1.0), vec3( 1.0, -1.0, -1.0), vec3(-1.0, -1.0, -1.0), vec3(-1.0,  1.0, -1.0),
        vec3( 1.0,  1.0,  0.0), vec3( 1.0, -1.0,  0.0), vec3(-1.0, -1.0,  0.0), vec3(-1.0,  1.0,  0.0),
        vec3( 1.0,  0.0,  1.0), vec3(-1.0,  0.0,  1.0), vec3( 1.0,  0.0, -1.0), vec3(-1.0,  0.0, -1.0),
        vec3( 0.0,  1.0,  1.0), vec3( 0.0, -1.0,  1.0), vec3( 0.0, -1.0, -1.0), vec3( 0.0,  1.0, -1.0)
);

float calculatePointShadow(vec3 fragment_world_position, vec3 normal, vec3 light_position, float light_radius) {
    vec3 light_to_fragment = fragment_world_position - light_position;
    float current_depth = length(light_to_fragment);

    if (current_depth >= light_radius) {
        return 0.0;
    }

    vec3 light_direction = normalize(light_to_fragment);

    // Calculate cosine of angle between normal and light direction
    float cos_theta = clamp(dot(normalize(normal), -light_direction), 0.0, 1.0);

    // Anti-acne bias
    float bias = max(0.5 * (1.0 - cos_theta), 0.02);

    float shadow_accumulator = 0.0;
    int total_samples = 20;
    float disk_radius = 0.05;

    for (int i = 0; i < total_samples; ++i) {
        vec3 sampling_vector = light_to_fragment + sample_directions[i] * disk_radius;
        float closest_depth = texture(u_point_shadow_map, sampling_vector).r * light_radius;

        if (current_depth - bias > closest_depth) {
            shadow_accumulator += 1.0;
        }
    }

    return shadow_accumulator / float(total_samples);
}

#endif