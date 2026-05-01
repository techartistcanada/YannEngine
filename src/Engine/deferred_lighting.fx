#ifndef DEFERRED_LIGHTING_FX
#define DEFERRED_LIGHTING_FX

#include "values.fx"
#include "func.fx"

#define Light3D_Idx      g_int_0
#define GBuffer_Position g_tex_0
#define GBuffer_Normal   g_tex_1
#define GBuffer_Albedo   g_tex_2
#define GBuffer_Data     g_tex_3  // R=Metallic, G=Roughness, B=AO, A=PBR flag

#define SHADOW_MAP       g_tex_4
#define LIGHT_ViewProj   g_mat_0

// IBL ambient
#define Irradiance_Map   g_texcube_1
#define Prefiltered_Map  g_texcube_2
#define BRDFLut          g_tex_5

// ===================================
// Directional Light
// 
// Shader Domain: DOMAIN::DOMAIN_DEFERRED_LIGHT
// MRT Type:      MRT_TYPE::DEFERRED_LIGHT
// Mesh:          Rect(FullScreen Quad)
// ===================================

struct VS_IN
{
    float3 vPos : POSITION;
    float2 vUV  : TEXCOORD;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vUV : TEXCOORD;
};

VS_OUT VS_DirLight(VS_IN _in)
{
    VS_OUT output = (VS_OUT) 0.f;

    // NOTE: to clip space
    output.vPosition = float4(_in.vPos.xy * 2, 0.f, 1.f);

    output.vUV = _in.vUV;

    return output;
}

struct PS_OUT
{
    float4 vDiffuse : SV_TARGET;
    float4 vSpecular : SV_TARGET1;
};

PS_OUT PS_DirLight(VS_OUT _in)
{
    PS_OUT output = (PS_OUT) 0.f;

    float4 vPosition = GBuffer_Position.Sample(g_Sam_0, _in.vUV);

    if (0.f == vPosition.a)
        discard;

    // * ---- shadow -----
    float4 vWorldPisition = mul(float4(vPosition.xyz, 1.0f), g_matViewInv);
    float4 vLightProjPos = mul(float4(vWorldPisition.xyz, 1.0f), LIGHT_ViewProj);
    float2 vProjXY = vLightProjPos.xy / vLightProjPos.w;
    float2 vShadowUV;
    vShadowUV.x = vProjXY.x / 2.f + 0.5f;
    vShadowUV.y = 1.f - (vProjXY.y / 2.f + 0.5f);
    float Depth = vLightProjPos.z / vLightProjPos.w;

    // Slope-scaled bias to reduce shadow acne
    float4 vNormal = GBuffer_Normal.Sample(g_Sam_0, _in.vUV);
    float3 vLightDirView = normalize(mul(float4(g_Light3D[Light3D_Idx].WorldDir, 0.f), g_matView).xyz);
    float NdotL = saturate(dot(normalize(vNormal.xyz), -vLightDirView));
    float fBias = max(0.005f * (1.0f - NdotL), 0.001f);

    // 5x5 PCF — blurs away residual rotation shimmer
    float ShadowPower = 0.f;
    float2 texDim;
    SHADOW_MAP.GetDimensions(texDim.x, texDim.y);
    float2 texelSize = 1.0f / texDim;

    [unroll]
    for (int x = -2; x <= 2; ++x)
    {
        [unroll]
        for (int y = -2; y <= 2; ++y)
        {
            float sampleDepth = SHADOW_MAP.Sample(g_Sam_0, vShadowUV + float2(x, y) * texelSize).r;
            ShadowPower += (Depth > sampleDepth + fBias) ? 1.0f : 0.0f;
        }
    }
    ShadowPower /= 25.0f;


    // Clamp: outside shadow map = no shadow
    if (vShadowUV.x < 0.f || vShadowUV.x > 1.f || vShadowUV.y < 0.f || vShadowUV.y > 1.f)
        ShadowPower = 0.f;
    // * ---- shadow -----

    float4 vData = GBuffer_Data.Sample(g_Sam_0, _in.vUV);

    tLight light = (tLight) 0.f;

    if (vData.a > 0.5f)
    {
        // PBR path
        float4 vAlbedo = GBuffer_Albedo.Sample(g_Sam_0, _in.vUV);
        float  fMetallic  = vData.r;
        float  fRoughness = vData.g;
        float  fAO        = vData.b;


        // ================
        // 0. Direct Lighting BRDF
        // ================
        CalcLight3D_PBR(Light3D_Idx, vPosition.xyz, vNormal.xyz,
                        vAlbedo.rgb, fMetallic, fRoughness, light);

        // ================
        // Ambient IBL
        // ================
        float3 N = normalize(vNormal.xyz);
        // camera space view direction
        float3 V = -normalize(vPosition.xyz); 
        // F0 = 正面(垂直入射)时的反射率
        // 金属: F0=Albedo，非金属: F0=0.04
        float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), vAlbedo.rgb, fMetallic);

        // Ambient IBL
        float NdotV = max(dot(N, V), 0.0);
        float3 F_ibl = FresnelSchlickRoughness(NdotV, F0, fRoughness);
        float3 kS_ibl = F_ibl;
        // diffuse = 剩余能量 x 非金属(kS + kD roughly equals 1) 
        float3 kD_ibl = (1.0 - kS_ibl) * (1.0 - fMetallic);
              // ================
        // Diffuse IBL
        // ================
        // 用 EnvRotationY 绕 Y 轴旋转世界空间法线，使 IBL 采样方向和天空盒一致
        float cosR = cos(0.f);
        float sinR = sin(0.f);
        float3 N_world = mul(float4(N, 0), g_matViewInv).xyz;
        N_world = float3( N_world.x * cosR + N_world.z * sinR,
                          N_world.y,
                         -N_world.x * sinR + N_world.z * cosR);

        float3 irradiance = Irradiance_Map.Sample(g_Sam_0, N_world).rgb * 1.f;
        float3 diffuseIBL = kD_ibl * irradiance * vAlbedo.rgb;

        // ================
        // Specular IBL
        // ================
        float3 R_View  = reflect(-V, N);
        float3 R_world = mul(float4(R_View, 0), g_matViewInv).xyz;
        // 反射方向也做同样的 Y 轴旋转
        R_world = float3( R_world.x * cosR + R_world.z * sinR,
                          R_world.y,
                         -R_world.x * sinR + R_world.z * cosR);

        float MAX_REFLECTION_LOD = 4.0f;
        float3 prefilteredColor = Prefiltered_Map.SampleLevel(g_Sam_0, R_world, fRoughness * MAX_REFLECTION_LOD).rgb * 1.f;

        float2 brdf = BRDFLut.Sample(g_Sam_Clamp, float2(NdotV, fRoughness)).rg;
        float3 specularIBL = prefilteredColor * (F_ibl * brdf.x + brdf.y);

        float3 ambient = (diffuseIBL + specularIBL) * fAO * 0.5f;


        output.vDiffuse.rgb = light.vDiffuse.rgb * (1.f - ShadowPower) + ambient;
        output.vSpecular.rgb = (float3) 0.f;
    }
    else
    {
        // Blinn-Phong
        CalcLight3D(Light3D_Idx, vPosition.xyz, vNormal.xyz, light);

        // FIX: apply shadow to Blinn-Phong path too
        output.vDiffuse  = light.vDiffuse * (1.f - ShadowPower) + light.vAmbient;
        output.vSpecular = light.vMaxSpecular * (1.f - ShadowPower);
    }

 
    // NOTE: store light touchinng object's depth in alpha channel for later use (e.g. point light)
    // because light could touch an object that's very far away
    output.vDiffuse.a = vPosition.z; 
    output.vSpecular.a = vPosition.z;

    return output;
}
// ===================================
// Point Light
// 
// Shader Domain: DOMAIN::DOMAIN_DEFERRED_LIGHT
// MRT Type:      MRT_TYPE::DEFERRED_LIGHT
// Mesh:          Sphere, radius 0.5f(local space)
// DS_TYPE:       
// ===================================
VS_OUT VS_PointLight(VS_IN _in)
{
    VS_OUT output = (VS_OUT) 0.f;

    output.vPosition = mul(float4(_in.vPos, 1.f), g_matWVP);

    output.vUV = _in.vUV;

    return output;
}


