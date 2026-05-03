#include "pch.h"

#include "CEditorMgr.h"

#include "CEditorObject.h"
#include <components.h>
#include <CRenderMgr.h>

#include "CEditorCameraScript.h"


CEditorMgr::CEditorMgr()
{
}

CEditorMgr::~CEditorMgr()
{
	Safe_Del_Vector(m_vecEditorObjects);
}

void CEditorMgr::init()
{
	CEditorObject* pEditorCam = new CEditorObject;
	pEditorCam->SetName(L"EditorCamera");
	pEditorCam->AddComponent(new CTransform);
	pEditorCam->AddComponent(new CCamera);
	pEditorCam->AddComponent(new CEditorCameraScript);

	pEditorCam->Camera()->LayerCheckAll();
	pEditorCam->Camera()->SetProjType(PROJ_TYPE::PERSPECTIVE);
	pEditorCam->Transform()->SetRelativePos(Vec3(-1037.f, 1557.f, -1520.f));
	pEditorCam->Transform()->SetRelativeRotation(Vec3(ToRadians(28.f), ToRadians(38.f), ToRadians(0.f)));
	CRenderMgr::GetInst()->RegisterEditorCamera(pEditorCam->Camera());
	
	m_vecEditorObjects.push_back(pEditorCam);
}

void CEditorMgr::tick()
{
	for (size_t i = 0; i < m_vecEditorObjects.size(); ++i)
	{
		m_vecEditorObjects[i]->tick();
	}

	for (size_t i = 0; i < m_vecEditorObjects.size(); ++i)
	{
		m_vecEditorObjects[i]->finaltick();
	}
}
