#pragma once
#include "CComputeShader.h"

class CStructuredBuffer;

class CParticleTickCS :
    public CComputeShader
{
private:
	CStructuredBuffer* m_ParticleBuffer;
	CStructuredBuffer* m_SpawnCountBuffer;
	CStructuredBuffer* m_ModuleDataBuffer;

	Ptr<CTexture>	   m_NoiseTexture;
	Vec3			   m_ParticleWorldPos;
public:
	virtual int Binding() override;
	virtual void CalculateNumGroups() override;
	virtual void Clear() override;
public:
	void SetModuleDataBuffer(CStructuredBuffer* _Buffer) { m_ModuleDataBuffer = _Buffer; }
	void SetParticleBuffer(CStructuredBuffer* _Buffer) { m_ParticleBuffer = _Buffer; }
	void SetSpawnCountBuffer(CStructuredBuffer* _Buffer) { m_SpawnCountBuffer = _Buffer; }


	void SetParticleWorldPos(const Vec3 _Pos) { m_ParticleWorldPos = _Pos; }
public:

	CParticleTickCS();
	~CParticleTickCS();
};

