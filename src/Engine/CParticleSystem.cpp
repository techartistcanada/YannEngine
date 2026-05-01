#include "pch.h"
#include "CParticleSystem.h"

#include "CStructuredBuffer.h"
#include "CTransform.h"

#include "CParticleTickCS.h"
#include "CTimeMgr.h"


CParticleSystem::CParticleSystem()
	: CRenderComponent(COMPONENT_TYPE::PARTICLESYSTEM)
	, m_ParticleBuffer(nullptr)
	, m_MaxParticleCount(100000)
	, m_AccumTime(0.f)
	, m_BurstAccumTime(0.f)
{
	SetFrustumCheck(false);
	SetCastDynamicShadow(false);

	// particle tick computer shader 计算着色器
	m_ParticleTickCS = (CParticleTickCS*)CAssetMgr::GetInst()->FindAsset<CComputeShader>(L"ParticleTickCS").Get();

	SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"PointMesh"));
	SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"ParticleMaterial"));

	// 创建粒子结构化缓冲区
	m_ParticleBuffer = new CStructuredBuffer;
	m_ParticleBuffer->Create(sizeof(tParticle), m_MaxParticleCount, SB_TYPE::SRV_UAV, false, nullptr);

	// 创建粒子生成计数结构化缓冲区
	m_SpawnCountBuffer = new CStructuredBuffer;
	m_SpawnCountBuffer->Create(sizeof(tSpawnCount), 1, SB_TYPE::SRV_UAV, true);

	// -----------------------
	// spawn模块参数初始化
	// -----------------------
	m_ModuleData.ModuleOnOff[(UINT)PARTICLE_MODULE::SPAWN] = true; // 开启spawn模块
	m_ModuleData.SpawnRate = 200;
	m_ModuleData.vSpawnColor = Vec3(1.f, 0.f, 0.f);
	m_ModuleData.vSpawnMinScale = Vec3(1.f, 1.f, 1.f);
	m_ModuleData.vSpawnMaxScale = Vec3(20.f, 20.f, 20.f);
	m_ModuleData.minLife = 10.f;
	m_ModuleData.maxLife = 40.f;

	m_ModuleData.SpawnShape = 1;
	m_ModuleData.SpawnShapeScale = Vec3(200.f, 200.f, 200.f);

	m_ModuleData.BlockingSpawnShape = 0;
	m_ModuleData.BlockingSpawnShapeScale = Vec3(190.f, 190.f, 190.f);

	// -----------------------
	// spawn burst模块参数初始化
	// -----------------------
	m_ModuleData.ModuleOnOff[(UINT)PARTICLE_MODULE::SPAWN_BURST] = true; // 开启spawn burst模块
	m_ModuleData.SpawnBurstRepeat = true;
	m_ModuleData.SpawnBurstRepeatTime = 3.f; // 每2秒生成一次
	m_ModuleData.SpawnBurstCount = 100;

	// -----------------------
	// Add Velocity模块参数初始化
	// -----------------------
	m_ModuleData.ModuleOnOff[(UINT)PARTICLE_MODULE::ADD_VELOCITY] = true; // 开启Add Velocity模块
	m_ModuleData.AddVelocityType = 3; // Random
	m_ModuleData.AddVelocityFixedDir = Vec3(0.f, 1.f, 0.f);
	m_ModuleData.AddVelocityMinSpeed = 100.f;
	m_ModuleData.AddVelocityMaxSpeed = 400.f;



	// 创建moduledata缓冲区
	m_ModuleDataBuffer = new CStructuredBuffer;
	m_ModuleDataBuffer->Create(sizeof(tParticleModule) + (16 - sizeof(tParticleModule) % 16), 1, SB_TYPE::SRV_UAV, true, &m_ModuleData);


}

CParticleSystem::CParticleSystem(const CParticleSystem& _Other)
	: CRenderComponent(_Other)
	, m_ParticleTickCS(_Other.m_ParticleTickCS)
	, m_ParticleBuffer(nullptr)
	, m_SpawnCountBuffer(nullptr)
	, m_ModuleDataBuffer(nullptr)
	, m_ParticleTex(_Other.m_ParticleTex)
	, m_AccumTime(0)
	, m_BurstAccumTime(0)
	, m_MaxParticleCount(_Other.m_MaxParticleCount)
	, m_ModuleData(_Other.m_ModuleData)
{
	assert(_Other.m_ParticleBuffer && _Other.m_SpawnCountBuffer && _Other.m_ModuleDataBuffer);

	m_ParticleBuffer = new CStructuredBuffer(*_Other.m_ParticleBuffer);
	m_SpawnCountBuffer = new CStructuredBuffer(*_Other.m_SpawnCountBuffer);
	m_ModuleDataBuffer = new CStructuredBuffer(*_Other.m_ModuleDataBuffer);
}

