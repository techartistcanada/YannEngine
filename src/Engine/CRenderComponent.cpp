#include "pch.h"
#include "CRenderComponent.h"

#include "CBoundingBox.h"
#include "CGameObject.h"

#include "CTransform.h"


CRenderComponent::CRenderComponent(COMPONENT_TYPE _Type)
	: CComponent(_Type)
	, m_bFrustumCheck(true)
	, m_bCastDynamicShadow(true)
{

}

CRenderComponent::CRenderComponent(const CRenderComponent& _Other)
	: CComponent(_Other)
	, m_Mesh(_Other.m_Mesh)
	, m_vecCurMaterials({})
	, m_vecSharedMaterials(_Other.m_vecSharedMaterials)
	, m_vecDynamicMaterials({})
	, m_bCastDynamicShadow(_Other.m_bCastDynamicShadow)
	, m_bFrustumCheck(_Other.m_bFrustumCheck)
{
	m_vecCurMaterials.resize(m_vecSharedMaterials.size());
	m_vecDynamicMaterials.resize(m_vecSharedMaterials.size());

	for (UINT i = 0; i < (UINT)m_vecSharedMaterials.size(); ++i)
	{
		if (nullptr != _Other.m_vecDynamicMaterials[i]
			&& _Other.m_vecDynamicMaterials[i] == _Other.m_vecCurMaterials[i])
		{
			m_vecDynamicMaterials[i] = m_vecSharedMaterials[i]->GetDynamicMaterial();
			m_vecCurMaterials[i] = m_vecDynamicMaterials[i];
		}
		else
		{
			m_vecCurMaterials[i] = m_vecSharedMaterials[i];
		}
	}
}

CRenderComponent::~CRenderComponent()
{
}


void CRenderComponent::SetMesh(Ptr<CMesh> _Mesh)
{
	m_Mesh = _Mesh;

	// Mesh 加载后, 根据SubMesh数量扩展材质槽
	if (nullptr != _Mesh && _Mesh->GetSubMeshCount() > 0)
	{
		UINT slotCount = _Mesh->GetSubMeshCount();
		if ((UINT)m_vecCurMaterials.size() < slotCount)
		{
			m_vecSharedMaterials.resize(slotCount);
			m_vecDynamicMaterials.resize(slotCount);
			m_vecCurMaterials.resize(slotCount);
		}
	}
	else if (m_vecCurMaterials.empty())
	{
		// 无SubMesh信息时至少保留1个槽位
		m_vecSharedMaterials.resize(1);
		m_vecDynamicMaterials.resize(1);
		m_vecCurMaterials.resize(1);
	}

    // 自动配置 BoundingBox (如果已经挂载到 GameObject 且 Mesh 有有效包围数据)
    if (nullptr == GetOwner() || nullptr == _Mesh || _Mesh->GetBoundRadius() <= 0.f)
        return;

    CBoundingBox* pBB = GetOwner()->BoundingBox();
    if (nullptr == pBB)
    {
        pBB = new CBoundingBox;
        GetOwner()->AddComponent(pBB);
    }

    pBB->SetOffsetPos(_Mesh->GetBoundCenter());
    pBB->SetRadius(_Mesh->GetBoundRadius());
	pBB->SetLocalHalfExtents(_Mesh->GetBoundHalfExtents());
}

void CRenderComponent::SetMaterial(Ptr<CMaterial> _Material, UINT _Slot)
{
	if (_Slot >= (UINT)m_vecCurMaterials.size())
	{
		m_vecSharedMaterials.resize(_Slot + 1);
		m_vecDynamicMaterials.resize(_Slot + 1);
		m_vecCurMaterials.resize(_Slot + 1);
	}

	assert(!_Material->IsDynamic() && "SetMaterial: can't set dynamic material as shared material");
	m_vecCurMaterials[_Slot] = m_vecSharedMaterials[_Slot] = _Material;
}

Ptr<CMaterial> CRenderComponent::GetMaterial(UINT _Slot)
{
	if (_Slot >= (UINT)m_vecCurMaterials.size())
		return nullptr;
	return m_vecCurMaterials[_Slot];
}

Ptr<CMaterial> CRenderComponent::GetDynamicMaterial(UINT _Slot)
{
	if (_Slot >= (UINT)m_vecSharedMaterials.size() || nullptr == m_vecSharedMaterials[_Slot])
		return nullptr;
	if (nullptr != m_vecDynamicMaterials[_Slot])
		return m_vecDynamicMaterials[_Slot];

	m_vecCurMaterials[_Slot] = m_vecDynamicMaterials[_Slot] = m_vecSharedMaterials[_Slot]->GetDynamicMaterial();
	return m_vecDynamicMaterials[_Slot];
}

void CRenderComponent::RestoreMaterial()
{
	for (UINT i = 0; i < (UINT)m_vecSharedMaterials.size(); ++i)
	{
		m_vecCurMaterials[i] = m_vecSharedMaterials[i];
		m_vecDynamicMaterials[i] = nullptr;
	}
}


void CRenderComponent::render_shadowmap()
{
	Transform()->Binding();

	Ptr<CMaterial> pShadowMapMaterial = CAssetMgr::GetInst()->FindAsset<CMaterial>(L"ShadowMapMaterial");
	pShadowMapMaterial->Binding();

	UINT subCount = GetMesh()->GetSubMeshCount();
	if (subCount == 0)
	{
		GetMesh()->render();
	}
	else
	{
		for (UINT i = 0; i < subCount; ++i)
		{
			GetMesh()->render_submesh(i);
		}
	}
}

void CRenderComponent::SaveToLevelFile(FILE* _File)
{
	SaveAssetRef(m_Mesh, _File);
	UINT mtrlCount = (UINT)m_vecSharedMaterials.size();
	fwrite(&mtrlCount, sizeof(UINT), 1, _File);
	for (UINT i = 0; i < mtrlCount; ++i)
		SaveAssetRef(m_vecSharedMaterials[i], _File);
}

void CRenderComponent::LoadFromLevelFile(FILE* _File)
{
	LoadAssetRef(m_Mesh, _File);

	UINT mtrlCount = 0;
	fread(&mtrlCount, sizeof(UINT), 1, _File);

	m_vecSharedMaterials.resize(mtrlCount);
	m_vecCurMaterials.resize(mtrlCount);
	m_vecDynamicMaterials.resize(mtrlCount);

	for (UINT i = 0; i < mtrlCount; ++i)
	{
		LoadAssetRef(m_vecSharedMaterials[i], _File);
		m_vecCurMaterials[i] = m_vecSharedMaterials[i];
	}
}
