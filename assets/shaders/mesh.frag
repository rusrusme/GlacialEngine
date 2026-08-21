#version 450 core

out vec4 frag_color;

in vec3 v_position;
in vec3 v_normal;
in vec2 v_uv;
in vec4 v_color;

struct DirLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float radius;
};

#define MAX_DIR_LIGHTS 4
#define MAX_POINT_LIGHTS 8

uniform DirLight u_dir_lights[MAX_DIR_LIGHTS];
uniform int u_num_dir_lights;

uniform PointLight u_point_lights[MAX_POINT_LIGHTS];
uniform int u_num_point_lights;

void main() {
    vec3 norm = normalize(v_normal);
    vec3 total_diffuse = vec3(0.0);

    // Directional Lights
    for (int i = 0; i < u_num_dir_lights && i < MAX_DIR_LIGHTS; ++i) {
        vec3 light_dir = normalize(-u_dir_lights[i].direction);
        float diff = max(dot(norm, light_dir), 0.0);
        total_diffuse += diff * u_dir_lights[i].color * u_dir_lights[i].intensity;
    }

    // Point Lights
    for (int i = 0; i < u_num_point_lights && i < MAX_POINT_LIGHTS; ++i) {
        vec3 light_vec = u_point_lights[i].position - v_position;
        float dist = length(light_vec);

        if (dist < u_point_lights[i].radius) {
            vec3 light_dir = normalize(light_vec);
            float attenuation = clamp(1.0 - (dist / u_point_lights[i].radius), 0.0, 1.0);
            float diff = max(dot(norm, light_dir), 0.0);
            total_diffuse += diff * u_point_lights[i].color * u_point_lights[i].intensity * attenuation;
        }
    }

    float ambient_strength = 0.25;
    vec3 ambient = ambient_strength * v_color.rgb;

    vec3 final_color = ambient + (total_diffuse * v_color.rgb);
    frag_color = vec4(final_color, v_color.a);
}