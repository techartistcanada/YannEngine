#include "pch.h"
#include "CCameraMoveScript.h"


#include <CCamera.h>

CCameraMoveScript::CCameraMoveScript()
	:CScript((UINT)SCRIPT_TYPE::CAMERAMOVESCRIPT)
	, m_Speed(100.0f)
{
}

CCameraMoveScript::~CCameraMoveScript()
{
}

void CCameraMoveScript::tick()
{
	// ====================
	// Toggle between perspective projection and orthographic projection
	// ====================
	if (KEY_TAP(KEY::P))
	{
		PROJ_TYPE type = Camera()->GetProjType();
		type == PROJ_TYPE::ORTHOGRAPHIC ? Camera()->SetProjType(PROJ_TYPE::PERSPECTIVE) : Camera()->SetProjType(PROJ_TYPE::ORTHOGRAPHIC);

		if (Camera()->GetProjType() == PROJ_TYPE::ORTHOGRAPHIC)
		{
			Vec3 vPos = Transform()->GetRelativePos();
			Transform()->SetRelativePos(Vec3(vPos.x, vPos.y, 0.f));
			Transform()->SetRelativeRotation(Vec3(0.f, 0.f, 0.f));
		}

	}

	// ====================
	// Move Camera
	// ====================
	if (Camera()->GetProjType() == PROJ_TYPE::PERSPECTIVE)
		MoveByPerspective();
	else
		MoveByOrthographic();
}

void CCameraMoveScript::MoveByPerspective()
{
	Vec3 vCurPos = Transform()->GetRelativePos();
	Vec3 vFront = Transform()->GetRelativeDir(DIR_TYPE::FRONT);
	Vec3 vRight = Transform()->GetRelativeDir(DIR_TYPE::RIGHT);

	if (KEY_PRESSED(KEY::UP))
	{
		vCurPos += DT * m_Speed * vFront;;
	}
	if (KEY_PRESSED(KEY::LEFT))
	{
		vCurPos -= DT * m_Speed * vRight;
	}
	if (KEY_PRESSED(KEY::DOWN))
	{
		vCurPos -= DT * m_Speed * vFront;
	}
	if (KEY_PRESSED(KEY::RIGHT))
	{
		vCurPos += DT * m_Speed * vRight;
	}
	if (KEY_PRESSED(KEY::Y))
	{
		Vec3 vRot = Transform()->GetRelativeRotation();
		vRot.y += DT * XM_PI;
		Transform()->SetRelativeRotation(vRot);
	}
	if (KEY_PRESSED(KEY::RBTN))
	{
		Vec2 vDrag = CKeyMgr::GetInst()->GetMouseDrag();
		Vec3 vRot = Transform()->GetRelativeRotation();
		vRot.y += vDrag.x * DT * XM_PI;
		vRot.x -= vDrag.y * DT * XM_PI;
		Transform()->SetRelativeRotation(vRot);
	}

	GetOwner()->Transform()->SetRelativePos(vCurPos);
}

void CCameraMoveScript::MoveByOrthographic()
{
	Vec3 vCurPos = Transform()->GetRelativePos();
	Vec3 vFront = Transform()->GetRelativeDir(DIR_TYPE::FRONT);
	Vec3 vRight = Transform()->GetRelativeDir(DIR_TYPE::RIGHT);

	if (KEY_PRESSED(KEY::UP))
	{
		vCurPos.y += DT * m_Speed;;
	}
	if (KEY_PRESSED(KEY::LEFT))
	{
		vCurPos.x -= DT * m_Speed;
	}
	if (KEY_PRESSED(KEY::DOWN))
	{
		vCurPos.y -= DT * m_Speed;
	}
	if (KEY_PRESSED(KEY::RIGHT))
	{
		vCurPos.x += DT * m_Speed;
	}

	if (KEY_PRESSED(KEY::U))
	{
		float scale = Camera()->GetScale();
		scale -= DT;
		if (scale < 0.01f)
		{
			scale = 0.01;
		}
		Camera()->SetScale(scale);
	}
	if (KEY_PRESSED(KEY::Y))
	{
		float scale = Camera()->GetScale();
		scale += DT;
		Camera()->SetScale(scale);
	}

	GetOwner()->Transform()->SetRelativePos(vCurPos);
}
