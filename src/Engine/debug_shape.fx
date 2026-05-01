#ifndef _DEBUG_SHAPE
#define _DEBUG_SHAPE
#include "values.fx"

struct VS_OUT
{
    float4 vPosition : SV_Position;
    float2 vUV : TEXCOORD;

    float3 vPosInView : POSITION;
    float3 vNormalInView: NORMAL;
};

VS_OUT VS_DebugShape(float3 vLocalPos : POSITION, float2 _vUV : TEXCOORD, float3 _vNormal: NORMAL)
{
    VS_OUT output = (VS_OUT)0.f;
    output.vPosition = mul(mul(mul(float4(vLocalPos, 1.0f), g_matWorld), g_matView), g_matProj);
    output.vUV = _vUV;

    output.vPosInView = mul(float4(vLocalPos, 1.f), g_matWV);
    output.vNormalInView = normalize(mul(float4(_vNormal, 0.f), g_matWV));
    return output;
}

float4 PS_DebugShape(VS_OUT _in) : SV_Target
{
    float Alpha = 1.f;
    // debug sphere
    if(4 == g_int_0)
    {
        float3 vEye = normalize(_in.vPosInView);
        Alpha = 1.f - saturate(dot(_in.vNormalInView, vEye));
        Alpha = pow(Alpha, 2.f);
    }
    float4 vOutColor = g_vec4_0;
    vOutColor.a = Alpha;

    return vOutColor;
}

#endif