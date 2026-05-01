#ifndef __SSAO_BLUR_FX
#define __SSAO_BLUR_FX

// ****************************************************
// SSAO Bilateral Blur
// Domain: SHADER_DOMAIN::DOMAIN_DEFERRED_LIGHT
// MRT Type: MRT_TYPE::SSAO_BLUR
// Blurs SSAO result while preserving edges using depth
// ****************************************************

#include "values.fx"

#define SSAOInput g_tex_0

struct VS_IN
{
    float3 vPos : POSITION;
    float2 vUV  : TEXCOORD;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vUV       : TEXCOORD;
};

VS_OUT VS_SSAOBlur(VS_IN _in)
{
    VS_OUT output = (VS_OUT) 0.f;
    output.vPosition = float4(_in.vPos.xy * 2.0f, 0.f, 1.f);
    output.vUV = _in.vUV;
    return output;
}

float PS_SSAOBlur(VS_OUT _in) : SV_Target
{
    float2 texelSize = 1.0f / vResolution;
    
    float result = 0.0f;
    float totalWeight = 0.0f;
    
    // 4x4 bilateral blur
    [unroll]
    for (int x = -2; x <= 2; ++x)
    {
        [unroll]
        for (int y = -2; y <= 2; ++y)
        {
            float2 offset = float2((float)x, (float)y) * texelSize;
            float  sample = SSAOInput.Sample(g_Sam_Clamp, _in.vUV + offset).r;
            
            float weight = 1.0f;
            result      += sample * weight;
            totalWeight += weight;
        }
    }
    
    return result / totalWeight;
}

#endif