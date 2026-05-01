#pragma once
#include "Singleton.h"
class CTaskMgr :
    public CSingleton<CTaskMgr>
{
    SINGLE(CTaskMgr)
private:
    vector<tTask>           m_vecTasks;
    vector<CGameObject*>    m_vecDead;

    bool                    m_bObjectChangeEvent; // for outlinerUI and contentUI, TODO: better naming
public:
	bool HasAnyObjectChangeEvent() { return m_bObjectChangeEvent;  }
public:
    void tick();
    void AddTask(const tTask& _Task)
    {
        m_vecTasks.push_back(_Task);
    }
private:
    void ExecuteTask(tTask& _Task);
    void ClearDeadObjects();
    void ClearEvent()
    {
		m_bObjectChangeEvent = false;
    }

};

