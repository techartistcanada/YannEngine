#pragma once
#include "CComponent.h"


class CBoundingBox :
    public CComponent
{
private:
    Vec3	m_OffsetPos;
    float	m_Radius;
	bool	m_Absolute;
	bool    m_bShowDebug;
	Matrix  m_WorldMat;

	// AABB(world space, computed in finaltick)
	Vec3    m_WorldAABBMin;
	Vec3    m_WorldAABBMax;
	// Local half-extents(set from cmesh)
	Vec3    m_LocalHalfExtents;

public:
	Vec3 GetOffsetPos() { return m_OffsetPos; }
	float GetRadius() { return m_Radius; }
	float GetWorldRadius() { return m_WorldMat._11; }
	Vec3 GetWorldPos() { return m_WorldMat.Translation(); }

	Vec3 GetWorldAABBMin() { return m_WorldAABBMin; }
	Vec3 GetWorldAABBMax() { return m_WorldAABBMax; }

	void SetOffsetPos(Vec3 _OffsetPos) { m_OffsetPos = _OffsetPos; }
	void SetRadius(float _Radius) { m_Radius = _Radius; }
	void SetAbsolute(bool _Absolute) { m_Absolute = _Absolute; }
	void SetLocalHalfExtents(Vec3 _HalfExtents) { m_LocalHalfExtents = _HalfExtents; }

	void IsAbsolute(bool _Absolute) { m_Absolute = _Absolute; }
public:
    virtual void finaltick() override;

public:
    CLONE(CBoundingBox);
	virtual void SaveToLevelFile(FILE* _File) override;
	virtual void LoadFromLevelFile(FILE* _File) override;
public:
    CBoundingBox();
    ~CBoundingBox();

};

