#include "pch.h"
#include "CParticleTickCS.h"
#include "CStructuredBuffer.h"

#include "CAssetMgr.h"

CParticleTickCS::CParticleTickCS()
	: CComputeShader(32, 1, 1)
	, m_ParticleBuffer(nullptr)
	, m_SpawnCountBuffer(nullptr)
{
	//m_NoiseTexture = CAssetMgr::GetInst()->Load<CTexture>(L"texture\\noise\\noise_03.jpg", L"texture\\noise\\noise_03.jpg");
}

CParticleTickCS::~CParticleTickCS()
{

}

int CParticleTickCS::Binding()
{
	if (nullptr == m_ParticleBuffer || nullptr == m_SpawnCountBuffer || nullptr == m_NoiseTexture)
	{
		return E_FAIL;
	}

	m_ParticleBuffer->Binding_CS_UAV(0);
	m_SpawnCountBuffer->Binding_CS_UAV(1);
	m_NoiseTexture->Binding_CS_SRV(20);
	m_ModuleDataBuffer->Binding_CS_SRV(21);

	m_MaterialConst.iArr[0] = m_ParticleBuffer->GetElementCount();
	m_MaterialConst.v4Arr[0] = m_ParticleWorldPos;

	return S_OK;
}

void CParticleTickCS::CalculateNumGroups()
{
	m_NumGroupX = m_ParticleBuffer->GetElementCount() / m_NumThreadPerGroupX;
	m_ParticleBuffer->GetElementCount() % m_NumThreadPerGroupX ? m_NumGroupX += 1 : m_NumGroupX;
	
	m_NumGroupY = 1;
	m_NumGroupZ = 1;
}

void CParticleTickCS::Clear()
{
	m_ParticleBuffer->Clear_UAV();
	m_ParticleBuffer = nullptr;
	
	m_SpawnCountBuffer->Clear_UAV();
	m_SpawnCountBuffer = nullptr;
}

