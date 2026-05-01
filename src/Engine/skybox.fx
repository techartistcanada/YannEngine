#ifndef _SKYBOX
#define _SKYBOX

#include "values.fx"

#include "func.fx"

#define SKYBOX_TYPE     g_int_0
#define ROTATION        g_float_0
#define EXPOSURE        g_float_1
#define ROUGHNESS_OVR   g_float_2

// IBL prefiltered specular cubemap (bound at t8 by CIBLManager)
#define PREFILTER_MAP   g_texcube_2
#define PREFILTER_MIPS  4.0f   // PREFILTER_MIP_LEVELS - 1

struct VS_IN
{
    float3 vPos : POSITION; 
    float2 vUV : TEXCOORD;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vUV : TEXCOORD;
    float3 vCubeUV : TEXCOORD1;
};

VS_OUT VS_SkyBox(VS_IN _in)
{
    VS_OUT output = (VS_OUT) 0.f;
    
    float4 vPosInView = mul(float4(_in.vPos, 0.f), g_matView);
    output.vPosition = mul(vPosInView, g_matProj);
    output.vPosition.z = output.vPosition.w;
    output.vUV = _in.vUV;
    output.vCubeUV = normalize(_in.vPos);

    return output;
}

// Rotate direction around Y-axis
float3 RotateY(float3 v, float angle)
{
    float s, c;
    sincos(angle, s, c);
    return float3(c * v.x + s * v.z, v.y, -s * v.x + c * v.z);
}

// Convert equirectangular UV → world direction
float3 UVToDirection(float2 uv)
{
    float theta = uv.x * 2.0f * PI;       // longitude [0, 2pi]
    float phi   = uv.y * PI;              // latitude  [0, pi]
    return float3(sin(phi) * cos(theta),
                  cos(phi),
                  sin(phi) * sin(theta));
}

float4 PS_SkyBox(VS_OUT _in) : SV_TARGET
{
    float4 vOutColor = float4(1.0f, 0.0f, 1.0f, 1.0f);

    // Sphere skybox (equirectangular HDR)
    if (SKYBOX_TYPE == 0)
    {
        if (ROUGHNESS_OVR >= 0.0f)
        {
            // Roughness override: sample IBL prefiltered cubemap instead
            float3 dir = UVToDirection(_in.vUV);
            dir = RotateY(dir, ROTATION);
            float mipLevel = ROUGHNESS_OVR * PREFILTER_MIPS;
            vOutColor = PREFILTER_MAP.SampleLevel(g_Sam_0, dir, mipLevel);
        }
        else if (g_bTex_0)
        {
            // Normal path: sample equirectangular texture with rotation
            float2 uv = _in.vUV;
            uv.x = frac(uv.x + ROTATION / (2.0f * PI));
            vOutColor = g_tex_0.Sample(g_Sam_0, uv);
        }
    }
    // Cube skybox
    else if (SKYBOX_TYPE == 1) 
    {
        if (g_bTexCube_0)
        {
            float3 dir = RotateY(_in.vCubeUV, ROTATION);

            if (ROUGHNESS_OVR >= 0.0f)
            {
                float mipLevel = ROUGHNESS_OVR * PREFILTER_MIPS;
                vOutColor = g_texcube_0.SampleLevel(g_Sam_0, dir, mipLevel);
            }
            else
            {
                vOutColor = g_texcube_0.Sample(g_Sam_0, dir);
            }
        }
    }

    // Apply exposure
    vOutColor.rgb *= EXPOSURE;

    return vOutColor;
}
#endif