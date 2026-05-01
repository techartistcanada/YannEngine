#ifndef _DEFERRED_DECAL_H_
#define _DEFERRED_DECAL_H_


#include "values.fx"
// ===========================================
// Deferred Decal
// 
// Mesh: CubeMesh
// MRT Type: MRT_TYPE::DEFERRED_DECAL
// Shader Domain: SHADER_DOMAIN::DEFERRED_DECAL
// ===========================================
#define AsEmissive          g_int_0
#define EmissiveIntensity   g_float_0
#define GBuffer_Position    g_tex_0
#define RenderTarget        g_tex_1

struct VS_IN
{
    float3 vPos : POSITION;
};

struct VS_OUT
{
    float4 vPosition : SV_Position;
};

VS_OUT VS_DeferredDecal(VS_IN _in)
{
    VS_OUT output = (VS_OUT) 0.f;

    output.vPosition = mul(float4(_in.vPos, 1.f), g_matWVP);

    return output;
}

struct PS_OUT
{
    float4 vColor       : SV_Target0;
    float4 vEmissive   : SV_Target1;
};

PS_OUT PS_DeferredDecal(VS_OUT _in)
{
    PS_OUT output = (PS_OUT) 0.f;

    // NOTE: vSrcenUV 像素在屏幕上的归一化坐标
    float2 vScreenUV = _in.vPosition.xy / vResolution;
    float4 vPosInView = GBuffer_Position.Sample(g_Sam_0, vScreenUV);

    if(0.f == vPosInView.a)
        discard;

    // NOTE: 这个屏幕像素，在 decal cube 本地坐标系里落在哪儿
    float4 vLocalPos = mul(mul(float4(vPosInView.xyz, 1.f), g_matViewInv), g_matWorldInv);

    if(0.5f < abs(vLocalPos.x) || 
       0.5f < abs(vLocalPos.y) ||
       0.5f < abs(vLocalPos.z))
        discard;

    float4 vOutputColor = float4(0.2f, 0.8f, 0.2f, 1.f);

    if(g_bTex_1)
    {
        float2 vUV = float2(vLocalPos.x + 0.5f, 1.f - (vLocalPos.y + 0.5f));
        vOutputColor = g_tex_1.Sample(g_Sam_0, vUV);
    }

    if(0 == AsEmissive)
    {
        output.vColor = vOutputColor;
        output.vColor.a *= EmissiveIntensity;
        output.vEmissive = float4(0.f, 0.f, 0.f, 1.f);
    }
    else
    {
        output.vColor = (float4) 0.f;
        output.vEmissive.rgb = vOutputColor.rgb * vOutputColor.a * EmissiveIntensity;
        output.vEmissive.a = 1.f;
    }



    return output;
}
#endif