CParticleSystem::~CParticleSystem()
{
	delete m_ParticleBuffer;
	delete m_SpawnCountBuffer;
	delete m_ModuleDataBuffer;
}

void CParticleSystem::SetMaxParticleCount(UINT _Count)
{
	m_MaxParticleCount = _Count;
	if (m_ParticleBuffer->GetElementCount() < m_MaxParticleCount)
	{
		m_ParticleBuffer->Create(sizeof(tParticle), m_MaxParticleCount, SB_TYPE::SRV_UAV, false, nullptr);
	}
}

void CParticleSystem::CalculateSpawnCount()
{	
	m_AccumTime += DT;
	tSpawnCount totalCount = { };
	// ---------------
	// 处理spawn模块
	// ---------------
	if (m_ModuleData.ModuleOnOff[(UINT)PARTICLE_MODULE::SPAWN])
	{
		float Term = 1.f / (float)m_ModuleData.SpawnRate;
		UINT SpawnCount = 0;
		// 计算本帧需要生成的粒子数量
		if(Term < m_AccumTime)
		{
			float Value = m_AccumTime / Term;
			SpawnCount = (UINT)Value;
			m_AccumTime -= Term * (float)SpawnCount;
		}
		totalCount.SpawnCount = SpawnCount;
	}
	// ---------------
	// 处理spawn burst模块
	// ---------------
	if (m_ModuleData.ModuleOnOff[(UINT)PARTICLE_MODULE::SPAWN_BURST])
	{
		UINT BurstCount = 0;
		if (0.0 == m_BurstAccumTime)
		{
			BurstCount = m_ModuleData.SpawnBurstCount;
		}

		m_BurstAccumTime += DT;
		
		if (m_ModuleData.SpawnBurstRepeat && m_ModuleData.SpawnBurstRepeatTime <= m_BurstAccumTime)
		{
			m_BurstAccumTime -= m_ModuleData.SpawnBurstRepeatTime;
			BurstCount += m_ModuleData.SpawnBurstCount;
		}

		totalCount.SpawnCount += BurstCount;
	}


	// 将需要生成的粒子数量写入结构化缓冲区
	m_SpawnCountBuffer->SetData(&totalCount);

}

void CParticleSystem::finaltick()
{
	CalculateSpawnCount();


	m_ParticleTickCS->SetParticleWorldPos(Transform()->GetWorldPos());
	m_ParticleTickCS->SetModuleDataBuffer(m_ModuleDataBuffer);
	m_ParticleTickCS->SetParticleBuffer(m_ParticleBuffer);
	m_ParticleTickCS->SetSpawnCountBuffer(m_SpawnCountBuffer);

	if (FAILED(m_ParticleTickCS->Execute()))
	{
		assert(nullptr);
	}

}

void CParticleSystem::render()
{
	m_ParticleBuffer->Binding(17);
	
	Transform()->Binding();
	GetMaterial()->SetTexParam(TEX_0, m_ParticleTex);
	GetMaterial()->Binding();
	GetMesh()->render_particle(m_MaxParticleCount);

	m_ParticleBuffer->Clear_SRV();
}

void CParticleSystem::SaveToLevelFile(FILE* _File)
{
	CRenderComponent::SaveToLevelFile(_File);

	// 1. 保存资源引用
	SaveAssetRef<CComputeShader>((CComputeShader*)m_ParticleTickCS.Get(), _File);

	SaveAssetRef(m_ParticleTex, _File);

	// 2. 保存MaxParticleCount
	fwrite(&m_MaxParticleCount, sizeof(UINT), 1, _File);

	// 3. 保存模块数据
	fwrite(&m_ModuleData, sizeof(tParticleModule), 1, _File);
}

void CParticleSystem::LoadFromLevelFile(FILE* _File)
{
	CRenderComponent::LoadFromLevelFile(_File);

	Ptr<CComputeShader> pTickCS = m_ParticleTickCS.Get();
	LoadAssetRef(pTickCS, _File);
	m_ParticleTickCS = (CParticleTickCS*)pTickCS.Get();

	LoadAssetRef(m_ParticleTex, _File);

	fread(&m_MaxParticleCount, sizeof(UINT), 1, _File);
	SetMaxParticleCount(m_MaxParticleCount);

	fread(&m_ModuleData, sizeof(tParticleModule), 1, _File);
}
