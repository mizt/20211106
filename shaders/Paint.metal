#include <metal_stdlib>
using namespace metal;

constexpr sampler linear(filter::linear,coord::pixel,address::clamp_to_edge);

kernel void processimage(
    texture2d<float,access::sample> src[[texture(0)]],
    texture2d<float,access::sample> tri[[texture(1)]],
    texture2d<float,access::write> dst[[texture(2)]],
    uint2 gid[[thread_position_in_grid]]) {
    float2 texcoord = float2(gid)+0.5;
    float4 triangle = tri.sample(linear,texcoord);
    dst.write(mix(src.sample(linear,texcoord),triangle,triangle.a),gid);
}
