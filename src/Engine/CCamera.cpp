#include "pch.h"
#include "CCamera.h"

#include "CLevelMgr.h"
#include "CLevel.h"
#include "CLayer.h"
#include "CGameObject.h"
#include "CRenderComponent.h"

#include "CRenderMgr.h"
#include "CTransform.h"
#include "components.h"

#ifdef USE_DX11
#include "DX11/DX11Device.h"
#else
#include "DX12/DX12Device.h"
#endif

CCamera::CCamera()
	: CComponent(COMPONENT_TYPE::CAMERA)
	, m_Frustum(this)
	, m_ProjType(PROJ_TYPE::PERSPECTIVE)
	, m_CamPriority(-1)
	, m_FOV((XM_PI / 3.f))
	, m_Far(10000.0f)
	, m_Scale(1.f)
	, m_LayerCheck(0)
{
	Vec2 vRenderResolution = RHI_DEVICE->GetRenderResolution();
	m_Width = vRenderResolution.x;
	m_AspectRatio = vRenderResolution.x / vRenderResolution.y;
}

CCamera::CCamera(const CCamera& _Other)
	: CComponent(_Other)
	, m_Frustum(_Other.m_Frustum)
	, m_ProjType(_Other.m_ProjType)
	, m_CamPriority(-1)
	, m_FOV(_Other.m_FOV)
	, m_Far(_Other.m_Far)
	, m_Width(_Other.m_Width)
	, m_AspectRatio(_Other.m_AspectRatio)
	, m_Scale(_Other.m_Scale)
	, m_LayerCheck(_Other.m_LayerCheck)
{
	m_Frustum.SetOwner(this);
}

CCamera::~CCamera()
{
}

void CCamera::begin()
{
	CRenderMgr::GetInst()->RegisterCamera(this, m_CamPriority);
}

void CCamera::finaltick()
{
	// ====================
	// view matrix 视图矩阵
	// ====================
	Vec3 vCamWorldPos =	GetOwner()->Transform()->GetWorldPos();
	Matrix matViewTrans = XMMatrixTranslation(-vCamWorldPos.x, -vCamWorldPos.y, -vCamWorldPos.z);

	// transform
	Vec3 vR = Transform()->GetWorldDir(DIR_TYPE::RIGHT);
	Vec3 vU = Transform()->GetWorldDir(DIR_TYPE::UP);
	Vec3 vF = Transform()->GetWorldDir(DIR_TYPE::FRONT);
	Matrix matViewRot = XMMatrixIdentity();
	matViewRot._11 = vR.x; matViewRot._12 = vU.x; matViewRot._13 = vF.x;
	matViewRot._21 = vR.y; matViewRot._22 = vU.y; matViewRot._23 = vF.y;
	matViewRot._31 = vR.z; matViewRot._32 = vU.z; matViewRot._33 = vF.z;

	m_matView = matViewTrans * matViewRot;
	// calculate inverse matrix
	m_matViewInv = XMMatrixInverse(nullptr, m_matView);

	// ====================
	// prjection matrix 投影矩阵
	// ====================
	if (PROJ_TYPE::PERSPECTIVE == m_ProjType)
	{
		m_matProj = XMMatrixPerspectiveFovLH(m_FOV, m_AspectRatio, 1.0f, m_Far);
	}
	else
	{
		m_matProj = XMMatrixOrthographicLH(m_Width * m_Scale, (m_Width / m_AspectRatio) * m_Scale, 1.f, m_Far);
	}
	// calculate inverse matrix
	m_matProjInv = XMMatrixInverse(nullptr, m_matProj);

	// calculate frustum
	m_Frustum.finaltick();
}

void CCamera::render()
{
}

void CCamera::SetCameraPriority(int _Priority)
{
	m_CamPriority = _Priority;
	
	//CRenderMgr::GetInst()->RegisterCamera(this, m_CamPriority);
}

void CCamera::LayerCheckOn(int _LayerIdx)
{
	// NOTE: 这里要做的是图层的开关
	if (m_LayerCheck & (1 << _LayerIdx))
	{
		m_LayerCheck &= ~(1 << _LayerIdx);
	}
	else
	{
		m_LayerCheck |= (1 << _LayerIdx);
	}
}

