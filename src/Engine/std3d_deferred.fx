#ifndef _STD3D_DEFERRED
#define _STD3D_DEFERRED

#include "values.fx"

// ************************************************
// Std 3d Deferred Shader
// g_btex_0 : Color Texture
// g_btex_1 : Normal Texture
// g_btex_2 : Specular Texture
// g_btex_3 : Heightmap Texture
// g_btex_4 : Emmissive Texture

// g_int_0  : Alpha Test Enable (1 = cutout, 0 = opaque)
// ************************************************

struct VS_IN
{
    float3 vPos : POSITION;
    float2 vUV : TEXCOORD;

    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
};

struct VS_OUT
{
    float4 vPosition        : SV_Position;
    float2 vUV              : TEXCOORD;

    float3 vPosInView       : POSITION;
    float3 vTangentInView   : TANGENT;
    float3 vNormalInView    : NORMAL;
    float3 vBinormalInView  : BINORMAL;
};

VS_OUT VS_Std3D_Deferred(VS_IN _in)
{
    VS_OUT output = (VS_OUT) 0.f;

    output.vPosition = mul(float4(_in.vPos, 1.f), g_matWVP);
    output.vUV = _in.vUV;

    output.vPosInView = mul(float4(_in.vPos, 1.f), g_matWV);
    output.vTangentInView = normalize(mul(float4(_in.vTangent, 0.f), g_matWV));
    output.vNormalInView = normalize(mul(float4(_in.vNormal, 0.f), g_matWV));
    output.vBinormalInView = normalize(mul(float4(_in.vBinormal, 0.f), g_matWV));

    return output;
}

struct PS_OUT
{
    float4 vColor       : SV_Target0;
    float4 vNormal      : SV_Target1;
    float4 vPosition    : SV_Target2;
    float4 vEmmissive   : SV_Target3;
    float4 vData        : SV_Target4;
};

PS_OUT PS_Std3D_Deferred(VS_OUT _in)
{
    PS_OUT output = (PS_OUT) 0.f;

    // 1. Object Color
    float4 vObjectColor = float4(1.f, 0.f, 1.f, 1.f);
    if(g_bTex_0)
    {
        vObjectColor = g_tex_0.Sample(g_Sam_0, _in.vUV);

        // Alpha test: only clip when material is flagged as cutout (g_int_0 == 1)
        if(g_int_0)
        {
            clip(vObjectColor.a - 0.5f);
        }
    }

    // 2. Normal Map
    float3 vNormalInView = _in.vNormalInView;
    if(g_bTex_1)
    {
        float3 vNormalMap = g_tex_1.Sample(g_Sam_1, _in.vUV);
        vNormalMap = vNormalMap * 2.f - 1.f; // [0, 1] -> [-1, 1]
        vNormalMap.y = -vNormalMap.y;
        float3x3 Rot =
        {
            _in.vTangentInView,
            _in.vBinormalInView,
            _in.vNormalInView,
        };

        vNormalInView = normalize(mul(vNormalMap, Rot));

    }
    output.vColor = vObjectColor;
    output.vNormal = float4(vNormalInView, 1.f);
    // 1.f 表示这是一个真实几何体像素
    output.vPosition = float4(_in.vPosInView, 1.f);

    return output;
}

#endif
