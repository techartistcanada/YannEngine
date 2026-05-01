#include "pch.h"
#include "CEditorCameraScript.h"
#include <CKeyMgr.h>


CEditorCameraScript::CEditorCameraScript()
	: CScript(-1)
	, m_Speed(200.f)
	, m_Velocity(0.f, 0.f, 0.f)
	, m_Damping(1.0f)         // lower = smoother coast, higher = snappier stop
	, m_RotSensitivity(0.75f)  // tune to taste
{
}

CEditorCameraScript::~CEditorCameraScript()
{
}

void CEditorCameraScript::tick()
{
	if (KEY_TAP(KEY::P))
	{
		Camera()->GetProjType() == PROJ_TYPE::ORTHOGRAPHIC ? Camera()->SetProjType(PROJ_TYPE::PERSPECTIVE) : Camera()->SetProjType(PROJ_TYPE::ORTHOGRAPHIC);

		if (Camera()->GetProjType() == PROJ_TYPE::ORTHOGRAPHIC)
		{
			Vec3 vPos = Transform()->GetRelativePos();
			Transform()->SetRelativePos(Vec3(vPos.x, vPos.y, 0.f));
			Transform()->SetRelativeRotation(Vec3(0.f, 0.f, 0.f));
		}
	}

	if (Camera()->GetProjType() == PROJ_TYPE::PERSPECTIVE)
		MoveByPerspective();
	else
		MoveByOrthographic();
}


void CEditorCameraScript::MoveByPerspective()
{
	float Speed = m_Speed;
	if (KEY_PRESSED(KEY::LSHIFT))
	{
		Speed *= 4.f;
	}

	Vec3 vFront = Transform()->GetRelativeDir(DIR_TYPE::FRONT);
	Vec3 vRight = Transform()->GetRelativeDir(DIR_TYPE::RIGHT);
	Vec3 vUp = Transform()->GetRelativeDir(DIR_TYPE::UP);

	// Accumulate target velocity from input
	Vec3 vTargetVel = Vec3(0.f, 0.f, 0.f);

	if (KEY_PRESSED(KEY::I))
		vTargetVel += vFront * Speed;

	if (KEY_PRESSED(KEY::K))
		vTargetVel -= vFront * Speed;

	if (KEY_PRESSED(KEY::J))
		vTargetVel -= vRight * Speed;

	if (KEY_PRESSED(KEY::L))
		vTargetVel += vRight * Speed;

	if (KEY_PRESSED(KEY::U))
		vTargetVel += vUp * Speed;

	if (KEY_PRESSED(KEY::O))
		vTargetVel -= vUp * Speed;

	// Smooth interpolation: blend current velocity toward target
	// Using exponential damping for frame-rate independent smoothing
	float smoothFactor = 1.f - expf(-m_Damping / DT_EDITOR);
	smoothFactor = min(smoothFactor, 1.f);

	m_Velocity.x += (vTargetVel.x - m_Velocity.x) * smoothFactor;
	m_Velocity.y += (vTargetVel.y - m_Velocity.y) * smoothFactor;
	m_Velocity.z += (vTargetVel.z - m_Velocity.z) * smoothFactor;

	Vec3 vCurPos = Transform()->GetRelativePos();
	vCurPos += m_Velocity * DT_EDITOR;

	// Mouse rotation (raw pixel delta, NOT normalized)
	if (KEY_PRESSED(KEY::RBTN))
	{
		Vec2 vDrag = CKeyMgr::GetInst()->GetMouseDrag();

		Vec3 vRot = Transform()->GetRelativeRotation();
		vRot.y -= vDrag.x * m_RotSensitivity * 0.001f;
		vRot.x -= vDrag.y * m_RotSensitivity * 0.001f;

		Transform()->SetRelativeRotation(vRot);
	}

	GetOwner()->Transform()->SetRelativePos(vCurPos);
}

void CEditorCameraScript::MoveByOrthographic()
{
	float Speed = m_Speed;
	if (KEY_PRESSED(KEY::LSHIFT))
	{
		Speed *= 4.f;
	}

	Vec3 vCurPos = Transform()->GetRelativePos();

	if (KEY_PRESSED(KEY::W))
		vCurPos.y += Speed * DT_EDITOR;

	if (KEY_PRESSED(KEY::S))
		vCurPos.y -= Speed * DT_EDITOR;

	if (KEY_PRESSED(KEY::A))
		vCurPos.x -= Speed * DT_EDITOR;

	if (KEY_PRESSED(KEY::D))
		vCurPos.x += Speed * DT_EDITOR;

	// Multiplicative zoom — feels natural at any zoom level
	if (KEY_PRESSED(KEY::NUM1))
	{
		float scale = Camera()->GetScale();
		scale *= powf(0.3f, DT_EDITOR);  // zoom in
		if (scale < 0.01f)
			scale = 0.01f;
		Camera()->SetScale(scale);
	}

	if (KEY_PRESSED(KEY::NUM2))
	{
		float scale = Camera()->GetScale();
		scale *= powf(3.0f, DT_EDITOR);  // zoom out
		Camera()->SetScale(scale);
	}

	GetOwner()->Transform()->SetRelativePos(vCurPos);
}