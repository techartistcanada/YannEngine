#pragma once
#include "CEntity.h"

enum FACE_TYPE
{
	FACE_NEAR,
	FACE_FAR,
	FACE_TOP,
	FACE_BOTTOM,
	FACE_LEFT,
	FACE_RIGHT,
};

class CFrustum :
    public CEntity
{
private:
	class CCamera* m_Owner;
	Vec3		   m_arrNDCCorners[8];
	Vec4		   m_arrFace[6];

public:
	void SetOwner(CCamera* _Owner) { m_Owner = _Owner; }
	CCamera* GetOwner() { return m_Owner; }

	bool IsPointInFrustum(Vec3 _WorldPos);
	bool IsSphereInFrustum(Vec3 _WorldPos, float _Radius);
	bool IsAABBInFrustum(Vec3 _AABBMin, Vec3 _AABBMax);
public:
	void finaltick();

public:
	CLONE(CFrustum);
	CFrustum(CCamera* _Camera);
	~CFrustum();
};

