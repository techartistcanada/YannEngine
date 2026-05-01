#pragma once
#include "CEntity.h"

class CGameObject;

class CLayer :
    public CEntity
{
private:
	// NOTE: 只管理父级, 子级由父级管理
    vector<CGameObject*> m_vecParents;
    vector<CGameObject*> m_vecObjects; // NOTE: 每帧会重置
    int                  m_LayerIdx;

public:
    void begin();
    void tick();
    void finaltick();
public:
    void AddObject(CGameObject* _Object, bool _ChildMove = false);
    void RegisterObject(CGameObject* _Object)
    {
        m_vecObjects.push_back(_Object);
	}

    void Clear()
    {
		m_vecObjects.clear();
    }
	const vector<CGameObject*>& GetParentObjects() { return m_vecParents; }
	const vector<CGameObject*>& GetObjects() { return m_vecObjects; }

    void DeRegisterAsParent(CGameObject* _Object);

	CLONE_DISABLED(CLayer)
public:
    CLayer();
    ~CLayer();

    friend class CLevel;
};

