#ifndef __DEFERRED_MERGING_FX
#define __DEFERRED_MERGING_FX

// ****************************************************
// Domain: SHADER_DOMAIN::DOMAIN_DEFERRED_LIGHT
// MRT Type: MRT_TYPE::HDR_SCENE

// Merges G-Buffer into a single HDR color.
// Tone mapping is now done in a separate final pass.
// ****************************************************


#include "values.fx"
#include "func.fx"

#define GBuffer_Color          g_tex_0
#define GBuffer_Diffuse        g_tex_1
#define GBuffer_Specular       g_tex_2
#define GBuffer_Emmisve        g_tex_3
//#define GBuffer_Position       g_tex_4
#define GBuffer_Data           g_tex_4
#define DebugViewRTTexture     g_tex_5
#define bDeferredDebugView     g_int_0

struct VS_IN
{
    float3 vPos : POSITION;
    float2 vUV  : TEXCOORD;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vUV : TEXCOORD;
};

VS_OUT VS_MergeDeferred(VS_IN _in)
{
    VS_OUT output = (VS_OUT) 0.f;

    // NOTE: to clip space (Rect Mesh)
    output.vPosition = float4(_in.vPos.xy * 2, 0.f, 1.f);

    output.vUV = _in.vUV;

    return output;
}

float4 PS_MergeDeferred(VS_OUT _in) : SV_Target
{
    float4 output = float4(0.f, 0.f, 0.f, 1.f);

    if(bDeferredDebugView)
    {
        output = DebugViewRTTexture.Sample(g_Sam_0, _in.vUV);
    }
    else
    {
        float4 vDiffuse = GBuffer_Diffuse.Sample(g_Sam_0, _in.vUV);
        float4 vSpecular = GBuffer_Specular.Sample(g_Sam_0, _in.vUV);
        float4 vEmissive = GBuffer_Emmisve.Sample(g_Sam_0, _in.vUV);
        float4 vColor = GBuffer_Color.Sample(g_Sam_0, _in.vUV);
        float4 vData  = GBuffer_Data.Sample(g_Sam_0, _in.vUV);

        if (vData.a > 0.5f) // PBR
        {
            output.rgb = vDiffuse.rgb + vEmissive.rgb;
        }
        else // Non-PBR Blinn-Phong
        {
            output.rgb = vColor.rgb * vDiffuse.rgb + vSpecular.rgb + vEmissive.rgb;
        }
    }

    return output;
}

#endif