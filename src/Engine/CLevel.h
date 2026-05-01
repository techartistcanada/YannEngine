#pragma once
#include "CEntity.h"

class CLayer;
class CGameObject;

class CLevel :
    public CEntity
{

private:
    CLayer*     m_arrLayer[MAX_LAYER];
	LEVEL_STATE m_State;
public:
    void begin();
    void tick();
    void finaltick();
	void ClearRegisteredObjects();

public:
    void AddObject(UINT _LayerIdx, CGameObject* _Object, bool _ChildMove = false);
    CLayer* GetLayer(int _iLayerIdx) { return m_arrLayer[_iLayerIdx]; }
	LEVEL_STATE GetState() { return m_State; }

	CGameObject* FindObjectByName(const wstring& _Name); // NOTE: only used in Editor
private:
    void ChangeState(LEVEL_STATE _NextState);
public:
    CLONE_DISABLED(CLevel)
public:
    CLevel();
    ~CLevel();

	friend class CTaskMgr;
};

