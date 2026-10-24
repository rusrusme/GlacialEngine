#pragma once

#include "PBR.gshader"

#common

struct DirectionalLight {
    vec3 direction; // Normalisierter Vektor ZUR Lichtquelle
    vec3 color;
    float intensity;
};

vec3 evaluate_directional_light(PBRSurface surface, DirectionalLight light) {
    vec3 L = normalize(light.direction);
    vec3 radiance = light.color * light.intensity;
    return evaluate_cook_torrance(surface, L, radiance);
}

// Hemisphärisches Ambient-Modell (Approximation bis IBL/Cubemaps vorhanden sind)
vec3 evaluate_ambient(PBRSurface surface, vec3 sky_color, vec3 ground_color, float ambient_intensity) {
    float NdotV = max(dot(surface.normal, surface.view_dir), 0.0);
    vec3 F = fresnel_schlick_roughness(NdotV, surface.F0, surface.roughness);

    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - surface.metallic);

    float hemi_factor = surface.normal.y * 0.5 + 0.5;
    vec3 env_irradiance = mix(ground_color, sky_color, hemi_factor) * ambient_intensity;

    vec3 diffuse_ambient = env_irradiance * surface.albedo * kD;

    // Einfache Umgebungsreflexion für glänzende/metallische Oberflächen
    vec3 R = reflect(-surface.view_dir, surface.normal);
    float spec_hemi = R.y * 0.5 + 0.5;
    vec3 prefiltered_env = mix(ground_color, sky_color, spec_hemi) * ambient_intensity;
    vec3 specular_ambient = prefiltered_env * F * (1.0 - surface.roughness * 0.75);

    return (diffuse_ambient + specular_ambient) * surface.ao;
}

vec3 srgb_to_linear(vec3 srgb) {
    return pow(max(srgb, vec3(0.0)), vec3(2.2));
}

vec3 linear_to_srgb(vec3 linear_color) {
    return pow(max(linear_color, vec3(0.0)), vec3(1.0 / 2.2));
}

// Narkowicz ACES Filmic Tone Mapping
vec3 tonemap_aces(vec3 x) {
    // TODO Shader Plugin doesn't support 'const <type> <identifier> = ...'
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}