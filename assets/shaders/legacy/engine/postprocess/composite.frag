#version 450 core

in vec2 v_uv;
out vec4 frag_color;

uniform sampler2D u_hdr_scene;

uniform float u_exposure;
uniform float u_gamma;
uniform int u_tone_mapper;

// Vignette
uniform bool u_enable_vignette;
uniform float u_vignette_radius;
uniform float u_vignette_softness;

// Chromatic Aberration
uniform bool u_enable_chromatic_aberration;
uniform float u_chromatic_strength;

// ACES Filmic Curve Approximation
vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

// Reinhard Tone Mapping
vec3 Reinhard(vec3 x) {
    return x / (x + vec3(1.0));
}

void main() {
    vec3 color;

    // 1. Chromatic Aberration (optional)
    if (u_enable_chromatic_aberration) {
        vec2 off = (v_uv - vec2(0.5)) * u_chromatic_strength;
        color.r = texture(u_hdr_scene, v_uv + off).r;
        color.g = texture(u_hdr_scene, v_uv).g;
        color.b = texture(u_hdr_scene, v_uv - off).b;
    } else {
        color = texture(u_hdr_scene, v_uv).rgb;
    }

    // 2. Exposure + Tone Mapping + Gamma — only when a mapper is selected
    if (u_tone_mapper != -1) {
        color *= u_exposure;
        if (u_tone_mapper == 0)      color = ACESFilm(color);
        else if (u_tone_mapper == 1) color = Reinhard(color);
        // TODO(settings): u_tone_mapper == 2  -> Uncharted2 (define it when UI exposes it)
        color = pow(color, vec3(1.0 / u_gamma));
    }
    // else: raw HDR value passes straight through (identity)

    // 3. Vignette (off by default)
    if (u_enable_vignette) {
        float dist = length(v_uv - vec2(0.5));
        color *= smoothstep(u_vignette_radius, u_vignette_radius - u_vignette_softness, dist);
    }

    frag_color = vec4(color, 1.0);
}