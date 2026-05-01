#pragma once
#include <CScript.h>
class CMonsterScript :
    public CScript
{
private:
    float        m_Speed;
public:
public:
    virtual void begin() override;
    virtual void tick() override;
public:
    virtual void BeginOverlap(CCollider2D* _OwnCollider, CGameObject* _OtherObject, CCollider2D* _OtherCollider) override;
    virtual void Overlap(CCollider2D* _OwnCollider, CGameObject* _OtherObject, CCollider2D* _OtherCollider) override;
    virtual void EndOverlap(CCollider2D* _OwnCollider, CGameObject* _OtherObject, CCollider2D* _OtherCollider) override;

	CLONE(CMonsterScript);
public:
    CMonsterScript();
    ~CMonsterScript();
};

