#include "pch.h"
#include "CSkyBox.h"

#include "CTransform.h"
#include "CIBLManager.h"

#ifdef USE_DX12
#include "DX12/DX12Device.h"
#endif


CSkyBox::CSkyBox()
	: CRenderComponent(COMPONENT_TYPE::SKYBOX)
	, m_Type(SKYBOX_TYPE::SPHERE)
	, m_bVisible(true)
	, m_fExposure(1.f)
	, m_fRoughnessOverride(-1.f)
{
	SetSkyBoxType(m_Type);

	SetFrustumCheck(false);
	SetCastDynamicShadow(false);
}

CSkyBox::~CSkyBox()
{
}

void CSkyBox::SetSkyBoxType(SKYBOX_TYPE _Type)
{
	if (m_Type != _Type)
		m_SkyBoxTexture = nullptr;

    m_Type = _Type;
	if (SKYBOX_TYPE::SPHERE == m_Type)
	{
		SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"SphereMesh"));
	}
	else if(SKYBOX_TYPE::CUBE == m_Type)
	{
		SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"CubeMesh"));
	}

	SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"SkyBoxMaterial"));
}

void CSkyBox::SetSkyBoxTexture(Ptr<CTexture> _Texture)
{
	if (m_SkyBoxTexture == _Texture)
		return;

	m_SkyBoxTexture = _Texture;

	if (m_SkyBoxTexture.Get())
	{
		if (m_Type == SKYBOX_TYPE::SPHERE)
		{
			// 在 SetSkyBoxTexture 中调用 IBL 之前
			// Equirectangular HDR → generate all IBL maps
			CIBLManager::GetInst()->GenerateFromEquirect(m_SkyBoxTexture);
		}
		else if (m_Type == SKYBOX_TYPE::CUBE)
		{
			// Already a cubemap → skip equirect step
			CIBLManager::GetInst()->GenerateFromCubemap(m_SkyBoxTexture);
		}
	}
}

void CSkyBox::finaltick()
{
}

void CSkyBox::render()
{
	if (nullptr == GetMesh().Get() || nullptr == GetMaterial().Get() || !m_bVisible)
	{
		return;
	}

	Transform()->Binding();

	Ptr<CMaterial> pMaterial = GetMaterial();
	pMaterial->SetScalarParam(SCALAR_PARAM::INT_0, (int)m_Type);
	pMaterial->SetScalarParam(SCALAR_PARAM::FLOAT_0, m_fRotationY);
	pMaterial->SetScalarParam(SCALAR_PARAM::FLOAT_1, m_fExposure);
	pMaterial->SetScalarParam(SCALAR_PARAM::FLOAT_2, m_fRoughnessOverride);

	if (SKYBOX_TYPE::SPHERE == m_Type)
		pMaterial->SetTexParam(TEX_PARAM::TEX_0, m_SkyBoxTexture);
	else if(SKYBOX_TYPE::CUBE == m_Type)
		pMaterial->SetTexParam(TEX_PARAM::TEX_CUBE_0, m_SkyBoxTexture);

	pMaterial->Binding();

	GetMesh()->render();
}