void CCamera::SortObjects()
{
	SortClear();

	CLevel* pCurLevel = CLevelMgr::GetInst()->GetCurrentLevel();

	for (UINT i = 0; i < MAX_LAYER; ++i)
	{
		if (m_LayerCheck & (1 << i))
		{
			CLayer* pLayer = pCurLevel->GetLayer(i);
			const vector<CGameObject*>& vecObjects = pLayer->GetObjects();
			for (size_t j = 0; j < vecObjects.size(); ++j)
			{
				if (nullptr == vecObjects[j]->GetRenderComponent() ||
					nullptr == vecObjects[j]->GetRenderComponent()->GetMaterial() ||
					nullptr == vecObjects[j]->GetRenderComponent()->GetMaterial()->GetShader())
					continue;
				
				// =====================
				// frustum culling
				// =====================
				if (vecObjects[j]->GetRenderComponent()->IsFrustumCheck())
				{
					if (vecObjects[j]->BoundingBox())
					{
						Vec3 vAABBMin = vecObjects[j]->BoundingBox()->GetWorldAABBMin();
						Vec3 vAABBMax = vecObjects[j]->BoundingBox()->GetWorldAABBMax();

						if (false == m_Frustum.IsAABBInFrustum(vAABBMin, vAABBMax))
						{
							continue;
						}
					}
					else
					{
						Vec3 vWorldPos = vecObjects[j]->Transform()->GetWorldPos();
						Vec3 vWorldScale = vecObjects[j]->Transform()->GetWorldScale();

						float Radius = vWorldScale.x;
						if (Radius < vWorldScale.y) Radius = vWorldScale.y;
						if (Radius < vWorldScale.z) Radius = vWorldScale.z;

						if (false == m_Frustum.IsSphereInFrustum(vWorldPos, Radius))
						{
							continue;
						}
					}
				}

				// =====================
				// Collect ALL domains this object has across sub-meshes
				// =====================
				UINT uDomainFlags = 0;
				CMesh* pMesh = vecObjects[j]->GetRenderComponent()->GetMesh().Get();
				UINT subCount = (pMesh != nullptr) ? pMesh->GetSubMeshCount() : 0;

				if (subCount > 0)
				{
					for (UINT s = 0; s < subCount; ++s)
					{
						Ptr<CMaterial> pMtrl = vecObjects[j]->GetRenderComponent()->GetMaterial(s);
						if (nullptr == pMtrl || nullptr == pMtrl->GetShader())
							continue;
						uDomainFlags |= (1 << (UINT)pMtrl->GetShader()->GetDomain());
					}
				}
				else
				{
					SHADER_DOMAIN domain = vecObjects[j]->GetRenderComponent()->GetMaterial()->GetShader()->GetDomain();
					uDomainFlags = (1 << (UINT)domain);
				}

				// Sort into every matching list
				if (uDomainFlags & (1 << (UINT)SHADER_DOMAIN::DOMAIN_DEFFERED))
					m_vecDeferredObjects.push_back(vecObjects[j]);
				if (uDomainFlags & (1 << (UINT)SHADER_DOMAIN::DOMAIN_DEFERRED_DECAL))
					m_vecDeferredDecalObjects.push_back(vecObjects[j]);
				if (uDomainFlags & (1 << (UINT)SHADER_DOMAIN::DOMAIN_OPAQUE))
					m_vecOpaqueObjects.push_back(vecObjects[j]);
				if (uDomainFlags & (1 << (UINT)SHADER_DOMAIN::DOMAIN_MASKED))
					m_vecMaskedObjects.push_back(vecObjects[j]);
				if (uDomainFlags & (1 << (UINT)SHADER_DOMAIN::DOMAIN_TRANSPARENT))
					m_vecTransparentObjects.push_back(vecObjects[j]);
				if (uDomainFlags & (1 << (UINT)SHADER_DOMAIN::DOMAIN_PARTICLE))
					m_vecParticleObjects.push_back(vecObjects[j]);
				if (uDomainFlags & (1 << (UINT)SHADER_DOMAIN::DOMAIN_POSTPROCESS))
					m_vecPostProcessObjects.push_back(vecObjects[j]);
			}
		}
	}
}

void CCamera::SortObjects_ShadowMap()
{
	m_vecShadowMapObjects.clear();

	CLevel* pCurLevel = CLevelMgr::GetInst()->GetCurrentLevel();

	for (UINT i = 0; i < MAX_LAYER; ++i)
	{
		if (m_LayerCheck & (1 << i))
		{
			CLayer* pLayer = pCurLevel->GetLayer(i);
			const vector<CGameObject*>& vecObjects = pLayer->GetObjects();
			for (size_t j = 0; j < vecObjects.size(); ++j)
			{
				if (nullptr == vecObjects[j]->GetRenderComponent() ||
					nullptr == vecObjects[j]->GetRenderComponent()->GetMaterial() ||
					nullptr == vecObjects[j]->GetRenderComponent()->GetMaterial()->GetShader() ||
					false == vecObjects[j]->GetRenderComponent()->IsCastDynamicShadow())
					continue;
				
				// =====================
				// frustum culling
				// =====================
				// TODO: proper shadow map culling (currently using camera frustum, which is not accurate)
				/*
				if (vecObjects[j]->GetRenderComponent()->IsFrustumCheck())
				{
					if (vecObjects[j]->BoundingBox())
					{
						Vec3 vAABBMin = vecObjects[j]->BoundingBox()->GetWorldAABBMin();
						Vec3 vAABBMax = vecObjects[j]->BoundingBox()->GetWorldAABBMax();

						if (false == m_Frustum.IsAABBInFrustum(vAABBMin, vAABBMax))
						{
							continue;
						}
					}
					else
					{
						Vec3 vWorldPos = vecObjects[j]->Transform()->GetWorldPos();
						Vec3 vWorldScale = vecObjects[j]->Transform()->GetWorldScale();

						float Radius = vWorldScale.x;
						if (Radius < vWorldScale.y) Radius = vWorldScale.y;
						if (Radius < vWorldScale.z) Radius = vWorldScale.z;

						if (false == m_Frustum.IsSphereInFrustum(vWorldPos, Radius))
						{
							continue;
						}
					}
				}
				*/
				m_vecShadowMapObjects.push_back(vecObjects[j]);
			}
		}
	}
}


