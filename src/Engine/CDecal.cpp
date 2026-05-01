#include "pch.h"
#include "CDecal.h"

#include "CTransform.h"

CDecal::CDecal()
	: CRenderComponent(COMPONENT_TYPE::DECAL)
	, m_bAsEmissive(false)
	, m_EmissiveIntensity(1.f)
{
	SetCastDynamicShadow(false);

	SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"CubeMesh"));
	SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"DeferredDecalMaterial"));
	GetMaterial()->SetTexParam(TEX_PARAM::TEX_0, CAssetMgr::GetInst()->FindAsset<CTexture>(L"GBuffer_Position"));
}

CDecal::~CDecal()
{
}

void CDecal::finaltick()
{
	DrawDebugCube(Transform()->GetWorldMat(), Vec4(0.f, 1.0f, 0.f, 1.0f), true, 0.f);
}

void CDecal::render()
{
	Transform()->Binding();

	if(nullptr != m_DecalTexture)
		GetMaterial()->SetTexParam(TEX_PARAM::TEX_1, m_DecalTexture);

	GetMaterial()->SetScalarParam(FLOAT_0, m_EmissiveIntensity);
	GetMaterial()->SetScalarParam(INT_0, (int)m_bAsEmissive);

	GetMaterial()->Binding();

	GetMesh()->render();
}

