#version 450 core

out vec4 frag_color;

in vec3 v_normal;
in vec2 v_uv;

uniform vec4 u_color;
uniform vec3 u_light_dir;

void main() {
//    frag_color = u_color;
    vec3 norm = normalize(v_normal);
    // Lichtvektor umkehren, damit er VOM Fragment ZUR Lichtquelle zeigt
    vec3 light_dir = normalize(-u_light_dir);

    // 1. Ambient (Grundausleuchtung)
    float ambient_strength = 0.25;
    vec3 ambient = ambient_strength * u_color.rgb;

    // 2. Diffuse (Lambert-Beleuchtung via Skalarprodukt)
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * u_color.rgb;

    // Endergebnis kombiniert
    vec3 final_color = ambient + diffuse;
    frag_color = vec4(final_color, u_color.a);
}