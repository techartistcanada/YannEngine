#include "pch.h"
#include "CTestLevelDX12.h"

#include <CLevel.h>
#include <CLayer.h>
#include <CCollisionManager.h>
#include <CGameObject.h>
#include <components.h>

#include <CPlayerScript.h>
#include <CMonsterScript.h>
#include <CCameraMoveScript.h>


#include <CSetColorShader.h>

#include <CPrefab.h>


void CTestLevelDX12::CreateTestLevel()
{

	//CAssetMgr::GetInst()->Load<CMaterial>(L"material\\Default Material 0.mat", L"material\\Default Material 0.mat");

	CLevel* pLevel = nullptr;
	pLevel = new CLevel;
	pLevel->GetLayer(0)->SetName(L"Default");
	pLevel->GetLayer(1)->SetName(L"Player");
	pLevel->GetLayer(2)->SetName(L"Monster");

	// =====================
	// Light
	// =====================
	CGameObject* pLight = new CGameObject;
	pLight->SetName(L"Dir Light");
	pLight->AddComponent(new CTransform);
	pLight->AddComponent(new CLight3D);

	pLight->Light3D()->SetLightType(LIGHT_TYPE::DIRECTIONAL);
	pLight->Light3D()->SetDiffuse(Vec3(1.f, 1.f, 1.f));
	pLight->Light3D()->SetSpecular(Vec3(.3f, .3f, .3f));
	pLight->Light3D()->SetAmbient(Vec3(.15f, .15f, .15f));
	//pLight->Light3D()->SetRange(400.f);

	pLight->Transform()->SetRelativePos(Vec3(200.f, 50.f, 500.f));
	pLight->Transform()->SetRelativeRotation(Vec3(XM_PI / 4.f, XM_PI/4.f, 0.f));

	pLevel->AddObject(0, pLight);

	// =====================
	// Player
	// =====================
	CGameObject* pPlayerCube = new CGameObject;
	pPlayerCube->SetName(L"Player");

	pPlayerCube->AddComponent(new CTransform);
	pPlayerCube->AddComponent(new CMeshRenderer);
	pPlayerCube->AddComponent(new CBoundingBox);

	pPlayerCube->Transform()->SetRelativePos(Vec3(0.f, 0.f, 500.f));
	pPlayerCube->Transform()->SetRelativeScale(Vec3(1000.f, 1000.f, 1000.f));
	//pPlayerCube->Transform()->SetRelativeRotation(Vec3(XM_PI / 2.f, 0.f, 0.f));
	
	pPlayerCube->MeshRenderer()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"SphereMesh"));
	pPlayerCube->MeshRenderer()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"Std3DDeferredMaterial"));
	pPlayerCube->MeshRenderer()->GetMaterial()->SetTexParam(TEX_PARAM::TEX_0, CAssetMgr::GetInst()->FindAsset<CTexture>(L"texture\\Background.jpg"));

	pPlayerCube->BoundingBox()->SetAbsolute(true);
	pPlayerCube->BoundingBox()->SetOffsetPos(Vec3(0.f, 0.f, 0.f));
	pPlayerCube->BoundingBox()->SetRadius(600.f);


	pLevel->AddObject(0, pPlayerCube);

	
	ChangeLevel(pLevel, LEVEL_STATE::STOP);
}

void CTestLevelDX12::CreatePrefab()
{
}

