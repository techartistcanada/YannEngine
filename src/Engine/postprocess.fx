#ifndef _POSTPROCESS
#define _POSTPROCESS

#include "values.fx"

struct VS_IN
{
    float3 vPos : POSITION;
    float2 vUV : TEXCOORD;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vUV : TEXCOORD;
};

// rect mesh
VS_OUT VS_PostProcess(VS_IN _in)
{
    VS_OUT output = (VS_OUT) 0.f;

    output.vPosition = float4(_in.vPos * 2, 1.0);
    output.vUV = _in.vUV;

    return output;
}

float4 PS_PostProcess(VS_OUT _in) : SV_Target
{
    // Sample the scene texture
    float4 color = float4(1.0f, 0.0f, 0.0f, 1.0f); // Placeholder: solid red color
    if(g_bTex_0)
    {
        float4 rtColor = g_tex_0.Sample(g_Sam_0, _in.vUV);
        color.g = rtColor.x;
    }
    return color;
}
#endif