#pragma once

#common

const float PI = 3.14159265359;
const float EPSILON = 0.0001;
const vec3 F0_DIELECTRIC = vec3(0.04);

struct PBRSurface {
    vec3 albedo;
    vec3 normal;
    vec3 view_dir;
    float roughness;
    float metallic;
    float ao;
    vec3 F0;
};

// Normal Distribution Function (Trowbridge-Reitz GGX)
float distribution_ggx(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return a2 / max(denom, EPSILON);
}

// Geometry Sub-Function (Schlick-GGX)
float geometry_schlick_ggx(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float denom = NdotV * (1.0 - k) + k;
    return NdotV / max(denom, EPSILON);
}

// Geometry Function (Smith's Method)
float geometry_smith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometry_schlick_ggx(NdotV, roughness);
    float ggx1 = geometry_schlick_ggx(NdotL, roughness);

    return ggx1 * ggx2;
}

// Fresnel Equation (Fresnel-Schlick)
vec3 fresnel_schlick(float cos_theta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}

// Fresnel Equation mit Roughness-Dämpfung für Ambient/IBL
vec3 fresnel_schlick_roughness(float cos_theta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}

// Cook-Torrance BRDF Auswertung für einen einzelnen Lichtstrahl L
vec3 evaluate_cook_torrance(PBRSurface surface, vec3 L, vec3 radiance) {
    vec3 N = surface.normal;
    vec3 V = surface.view_dir;
    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);

    // Cook-Torrance Specular BRDF
    float D = distribution_ggx(N, H, surface.roughness);
    float G = geometry_smith(N, V, L, surface.roughness);
    vec3 F  = fresnel_schlick(max(dot(H, V), 0.0), surface.F0);

    vec3 numerator = D * G * F;
    float denominator = 4.0 * NdotV * NdotL + EPSILON;
    vec3 specular = numerator / denominator;

    // Energieerhaltung: Diffuse + Specular <= 1.0 (Metalle haben kein diffuses Licht)
    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - surface.metallic);

    vec3 diffuse = kD * surface.albedo / PI;

    return (diffuse + specular) * radiance * NdotL;
}