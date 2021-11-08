#include <metal_stdlib>
using namespace metal;

constexpr sampler linear(filter::linear,coord::pixel,address::clamp_to_edge);

kernel void processimage(
    texture2d<float,access::sample> src[[texture(0)]],
    texture2d<float,access::sample> brush[[texture(1)]],
    texture2d<float,access::write> dst[[texture(2)]],
    constant float2 &resolution[[buffer(0)]],
    constant float &amount[[buffer(1)]],
    uint2 gid[[thread_position_in_grid]]) {
    float2 texcoord = float2(gid)+0.5;
    float2 map = ((brush.sample(linear,texcoord).gr)*65535.0-32768.0)/1024.0;
    dst.write(src.sample(linear,texcoord+map),gid);
}
