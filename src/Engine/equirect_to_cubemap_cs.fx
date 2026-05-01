#ifndef _EQUIRECT_TO_CUBEMAP_CS
#define _EQUIRECT_TO_CUBEMAP_CS

#include "values.fx"

// Input: equirectangular HDR (t0 via Binding_CS_SRV)
// Output: cubemap 6-face array (u0 via Binding_CS_UAV)

Texture2D<float4>            g_EquirectMap : register(t0);
RWTexture2DArray<float4>     g_OutCubemap  : register(u0);

#define CubeMapSize g_int_0

// Direction from cubemap face + UV
float3 GetCubemapDir(uint face, float2 uv)
{
    // uv: [0,1] → [-1,1]
    float cx = uv.x * 2.0f - 1.0f;
    float cy = 1.0f - uv.y * 2.0f; // flip Y

    float3 dir = float3(0, 0, 0);
    switch (face)
    {
        case 0: dir = float3( 1, cy, -cx); break; // +X
        case 1: dir = float3(-1, cy,  cx); break; // -X
        case 2: dir = float3( cx, 1, -cy); break; // +Y
        case 3: dir = float3( cx,-1,  cy); break; // -Y
        case 4: dir = float3( cx, cy,  1); break; // +Z
        case 5: dir = float3(-cx, cy, -1); break; // -Z
    }
    return normalize(dir);
}

// Convert 3D direction → equirectangular UV
float2 DirToEquirectUV(float3 dir)
{
    // atan2(z,x) → longitude [-π, π]
    // asin(y)    → latitude  [-π/2, π/2]
    float phi   = atan2(dir.z, dir.x);
    float theta = asin(clamp(dir.y, -1.0f, 1.0f));

    float u = phi   / (2.0f * PI) + 0.5f;
    float v = theta / PI + 0.5f;
    return float2(u, 1.0f - v);
}

// Soft clamp: preserves colors below threshold, gently compresses above
// This prevents the sun disk (which can be 50,000+) from dominating IBL prefilter
float3 SoftClampHDR(float3 color, float threshold)
{
    float lum = max(color.r, max(color.g, color.b));
    if (lum > threshold)
    {
        // Reinhard-style soft knee on luminance only — preserves hue
        float compressed = threshold + (lum - threshold) / (1.0f + (lum - threshold) / threshold);
        color *= compressed / lum;
    }
    return color;
}

[numthreads(16, 16, 1)]
void CS_EquirectToCube(uint3 DTid : SV_DispatchThreadID)
{
    uint face = DTid.z;
    if (face >= 6)
        return;

    uint size = (uint)CubeMapSize;
    if (DTid.x >= size || DTid.y >= size)
        return;

    float2 uv = (float2(DTid.xy) + 0.5f) / float(size);
    float3 dir = GetCubemapDir(face, uv);
    float2 eqUV = DirToEquirectUV(dir);

    float4 color = g_EquirectMap.SampleLevel(g_Sam_1, eqUV, 0);

    // Soft-clamp extreme HDR values (sun disk etc.)
    // Threshold ~64 preserves sky gradients while taming the sun
    color.rgb = SoftClampHDR(color.rgb, 64.0f);

    g_OutCubemap[DTid] = color;
}

#endif