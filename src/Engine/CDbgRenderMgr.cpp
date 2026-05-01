#include "pch.h"
#include "CDbgRenderMgr.h"

#include "CTimeMgr.h"
#include "CAssetMgr.h"
#include "CMesh.h"
#include "CMaterial.h"

#include "CGameObject.h"
#include "CTransform.h"
#include "CMeshRenderer.h"

CDbgRenderMgr::CDbgRenderMgr()
{
	m_DebugRenderObj = new CGameObject;
	m_DebugRenderObj->AddComponent(new CTransform);
	m_DebugRenderObj->AddComponent(new CMeshRenderer);
	m_DebugRenderObj->MeshRenderer()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"DebugShapeMaterial"));
}

CDbgRenderMgr::~CDbgRenderMgr()
{
	delete m_DebugRenderObj;
	m_DebugRenderObj = nullptr;
}

void CDbgRenderMgr::render()
{
	list<tDebugShapeInfo>::iterator	iter = m_ShapeInfos.begin();
	for (; iter != m_ShapeInfos.end(); )
	{
		// 更新世界矩阵 update world matrix
		if (iter->matWorld == XMMatrixIdentity())
		{
			m_DebugRenderObj->Transform()->SetRelativePos(iter->Position);
			m_DebugRenderObj->Transform()->SetRelativeScale(iter->Scale);
			m_DebugRenderObj->Transform()->SetRelativeRotation(iter->Rotation);
			// 更新变换矩阵
			m_DebugRenderObj->Transform()->finaltick();
		}
		else
		{
			m_DebugRenderObj->Transform()->SetWorldMat(iter->matWorld);
		}
		// 设置网格 set mesh
		switch (iter->Shape)
		{
			case DEBUG_SHAPE::RECT:
				m_DebugRenderObj->MeshRenderer()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"RectMesh_Debug"));
				m_DebugRenderObj->MeshRenderer()->GetMaterial()->GetShader()->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
				m_DebugRenderObj->MeshRenderer()->GetMaterial()->GetShader()->SetRSType(RS_TYPE::CULL_NONE);
				break;
			case DEBUG_SHAPE::CIRCLE:
				m_DebugRenderObj->MeshRenderer()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"CircleMesh_Debug"));
				m_DebugRenderObj->MeshRenderer()->GetMaterial()->GetShader()->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
				m_DebugRenderObj->MeshRenderer()->GetMaterial()->GetShader()->SetRSType(RS_TYPE::CULL_NONE);
				break;
			case DEBUG_SHAPE::LINE:
				m_DebugRenderObj->MeshRenderer()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"LineMesh"));
				m_DebugRenderObj->MeshRenderer()->GetMaterial()->GetShader()->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
				m_DebugRenderObj->MeshRenderer()->GetMaterial()->GetShader()->SetRSType(RS_TYPE::CULL_BACK);
				break;
			case DEBUG_SHAPE::CUBE:
				m_DebugRenderObj->MeshRenderer()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"CubeMesh_Debug"));
				m_DebugRenderObj->MeshRenderer()->GetMaterial()->GetShader()->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
				m_DebugRenderObj->MeshRenderer()->GetMaterial()->GetShader()->SetRSType(RS_TYPE::CULL_NONE);
				break;
			case DEBUG_SHAPE::SPHERE:
				m_DebugRenderObj->MeshRenderer()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"SphereMesh"));
				m_DebugRenderObj->MeshRenderer()->GetMaterial()->GetShader()->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				m_DebugRenderObj->MeshRenderer()->GetMaterial()->GetShader()->SetRSType(RS_TYPE::CULL_FRONT);
				break;
			default:
				break;
		}

		// 设置颜色信息
		m_DebugRenderObj->MeshRenderer()->GetMaterial()->SetScalarParam(INT_0, iter->Shape);
		m_DebugRenderObj->MeshRenderer()->GetMaterial()->SetScalarParam(VEC4_0, iter->Color);


		// depth test
		if (iter->DepthTest)
		{
			m_DebugRenderObj->MeshRenderer()->GetMaterial()->GetShader()->SetDSType(DS_TYPE::NO_WRITE);
		}
		else
		{
			m_DebugRenderObj->MeshRenderer()->GetMaterial()->GetShader()->SetDSType(DS_TYPE::NO_TEST_NO_WRITE);
		}
		// 渲染
		m_DebugRenderObj->render();
		

		iter->Age += DT_EDITOR;
		if (iter->Duration <= iter->Age)
		{
			iter = m_ShapeInfos.erase(iter);
		}
		else
		{
			++iter;
		}
	}


}
