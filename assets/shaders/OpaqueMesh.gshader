#pragma once

#include "ShaderData.gshader"
#include "PBR.gshader"
#include "Lighting.gshader"

#vert

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_UV;
layout(location = 3) in vec4 a_Tangent;

layout(location = 0) out vec4 v_Color;
layout(location = 1) out vec2 v_UV;
layout(location = 2) out vec3 v_WorldPos;
layout(location = 3) out vec4 v_MaterialParams;
layout(location = 4) out mat3 v_TBN;

void main() {
    InstanceData instance = instances[gl_BaseInstanceARB + gl_InstanceID];
    mat4 model = instance.model;

    vec4 world_pos = model * vec4(a_Position, 1.0);

    v_Color = instance.color;
    v_UV = a_UV;
    v_WorldPos = world_pos.xyz;
    v_MaterialParams = instance.material_params;

    vec3 T = normalize(mat3(model) * a_Tangent.xyz);
    vec3 N = normalize(mat3(transpose(inverse(model))) * a_Normal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T) * a_Tangent.w;
    v_TBN = mat3(T, B, N);

    gl_Position = u_ViewProjection * world_pos;
}

#frag

layout(location = 0) in vec4 v_Color;
layout(location = 1) in vec2 v_UV;
layout(location = 2) in vec3 v_WorldPos;
layout(location = 3) in vec4 v_MaterialParams;
layout(location = 4) in mat3 v_TBN;

layout(binding = 2) uniform sampler2D u_AlbedoTexture;
layout(binding = 3) uniform sampler2D u_NormalMap;
layout(binding = 4) uniform sampler2D u_MetallicRoughnessMap;

layout(location = 0) out vec4 FragColor;

void main() {
    // 1. Albedo (Konvertierung von sRGB nach Linear Space für korrekte PBR-Mathematik)
    vec4 albedo_sample = texture(u_AlbedoTexture, v_UV);
    vec3 albedo = srgb_to_linear(albedo_sample.rgb) * v_Color.rgb;
    float alpha = albedo_sample.a * v_Color.a;

    // 2. Normal Mapping
    vec3 local_normal = texture(u_NormalMap, v_UV).rgb * 2.0 - 1.0;
    local_normal.xy *= v_MaterialParams.w; // Normal-Scale
    vec3 N = normalize(v_TBN * normalize(local_normal));
    vec3 V = normalize(u_CameraPosition.xyz - v_WorldPos);

    // 3. Metallic, Roughness & AO (G = Roughness, B = Metallic)
    vec4 mr_sample = texture(u_MetallicRoughnessMap, v_UV);
    float metallic  = clamp(mr_sample.b * v_MaterialParams.x, 0.0, 1.0);
    float roughness = clamp(mr_sample.g * v_MaterialParams.y, 0.04, 1.0);
    float ao        = clamp(v_MaterialParams.z, 0.0, 1.0);

    // 4. PBR Surface aufbauen
    PBRSurface surface;
    surface.albedo    = albedo;
    surface.normal    = N;
    surface.view_dir  = V;
    surface.roughness = roughness;
    surface.metallic  = metallic;
    surface.ao        = ao;
    surface.F0        = mix(F0_DIELECTRIC, albedo, metallic);

    // 5. Direkte Beleuchtung + Ambient
    DirectionalLight sun_light;
    sun_light.direction = normalize(vec3(0.6, 1.0, 0.4));
    sun_light.color     = vec3(1.0, 0.98, 0.95);
    sun_light.intensity = 3.0;

    vec3 Lo = evaluate_directional_light(surface, sun_light);
    vec3 ambient = evaluate_ambient(surface, vec3(0.35, 0.4, 0.5), vec3(0.15, 0.12, 0.1), 0.5);

    vec3 color = ambient + Lo;

    // 6. HDR Tonemapping & Gamma-Korrektur zurück nach sRGB
    color = tonemap_aces(color);
    color = linear_to_srgb(color);

    FragColor = vec4(color, alpha);
}