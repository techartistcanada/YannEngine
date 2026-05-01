#ifndef _PARTICLE
#define _PARTICLE

#include "values.fx"
#include "func.fx"

StructuredBuffer<tParticle> g_ParticleBuffer : register(t17);

#define Particle g_ParticleBuffer[_in[0].InstID]

struct VS_PARTICLE_IN
{
    float3 vPos     : POSITION;
    float2 vUV      : TEXCOORD;
    uint   InstID : SV_InstanceID;
};

struct VS_PARTICLE_OUT
{
    float3 vPos : POSITION;
    float2 vUV : TEXCOORD;
    uint InstID : INSTANCE_ID;
};

// 这里不做任何事情，但是如果没有顶点着色器，渲染管线将无法工作
VS_PARTICLE_OUT VS_Particle(VS_PARTICLE_IN _in)
{
    VS_PARTICLE_OUT output = (VS_PARTICLE_OUT) 0.f;

    
    output.vPos = _in.vPos;
    output.vUV = _in.vUV;
    output.InstID = _in.InstID;


    return output;
}

struct GS_OUT
{
    float4 vPos : SV_Position;
    float2 vUV : TEXCOORD;
    uint InstID : INSTANCE_ID;
};

[maxvertexcount(6)]
// 为什么是_in[1]？因为是point mesh只有一个顶点,如果是line mesh则是_in[2], 如果是triangle mesh则是_in[3]
void GS_Particle(point VS_PARTICLE_OUT _in[1], inout TriangleStream<GS_OUT> _OutStream)
{
    GS_OUT output[4] = { (GS_OUT) 0.f, (GS_OUT) 0.f, (GS_OUT) 0.f, (GS_OUT) 0.f };
    // 1.通过不输出任何顶点实现 “discard” inactive 粒子
    if(0 == Particle.Active)
    {
        return;
    }
    // 2.从point mesh生成一个面向摄像机的四边形(billboard)
    float4 vViewPos = mul(float4(Particle.vWorldPos, 1.0f), g_matView);
    // 0 -- 1
    // | \  |
    // 3 -- 2
    output[0].vPos = vViewPos + float4(-Particle.vWorldScale.x * 0.5f, Particle.vWorldScale.y * 0.5f, 0.0f, 0.0f);
    output[1].vPos = vViewPos + float4(Particle.vWorldScale.x * 0.5f, Particle.vWorldScale.y * 0.5f, 0.0f, 0.0f);
    output[2].vPos = vViewPos + float4(Particle.vWorldScale.x * 0.5f, -Particle.vWorldScale.y * 0.5f, 0.0f, 0.0f);
    output[3].vPos = vViewPos + float4(-Particle.vWorldScale.x * 0.5f, -Particle.vWorldScale.y * 0.5f, 0.0f, 0.0f);

    for(int i = 0; i < 4; i++)
    {
        output[i].vPos = mul(output[i].vPos, g_matProj);
    }

    output[0].vUV = float2(0.0f, 0.0f);
    output[1].vUV = float2(1.0f, 0.0f);
    output[2].vUV = float2(1.0f, 1.0f);
    output[3].vUV = float2(0.0f, 1.0f);

    output[0].InstID = output[1].InstID = output[2].InstID = output[3].InstID = _in[0].InstID;
    // 3.输出6个顶点组成2个三角形
    _OutStream.Append(output[0]);
    _OutStream.Append(output[1]);
    _OutStream.Append(output[2]);
    _OutStream.RestartStrip();

    _OutStream.Append(output[0]);
    _OutStream.Append(output[2]);
    _OutStream.Append(output[3]);
    _OutStream.RestartStrip();
}

float4 PS_Particle(GS_OUT _in) : SV_Target
{
    float4 vOutColor = (float4) 0.f;
    if(g_bTex_0)
    {
        vOutColor = g_tex_0.Sample(g_Sam_0, _in.vUV);
    }
    else
    {
        vOutColor = float4(1.f, 1.f, 1.f, 1.f);
    }

    return vOutColor;
}
#endif