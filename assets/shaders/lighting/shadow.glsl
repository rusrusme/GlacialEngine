#ifndef SHADOW_GLSL
#define SHADOW_GLSL

uniform sampler2DShadow u_shadow_map;
uniform mat4 u_light_space_matrix;

float calculateShadow(vec3 world_pos, vec3 N, vec3 L) {
    float NdotL = dot(N, L);

    if (NdotL <= 0.0) {
        return 1.0;
    }

    float normal_bias = max(0.04 * (1.0 - NdotL), 0.005);
    vec4 light_space_pos = u_light_space_matrix * vec4(world_pos + N * normal_bias, 1.0);

    vec3 proj_coords = light_space_pos.xyz / light_space_pos.w;
    proj_coords = proj_coords * 0.5 + 0.5;

    if (proj_coords.z > 1.0 || proj_coords.x < 0.0 || proj_coords.x > 1.0 || proj_coords.y < 0.0 || proj_coords.y > 1.0) {
        return 0.0;
    }
    float current_depth = proj_coords.z;

    float visibility = 0.0;
    vec2 texel_size = 1.0 / textureSize(u_shadow_map, 0);

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            visibility += texture(u_shadow_map, vec3(proj_coords.xy + vec2(x, y) * texel_size, current_depth));
        }
    }
    return 1.0 - (visibility / 9.0);
}

#endif