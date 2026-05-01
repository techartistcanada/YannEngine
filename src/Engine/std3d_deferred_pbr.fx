#ifndef _STD3D_DEFERRED_PBR
#define _STD3D_DEFERRED_PBR

#include "values.fx"

// ************************************************
// Unified Deferred PBR Shader
//
// g_tex_0 : Base Color (Albedo) — alpha used for cutout if g_int_0=1
// g_tex_1 : Normal Map
// g_tex_2 : Packed texture (format depends on g_int_2)
// g_tex_3 : Emissive
//
// g_int_0   : Alpha cutout (1 = MASK, 0 = opaque)
// g_int_1   : Flip Normal Y
// g_int_2   : ORM format (0 = ORM: R=AO,G=Rough,B=Metal;  1 = glTF MR: R=unused,G=Rough,B=Metal)
// g_int_3   : Linear albedo (1 = TEX_0 already linear, skip sRGB→linear)
// g_float_0 : Metallic factor  (scalar fallback / multiplier)
// g_float_1 : Roughness factor (scalar fallback / multiplier)
// g_vec4_0  : Base Color factor (RGBA multiplier, default white)
// g_vec4_1  : Emissive factor   (RGB multiplier, default black)
// ************************************************

struct VS_IN
{
    float3 vPos         : POSITION;
    float2 vUV          : TEXCOORD;
    float3 vNormal      : NORMAL;
    float3 vTangent     : TANGENT;
    float3 vBinormal    : BINORMAL;
};

struct VS_OUT
{
    float4 vPosition       : SV_Position;
    float2 vUV             : TEXCOORD;
    float3 vPosInView      : POSITION;
    float3 vTangentInView  : TANGENT;
    float3 vNormalInView   : NORMAL;
    float3 vBinormalInView : BINORMAL;
};

VS_OUT VS_Std3D_Deferred_PBR(VS_IN _in)
{
    VS_OUT output = (VS_OUT) 0.f;

    output.vPosition       = mul(float4(_in.vPos, 1.0f), g_matWVP);
    output.vUV             = _in.vUV;
    output.vPosInView      = mul(float4(_in.vPos, 1.0f), g_matWV).xyz;
    output.vTangentInView  = normalize(mul(float4(_in.vTangent, 0.f), g_matWV).xyz);
    output.vNormalInView   = normalize(mul(float4(_in.vNormal, 0.f), g_matWV).xyz);
    output.vBinormalInView = normalize(mul(float4(_in.vBinormal, 0.f), g_matWV).xyz);

    return output;
}

struct PS_OUT
{
    float4 vColor      : SV_Target0; // RGB = Albedo
    float4 vNormal     : SV_Target1; // RGB = View-space normal
    float4 vPosition   : SV_Target2; // RGB = View-space position, A = geometry flag
    float4 vEmissive   : SV_Target3; // RGB = Emissive
    float4 vCustomData : SV_Target4; // R = Metallic, G = Roughness, B = AO, A = 1.0 (PBR flag)
};

PS_OUT PS_Std3D_Deferred_PBR(VS_OUT _in)
{
    PS_OUT output = (PS_OUT) 0.f;

    // ---- 1. Base Color ----
    float4 vAlbedo = float4(1.f, 1.f, 1.f, 1.f);
    if (g_bTex_0)
    {
        vAlbedo = g_tex_0.Sample(g_Sam_0, _in.vUV);

        // alpha cutout (use combined alpha)
        if (g_int_0)
            clip(vAlbedo.a * g_vec4_0.a - 0.333f);
    }
    // sRGB to linear (skip if g_int_3 == 1, i.e. texture already in linear space)
    if (!g_int_3)
    {
        vAlbedo.rgb = pow(abs(vAlbedo.rgb), 2.2f);
    }
    // Apply baseColorFactor in linear space (not gamma-corrected)
    vAlbedo.rgb *= g_vec4_0.rgb;
    vAlbedo.a   *= g_vec4_0.a;

    // ---- 2. Normal Map ----
    float3 vNormalInView = _in.vNormalInView;
    if (g_bTex_1)
    {
        float3 vNormalMap = g_tex_1.Sample(g_Sam_1, _in.vUV).rgb;
        vNormalMap = vNormalMap * 2.f - 1.f;
        vNormalMap.y = -vNormalMap.y;

        if (g_int_1)
            vNormalMap.y = -vNormalMap.y;

        float3x3 TBN =
        {
            _in.vTangentInView,
            _in.vBinormalInView,
            _in.vNormalInView,
        };
        vNormalInView = normalize(mul(vNormalMap, TBN));
    }

    // ---- 3. Metallic / Roughness / AO ----
    float fMetallic  = g_float_0;
    float fRoughness = g_float_1;
    float fAO        = 1.f;
    if (g_bTex_2)
    {
        float4 vPacked = g_tex_2.Sample(g_Sam_0, _in.vUV);

        if (g_int_2 == 0)
        {
            // ORM format: R=AO, G=Roughness, B=Metallic  (Unreal-style)
            fAO        = vPacked.r;
            fRoughness = vPacked.g;
            fMetallic  = vPacked.b;
        }
        else
        {
            // glTF MR format: R=unused, G=Roughness, B=Metallic
            fRoughness = vPacked.g * g_float_1;
            fMetallic  = vPacked.b * g_float_0;
        }
    }
    fRoughness = clamp(fRoughness, 0.04f, 1.f);
    fMetallic  = saturate(fMetallic);

    // ---- 4. Emissive ----
    float3 vEmissive = g_vec4_1.rgb; // emissiveFactor
    if (g_bTex_3)
    {
        float3 vEmissiveTex = g_tex_3.Sample(g_Sam_0, _in.vUV).rgb;
        vEmissiveTex = pow(vEmissiveTex, 2.2f);
        vEmissive *= vEmissiveTex;
    }

    // ---- Output ----
    output.vColor      = vAlbedo;
    output.vNormal     = float4(vNormalInView, 1.f);
    output.vPosition   = float4(_in.vPosInView, 1.f);
    output.vEmissive   = float4(vEmissive, 1.f);
    output.vCustomData = float4(fMetallic, fRoughness, fAO, 1.f);

    return output;
}

#endif