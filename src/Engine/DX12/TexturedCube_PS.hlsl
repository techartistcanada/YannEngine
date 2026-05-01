Texture2D    g_Texture : register(t0);
SamplerState g_Sampler : register(s0);

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
    return g_Texture.Sample(g_Sampler, input.TexCoord);
}