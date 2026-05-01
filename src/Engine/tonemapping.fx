#ifndef _TONEMAPPING
#define _TONEMAPPING

#include "values.fx"

// ****************************************************
// Final Tone Mapping + Gamma Correction Pass
//
// Domain: DOMAIN_POSTPROCESS (or a dedicated domain)
// Input:  g_tex_0 = HDRSceneTex (R16G16B16A16_FLOAT)
// Output: SV_Target = SWAPCHAIN (R8G8B8A8_UNORM / sRGB)
//
// g_float_0 : Exposure (default = 1.0)
// g_int_0   : Tone map operator (0 = ACES, 1 = Reinhard, 2 = Uncharted2)
// ****************************************************

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

VS_OUT VS_ToneMapping(VS_IN _in)
{
    VS_OUT output = (VS_OUT) 0.f;
    output.vPosition = float4(_in.vPos.xy * 2.f, 0.f, 1.f);
    output.vUV = _in.vUV;
    return output;
}

// ---- Tone Map Operators ----

// ACES Filmic (Stephen Hill approximation)
float3 ACESFilm(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

// Reinhard
float3 ReinhardToneMap(float3 x)
{
    return x / (x + 1.0f);
}

// Uncharted 2 (John Hable)
float3 Uncharted2Partial(float3 x)
{
    float A = 0.15f;
    float B = 0.50f;
    float C = 0.10f;
    float D = 0.20f;
    float E = 0.02f;
    float F = 0.30f;
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float3 Uncharted2ToneMap(float3 x)
{
    float  exposureBias = 2.0f;
    float3 curr         = Uncharted2Partial(x * exposureBias);
    float3 whiteScale   = 1.0f / Uncharted2Partial(float3(11.2f, 11.2f, 11.2f));
    return curr * whiteScale;
}

float4 PS_ToneMapping(VS_OUT _in) : SV_Target
{
    float3 hdrColor = g_tex_0.Sample(g_Sam_0, _in.vUV).rgb;

    // Exposure
    float fExposure = (g_float_0 > 0.f) ? g_float_0 : 1.0f;
    hdrColor *= fExposure;

    // Tone mapping
    float3 mapped;
    if (g_int_0 == 1)
        mapped = ReinhardToneMap(hdrColor);
    else if (g_int_0 == 2)
        mapped = Uncharted2ToneMap(hdrColor);
    else
        mapped = ACESFilm(hdrColor); // default: ACES

    // Gamma correction (linear -> sRGB)
    mapped = pow(mapped, 1.0f / 2.2f);

    return float4(mapped, 1.0f);
}

#endif