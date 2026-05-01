#ifndef _STD2D
#define _STD2D

#include "values.fx"
#include "func.fx"

StructuredBuffer<float4> g_TestBuffer : register(t20);

struct VS_IN
{
    float3 vPos : POSITION;
    float4 vColor : COLOR;
    float2 vUV : TEXCOORD;
};

struct VS_OUT
{
    float4 vPosition : SV_Position;
    float4 vColor : COLOR;
    float2 vUV : TEXCOORD;

    float3 vWorldPos : POSITION;
};

VS_OUT VS_Std2D(VS_IN _in)
{
    VS_OUT output = (VS_OUT) 0.f;
    float4 vWorldPos = mul(float4(_in.vPos, 1.0f), g_matWorld);
    // View Space 相机空间
    float4 vViewPos = mul(vWorldPos, g_matView);

    // Clip  Space 裁剪空间
    float4 vProjPos = mul(vViewPos, g_matProj);

    // NDC 空间
    //vProjPos.x /= vProjPos.w;
    //vProjPos.y /= vProjPos.w;

    output.vWorldPos = vWorldPos.xyz;
    output.vPosition = vProjPos;
    output.vColor = _in.vColor;
    output.vUV = _in.vUV;

    return output;
}

// =================================================
// Alpha Blend Version 
// =================================================
float4 PS_Std2D_AB(VS_OUT _in) : SV_Target
{
    float4 vColor = (float4) 0.f;
    if(UseAnim2D)
    {
        float2 vBackgroundLT = vLeftTop - (vBackground - vSliceSize) * 0.5f;
        float2 vUV = (vBackgroundLT + _in.vUV * vBackground) - vOffset;

        if(vUV.x < vLeftTop.x || vLeftTop.x + vSliceSize.x < vUV.x ||
            vUV.y < vLeftTop.y || vLeftTop.y + vSliceSize.y < vUV.y)
        {
            //vColor = float4(1.f, 1.f, 0.f, 1.f);
            discard;
        }
        else
        {
            vColor = g_Atlas.Sample(g_Sam_0, vUV);
        }
    }
    else
    {
        vColor = g_tex_0.Sample(g_Sam_0, _in.vUV);
    }
    // =================================================
    // Lighting
    // =================================================
    float3 vLightPow = (float3) 0.f;
    for(int i = 0; i < Light2DCount; ++i)
    {
        vLightPow += CalcLight2D(i, _in.vWorldPos);
    }
    vColor.rgb = vColor.rgb * vLightPow;
    return vColor;
}
// =================================================
// Masked Version
// =================================================
float4 PS_Std2D(VS_OUT _in) : SV_Target
{
    float4 vColor = (float4) 0.f;
    if(UseAnim2D)
    {
        float2 vBackgroundLT = vLeftTop - (vBackground - vSliceSize) * 0.5f;
        float2 vUV = (vBackgroundLT + _in.vUV * vBackground) - vOffset;

        if(vUV.x < vLeftTop.x || vLeftTop.x + vSliceSize.x < vUV.x ||
            vUV.y < vLeftTop.y || vLeftTop.y + vSliceSize.y < vUV.y)
        {
            //vColor = float4(1.f, 1.f, 0.f, 1.f);
            discard;
        }
        else
        {
            vColor = g_Atlas.Sample(g_Sam_0, vUV);
        }
    }
    else
    {
        vColor = g_tex_0.Sample(g_Sam_0, _in.vUV);
    }
    // =================================================
    // Lighting
    // =================================================
    float3 vLightPow = (float3) 0.f;
    for(int i = 0; i < Light2DCount; ++i)
    {
        vLightPow += CalcLight2D(i, _in.vWorldPos);
    }
    vColor.rgb = vColor.rgb * vLightPow;

    if(vColor.a == 0.f)
    {
        discard;
    }

    if (g_int_0)
        vColor.r *= 3.f;

    return vColor;
}

#endif