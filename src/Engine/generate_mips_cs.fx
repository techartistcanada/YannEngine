#ifndef _GENERATE_MIPS_CS
#define _GENERATE_MIPS_CS

#include "values.fx"

// Input:  source mip (SRV as Texture2D, single mip slice bound)
// Output: destination mip (UAV as RWTexture2D)

Texture2D<float4>   g_SrcMip : register(t0);
RWTexture2D<float4> g_DstMip : register(u0);

#define DstMipSize g_int_0

// Karis average: luminance-weighted downsampling (safe for HDR albedo maps too)
float KarisWeight(float3 c)
{
    float luma = dot(c, float3(0.2126f, 0.7152f, 0.0722f));
    return 1.0f / (1.0f + luma);
}

[numthreads(16, 16, 1)]
void CS_GenerateMips(uint3 DTid : SV_DispatchThreadID)
{
    uint size = (uint)DstMipSize;
    if (DTid.x >= size || DTid.y >= size)
        return;

    uint2 srcCoord = DTid.xy * 2;

    float3 c00 = g_SrcMip[srcCoord + uint2(0, 0)].rgb;
    float3 c10 = g_SrcMip[srcCoord + uint2(1, 0)].rgb;
    float3 c01 = g_SrcMip[srcCoord + uint2(0, 1)].rgb;
    float3 c11 = g_SrcMip[srcCoord + uint2(1, 1)].rgb;

    float w00 = KarisWeight(c00);
    float w10 = KarisWeight(c10);
    float w01 = KarisWeight(c01);
    float w11 = KarisWeight(c11);

    float  wSum  = w00 + w10 + w01 + w11;
    float3 result = (c00 * w00 + c10 * w10 + c01 * w01 + c11 * w11) / wSum;

    g_DstMip[DTid.xy] = float4(result, 1.0f);
}

#endif