PS_OUT PS_PointLight(VS_OUT _in)
{
    PS_OUT output = (PS_OUT) 0.f;

    float2 vScreenUV = _in.vPosition.xy / vResolution;
    float4 vPosition = GBuffer_Position.Sample(g_Sam_0, vScreenUV);

    if(0.f == vPosition.a)
        discard;

    // ================
    // check if pixle is inside lighting volume (sphere)
    // ================
    // obj pos from world space to lighting volume local space
    float4 vPosLocal = mul(mul(float4(vPosition.xyz, 1.f), g_matViewInv), g_matWorldInv);
    if(0.5 < length(vPosLocal.xyz))
        discard;

    // ================
    // if pixel is inside lighting volume, calculate lighting
    // ================
    float4 vNormal = GBuffer_Normal.Sample(g_Sam_0, vScreenUV);
    float4 vData   = GBuffer_Data.Sample(g_Sam_0, vScreenUV);

    tLight light = (tLight) 0.f;

    if (vData.a > 0.5f)
    {
        // PBR
        float4 vAlbedo = GBuffer_Albedo.Sample(g_Sam_0, vScreenUV);
        CalcLight3D_PBR(Light3D_Idx, vPosition.xyz, vNormal.xyz,
                        vAlbedo.rgb, vData.r, vData.g, light);

        output.vDiffuse.rgb  = light.vDiffuse.rgb + light.vAmbient.rgb * vData.b;
        output.vSpecular.rgb = (float3) 0.f;
    }
    else
    {
        // Blinn-Phong
        CalcLight3D(Light3D_Idx, vPosition.xyz, vNormal.xyz, light);
        output.vDiffuse  = light.vDiffuse + light.vAmbient;
        output.vSpecular = light.vMaxSpecular;
    }

    // NOTE: store light touchinng object's depth in alpha channel for later use (e.g. point light)
    // because light could touch an object that's very far away
    output.vDiffuse.a = vPosition.z; 
    output.vSpecular.a = vPosition.z;

    return output;
}

// ===================================
// Spot Light
// ===================================

#endif