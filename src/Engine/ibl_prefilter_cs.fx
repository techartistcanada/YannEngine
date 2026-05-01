#ifndef _PREFILTER_CS
#define _PREFILTER_CS

#include "values.fx"

TextureCube<float4>          g_EnvCubemap    : register(t0);
RWTexture2DArray<float4>     g_OutPrefilter  : register(u0);

#define MipSize    g_int_0
#define Roughness  g_float_0
#define EnvMapSize g_int_1   // <-- NEW: source cubemap resolution

float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}

float2 Hammersley(uint i, uint N)
{
    return float2(float(i) / float(N), RadicalInverse_VdC(i));
}

float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
    float a = roughness * roughness;

    float phi = 2.0f * PI * Xi.x;
    float cosTheta = sqrt((1.0f - Xi.y) / (1.0f + (a * a - 1.0f) * Xi.y));
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

    // Spherical to cartesian
    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    // Tangent space to world
    float3 up = abs(N.y) < 0.999f ? float3(0, 1, 0) : float3(1, 0, 0);
    float3 tangent = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);

    float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

// GGX NDF for PDF calculation
float DistributionGGX(float NdotH, float roughness)
{
    float a  = roughness * roughness;
    float a2 = a * a;
    float d  = (NdotH * NdotH) * (a2 - 1.0f) + 1.0f;
    return a2 / (PI * d * d);
}

float3 GetCubemapDir(uint face, float2 uv)
{
    float cx = uv.x * 2.0f - 1.0f;
    float cy = 1.0f - uv.y * 2.0f;

    float3 dir = float3(0, 0, 0);
    switch (face)
    {
        case 0: dir = float3( 1, cy, -cx); break;
        case 1: dir = float3(-1, cy,  cx); break;
        case 2: dir = float3( cx, 1, -cy); break;
        case 3: dir = float3( cx,-1,  cy); break;
        case 4: dir = float3( cx, cy,  1); break;
        case 5: dir = float3(-cx, cy, -1); break;
    }
    return normalize(dir);
}

[numthreads(16, 16, 1)]
void CS_Prefilter(uint3 DTid : SV_DispatchThreadID)
{
    uint face = DTid.z;
    if (face >= 6)
        return;

    uint size = (uint) MipSize;
    if (DTid.x >= size || DTid.y >= size)
        return;

    float2 uv = (float2(DTid.xy) + 0.5f) / float(size);
    float3 N = GetCubemapDir(face, uv);
    float3 R = N;
    float3 V = R;

    float resolution = float(EnvMapSize); // source cubemap face resolution
    const uint SAMPLE_COUNT = 1024u;
    float3 prefilteredColor = float3(0, 0, 0);
    float  totalWeight = 0.0f;

    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        float2 Xi = Hammersley(i, SAMPLE_COUNT);
        float3 H = ImportanceSampleGGX(Xi, N, Roughness);
        float3 L = normalize(2.0f * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0f);
        if (NdotL > 0.0f)
        {
            float NdotH = max(dot(N, H), 0.0f);
            float HdotV = max(dot(H, V), 0.0f);

            // PDF of the GGX importance sampling
            float D   = DistributionGGX(NdotH, Roughness);
            float pdf = (D * NdotH / (4.0f * HdotV)) + 0.0001f;

            // Solid angle of one texel on the source cubemap
            float saTexel  = 4.0f * PI / (6.0f * resolution * resolution);
            // Solid angle of one sample
            float saSample = 1.0f / (float(SAMPLE_COUNT) * pdf + 0.0001f);

            // Mip level: larger solid angle per sample → higher mip (more blur)
            float mipLevel = (Roughness == 0.0f) ? 0.0f : 0.5f * log2(saSample / saTexel);

            prefilteredColor += g_EnvCubemap.SampleLevel(g_Sam_1, L, mipLevel).rgb * NdotL;
            totalWeight += NdotL;
        }
    }
    prefilteredColor /= max(totalWeight, 0.001f);

    g_OutPrefilter[uint3(DTid.xy, face)] = float4(prefilteredColor, 1.0f);
}

#endif