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

void main() {
    vec3 ray_dir = v_far_point - v_near_point;
    float t = -v_near_point.y / ray_dir.y;

    if (t <= 0.0) discard;

    vec3 world_pos = v_near_point + t * ray_dir;

    vec3 depth_pos = vec3(world_pos.x, world_pos.y + 0.0005, world_pos.z);
    vec4 clip_space_pos = u_projection * u_view * vec4(depth_pos, 1.0);
    gl_FragDepth = (clip_space_pos.z / clip_space_pos.w) * 0.5 + 0.5;

    vec2 coord = world_pos.xz;
    float grid1 = draw_grid(coord, 1.0);
    float grid10 = draw_grid(coord, 10.0);

    float dist = length(world_pos - v_near_point);
    float fade = max(0.0, 1.0 - (dist / 1000.0));

    vec4 color = vec4(0.65, 0.65, 0.65, grid1 * 0.2);
    color = mix(color, vec4(0.85, 0.85, 0.85, 0.4), grid10);

    if (color.a < 0.01) discard;

    frag_color = vec4(color.rgb, color.a * fade);
}