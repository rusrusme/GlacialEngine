#vert
#pragma once

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_UV;
layout(location = 3) in vec4 a_Color;

layout(std140, binding = 0) uniform FrameData {
    mat4 u_ViewProjection;
};

layout(std430, binding = 1) readonly buffer ObjectBuffer {
    mat4 u_Models[];
};

layout(location = 0) out vec4 v_Color;
layout(location = 1) out vec3 v_Normal;
layout(location = 2) out vec2 v_UV;

void main() {
    mat4 model = u_Models[gl_BaseInstanceARB + gl_InstanceID];

    v_Color = a_Color;
    v_Normal = normalize(mat3(transpose(inverse(model))) * a_Normal);
    v_UV = a_UV;

    gl_Position = u_ViewProjection * model * vec4(a_Position, 1.0);
}

#frag
#pragma once

layout(location = 0) in vec4 v_Color;
layout(location = 1) in vec3 v_Normal;
layout(location = 2) in vec2 v_UV;

layout(binding = 2) uniform sampler2D u_AlbedoTexture;

layout(location = 0) out vec4 FragColor;

void main() {
    vec3 normal = normalize(v_Normal);
    vec3 light_dir = normalize(vec3(0.6, 1.0, 0.4));

    float ambient = 0.25;
    float diff = max(dot(normal, light_dir), 0.0);

    vec4 tex_color = texture(u_AlbedoTexture, v_UV);

    vec3 lit_color = tex_color.rgb * (ambient + diff * 0.75);
    FragColor = vec4(lit_color, 1.0);
}