#pragma once
#include "CComponent.h"

#include "CTimeMgr.h"
#include "CKeyMgr.h"

#include "components.h"

enum class SCRIPT_PARAM_TYPE
{
    INT,
    FLOAT,
    VEC2,
	VEC3,
    VEC4,
    TEXTURE
};

struct tScriptParam
{
    SCRIPT_PARAM_TYPE Type;
    string            Desc;
    void*             pData;
};



class CScript :
    public CComponent
{
private:
    const int            m_iScriptType;
	vector<tScriptParam> m_vecParams;

public:
    virtual void tick() = 0;
    virtual void finaltick() final {}

	const vector<tScriptParam>& GetScriptParams() { return m_vecParams; }

protected:
	void AddScriptParam(SCRIPT_PARAM_TYPE _Type, const string& _Desc, void* _pData)
	{
		m_vecParams.push_back(tScriptParam{ _Type, _Desc, _pData });
	}
public:
	UINT GetScriptType() { return m_iScriptType; }

public:
    CGameObject* Instantiate(Ptr<CPrefab> _Prefab, int _LayerIdx, const Vec3& _WorldPos);
    virtual void BeginOverlap(CCollider2D* _OwnCollider, CGameObject* _OtherObject, CCollider2D* _OtherCollider) {}
    virtual void Overlap(CCollider2D* _OwnCollider, CGameObject* _OtherObject, CCollider2D* _OtherCollider) {}
    virtual void EndOverlap(CCollider2D* _OwnCollider, CGameObject* _OtherObject, CCollider2D* _OtherCollider) {}

    virtual void SaveToLevelFile(FILE* _File) override {};
	virtual void LoadFromLevelFile(FILE* _File) override {};
public:
    CScript(UINT _ScriptType);
    ~CScript();
};

