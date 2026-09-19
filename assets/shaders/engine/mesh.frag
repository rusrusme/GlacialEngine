#version 450 core

out vec4 fragment_color;

in vec3 varying_position;
in vec3 varying_normal;
in vec2 varying_uv_coordinates;
in vec3 varying_tangent;
in vec4 varying_color;
in vec2 varying_pbr_properties;
in vec4 varying_light_space_position;

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
    vec4 texture_color = (u_use_albedo_texture != 0) ? texture(u_albedo_texture, varying_uv_coordinates) : vec4(1.0);
    vec4 albedo = varying_color * texture_color;

    if (albedo.a < 0.01) discard;

    albedo.rgb = pow(albedo.rgb, vec3(2.2));

    vec3 normal = normalize(varying_normal);
    if (u_use_normal_texture != 0) {
        vec3 tangent = normalize(varying_tangent - dot(varying_tangent, normal) * normal);
        vec3 bitangent = cross(normal, tangent);
        mat3 tangent_bitangent_normal_matrix = mat3(tangent, bitangent, normal);

        vec3 sampled_normal = texture(u_normal_texture, varying_uv_coordinates).rgb * 2.0 - 1.0;
        normal = normalize(tangent_bitangent_normal_matrix * sampled_normal);
    }

    float roughness = varying_pbr_properties.x;
    float metallic = varying_pbr_properties.y;

    if (u_use_metallic_roughness_texture != 0) {
        vec4 metallic_roughness_sample = texture(u_metallic_roughness_texture, varying_uv_coordinates);
        roughness = metallic_roughness_sample.g;
        metallic = metallic_roughness_sample.b;
    }
    roughness = max(roughness, 0.05);

    vec3 camera_position = inverse(u_view)[3].xyz;
    vec3 view_direction = normalize(camera_position - varying_position);

    vec3 base_reflectivity = vec3(0.04);
    base_reflectivity = mix(base_reflectivity, albedo.rgb, metallic);

    vec3 outgoing_radiance = vec3(0.0);

    // Directional Lights
    for (int i = 0; i < u_num_dir_lights && i < MAX_DIR_LIGHTS; ++i) {
        vec3 light_direction = normalize(-u_dir_lights[i].direction);
        vec3 halfway_vector = normalize(view_direction + light_direction);

        vec3 radiance = u_dir_lights[i].color * u_dir_lights[i].intensity;

        float normal_distribution = DistributionGGX(normal, halfway_vector, roughness);
        float geometry_shadowing = GeometrySmith(normal, view_direction, light_direction, roughness);
        vec3 fresnel = fresnelSchlick(max(dot(halfway_vector, view_direction), 0.0), base_reflectivity);

        vec3 numerator = normal_distribution * geometry_shadowing * fresnel;
        float denominator = 4.0 * max(dot(normal, view_direction), 0.0) * max(dot(normal, light_direction), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 fresnel_dielectric = fresnelSchlick(max(dot(halfway_vector, view_direction), 0.0), vec3(0.04));
        vec3 diffuse_fraction = (vec3(1.0) - fresnel_dielectric) * (1.0 - metallic);

        float normal_dot_light = max(dot(normal, light_direction), 0.0);
        float shadow = (i == 0) ? calculateShadow(varying_position, normal, light_direction) : 0.0;

        outgoing_radiance += (1.0 - shadow) * (diffuse_fraction * albedo.rgb / PI + specular) * radiance * normal_dot_light;
    }

    // Point Lights
    for (int i = 0; i < u_num_point_lights && i < MAX_POINT_LIGHTS; ++i) {
        vec3 light_vector = u_point_lights[i].position - varying_position;
        float distance = length(light_vector);

        if (distance < u_point_lights[i].radius) {
            vec3 light_direction = normalize(light_vector);
            vec3 halfway_vector = normalize(view_direction + light_direction);

            float attenuation = clamp(1.0 - (distance / u_point_lights[i].radius), 0.0, 1.0);
            vec3 radiance = u_point_lights[i].color * u_point_lights[i].intensity * attenuation;

            float normal_distribution = DistributionGGX(normal, halfway_vector, roughness);
            float geometry_shadowing = GeometrySmith(normal, view_direction, light_direction, roughness);
            vec3 fresnel = fresnelSchlick(max(dot(halfway_vector, view_direction), 0.0), base_reflectivity);

            vec3 numerator = normal_distribution * geometry_shadowing * fresnel;
            float denominator = 4.0 * max(dot(normal, view_direction), 0.0) * max(dot(normal, light_direction), 0.0) + 0.0001;
            vec3 specular = numerator / denominator;

            vec3 fresnel_dielectric = fresnelSchlick(max(dot(halfway_vector, view_direction), 0.0), vec3(0.04));
            vec3 diffuse_fraction = (vec3(1.0) - fresnel_dielectric) * (1.0 - metallic);

            float normal_dot_light = max(dot(normal, light_direction), 0.0);

            outgoing_radiance += (diffuse_fraction * albedo.rgb / PI + specular) * radiance * normal_dot_light;
        }
    }

    vec3 ambient = vec3(0.01) * albedo.rgb;
    vec3 color = ambient + outgoing_radiance;

    color = toneMapACES(color * 1.2);
    color = pow(color, vec3(1.0 / 2.2));

    fragment_color = vec4(color, albedo.a);
}