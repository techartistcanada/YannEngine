#ifndef _STRUCT
#define _STRUCT
// structured buffer
struct tLight
{
	float4 vDiffuse;
	float4 vAmbient;
	float4 vMaxSpecular;
};

struct tLightInfo
{
	tLight	Light;
	uint	LightType;
	float3	WorldDir;
    float3  WorldPos;
	float	Range;
	float	Angle;

	float3  Padding;
};

// particles
struct tSpawnCount
{
    int spawnCount;
    uint3 padding;
};

struct tParticle
{
	float4   vColor;

	float3   vLocalPos;
	float3   vWorldPos;
	float3   vWorldScale;
	float3   vWorldRotation;

    float3	 vForce;
	float3   vVelocity;

	float    Mass;

	float	 Life;
	float    Age;
	float    NormalizeAge;
	int      Active;

    float   PaddingParticle;
};

struct tParticleModule
{
	// Spawn Module
    uint   SpawnRate;
	float4 vSpawnColor;
	float4 vSpawnMinScale;
	float4 vSpawnMaxScale;

	uint   SpawnShape;
	float3 SpawnShapeScale;

	uint   BlockingSpawnShape;
	float3 BlockingSpawnShapeScale;

	float  minLife;
	float  maxLife;

	// Spawn Burst Module
	uint  SpawnBurstCount;
	uint  SpawnBurstRepeatTime;
	uint  SpawnBurstRepeat;

	// Add Velocity Module
	uint    AddVelocityType; // 0:Random 1:From Center 2: ToCenter 3: Fixed
	float3  AddVelocityFixedDir;
	float   AddVelocityMinSpeed;
	float   AddVelocityMaxSpeed;

	// Scale Module
    float3  StartScale;
    float3  EndScale;

	// Module On or Off
	int   ModuleOnOff[7];

    float3 Padding;
};
#endif
