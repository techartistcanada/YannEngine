#ifndef _IRRADIANCE_CS
#define _IRRADIANCE_CS

#include "values.fx"

// Input: environment cubemap (SRV t0)
// Output: irradiance cubemap (UAV u0)

TextureCube<float4>          g_EnvCubemap   : register(t0);
RWTexture2DArray<float4>     g_OutIrradiance : register(u0);

#define IrradianceSize g_int_0

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
void CS_Irradiance(uint3 DTid : SV_DispatchThreadID)
{
    uint face = DTid.z;
    if (face >= 6)
        return;

    uint size = (uint) IrradianceSize;
    if (DTid.x >= size || DTid.y >= size)
        return;

    float2 uv = (float2(DTid.xy) + 0.5f) / float(size);
    float3 N = GetCubemapDir(face, uv);

    // Build tangent frame
    float3 up = abs(N.y) < 0.999f ? float3(0, 1, 0) : float3(1, 0, 0);
    float3 right = normalize(cross(up, N));
    up = cross(N, right);

    // Hemisphere convolution via uniform sampling
    float3 irradiance = float3(0, 0, 0);
    float  sampleDelta = 0.025f;
    float  nrSamples = 0.0f;

    for (float phi = 0.0f; phi < 2.0f * PI; phi += sampleDelta)
    {
        for (float theta = 0.0f; theta < 0.5f * PI; theta += sampleDelta)
        {
            // Spherical to cartesian (tangent space)
            float3 tangentSample = float3(sin(theta) * cos(phi),
                                          sin(theta) * sin(phi),
                                          cos(theta));

            // Tangent space to world
            float3 sampleVec = tangentSample.x * right
                             + tangentSample.y * up
                             + tangentSample.z * N;

            irradiance += g_EnvCubemap.SampleLevel(g_Sam_1, sampleVec, 0).rgb
                        * cos(theta) * sin(theta);
            nrSamples += 1.0f;
        }
    }
    irradiance = PI * irradiance / nrSamples;

    g_OutIrradiance[uint3(DTid.xy, face)] = float4(irradiance, 1.0f);
}

#endif