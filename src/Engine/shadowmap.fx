#ifndef _SHADOW_MAP_
#define _SHADOW_MAP_

#include "values.fx"

struct VS_IN
{
    float3 vPos : POSITION;
};

struct VS_OUT
{
    // * NOTE: vPosition(SV_POSITION) is clip space when output from vertex shader,
    // * when input to pixel shader, it has already been processed by Rasterizer, and is in screen space. 
    // * so we need to pass the clip space position to pixel shader for depth calculation.
    // *     1. Perspective Division: clip space -> NDC space (x/w, y/w, z/w)
    // *     2. Viewport Transformation: NDC space -> Screen space
    // *     3. Interpolation: Screen space -> Pixel shader input
    // * PS_INPUT: x,y is pixel screen coordinates, z is depth value(0 -> 1), w is ?
    float4 vPosition : SV_POSITION; 
    float4 vProjPos : POSITION;
};

VS_OUT VS_ShadowMap(VS_IN _in)
{
    VS_OUT output = (VS_OUT) 0.f;

    output.vPosition = mul(float4(_in.vPos, 1.0f), g_matWVP);
    output.vProjPos = output.vPosition;

    return output;
}

float4 PS_ShadowMap(VS_OUT _in) : SV_TARGET
{
    float ProjZ = _in.vProjPos.z / _in.vProjPos.w;
    return ProjZ;
}

#endif