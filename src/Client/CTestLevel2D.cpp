#include "pch.h"

#include "CTestLevel2D.h"

#include <CLevel.h>
#include <CLayer.h>
#include <CCollisionManager.h>
#include <CGameObject.h>
#include <components.h>

#include <CPlayerScript.h>
#include <CMonsterScript.h>
#include <CCameraMoveScript.h>


#include <CAnim2D.h>
#include <CSetColorShader.h>

#include <CPrefab.h>


void CTestLevel2D::CreateTestLevel()
{

	//CAssetMgr::GetInst()->Load<CMaterial>(L"material\\Default Material 0.mat", L"material\\Default Material 0.mat");

	CLevel* pLevel = nullptr;
	pLevel = new CLevel;
	pLevel->GetLayer(0)->SetName(L"Default");
	pLevel->GetLayer(1)->SetName(L"Player");
	pLevel->GetLayer(2)->SetName(L"Monster");

	//ChangeLevel(pLevel, LEVEL_STATE::STOP);
	//return;



	// ==========
	// Particle System
	// ==========
	CGameObject* pParticleObj = new CGameObject;
	pParticleObj->SetName(L"ParticleSystem");
	pParticleObj->AddComponent(new CTransform);
	pParticleObj->AddComponent(new CParticleSystem);

	pParticleObj->ParticleSystem()->SetParticleTexture(CAssetMgr::GetInst()->Load<CTexture>(L"texture\\particle\\AlphaCircle.png", L"texture\\particle\\AlphaCircle.png"));
	pParticleObj->Transform()->SetRelativePos(0.f, 0.f, 100.f);
	pLevel->AddObject(0, pParticleObj, false);

	// Prefab
	//Ptr<CPrefab> pParticlePrefab = new CPrefab(pParticleObj);
	//CAssetMgr::GetInst()->AddAsset<CPrefab>(L"ParticlePrefab", pParticlePrefab);
	
	// ==========
	// Camera 摄像机
	// ==========
	CGameObject* pCamObj = new CGameObject;
	pCamObj->SetName(L"MainCamera");
	pCamObj->AddComponent(new CTransform);
	pCamObj->AddComponent(new CCamera);
	pCamObj->AddComponent(new CCameraMoveScript);

	pCamObj->Camera()->LayerCheckAll();
	pCamObj->Camera()->SetCameraPriority(0);
	pCamObj->Camera()->SetProjType(PROJ_TYPE::ORTHOGRAPHIC);

	pLevel->AddObject(0, pCamObj);
	// ==========
	// Lights
	// ==========
	CGameObject* pLightObj = new CGameObject;
	pLightObj->SetName(L"PointLight1");
	pLightObj->AddComponent(new CTransform);
	pLightObj->AddComponent(new CLight2D);
	pLightObj->Transform()->SetRelativePos(300.f, 0.f, 0.f);
	pLightObj->Light2D()->SetLightType(LIGHT_TYPE::DIRECTIONAL);
	pLightObj->Light2D()->SetDiffuse(Vec3(1.f, 1.f, 1.f));
	pLightObj->Light2D()->SetAmbient(Vec3(0.2f, 0.2f, 0.2f));
	pLightObj->Light2D()->SetRange(300.f);
	
	pLevel->AddObject(0, pLightObj);



	// ==========
	// Player 玩家
	// ==========
	CGameObject* pPlayer = new CGameObject;
	pPlayer->SetName(L"Player");
	pPlayer->AddComponent(new CTransform);
	pPlayer->AddComponent(new CMeshRenderer);
	pPlayer->AddComponent(new CCollider2D);
	pPlayer->AddComponent(new CAnimator2D);
	pPlayer->AddComponent(new CPlayerScript);
	// Add light
	pPlayer->AddComponent(new CLight2D);
	pPlayer->Transform()->SetRelativePos(300.f, 0.f, 0.f);
	pPlayer->Light2D()->SetLightType(LIGHT_TYPE::POINT);
	pPlayer->Light2D()->SetDiffuse(Vec3(0.7f, 0.04f, 0.04f));
	pPlayer->Light2D()->SetAmbient(Vec3(0.2f, 0.2f, 0.2f));
	pPlayer->Light2D()->SetRange(100.f);

	pPlayer->Transform()->SetRelativePos(0.0f, 0.0f, 100.0f);
	pPlayer->Transform()->SetRelativeScale(200.0f, 200.0f, 0.5f);

	pPlayer->MeshRenderer()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"RectMesh"));
	pPlayer->MeshRenderer()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"Std2DMaterial"));
	pPlayer->MeshRenderer()->GetMaterial()->SetScalarParam<int>(INT_0, 0);
	pPlayer->MeshRenderer()->GetMaterial()->SetTexParam(TEX_0, CAssetMgr::GetInst()->FindAsset<CTexture>(L"texture\\Character.png"));

	pPlayer->Collider2D()->SetAbsolute(false);
	pPlayer->Collider2D()->SetOffset(Vec3(0.f, 0.f, 0.f));
	pPlayer->Collider2D()->SetScale(Vec3(1.f, 1.f, 1.f));


	Ptr<CTexture> pAtlas = CAssetMgr::GetInst()->Load<CTexture>(L"texture\\link.png", L"texture\\link.png");
	//pPlayer->Animator2D()->CreateAnimation(L"MOVE_DOWN", pAtlas, Vec2(0.f, 520.f), Vec2(120.f, 130.f), Vec2(240.f, 260.f), 10, 12);
	//pPlayer->Animator2D()->CreateAnimation(L"IDLE_RIGHT", pAtlas, Vec2(0.f, 390.f), Vec2(120.f, 130.f), Vec2(240.f, 260.f), 3, 2);

	//pPlayer->Animator2D()->FindAnimation(L"MOVE_DOWN")->Save(L"Animation\\");
	pPlayer->Animator2D()->LoadAnimation(L"Animation\\MOVE_DOWN.anim");

	pPlayer->Animator2D()->Play(L"MOVE_DOWN", true);

	pLevel->AddObject(1, pPlayer, false);
	//CGameObject* pPlayerClone = pPlayer->Clone();
	//pPlayer->Transform()->SetRelativePos(100.f, 0.f, 100.f);
	//m_CurLevel->AddObject(1, pPlayerClone, false);
	// ==========
	// Monster 
	// ==========
	CGameObject* pMonster = new CGameObject;
	pMonster->SetName(L"Monster");
	pMonster->AddComponent(new CTransform);
	pMonster->AddComponent(new CMeshRenderer);
	//pMonster->AddComponent(new CCollider2D);
	//pMonster->AddComponent(new CMonsterScript);

	// BUG: TODO 当为child时,将单位改成像素而非相对比例
	pMonster->Transform()->SetRelativePos(100.0f, 0.0f, 100.0f);
	pMonster->Transform()->SetRelativeScale(200.f, 200.f, 0.f);
	//pMonster->Transform()->SetAbsolute(false);

	pMonster->MeshRenderer()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"RectMesh"));
	pMonster->MeshRenderer()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"Std2DMaterial"));
	pMonster->MeshRenderer()->GetMaterial()->SetScalarParam<int>(INT_0, 0);
	pMonster->MeshRenderer()->GetMaterial()->SetTexParam(TEX_0, CAssetMgr::GetInst()->FindAsset<CTexture>(L"texture\\Character.png"));

	//pMonster->Collider2D()->SetAbsolute(false);
	//pMonster->Collider2D()->SetOffset(Vec3(0.f, 0.f, 0.f));
	//pMonster->Collider2D()->SetScale(Vec3(1.f, 1.f, 1.f));


	pLevel->AddObject(2, pMonster, false);
	//pPlayer->AddChild(pMonster);

	//pPlayer->GetScript<CPlayerScript>()->SetTarget(pMonster);
	// ==========
	// Background
	// ==========
	CGameObject* pBg = new CGameObject;
	pBg->SetName(L"Background");
	pBg->AddComponent(new CTransform);
	pBg->AddComponent(new CMeshRenderer);
	
	pBg->Transform()->SetRelativePos(0.0f, 0.0f, 200.f);
	pBg->Transform()->SetRelativeScale(1280.0f, 720.0f, 0.0f);
	pBg->MeshRenderer()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"RectMesh"));
	pBg->MeshRenderer()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"BgMaterial"));
	Ptr<CMaterial> pBgMat = pBg->MeshRenderer()->GetMaterial();
	pBgMat->SetTexParam(TEX_0, CAssetMgr::GetInst()->Load<CTexture>(L"texture\\Background.jpg", L"texture\\Background.jpg"));
	pBgMat->SetTexParam(TEX_1, CAssetMgr::GetInst()->Load<CTexture>(L"texture\\noise\\noise_03.jpg", L"texture\\noise\\noise_03.jpg"));

	pLevel->AddObject(0, pBg, false);
	// ==========
	// PostProcess Game Object 后期处理游戏对象
	// ==========
	CGameObject* pPostProcessObj = new CGameObject;
	pPostProcessObj->SetName(L"PostProcessObject");
	pPostProcessObj->AddComponent(new CTransform);
	pPostProcessObj->AddComponent(new CMeshRenderer);

	pPostProcessObj->MeshRenderer()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"RectMesh"));
	pPostProcessObj->MeshRenderer()->SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"FilterMaterial"));
	pPostProcessObj->MeshRenderer()->GetMaterial()->SetTexParam(TEX_0, CAssetMgr::GetInst()->FindAsset<CTexture>(L"RenderTargetTexCopy"));
	//m_CurLevel->AddObject(0, pPostProcessObj, false);




	// ===================
	// Debug
	// ===================
	CCollisionManager::GetInst()->LayerCheck(1, 2);

	ChangeLevel(pLevel, LEVEL_STATE::STOP);
}

void CTestLevel2D::CreatePrefab()
{
}
