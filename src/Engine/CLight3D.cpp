#include "pch.h"
#include "CLight3D.h"

#include "CRenderMgr.h"
#include "CTransform.h"
#include "CCamera.h"
#include "CTexture.h"
#include "CIBLManager.h"

#include "CRenderTargetSet.h"

CLight3D::CLight3D()
	: CComponent(COMPONENT_TYPE::LIGHT3D)
	, m_Info{}
	, m_bDrawGizmo(false)
	, m_bRenderShadow(false)
	, m_ShadowMapMRT(nullptr)
{
	// for rendering shadow map
	m_LightCamObj = new CGameObject;
	m_LightCamObj->AddComponent(new CTransform);
	m_LightCamObj->AddComponent(new CCamera);
	m_LightCamObj->SetName(GetName() + L"_Camera");

}

CLight3D::CLight3D(const CLight3D& _Origin)
	: CComponent(_Origin)
	, m_Info(_Origin.m_Info)
	, m_bDrawGizmo(_Origin.m_bDrawGizmo)
	, m_LightIdx(-1)
	, m_ShadowMapMRT(nullptr)
{
}


CLight3D::~CLight3D()
{
	if(nullptr != m_ShadowMapMRT)
	{
		delete m_ShadowMapMRT;
		m_ShadowMapMRT = nullptr;
	}

	if (nullptr != m_LightCamObj)
	{
		delete m_LightCamObj;
		m_LightCamObj = nullptr;
	}
}

void CLight3D::finaltick()
{
	m_Info.WorldPos = Transform()->GetWorldPos();
	m_Info.WorldDir = Transform()->GetWorldDir(DIR_TYPE::FRONT);

	m_LightIdx = CRenderMgr::GetInst()->RegisterLight3D(this);

	// DEBUG
	if (m_bDrawGizmo)
	{
		if (m_Info.LightType == (UINT)LIGHT_TYPE::POINT)
		{
			DrawDebugSphere(m_Info.WorldPos, m_Info.Range, Vec4(1.f, 1.f, 0.f, 1.f), true, 0.f);
		}
		else
		{
			DrawDebugCube(m_Info.WorldPos, Vec3(50.f, 50.f, 200.f), Transform()->GetRelativeRotation(), Vec4(1.f, 1.f, 0.f, 1.f), false, 0.f);
		}
	}

	// copy light transform to light camera (for shadow mapping)
	*m_LightCamObj->Transform() = *Transform();
	m_LightCamObj->Camera()->finaltick();
}

void CLight3D::RenderShadowMap()
{
	if (LIGHT_TYPE::DIRECTIONAL != (LIGHT_TYPE)m_Info.LightType)
	return;

    m_ShadowMapMRT->ClearTargets();
    m_ShadowMapMRT->ClearDepthStencil();
    m_ShadowMapMRT->OMSet();

    CCamera* pLightCam = m_LightCamObj->Camera();

    Matrix matView = pLightCam->GetViewMat();
    Matrix matProj = pLightCam->GetProjMat();

    // ==============================
    // Texel Snapping (sign-safe)
    // ==============================
    float fHalfRes = (float)SHADOWMAP_RESOLUTION_HIGH * 0.5f;

    Vec4 vOrigin = XMVector4Transform(XMVectorSet(0, 0, 0, 1), matView * matProj);

    // Convert clip-space to texel coordinates, round, convert back
    float snappedX = roundf(vOrigin.x * fHalfRes) / fHalfRes;
    float snappedY = roundf(vOrigin.y * fHalfRes) / fHalfRes;

    matProj._41 -= (vOrigin.x - snappedX);
    matProj._42 -= (vOrigin.y - snappedY);

    m_matLightProjSnapped = matProj;

    g_Trans.matView    = matView;
    g_Trans.matViewInv = pLightCam->GetViewMatInv();
    g_Trans.matProj    = matProj;
    g_Trans.matProjInv = XMMatrixInverse(nullptr, matProj);

    pLightCam->SortObjects_ShadowMap();
    pLightCam->render_shadowmap();
}

void CLight3D::ApplyLighting()
{
	Binding();

	m_VolumeMesh->render();
}

