#version 450 core

out vec4 frag_color;

in vec4 v_color;
in vec2 v_uv;

uniform sampler2D u_texture;
uniform bool u_use_texture;

void main() {
    if (u_use_texture) {
        frag_color = v_color * texture(u_texture, v_uv);
    } else {
        frag_color = v_color;
    }
}