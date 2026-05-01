#ifndef __SSAO_FX
#define __SSAO_FX

// ****************************************************
// SSAO — Screen-Space Ambient Occlusion
// Domain: SHADER_DOMAIN::DOMAIN_DEFERRED_LIGHT
// MRT Type: MRT_TYPE::SSAO
// Input:  GBuffer_Position (view-space xyz, geometry flag a)
//         GBuffer_Normal   (view-space normal)
//         Noise texture    (4x4 random rotation)
// Output: R8_UNORM — occlusion factor (1 = no occlusion)
// ****************************************************

#include "values.fx"

#define GBuffer_Position  g_tex_0
#define GBuffer_Normal    g_tex_1
#define NoiseTex          g_tex_2

// SSAO parameters — tune SSAO_RADIUS to match your scene scale
#define SSAO_KERNEL_SIZE  32
#define SSAO_RADIUS       50.f   // view-space units; increase if scene is larger scale
#define SSAO_BIAS         2.5f // prevents self-occlusion acne; scale with RADIUS
#define SSAO_POWER        1.0f
#define NOISE_TILE_SIZE   4.0f

// Hemisphere kernel (deterministic, cosine-weighted, biased toward origin)
float3 GetKernelSample(int i)
{
    float fi = (float)i;
    float x = frac(sin(fi * 127.1f) * 43758.5453f) * 2.0f - 1.0f;
    float y = frac(sin(fi * 269.5f) * 43758.5453f) * 2.0f - 1.0f;
    float z = frac(sin(fi * 419.2f) * 43758.5453f); // [0,1] → upper hemisphere only

    float3 s = normalize(float3(x, y, z));

    // Accelerating interpolation: more samples closer to the surface
    float scale = (float)i / (float)SSAO_KERNEL_SIZE;
    scale = lerp(0.1f, 1.0f, scale * scale);
    s *= scale;

    return s;
}

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

VS_OUT VS_SSAO(VS_IN _in)
{
    VS_OUT output = (VS_OUT) 0.f;
    output.vPosition = float4(_in.vPos.xy * 2.0f, 0.f, 1.f);
    output.vUV = _in.vUV;
    return output;
}

float PS_SSAO(VS_OUT _in) : SV_Target
{
    // --- Read origin pixel ---
    float4 vPosData = GBuffer_Position.Sample(g_Sam_0, _in.vUV);

    // Skip sky / non-geometry pixels
    if (vPosData.a < 0.5f)
        return 1.0f;

    float3 vPos    = vPosData.xyz;
    float3 vNormal = normalize(GBuffer_Normal.Sample(g_Sam_0, _in.vUV).xyz);

    // Noise-based random rotation (tiled across screen)
    float2 noiseUV  = _in.vUV * (vResolution / NOISE_TILE_SIZE);
    float3 vRandVec = NoiseTex.Sample(g_Sam_0, noiseUV).xyz * 2.0f - 1.0f; // [0,1]→[-1,1]

    // Build TBN (Gram-Schmidt)
    float3 vTangent   = normalize(vRandVec - vNormal * dot(vRandVec, vNormal));
    float3 vBitangent = cross(vNormal, vTangent);
    float3x3 TBN      = float3x3(vTangent, vBitangent, vNormal);

    // --- Accumulate occlusion ---
    float fOcclusion  = 0.0f;
    float fTotalValid = 0.0f;  // track valid (non-sky) samples for correct normalization

    [unroll]
    for (int i = 0; i < SSAO_KERNEL_SIZE; ++i)
    {
        // Orient kernel sample into view space via TBN, scale by radius
        float3 vSample = mul(GetKernelSample(i), TBN);
        vSample = vPos + vSample * SSAO_RADIUS;

        // Project sample to screen UV
        float4 vOffset = float4(vSample, 1.0f);
        vOffset = mul(vOffset, g_matProj);  // clip space
        vOffset.xy /= vOffset.w;            // perspective divide

        // Guard against samples behind the camera (w<=0)
        if (vOffset.w <= 0.0f)
            continue;

        float2 sampleUV;
        sampleUV.x =  vOffset.x * 0.5f + 0.5f;
        sampleUV.y = -vOffset.y * 0.5f + 0.5f; // flip Y for DX UV convention

        // Discard samples that project outside screen
        if (sampleUV.x < 0.0f || sampleUV.x > 1.0f ||
            sampleUV.y < 0.0f || sampleUV.y > 1.0f)
            continue;

        // Sample geometry at projected position
        float4 vSamplePosData = GBuffer_Position.Sample(g_Sam_Clamp, sampleUV);

        // BUG FIX: Skip sky / invalid pixels — depth=0 sky pixels must NOT count as occluders
        if (vSamplePosData.a < 0.5f)
            continue;

        float fSampleDepth = vSamplePosData.z;

        // Range check: discard samples whose geometry is too far from origin (avoids halos)
        // abs(depth difference) should be within the sampling radius
        float fDepthDelta = abs(vPos.z - fSampleDepth);
        float fRangeCheck = (fDepthDelta < SSAO_RADIUS) ? 
            smoothstep(0.0f, 1.0f, 1.0f - fDepthDelta / SSAO_RADIUS) : 0.0f;

        // Occlusion test (Right-Handed view space: larger Z = closer to camera)
        // Geometry occludes if it is closer to camera than our sample point
        // i.e., fSampleDepth > vSample.z (for right-handed, "greater Z = closer")
        fOcclusion  += (fSampleDepth <= vSample.z - SSAO_BIAS ? 1.0f : 0.0f) * fRangeCheck;
        fTotalValid += 1.0f;
    }

    // Normalize against VALID samples only to avoid bias from sky/clipped samples
    float fAO = (fTotalValid > 0.0f) ? (1.0f - fOcclusion / fTotalValid) : 1.0f;
    return pow(saturate(fAO), SSAO_POWER);
}

#endif