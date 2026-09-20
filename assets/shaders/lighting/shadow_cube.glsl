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
    vec3 light_to_frag_raw = fragment_world_position - light_position;
    float current_depth = length(light_to_frag_raw);

    if (current_depth >= light_radius) {
        return 0.0;
    }

    vec3 N = normalize(normal);
    vec3 light_direction = normalize(light_to_frag_raw);
    float cos_theta = clamp(dot(N, -light_direction), 0.0, 1.0);

    float point_shadow_res = 1024.0;
    float texel_size = (2.0 * current_depth) / point_shadow_res;

    // 2. Compute sine of angle to scale offset stronger at grazing angles
    float shadow_sine = sqrt(max(0.0, 1.0 - cos_theta * cos_theta));

    // 3. Offset position along surface normal (0.5 texels is standard)
    float normal_bias_units = 0.5;
    vec3 biased_position = fragment_world_position + N * (shadow_sine * normal_bias_units * texel_size);

    // Recalculate sampling vector using the biased position
    vec3 light_to_fragment = biased_position - light_position;
    current_depth = length(light_to_fragment);

    // 4. Keep constant depth bias tiny because glCullFace(GL_FRONT) does the heavy lifting
    float depth_bias = 0.0005;

    float shadow_accumulator = 0.0;
    int total_samples = 20;

    // Scale PCF disk radius by depth so filter width remains consistent
    float disk_radius = 0.02 * (current_depth / light_radius);

    for (int i = 0; i < total_samples; ++i) {
        vec3 sampling_vector = light_to_fragment + sample_directions[i] * disk_radius;
        float closest_depth = texture(u_point_shadow_map, sampling_vector).r * light_radius;

        if (current_depth - depth_bias > closest_depth) {
            shadow_accumulator += 1.0;
        }
    }

    return shadow_accumulator / float(total_samples);
}

#endif