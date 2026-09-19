#version 450 core

out vec4 frag_color;

in vec3 v_position;
in vec3 v_normal;
in vec2 v_uv;
in vec3 v_tangent;
in vec4 v_color;
in vec2 v_pbr;
in vec4 v_light_space_pos;

#include "common/camera.glsl"
#include "common/math.glsl"
#include "lighting/lights.glsl"
#include "lighting/pbr.glsl"
#include "lighting/shadow.glsl"

uniform sampler2D u_albedo_texture;
uniform int u_use_albedo_texture;

uniform sampler2D u_normal_texture;
uniform int u_use_normal_texture;

uniform sampler2D u_metallic_roughness_texture;
uniform int u_use_metallic_roughness_texture;

void main() {
    vec4 tex_color = (u_use_albedo_texture != 0) ? texture(u_albedo_texture, v_uv) : vec4(1.0);
    vec4 albedo = v_color * tex_color;

    if (albedo.a < 0.01) discard;

    albedo.rgb = pow(albedo.rgb, vec3(2.2));

    vec3 N = normalize(v_normal);
    if (u_use_normal_texture != 0) {
        vec3 T = normalize(v_tangent - dot(v_tangent, N) * N);
        vec3 B = cross(N, T);
        mat3 TBN = mat3(T, B, N);

        vec3 sampled_normal = texture(u_normal_texture, v_uv).rgb * 2.0 - 1.0;
        N = normalize(TBN * sampled_normal);
    }

    float roughness = v_pbr.x;
    float metallic = v_pbr.y;

    if (u_use_metallic_roughness_texture != 0) {
        vec4 mr_sample = texture(u_metallic_roughness_texture, v_uv);
        roughness = mr_sample.g;
        metallic = mr_sample.b;
    }
    roughness = max(roughness, 0.05);

    vec3 cam_pos = inverse(u_view)[3].xyz;
    vec3 V = normalize(cam_pos - v_position);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo.rgb, metallic);

    vec3 Lo = vec3(0.0);

    // Directional Lights
    for (int i = 0; i < u_num_dir_lights && i < MAX_DIR_LIGHTS; ++i) {
        vec3 L = normalize(-u_dir_lights[i].direction);
        vec3 H = normalize(V + L);

        vec3 radiance = u_dir_lights[i].color * u_dir_lights[i].intensity;

        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 F_dielectric = fresnelSchlick(max(dot(H, V), 0.0), vec3(0.04));
        vec3 kD = (vec3(1.0) - F_dielectric) * (1.0 - metallic);

        float NdotL = max(dot(N, L), 0.0);
        float shadow = (i == 0) ? calculateShadow(v_position, N, L) : 0.0;

        Lo += (1.0 - shadow) * (kD * albedo.rgb / PI + specular) * radiance * NdotL;
    }

    // Point Lights
    for (int i = 0; i < u_num_point_lights && i < MAX_POINT_LIGHTS; ++i) {
        vec3 light_vec = u_point_lights[i].position - v_position;
        float dist = length(light_vec);

        if (dist < u_point_lights[i].radius) {
            vec3 L = normalize(light_vec);
            vec3 H = normalize(V + L);

            float attenuation = clamp(1.0 - (dist / u_point_lights[i].radius), 0.0, 1.0);
            vec3 radiance = u_point_lights[i].color * u_point_lights[i].intensity * attenuation;

            float NDF = DistributionGGX(N, H, roughness);
            float G = GeometrySmith(N, V, L, roughness);
            vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

            vec3 numerator = NDF * G * F;
            float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
            vec3 specular = numerator / denominator;

            vec3 F_dielectric = fresnelSchlick(max(dot(H, V), 0.0), vec3(0.04));
            vec3 kD = (vec3(1.0) - F_dielectric) * (1.0 - metallic);

            float NdotL = max(dot(N, L), 0.0);

            Lo += (kD * albedo.rgb / PI + specular) * radiance * NdotL;
        }
    }

    vec3 ambient = vec3(0.01) * albedo.rgb;
    vec3 color = ambient + Lo;

    color = toneMapACES(color * 1.2);
    color = pow(color, vec3(1.0 / 2.2));

    frag_color = vec4(color, albedo.a);
}