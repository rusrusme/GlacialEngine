#pragma once

#include "ShaderData.gshader"

#vert

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_UV;
layout(location = 3) in vec4 a_Tangent;

layout(location = 0) out vec4 v_Color;
layout(location = 1) out vec2 v_UV;
layout(location = 2) out mat3 v_TBN;

void main() {
    InstanceData instance = instances[gl_BaseInstanceARB + gl_InstanceID];
    mat4 model = instance.model;

    v_Color = instance.color;
    v_UV = a_UV;

    vec3 T = normalize(mat3(model) * a_Tangent.xyz);
    vec3 N = normalize(mat3(transpose(inverse(model))) * a_Normal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T) * a_Tangent.w;
    v_TBN = mat3(T, B, N);

    gl_Position = u_ViewProjection * model * vec4(a_Position, 1.0);
}

#frag

layout(location = 0) in vec4 v_Color;
layout(location = 1) in vec2 v_UV;
layout(location = 2) in mat3 v_TBN;

layout(binding = 2) uniform sampler2D u_AlbedoTexture;
layout(binding = 3) uniform sampler2D u_NormalMap;

layout(location = 0) out vec4 FragColor;

void main() {
    vec3 local_normal = texture(u_NormalMap, v_UV).rgb * 2.0 - 1.0;
    vec3 normal = normalize(v_TBN * local_normal);
    vec3 light_dir = normalize(vec3(0.6, 1.0, 0.4));

    float ambient = 0.25;
    float diff = max(dot(normal, light_dir), 0.0);

    vec4 tex_color = texture(u_AlbedoTexture, v_UV) * v_Color;

    vec3 lit_color = tex_color.rgb * (ambient + diff * 0.75);
    FragColor = vec4(lit_color, tex_color.a);
}