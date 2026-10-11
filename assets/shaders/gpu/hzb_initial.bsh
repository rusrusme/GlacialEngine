#comp
#pragma once

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(binding = 0) uniform sampler2D u_SourceDepth;
layout(r32f, binding = 0) writeonly uniform image2D u_TargetHZB;

void main() {
    ivec2 output_pos = ivec2(gl_GlobalInvocationID.xy);
    ivec2 output_size = imageSize(u_TargetHZB);

    if (output_pos.x >= output_size.x || output_pos.y >= output_size.y) {
        return;
    }

    ivec2 input_pos = output_pos * 2;

    float d0 = texelFetch(u_SourceDepth, input_pos + ivec2(0, 0), 0).r;
    float d1 = texelFetch(u_SourceDepth, input_pos + ivec2(1, 0), 0).r;
    float d2 = texelFetch(u_SourceDepth, input_pos + ivec2(0, 1), 0).r;
    float d3 = texelFetch(u_SourceDepth, input_pos + ivec2(1, 1), 0).r;

    float max_depth = max(max(d0, d1), max(d2, d3));
    imageStore(u_TargetHZB, output_pos, vec4(max_depth, 0.0, 0.0, 0.0));
}