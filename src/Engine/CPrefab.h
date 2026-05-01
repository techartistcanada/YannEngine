#pragma once
#include "CAsset.h"

class CGameObject;

class CPrefab :
    public CAsset
{
private:
    CGameObject* m_ProtoObj;
public:
	CGameObject* Instantiate();

    virtual int Load(const wstring& _FilePath) { return S_OK; }
    virtual int Save(const wstring& _FilePath) { return S_OK; }

    CLONE(CPrefab);
public:
    CPrefab(bool _bEngineAsset = false);
	CPrefab(CGameObject* _ProtoObj);
    CPrefab(const CPrefab& _Origin);
    ~CPrefab();
};

