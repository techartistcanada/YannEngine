#ifndef _STD3D
#define _STD3D

#include "values.fx"

#include "func.fx"

struct VS_IN
{
    // 和input layout中的语义要一致
    float3 vPos : POSITION; 
    float2 vUV : TEXCOORD;

    float3 vTangent : TANGENT;
    float3 vNormal : NORMAL;
    float3 vBinormal : BINORMAL;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION; // 这个语义是固定的，必须是SV_POSITION
    float2 vUV : TEXCOORD;

    float3 vPosInView : POSITION;

    float3 vTangentInView : TANGENT;
    float3 vNormalInView : NORMAL;
    float3 vBinormalInView : BINORMAL;

};

VS_OUT VS_Std3D(VS_IN _in)
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

float4 PS_Std3D(VS_OUT _in) : SV_TARGET
{

    // 1. Object Color
    float3 vObjectColor = float3(1.0f, 0.0f, 1.f);
    float Alpha = 1.f;
    if(g_bTex_0)
    {
        float4 vColor = g_tex_0.Sample(g_Sam_0, _in.vUV);
        vObjectColor = vColor.rgb;
        Alpha = vColor.a;
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

    // 3. calc light
    tLight Light = (tLight) 0.f;
    for (int i = 0; i < Light3DCount; ++i) // Light3DCount is from "values.fx" GLOBALDATA feeded by CRenderMgr::DataBinding
    {
        CalcLight3D(i, _in.vPosInView, vNormalInView, Light);
    }

    // 4. output
    float4 output = float4(0.f, 0.f, 0.f, 1.f);
    output.rgb = vObjectColor.rgb * Light.vDiffuse.rgb
                 + vObjectColor.rgb * Light.vAmbient.rgb
                 + Light.vMaxSpecular.rgb;

    return output;
}

#endif