void CLight3D::Binding()
{
	m_DeferredLightingMaterial->SetScalarParam(SCALAR_PARAM::INT_0, m_LightIdx);

    if ((LIGHT_TYPE)m_Info.LightType == LIGHT_TYPE::DIRECTIONAL)
    {
        Ptr<CTexture> pShadowMap = m_ShadowMapMRT->GetRTTexture(0);
        m_DeferredLightingMaterial->SetTexParam(TEX_PARAM::TEX_4, pShadowMap);

        Matrix matVP = m_LightCamObj->Camera()->GetViewMat() * m_matLightProjSnapped;
        m_DeferredLightingMaterial->SetScalarParam(SCALAR_PARAM::MAT_0, matVP);
    }

    if ((LIGHT_TYPE)m_Info.LightType == LIGHT_TYPE::POINT)
    {
        Transform()->Binding();
    }

    // All params set -> now upload constant buffer to GPU
    m_DeferredLightingMaterial->Binding();

	// Re-bind IBL textures AFTER material binding (which clears unused slots)
    CIBLManager::GetInst()->Binding();

	// Re-bind SSAO to t17 AFTER material binding resets the descriptor table
    Ptr<CTexture> pSSAOBlurTex = CAssetMgr::GetInst()->FindAsset<CTexture>(L"SSAOBlurTex");
    if (pSSAOBlurTex != nullptr)
    {
        pSSAOBlurTex->Binding(17);
    }
}

void CLight3D::SetLightType(LIGHT_TYPE _Type)
{
	m_Info.LightType = (UINT)_Type;
	if (LIGHT_TYPE::DIRECTIONAL == (LIGHT_TYPE)m_Info.LightType)
	{
		m_VolumeMesh = CAssetMgr::GetInst()->FindAsset<CMesh>(L"RectMesh");
		m_DeferredLightingMaterial = CAssetMgr::GetInst()->FindAsset<CMaterial>(L"DeferredDirLightingMaterial");

		if (nullptr != m_ShadowMapMRT)
		{
			delete m_ShadowMapMRT;
			m_ShadowMapMRT = nullptr;
		}

		m_ShadowMapMRT = new CRenderTargetSet();
		// shadow map target texture
		Ptr<CTexture> pShadowMap = CAssetMgr::GetInst()->CreateTexture(
			L"ShadowMapTargetTex",
			SHADOWMAP_RESOLUTION_HIGH,
			SHADOWMAP_RESOLUTION_HIGH,
			DXGI_FORMAT_R32_FLOAT,
			RHI_BIND_FLAG::RENDER_TARGET | RHI_BIND_FLAG::SHADER_RESOURCE);

		// depth stencil target texture
		Ptr<CTexture> pDSTex = CAssetMgr::GetInst()->CreateTexture(
			L"ShadowMapDSTex",
			SHADOWMAP_RESOLUTION_HIGH,
			SHADOWMAP_RESOLUTION_HIGH,
			DXGI_FORMAT_D24_UNORM_S8_UINT,
			RHI_BIND_FLAG::DEPTH_STENCIL);

		m_ShadowMapMRT->Init(&pShadowMap, 1, pDSTex);
		m_ShadowMapMRT->SetClearColor(0, Vec4(1.f, 1.f, 1.f, 1.f)); // clear to max depth!
		m_LightCamObj->Camera()->SetProjType(PROJ_TYPE::ORTHOGRAPHIC);
		m_LightCamObj->Camera()->SetWidth(SHADOWMAP_RESOLUTION_HIGH);
		m_LightCamObj->Camera()->SetAspectRatio(1.f);
		m_LightCamObj->Camera()->SetFar(5000.f);
		m_LightCamObj->Camera()->LayerCheckAll();
	}
	else if (LIGHT_TYPE::POINT == (LIGHT_TYPE)m_Info.LightType)
	{
		m_VolumeMesh = CAssetMgr::GetInst()->FindAsset<CMesh>(L"SphereMesh");
		m_DeferredLightingMaterial = CAssetMgr::GetInst()->FindAsset<CMaterial>(L"DeferredPointLightingMaterial");
	}
	else if (LIGHT_TYPE::SPOT == (LIGHT_TYPE)m_Info.LightType)
	{
		m_VolumeMesh = CAssetMgr::GetInst()->FindAsset<CMesh>(L"ConeMesh");
		// TODO: spot light
	}
}

void CLight3D::SetRange(float _Range)
{
    m_Info.Range = _Range;
	Transform()->SetRelativeScale(m_Info.Range * 2.f, m_Info.Range * 2.f, m_Info.Range * 2.f);
}

void CLight3D::SaveToLevelFile(FILE* _File)
{
	fwrite(&m_Info, sizeof(tLightInfo), 1, _File);
}

void CLight3D::LoadFromLevelFile(FILE* _File)
{
	fread(&m_Info, sizeof(tLightInfo), 1, _File);
}
