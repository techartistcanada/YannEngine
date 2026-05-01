#include "pch.h"
#include "CPlayerScript.h"
#include "CMissileScript.h"
#include <CTaskMgr.h>

#include <CCollider2D.h>

#include <CStructuredBuffer.h>


CPlayerScript::CPlayerScript()
	: CScript((UINT)SCRIPT_TYPE::PLAYERSCRIPT)
	, m_Speed(100.0f)
{
	AddScriptParam(SCRIPT_PARAM_TYPE::FLOAT, "Speed", &m_Speed);
}

CPlayerScript::~CPlayerScript()
{
}

void CPlayerScript::begin()
{
	//m_ParticlePrefab = CAssetMgr::GetInst()->FindAsset<CPrefab>(L"ParticlePrefab");
	//m_MisslePrefab = CAssetMgr::GetInst()->FindAsset<CPrefab>(L"MisslePrefab");
}

void CPlayerScript::tick()
{
	Vec3 vCurPos = GetOwner()->Transform()->GetRelativePos();

	if (KEY_PRESSED(KEY::O))
	{
		Ptr<CMaterial> pMaterial = GetRenderComponent()->GetDynamicMaterial();
		pMaterial->SetScalarParam(INT_0, 1);
	}
	else if (KEY_RELEASED(KEY::O))
	{
		GetRenderComponent()->RestoreMaterial();
	}

	if (KEY_PRESSED(KEY::I))
	{
		vCurPos.y += DT * m_Speed;
	}
	if (KEY_PRESSED(KEY::J))
	{
		vCurPos.x -= DT * m_Speed;
	}
	if (KEY_PRESSED(KEY::K))
	{
		vCurPos.y -= DT * m_Speed;
	}
	if (KEY_PRESSED(KEY::L))
	{
		vCurPos.x += DT * m_Speed;
	}
	if(KEY_TAP(KEY::SPACE))
	{
		Instantiate(m_MisslePrefab, 0, Transform()->GetRelativePos());
	}

	if (IsValid(m_TargetMon))
	{
	}

	GetOwner()->Transform()->SetRelativePos(vCurPos);

}

void CPlayerScript::BeginOverlap(CCollider2D* _OwnCollider, CGameObject* _OtherObject, CCollider2D* _OtherCollider)
{
}

void CPlayerScript::Overlap(CCollider2D* _OwnCollider, CGameObject* _OtherObject, CCollider2D* _OtherCollider)
{
}

void CPlayerScript::EndOverlap(CCollider2D* _OwnCollider, CGameObject* _OtherObject, CCollider2D* _OtherCollider)
{
}

void CPlayerScript::SaveToLevelFile(FILE* _File)
{
	fwrite(&m_Speed, sizeof(float), 1, _File);
	SaveAssetRef(m_ParticlePrefab, _File);
	SaveAssetRef(m_MisslePrefab, _File);
}

void CPlayerScript::LoadFromLevelFile(FILE* _File)
{
	fread(&m_Speed, sizeof(float), 1, _File);
	LoadAssetRef(m_ParticlePrefab, _File);
	LoadAssetRef(m_MisslePrefab, _File);
}
