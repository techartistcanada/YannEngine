#ifndef _VALUE
#define _VALUE

#include "struct.fx"

// corresponds to tTransform in struct.h
// passed in from CTransform and CRenderMgr(from CCamera)
cbuffer TRANSFORM : register(b0)
{
    row_major matrix g_matWorld;
    row_major matrix g_matWorldInv;

    row_major matrix g_matView;
    row_major matrix g_matViewInv;

    row_major matrix g_matProj;
    row_major matrix g_matProjInv;

    row_major matrix g_matWV;
    row_major matrix g_matWVP;
};

cbuffer MATERIAL_CONST : register(b1)
{
	//int iArr[4];
    int g_int_0;
    int g_int_1;
    int g_int_2;
    int g_int_3;
	//float fArr[4];
    float g_float_0;
    float g_float_1;
    float g_float_2;
    float g_float_3;
	//float2 v2Arr[4];
    float2 g_vec2_0;
    float2 g_vec2_1;
    float2 g_vec2_2;
    float2 g_vec2_3;

	//float4 v4Arr[4];
    float4 g_vec4_0;
    float4 g_vec4_1;
    float4 g_vec4_2;
    float4 g_vec4_3;
	//Matrix matArr[4];
    row_major Matrix g_mat_0;
    row_major Matrix g_mat_1;
    row_major Matrix g_mat_2;
    row_major Matrix g_mat_3;

    // whether there is a texture bound to slot
    uint             g_bTex_0;
    uint             g_bTex_1;
    uint             g_bTex_2;
    uint             g_bTex_3;
    uint             g_bTex_4;
    uint             g_bTex_5;
    uint             g_bTexCube_0;
    uint             g_bTexCube_1;
    uint             g_bTexCube_2;
    uint             g_bTexCube_3;
    uint             g_bTexArray_0;
    uint             g_bTexArray_1;
    uint             g_bTexArray_2;
    uint             g_bTexArray_3;
};

cbuffer ANIMATION2D : register(b2)
{
    float2 vLeftTop;
    float2 vSliceSize;
    float2 vOffset;
    float2 vBackground;
    int UseAnim2D;
    int3 _Pad2;
};

cbuffer GLOBALDATA : register(b3)
{
    float2 vResolution;
    float  DeltaTime;
    float  Time;

    float  EngineDeltaTime;
    float  EngineTime;
	
    int    Light2DCount;
    int    Light3DCount;

	float  IBLIntensity;
	float  EnvRotationY;
	float  EnvExposure;
	float  GlobalDataPadding;
};

SamplerState g_Sam_0 : register(s0);
SamplerState g_Sam_1 : register(s1);
SamplerState g_Sam_Clamp : register(s2);

Texture2D g_tex_0 : register(t0);
Texture2D g_tex_1 : register(t1);
Texture2D g_tex_2 : register(t2);
Texture2D g_tex_3 : register(t3);
Texture2D g_tex_4 : register(t4);
Texture2D g_tex_5 : register(t5);

TextureCube g_texcube_0 : register(t6);
TextureCube g_texcube_1 : register(t7);
TextureCube g_texcube_2 : register(t8);
TextureCube g_texcube_3 : register(t9);

Texture2DArray g_texarr_0 : register(t10);
Texture2DArray g_texarr_1 : register(t11);
Texture2DArray g_texarr_2 : register(t12);
Texture2DArray g_texarr_3 : register(t13);

// Animation Atlas
Texture2D g_Atlas : register(t14);


// Lights 2D
// Lights 3D
StructuredBuffer<tLightInfo> g_Light2D : register(t15);
StructuredBuffer<tLightInfo> g_Light3D : register(t16);

#define PI 3.14159265359f

#endif
