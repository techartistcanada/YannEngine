#pragma once
#include <CScript.h>

class CEditorCameraScript :
    public CScript
{
private:
	float m_Speed;
	Vec3  m_Velocity;
	float m_Damping;
	float m_RotSensitivity;

private:
	void MoveByPerspective();
	void MoveByOrthographic();

public:
	virtual void tick() override;
	CLONE(CEditorCameraScript);
	CEditorCameraScript();
	~CEditorCameraScript();
};

