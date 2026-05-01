#include "pch.h"
#include "CTaskMgr.h"

#include "CGameObject.h"
#include "CLevelMgr.h"
#include "CLevel.h"
#include "CLayer.h"
#include "CCollider2D.h"


CTaskMgr::CTaskMgr()
	: m_bObjectChangeEvent(false)
{

}

CTaskMgr::~CTaskMgr()
{

}

void CTaskMgr::tick()
{
	ClearDeadObjects();

	ClearEvent();

	for (size_t i = 0; i < m_vecTasks.size(); ++i)
	{
		ExecuteTask(m_vecTasks[i]);
	}

	m_vecTasks.clear();
}

void CTaskMgr::ExecuteTask(tTask& _Task)
{
	switch (_Task.Type)
	{
		case TASK_TYPE::SPAWN_OBJECT:
			// Param 0 : Layer Index
			// Param 1 : Object Address
		{
			int LayerIdx = (int)_Task.dwParam_0;
			CGameObject* pSpawnObj = (CGameObject*)_Task.dwParam_1;
			CLevel* pCurLevel = CLevelMgr::GetInst()->GetCurrentLevel();
			pCurLevel->AddObject(LayerIdx, pSpawnObj);
			
			if (pCurLevel->GetState() == LEVEL_STATE::PLAY)
			{
				pSpawnObj->begin();
			}
			// NOTE: 这明显是TODO:
			m_bObjectChangeEvent = true;
		}
		break;
		case TASK_TYPE::DESTROY_OBJECT:
		{
			// NOTE: 死亡状态dead state只持续一帧j
			CGameObject* pObject = (CGameObject*)_Task.dwParam_0;

			if (!pObject->IsDead())
			{
				pObject->m_IsDead = true;
				m_vecDead.push_back(pObject);
			}

			m_bObjectChangeEvent = true;
		}
		break;
		case TASK_TYPE::COLLIDER_SEMI_DEACTIVATE:
		{
			CCollider2D* pCollider = (CCollider2D*)_Task.dwParam_0;
			pCollider->m_SemiInactive = true;
		}
		break;
		case TASK_TYPE::COLLIDER_DEACTIVATE:
		{
			CCollider2D* pCollider = (CCollider2D*)_Task.dwParam_0;
			pCollider->m_SemiInactive = false;
			pCollider->m_Active = false;
		}
		break;
		case TASK_TYPE::CHANGE_LEVEL:
		{
			CLevel* pNextLevel = (CLevel*)_Task.dwParam_0;
			LEVEL_STATE NextLevelState = (LEVEL_STATE)_Task.dwParam_1;

			CLevelMgr::GetInst()->ChangeLevel(pNextLevel);
			pNextLevel->ChangeState(NextLevelState);
			// TODO: 这明显是NOTE:
			m_bObjectChangeEvent = true;
		}
		break;
		case TASK_TYPE::CHANGE_LEVEL_STATE:
		{
			LEVEL_STATE NextLevelState = (LEVEL_STATE)_Task.dwParam_0;

			CLevel* pCurLevel = CLevelMgr::GetInst()->GetCurrentLevel();
			pCurLevel->ChangeState(NextLevelState);
		}
		break;
		case TASK_TYPE::DEL_ASSET:
		{
			CAsset* pAsset = (CAsset*)_Task.dwParam_0;
			int RefCount = pAsset->GetRefCount();

			if (1 < RefCount)
			{
				int id = MessageBox(nullptr, L"Asset is being referenced by other objects. Do you want to force delete it?", L"Warning", MB_ICONWARNING | MB_YESNO);

				if (IDCANCEL == id)
					break;
			}

			CAssetMgr::GetInst()->DeleteAsset(pAsset->GetAssetType(), pAsset->GetKey());
		}
		break;
	}
}

void CTaskMgr::ClearDeadObjects()
{
	for (size_t i = 0; i < m_vecDead.size(); ++i)
	{
		delete m_vecDead[i];
	}
	
	m_vecDead.clear();
}
