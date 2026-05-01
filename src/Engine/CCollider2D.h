#pragma once
#include "CComponent.h"
class CCollider2D :
    public CComponent
{
private:
    Vec3 m_Offset;
    Vec3 m_FinalPos;
    Vec3 m_Scale;
    Vec3 m_Rotation;
    Matrix m_matColWorld;
    bool m_Absolute;
    bool m_Active;
    bool m_SemiInactive; // NOTE: about to be deactivated 下一帧才真正禁用

    int  m_OverlapCount; // number of colliders u r colliding with

public:
	void SetOffset(const Vec3& _Offset) { m_Offset = _Offset; }
	void SetScale(const Vec3& _Scale) { m_Scale = _Scale;}
    void SetRotationZ(float _Angle) { m_Rotation.z = _Angle; }

	Vec3 GetOffset() { return m_Offset; }
	Vec3 GetScale() { return m_Scale; }
	Vec3 GetFinalPos() { return m_FinalPos; }
    float GetRotationZ() { return m_Rotation.z; }


	const Matrix& GetWorldMatrix() { return m_matColWorld; }
	void SetAbsolute(bool _Absolute) { m_Absolute = _Absolute; }
	bool IsAbsolute() { return m_Absolute; }

    void Activate();
    void Deactivate();

    bool IsActive() { return m_Active; }
    bool IsSemiInactive() { return m_SemiInactive; }

public:

    virtual void finaltick() override;
public:
    void BeginOverlap(CCollider2D* _OtherCollider);
	void Overlap(CCollider2D* _OtherCollider);
	void EndOverlap(CCollider2D* _OtherCollider);

	virtual void SaveToLevelFile(FILE* _File) override;
	virtual void LoadFromLevelFile(FILE* _File) override;

    CLONE(CCollider2D)
public:
    CCollider2D();
	CCollider2D(const CCollider2D& _Other);
    ~CCollider2D();

    friend class CTaskMgr;
};

