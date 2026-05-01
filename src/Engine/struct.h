#pragma once


struct Vertex
{
	Vec3 Pos;
	Vec4 Color;
	Vec2 UV;

	Vec3 vTangent;
	Vec3 vNormal;
	Vec3 vBinormal;
};

struct tTransform
{
	Matrix matWorld;
	Matrix matWorldInv;
	Matrix matView;
	Matrix matViewInv;
	Matrix matProj;
	Matrix matProjInv;

	Matrix matWV;
	Matrix matWVP;
};

extern tTransform g_Trans;

// 16 字节对齐, 也就是说16的倍数
struct tMaterialConst
{
	int iArr[4];
	float fArr[4];
	Vec2 v2Arr[4];
	Vec4 v4Arr[4];
	Matrix matArr[4];
	UINT   bText[16];
};

struct tDebugShapeInfo
{
	DEBUG_SHAPE Shape;
	Vec3		Position;
	Vec3		Scale;
	Vec3		Rotation;
	Matrix      matWorld;
	Vec4		Color;
	float       Duration;
	float       Age;
	bool		DepthTest;
};


struct tAnim2DInfo
{
	Vec2 vLeftTop;
	Vec2 vSliceSize;

	Vec2 vOffset;
	Vec2 vBackground;

	int UseAnim2D;
	int Padding[3];
};

struct tTask
{
	TASK_TYPE Type;

	DWORD_PTR dwParam_0;
	DWORD_PTR dwParam_1;
	DWORD_PTR dwParam_2;
};


// structured buffer
struct tLight
{
	Vec4 vDiffuse;
	Vec4 vAmbient;
	Vec4 vMaxSpecular;
};

// ===============================================
// particles
// ===============================================
struct tParticle
{
	Vec4   vColor;

	Vec3   vLocalPos;
	Vec3   vWorldPos;
	Vec3   vWorldScale;
	Vec3   vWorldRotation;

	Vec3   vForce;
	Vec3   vVelocity;

	float  Mass;

	float  Life;
	float  Age;
	float  NormalizeAge;
	int    Active;

	float   Padding;
};

struct tParticleModule
{
	// Spawn Module
	UINT  SpawnRate; // 每秒生成粒子数量

	Vec4 vSpawnColor;
	Vec4 vSpawnMinScale;
	Vec4 vSpawnMaxScale;

	UINT SpawnShape; // 0:立方体 1:球体
	Vec3 SpawnShapeScale;

	UINT BlockingSpawnShape; // 0:立方体 1:球体
	Vec3 BlockingSpawnShapeScale;

	float minLife;
	float maxLife;

	// Spawn Burst Module
	UINT SpawnBurstCount; // 一次性生成粒子数量
	UINT SpawnBurstRepeatTime;	 // 重复时间
	UINT SpawnBurstRepeat;

	// Add Velocity Module
	UINT  AddVelocityType; // 0:Random 1:From Center 2: ToCenter 3: Fixed
	Vec3  AddVelocityFixedDir;
	float AddVelocityMinSpeed;
	float AddVelocityMaxSpeed;

	// Scale Module
	Vec3 StartScale;
	Vec3 EndScale;

	// Module On or Off
	int ModuleOnOff[(UINT)PARTICLE_MODULE::END];
};

// ===============================================
// light
// ===============================================
struct tLightInfo
{
	tLight Light;
	UINT   LightType;
	Vec3   WorldDir;
	Vec3   WorldPos;
	float  Range;
	float  Angle;

	Vec3   Padding;
};

struct tGlobalData
{
    Vec2 vResolution;
    float  DeltaTime;
    float  Time;
	
	float EditorDeltaTime;
	float EditorTime;

    int    Light2DCount;
    int    Light3DCount;
};
extern tGlobalData g_GlobalData;
