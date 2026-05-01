#ifndef _GOURAUD_3D
#define _GOURAUD_3D

#include "values.fx"


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

    float LightPow : FOG;
};

VS_OUT VS_Std3D(VS_IN _in)
{
    VS_OUT output = (VS_OUT) 0.f;
    
    output.vPosition = mul(float4(_in.vPos, 1.f), g_matWVP);
    output.vUV = _in.vUV;

    float3 vWorldNormal = normalize(mul(float4(_in.vNormal, 0.f), g_matWorld));
    tLightInfo LightInfo = g_Light3D[0];
    float3 vLightDir = normalize(LightInfo.WorldDir);

    output.LightPow = saturate(dot(vWorldNormal, -vLightDir));

    return output;
}

float4 PS_Std3D(VS_OUT _in) : SV_TARGET
{

    float3 vObjectColor = float3(0.7f, 0.7f, 0.7f);
    tLightInfo LightInfo = g_Light3D[0];

    float3 output = vObjectColor * LightInfo.Light.vDiffuse.rgb * _in.LightPow + LightInfo.Light.vAmbient.rgb * vObjectColor;
    return float4(output, 1.f);
}

#endif
