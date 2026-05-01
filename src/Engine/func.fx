#ifndef _FUNC
#define _FUNC

#include "values.fx"
#include "pbr.fx"

float3 CalcLight2D(int _LightIdx, float3 _vWorldPos)
{
    tLightInfo info = g_Light2D[_LightIdx];

    float3 vLightPow = (float3) 0.f;
    if(0 == info.LightType)
    {
        // Directional Light
        vLightPow = info.Light.vDiffuse.rgb + info.Light.vAmbient.rgb;
    }
    else if(1 == info.LightType)
    {
        // Point Light
        float fDistance = distance(_vWorldPos.xy, info.WorldPos.xy);
        
        //float fRatio = 1.f - saturate(fDistance / info.Range); 
        float fRatio = cos((fDistance / info.Range) * (PI / 2.f)); 
        if(fDistance < info.Range)
        {
            vLightPow = info.Light.vDiffuse * fRatio;
        }
    }
    else if(2 == info.LightType)
    {
        // Spot Light
    }

    return vLightPow;

}

void CalcLight3D(int _LightIdx, float3 _PosInView, float3 _NormalInView, inout tLight _Light)
{

    tLightInfo LightInfo = g_Light3D[_LightIdx];
    float LightPow = 0.f;
    float3 vLightDirInView = 0.f;
    _NormalInView = normalize(_NormalInView);
    float DistancePower = 1.f;
    // ==============================
    // Directional Light
    // ==============================
    if(0 == LightInfo.LightType)
    {
        vLightDirInView = normalize(mul(float4(LightInfo.WorldDir, 0.f), g_matView).xyz);

        LightPow = saturate(dot(_NormalInView, -vLightDirInView));
    }
    // ==============================
    // Point Light
    // ==============================
    if(1 == LightInfo.LightType)
    {
        float3 vLightPosInView = mul(float4(LightInfo.WorldPos.xyz, 1.f), g_matView);
        vLightDirInView = normalize(_PosInView - vLightPosInView);
        LightPow = saturate(dot(_NormalInView, -vLightDirInView));
        float Distance = distance(_PosInView, vLightPosInView);
        DistancePower = saturate(cos(saturate(Distance / LightInfo.Range) * (PI / 2.f)));
    }
    // ==============================
    // Spot Light
    // ==============================
    if(2 == LightInfo.LightType)
    {
        
    }




    // reflect
    float3 vReflect = normalize(vLightDirInView + dot(-vLightDirInView, _NormalInView) * 2.f * _NormalInView);

    float3 vEye = normalize(_PosInView);

    float ReflectPow = saturate(dot(-normalize(vEye), normalize(vReflect)));
    ReflectPow = pow(ReflectPow, 20);

    // return
    _Light.vDiffuse.rgb += LightInfo.Light.vDiffuse.rgb * LightPow * DistancePower;
    _Light.vAmbient.rgb += LightInfo.Light.vAmbient.rgb;
    _Light.vMaxSpecular.rgb += LightInfo.Light.vDiffuse.rgb * LightInfo.Light.vMaxSpecular.rgb * ReflectPow * DistancePower;
}

// ============================================================
// PBR Cook-Torrance lighting (used by deferred lighting pass)
// ============================================================
void CalcLight3D_PBR(int _LightIdx, float3 _PosInView, float3 _NormalInView,
                     float3 _Albedo, float _Metallic, float _Roughness,
                     inout tLight _Light)
{
    tLightInfo LightInfo = g_Light3D[_LightIdx];

    float3 N = normalize(_NormalInView);
    float3 V = -normalize(_PosInView); // camera at origin in view space

    float3 vLightDirInView = (float3) 0.f;
    float  fAttenuation = 1.f;

    // Directional
    if (0 == LightInfo.LightType)
    {
        vLightDirInView = normalize(mul(float4(LightInfo.WorldDir, 0.f), g_matView).xyz);
    }
    // Point
    else if (1 == LightInfo.LightType)
    {
        float3 vLightPosInView = mul(float4(LightInfo.WorldPos.xyz, 1.f), g_matView).xyz;
        vLightDirInView = normalize(_PosInView - vLightPosInView);
        float fDist = distance(_PosInView, vLightPosInView);
        fAttenuation = saturate(cos(saturate(fDist / LightInfo.Range) * (PI / 2.f)));
    }
    // Spot (TODO)
    else
    {
        return;
    }

    float3 L = -vLightDirInView;
    float3 H = normalize(V + L);
    float  NdotL = max(dot(N, L), 0.0f);

    // ambient always applied
    _Light.vAmbient.rgb += LightInfo.Light.vAmbient.rgb;

    if (NdotL <= 0.0f)
        return;

    float3 vRadiance = LightInfo.Light.vDiffuse.rgb * fAttenuation;

    // F0: dielectric=0.04, metal=albedo
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), _Albedo, _Metallic);

    // Cook-Torrance specular BRDF
    float  NDF = DistributionGGX(N, H, _Roughness);
    float  G   = GeometrySmith(N, V, L, _Roughness);
    float3 F   = FresnelSchlick(max(dot(H, V), 0.0f), F0);

    float3 numerator  = NDF * G * F;
    float  denominator = 4.0f * max(dot(N, V), 0.0f) * NdotL + 0.0001f;
    float3 specular   = numerator / denominator;

    // energy conservation
    float3 kS = F;
    float3 kD = (1.0f - kS) * (1.0f - _Metallic);

    float3 diffuse = kD * _Albedo / PI;

    // combined outgoing radiance
    float3 Lo = (diffuse + specular) * vRadiance * NdotL;

    _Light.vDiffuse.rgb     += Lo;
    _Light.vMaxSpecular.rgb  = (float3) 0.f; // specular already baked into Lo
}

int IsBinding(in Texture2D _tex)
{
    uint width = 0;
    uint height = 0;
    _tex.GetDimensions(width, height);
    if(width && height)
        return 1;
    else 
        return 0;
}

float3 GetRandom(in Texture2D _Noise, float _NormalizedThreadID)
{
    float2 vUV = (float2) 0.f;

    vUV.x = _NormalizedThreadID + (Time * 0.1f);
    vUV.y = (sin((vUV.x - Time) * PI * 20.f) * 0.5f) + (Time * 0.2f);

    float3 vNoise = _Noise.SampleLevel(g_Sam_0, vUV, 0).xyz;

    return vNoise;
}
#endif
