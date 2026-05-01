#include "pch.h"
#include "CMeshRenderer.h"


#include "CTransform.h"
#include "CMaterial.h"
#include "CAnimator2D.h"
#include "CAnim2D.h"

#include "CRenderMgr.h"


CMeshRenderer::CMeshRenderer()
	: CRenderComponent(COMPONENT_TYPE::MESHRENDERER)
{

}

CMeshRenderer::~CMeshRenderer()
{

}

void CMeshRenderer::finaltick()
{
}


void CMeshRenderer::render()
{
	if (nullptr == GetMesh().Get() || nullptr == GetMaterial().Get())
	{
		return;
	}
	Transform()->Binding();

	if (Animator2D())
	{
		Animator2D()->Binding();
	}
	else
	{
		CAnim2D::Clear();
	}

	UINT subCount = GetMesh()->GetSubMeshCount();
	SHADER_DOMAIN eCurDomain = CRenderMgr::GetInst()->GetCurRenderDomain();

	if (subCount == 0)
	{
		Ptr<CMaterial> pMtrl = GetMaterial(0);
		if (nullptr != pMtrl)
		{
			pMtrl->Binding();
			GetMesh()->render();
		}
	}
	else
	{
		for (UINT i = 0; i < subCount; ++i)
		{
			Ptr<CMaterial> pMtrl = GetMaterial(i);
			if (nullptr == pMtrl)
				continue;

			// Skip sub-meshes that don't belong to the current render pass
			if (pMtrl->GetShader()->GetDomain() != eCurDomain)
				continue;

			pMtrl->Binding();
			GetMesh()->render_submesh(i);
		}
	}
}