void CCamera::SortClear()
{
	m_vecDeferredObjects.clear();
	m_vecDeferredDecalObjects.clear();

	m_vecOpaqueObjects.clear();
	m_vecMaskedObjects.clear();
	m_vecTransparentObjects.clear();
	m_vecParticleObjects.clear();
	m_vecPostProcessObjects.clear();
}

void CCamera::render_deferred()
{
	CRenderMgr::GetInst()->SetCurRenderDomain(SHADER_DOMAIN::DOMAIN_DEFFERED);
	for (size_t i = 0; i < m_vecDeferredObjects.size(); ++i)
	{
		m_vecDeferredObjects[i]->render();
		CRenderMgr::GetInst()->IncrDrawCallCount();
	}

}

void CCamera::render_deferred_decal()
{
	CRenderMgr::GetInst()->SetCurRenderDomain(SHADER_DOMAIN::DOMAIN_DEFERRED_DECAL);
	for (size_t i = 0; i < m_vecDeferredDecalObjects.size(); ++i)
	{
		m_vecDeferredDecalObjects[i]->render();
		CRenderMgr::GetInst()->IncrDrawCallCount();
	}

}

void CCamera::render_opaque()
{
	CRenderMgr::GetInst()->SetCurRenderDomain(SHADER_DOMAIN::DOMAIN_OPAQUE);
	for (size_t i = 0; i < m_vecOpaqueObjects.size(); ++i)
	{
		m_vecOpaqueObjects[i]->render();
		CRenderMgr::GetInst()->IncrDrawCallCount();
	}
}


void CCamera::render_masked()
{
	CRenderMgr::GetInst()->SetCurRenderDomain(SHADER_DOMAIN::DOMAIN_MASKED);
	for (size_t i = 0; i < m_vecMaskedObjects.size(); ++i)
	{
		m_vecMaskedObjects[i]->render();
		CRenderMgr::GetInst()->IncrDrawCallCount();
	}

}


void CCamera::render_transparent()
{
	CRenderMgr::GetInst()->SetCurRenderDomain(SHADER_DOMAIN::DOMAIN_TRANSPARENT);
	for (size_t i = 0; i < m_vecTransparentObjects.size(); ++i)
	{
		m_vecTransparentObjects[i]->render();
		CRenderMgr::GetInst()->IncrDrawCallCount();
	}

}

void CCamera::render_particle()
{
	CRenderMgr::GetInst()->SetCurRenderDomain(SHADER_DOMAIN::DOMAIN_PARTICLE);
	for (size_t i = 0; i < m_vecParticleObjects.size(); ++i)
	{
		m_vecParticleObjects[i]->render();
		CRenderMgr::GetInst()->IncrDrawCallCount();
	}

}

void CCamera::render_postprocess()
{
	CRenderMgr::GetInst()->SetCurRenderDomain(SHADER_DOMAIN::DOMAIN_POSTPROCESS);
	CRenderMgr::GetInst()->CopyRenderTarget();

	for (size_t i = 0; i < m_vecPostProcessObjects.size(); ++i)
	{
		m_vecPostProcessObjects[i]->render();
		CRenderMgr::GetInst()->IncrDrawCallCount();
	}

}

void CCamera::render_shadowmap()
{
	for (size_t i = 0; i < m_vecShadowMapObjects.size(); ++i)
	{
		m_vecShadowMapObjects[i]->GetRenderComponent()->render_shadowmap();
		CRenderMgr::GetInst()->IncrDrawCallCount();
	}
}

void CCamera::SaveToLevelFile(FILE* _File)
{

	fwrite(&m_ProjType, sizeof(PROJ_TYPE), 1, _File);
	fwrite(&m_CamPriority, sizeof(int), 1, _File);
	fwrite(&m_FOV, sizeof(float), 1, _File);
	fwrite(&m_Far, sizeof(float), 1, _File);
	fwrite(&m_Width, sizeof(float), 1, _File);
	fwrite(&m_AspectRatio, sizeof(float), 1, _File);
	fwrite(&m_Scale, sizeof(float), 1, _File);
	fwrite(&m_LayerCheck, sizeof(UINT), 1, _File);
}

void CCamera::LoadFromLevelFile(FILE* _File)
{
	fread(&m_ProjType, sizeof(PROJ_TYPE), 1, _File);
	fread(&m_CamPriority, sizeof(int), 1, _File);
	fread(&m_FOV, sizeof(float), 1, _File);
	fread(&m_Far, sizeof(float), 1, _File);
	fread(&m_Width, sizeof(float), 1, _File);
	fread(&m_AspectRatio, sizeof(float), 1, _File);
	fread(&m_Scale, sizeof(float), 1, _File);
	fread(&m_LayerCheck, sizeof(UINT), 1, _File);

	//SetCameraPriority(m_CamPriority);
}
