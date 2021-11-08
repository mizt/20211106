#include <metal_stdlib>
using namespace metal;

kernel void processimage(
    texture2d<float,access::write> dst[[texture(0)]],
    constant float2 &resolution[[buffer(0)]],
    uint2 gid[[thread_position_in_grid]]) {
    float2 uv = float2(gid)/(resolution-1)*2.0-1.0;
    float alpha = smoothstep(0.0625,0.1125,1.0-length(uv));
    dst.write(float4(float3(1.0),alpha),gid);
}
