#include <metal_stdlib>
using namespace metal;

constexpr sampler linear(filter::linear,coord::pixel,address::clamp_to_edge);

kernel void processimage(
    texture2d<float,access::sample> src[[texture(0)]],
    texture2d<float,access::write> dst[[texture(1)]],
    constant float2 &resolution[[buffer(0)]],
    constant float2 &cursor[[buffer(1)]],
    constant float2 &prevCursor[[buffer(2)]],
    uint2 gid[[thread_position_in_grid]]) {
    float2 uv = (float2(gid)+0.5)/(resolution);
    float radius=0.125;
    float2 aspect = float2(1.0,resolution[1]/resolution[0]);
    float mask = smoothstep(radius,0.0,distance((uv*2.0-1.0)*aspect,((cursor/(resolution))*2.0-1.0)*aspect));
    if(mask<=2.0/255.0) mask = 0.0;
    float2 vector = float2(prevCursor-cursor)/resolution;
    dst.write(float4(mix(src.sample(linear,float2(gid)+0.5),float4(0.5+vector.x*4.0,0.5+vector.y*4.0,1.0,1.0),float4(mask))),gid);
}
