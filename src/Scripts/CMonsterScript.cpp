#include "pch.h"
#include "CMonsterScript.h"


CMonsterScript::CMonsterScript()
	: CScript((UINT)SCRIPT_TYPE::MONSTERSCRIPT)
	, m_Speed(100.0f)
{


}

CMonsterScript::~CMonsterScript()
{
}

void CMonsterScript::begin()
{
}

void CMonsterScript::tick()
{
	Vec3 vCurPos = GetOwner()->Transform()->GetRelativePos();

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
		if (Collider2D()->IsActive())
		{
			//_OtherObject->Destroy();
			Collider2D()->Deactivate();
			m_Speed = 10.f;
		}
		else
		{
			Collider2D()->Activate();
		}

	}


	GetOwner()->Transform()->SetRelativePos(vCurPos);

}

void CMonsterScript::BeginOverlap(CCollider2D* _OwnCollider, CGameObject* _OtherObject, CCollider2D* _OtherCollider)
{
}

void CMonsterScript::Overlap(CCollider2D* _OwnCollider, CGameObject* _OtherObject, CCollider2D* _OtherCollider)
{
}

void CMonsterScript::EndOverlap(CCollider2D* _OwnCollider, CGameObject* _OtherObject, CCollider2D* _OtherCollider)
{
}
