#pragma once
#include "CComponent.h"
class CTransform :
    public CComponent
{
private:
    Vec3    m_RelativePos;
    Vec3    m_RelativeScale;
    Vec3    m_RelativeRotation;
    Vec3    m_RelativeDir[3];
    Vec3    m_WorldDir[3];

    Matrix  m_matWorld;
    Matrix  m_matWorldInv;

    bool    m_Absolute;

public:
    virtual void finaltick() override;
    void Binding();
public:
    Vec3 GetRelativePos() { return m_RelativePos; }
    Vec3 GetRelativeScale() { return m_RelativeScale; }
    Vec3 GetRelativeRotation() { return m_RelativeRotation; }

    Vec3 GetWorldPos();
	Vec3 GetWorldScale();
	Vec3 GetWorldRotation();

    Vec3 GetRelativeDir(DIR_TYPE _Type) { return m_RelativeDir[(UINT)_Type]; }
    Vec3 GetWorldDir(DIR_TYPE _Type) { return m_WorldDir[(UINT)_Type]; }
    
    const Matrix& GetWorldMat() { return m_matWorld; }

    void SetRelativePos(const Vec3& _vPos) { m_RelativePos = _vPos; }
    void SetRelativeScale(const Vec3& _vScale) { m_RelativeScale = _vScale; }
    void SetRelativeRotation(const Vec3& _vRotation) { m_RelativeRotation = _vRotation; }

	void SetWorldMat(const Matrix& _matWorld) { m_matWorld = _matWorld; }

    void SetRelativePos(float _x, float _y, float _z) { m_RelativePos = Vec3(_x, _y, _z); }
    void SetRelativeScale(float _x, float _y, float _z) { m_RelativeScale = Vec3(_x, _y, _z); }
    void SetRelativeRotation(float _x, float _y, float _z) { m_RelativeRotation = Vec3(_x, _y, _z); }

	void SetAbsolute(bool _Absolute) { m_Absolute = _Absolute; }
	bool IsAbsolute() { return m_Absolute; }

	virtual void SaveToLevelFile(FILE* _File) override;
	virtual void LoadFromLevelFile(FILE* _File) override;

    CTransform& operator = (const CTransform& _Other);
    // 因为所有 member variables 都是“简单值类型”（无资源所有权），所以 compiler 自动生成的默认 copy constructor 是安全且语义正确的
    CLONE(CTransform);
public:
    CTransform();
    ~CTransform();
};

