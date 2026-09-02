#version 450 core

out vec4 frag_color;

in vec3 v_position;
in vec3 v_normal;
in vec2 v_uv;
in vec3 v_tangent;
in vec4 v_color;
in vec2 v_pbr;
in vec4 v_light_space_pos;

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

uniform sampler2D u_albedo_texture;
uniform int u_use_albedo_texture;

uniform sampler2D u_normal_texture;
uniform int u_use_normal_texture;

uniform sampler2D u_metallic_roughness_texture;
uniform int u_use_metallic_roughness_texture;

uniform sampler2DShadow u_shadow_map;

layout(std140, binding = 0) uniform CameraData {
    mat4 u_view;
    mat4 u_projection;
};

const float PI = 3.14159265359;

// --- PBR Cook-Torrance Helpers ---
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / max(denom, 0.0001);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / max(denom, 0.0001);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ACES Narkowicz Filmic Tone Mapping Curve
vec3 toneMapACES(vec3 color) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

// 3x3 Percentage-Closer Filtering (PCF) Directional Shadow Calculation
float calculateShadow(vec4 light_space_pos, vec3 N, vec3 L) {
    vec3 proj_coords = light_space_pos.xyz / light_space_pos.w;
    proj_coords = proj_coords * 0.5 + 0.5;

    // Out-of-bounds check
    if (proj_coords.z > 1.0 || proj_coords.x < 0.0 || proj_coords.x > 1.0 || proj_coords.y < 0.0 || proj_coords.y > 1.0) {
        return 0.0;
    }

    // Small bias (PolygonOffset does the heavy lifting now)
    float NdotL = max(dot(N, L), 0.0);
    float bias = max(0.0005 * (1.0 - NdotL), 0.0001);
    float current_depth = proj_coords.z - bias;

    // 3x3 PCF Kernel
    float visibility = 0.0;
    vec2 texel_size = 1.0 / textureSize(u_shadow_map, 0);

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            visibility += texture(u_shadow_map, vec3(proj_coords.xy + vec2(x, y) * texel_size, current_depth));
        }
    }

    // visibility / 9.0 gives 1.0 for unshadowed, 0.0 for shadowed
    return 1.0 - (visibility / 9.0);
}

void main() {
    vec4 tex_color = (u_use_albedo_texture != 0) ? texture(u_albedo_texture, v_uv) : vec4(1.0);
    vec4 albedo = v_color * tex_color;

    if (albedo.a < 0.01) discard;

    // Convert sRGB input color to Linear Space
    albedo.rgb = pow(albedo.rgb, vec3(2.2));

    // Normal Mapping
    vec3 N = normalize(v_normal);
    if (u_use_normal_texture != 0) {
        vec3 T = normalize(v_tangent - dot(v_tangent, N) * N);
        vec3 B = cross(N, T);
        mat3 TBN = mat3(T, B, N);

        vec3 sampled_normal = texture(u_normal_texture, v_uv).rgb * 2.0 - 1.0;
        N = normalize(TBN * sampled_normal);
    }

    // PBR Properties
    float roughness = v_pbr.x;
    float metallic = v_pbr.y;

    if (u_use_metallic_roughness_texture != 0) {
        vec4 mr_sample = texture(u_metallic_roughness_texture, v_uv);
        roughness = mr_sample.g;
        metallic = mr_sample.b;
    }
    roughness = max(roughness, 0.05);

    // Extract Camera World Position from View Matrix
    vec3 cam_pos = inverse(u_view)[3].xyz;
    vec3 V = normalize(cam_pos - v_position);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo.rgb, metallic);

    vec3 Lo = vec3(0.0);

    // Directional Lights (Shadows applied to primary directional light)
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

        // Compute shadow factor for primary directional light
        float shadow = (i == 0) ? calculateShadow(v_light_space_pos, N, L) : 0.0;

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

    // ACES Filmic Tone Mapping & Gamma Correction back to sRGB
    color = toneMapACES(color * 1.2);
    color = pow(color, vec3(1.0 / 2.2));

    frag_color = vec4(color, albedo.a);
}