#ifndef _STD3D_FORWARD_PBR_GLASS
#define _STD3D_FORWARD_PBR_GLASS

#include "values.fx"

// ************************************************
// Forward Transparent PBR Shader — Stained Glass
//
// g_tex_0 : Base Color (Albedo)
// g_tex_1 : Normal Map (optional)
// g_tex_2 : Roughness
// g_tex_3 : Opacity  (single channel)
//
// g_float_0 : Opacity (0..1, controls glass transparency)
// g_float_1 : Roughness factor (scalar fallback / multiplier)
// g_vec4_0  : Tint color (RGBA multiplier)
// ************************************************

struct VS_IN
{
    float3 vPos      : POSITION;
    float2 vUV       : TEXCOORD;
    float3 vNormal   : NORMAL;
    float3 vTangent  : TANGENT;
    float3 vBinormal : BINORMAL;
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

VS_OUT VS_Glass(VS_IN _in)
{
    VS_OUT output = (VS_OUT)0.f;

    output.vPosition       = mul(float4(_in.vPos, 1.0f), g_matWVP);
    output.vUV             = _in.vUV;
    output.vPosInView      = mul(float4(_in.vPos, 1.0f), g_matWV).xyz;
    output.vTangentInView  = normalize(mul(float4(_in.vTangent,  0.f), g_matWV).xyz);
    output.vNormalInView   = normalize(mul(float4(_in.vNormal,   0.f), g_matWV).xyz);
    output.vBinormalInView = normalize(mul(float4(_in.vBinormal, 0.f), g_matWV).xyz);

    return output;
}

float4 PS_Glass(VS_OUT _in) : SV_Target
{
    // ---- 1. Base Color ----
    float4 vAlbedo = g_vec4_0; // tint color (default white)
    if (g_bTex_0)
    {
        float4 vTexColor = g_tex_0.Sample(g_Sam_0, _in.vUV);
        vAlbedo *= vTexColor;
    }
    // sRGB to linear
    vAlbedo.rgb = pow(abs(vAlbedo.rgb), 2.2f);

    // ---- 2. Normal ----
    float3 vNormal = _in.vNormalInView;
    if (g_bTex_1)
    {
        float3 vNormalMap = g_tex_1.Sample(g_Sam_1, _in.vUV).rgb;
        vNormalMap = vNormalMap * 2.f - 1.f;
        vNormalMap.y = -vNormalMap.y;

        float3x3 TBN = { _in.vTangentInView, _in.vBinormalInView, _in.vNormalInView };
        vNormal = normalize(mul(vNormalMap, TBN));
    }

    // ---- 3. Roughness ----
    float fRoughness = g_float_1;
    if (g_bTex_2)
        fRoughness = g_tex_2.Sample(g_Sam_0, _in.vUV).r * g_float_1;
    fRoughness = clamp(fRoughness, 0.04f, 1.f);

    // ---- 4. Lighting ----
    // Stained glass = two components:
    //   1) Transmission: light passes THROUGH the glass and gets tinted by albedo.
    //      This is always visible (it's what makes stained glass colorful).
    //   2) Surface reflection: specular highlights ON the glass surface from direct lights.
    //      This is additive and view-dependent.

    float3 vTransmission = float3(0.f, 0.f, 0.f);
    float3 vReflection   = float3(0.f, 0.f, 0.f);

    float3 vViewDir = normalize(-_in.vPosInView);

    for (int i = 0; i < Light3DCount; ++i)
    {
        float3 vLightDirView = float3(0.f, 0.f, 0.f);
        float  fAtten = 1.f;

        // Directional light
        if (g_Light3D[i].LightType == 0)
        {
            vLightDirView = normalize(mul(float4(g_Light3D[i].WorldDir.xyz, 0.f), g_matView).xyz);
        }
        // Point light
        else if (g_Light3D[i].LightType == 1)
        {
            float3 vLightPosView = mul(float4(g_Light3D[i].WorldPos.xyz, 1.f), g_matView).xyz;
            vLightDirView = normalize(_in.vPosInView - vLightPosView);
            float fDist = distance(_in.vPosInView, vLightPosView);
            fAtten = saturate(cos(saturate(fDist / g_Light3D[i].Range) * (PI / 2.f)));
        }
        else
        {
            continue;
        }

        float3 L = -vLightDirView;
        float  NdotL = dot(vNormal, L);
        float3 vRadiance = g_Light3D[i].Light.vDiffuse.rgb * fAtten;

        // --- Transmission ---
        // Real stained glass transmits light from BOTH sides.
        // Use abs(NdotL) so backlit glass is just as colorful as frontlit.
        // wrap slightly so grazing angles still get some color.
        float fTransmit = saturate(abs(NdotL) * 0.8f + 0.2f);
        vTransmission += vRadiance * fTransmit;

        // --- Surface specular (front face only) ---
        if (NdotL > 0.f)
        {
            float3 vHalf   = normalize(vViewDir + L);
            float  NdotH   = saturate(dot(vNormal, vHalf));
            float  specPow = max(2.f / (fRoughness * fRoughness + 0.001f) - 2.f, 1.f);

            // Fresnel: glass reflects more at grazing angles
            float  VdotH = saturate(dot(vViewDir, vHalf));
            float  fFresnel = 0.04f + 0.96f * pow(1.f - VdotH, 5.f);

            vReflection += vRadiance * pow(NdotH, specPow) * fFresnel;
        }

        // --- Ambient (constant base illumination) ---
        vTransmission += g_Light3D[i].Light.vAmbient.rgb;
    }

    // Tint the transmitted light by albedo (this IS the stained glass effect)
    float3 vFinalColor = vTransmission * vAlbedo.rgb + vReflection;

    // ---- 5. Opacity ----
    float fOpacity = g_float_0;
    if (g_bTex_3)
        fOpacity *= g_tex_3.Sample(g_Sam_0, _in.vUV).r;

    // Fresnel: glass edges appear more opaque (real glass behavior)
    float fNdotV = saturate(dot(vNormal, vViewDir));
    float fFresnelOpacity = 1.f - pow(fNdotV, 2.f); // 1.0 at edges, 0.0 head-on
    fOpacity = lerp(fOpacity, 1.f, fFresnelOpacity * 0.3f); // subtle edge darkening

    return float4(vFinalColor, fOpacity);
}

#endif