#version 450 core

in vec3 v_near_point;
in vec3 v_far_point;

out vec4 frag_color;

layout(std140, binding = 0) uniform CameraData {
    mat4 u_view;
    mat4 u_projection;
};

float draw_grid(vec2 pt, float spacing) {
    vec2 coord = pt / spacing;
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);
    float line = min(grid.x, grid.y);
    return 1.0 - min(line, 1.0);
}

float log10(float x) {
    return log(x) * 0.4342944819; // 1 / ln(10)
}

void main() {
    vec3 ray_dir = v_far_point - v_near_point;
    float t = -v_near_point.y / ray_dir.y;

    if (t <= 0.0) discard;

    vec3 world_pos = v_near_point + t * ray_dir;

    vec3 depth_pos = vec3(world_pos.x, world_pos.y + 0.0005, world_pos.z);
    vec4 clip_space_pos = u_projection * u_view * vec4(depth_pos, 1.0);
    gl_FragDepth = (clip_space_pos.z / clip_space_pos.w) * 0.5 + 0.5;

    vec2 coord = world_pos.xz;
    vec2 dxy = fwidth(coord);
    float pixel_size = max(dxy.x, dxy.y);

    float min_pixels_per_cell = 100.0;
    float log_scale = log10(pixel_size * min_pixels_per_cell);
    float lod = floor(log_scale);
    float fade = smoothstep(0.0, 1.0, fract(log_scale));

    // Three parallel grid scales
    float cell_size0 = pow(10.0, lod);
    float cell_size1 = cell_size0 * 10.0;
    float cell_size2 = cell_size1 * 10.0;

    float g0 = draw_grid(coord, cell_size0);
    float g1 = draw_grid(coord, cell_size1);
    float g2 = draw_grid(coord, cell_size2);

    float a0 = 0.20 * (1.0 - fade);
    float a1 = mix(0.55, 0.20, fade);
    float a2 = 0.55;

    float alpha = max(g0 * a0, max(g1 * a1, g2 * a2));

    // Fade
    float dist = length(world_pos - v_near_point);
    float distance_fade = max(0.0, 1.0 - (dist * 0.0005));

    if (alpha < 0.001) discard;

    frag_color = vec4(vec3(0.5), alpha * distance_fade);
}