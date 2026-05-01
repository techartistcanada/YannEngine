#pragma once
#include "CRenderComponent.h"


struct tSpawnCount
{
	int		SpawnCount;
	UINT	Padding[3];
};
class CStructuredBuffer;
class CParticleTickCS;


class CParticleSystem :
    public CRenderComponent
{
private:
	Ptr<CParticleTickCS>	m_ParticleTickCS;
	CStructuredBuffer*		m_ParticleBuffer;
	CStructuredBuffer*		m_SpawnCountBuffer;
	CStructuredBuffer*      m_ModuleDataBuffer;

	Ptr<CTexture>			m_ParticleTex;

	float					m_AccumTime; 
	float 				    m_BurstAccumTime;
	UINT					m_MaxParticleCount;

	tParticleModule         m_ModuleData;
public:
	void SetParticleTexture(Ptr<CTexture> _Tex) { m_ParticleTex = _Tex; }
	void SetMaxParticleCount(UINT _Count);
private:
	void CalculateSpawnCount();
public:
	virtual void finaltick() override;
	virtual void render() override;

	virtual void SaveToLevelFile(FILE* _File) override;
	virtual void LoadFromLevelFile(FILE* _File) override;

	CLONE(CParticleSystem);
public:
	CParticleSystem();
	CParticleSystem(const CParticleSystem& _Other);
	~CParticleSystem();
};

