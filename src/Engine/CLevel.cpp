#include "pch.h"
#include "CLevel.h"

#include "CLayer.h"
#include "CGameObject.h"
#include "CRenderMgr.h"


CLevel::CLevel()
	: m_arrLayer{}
	, m_State(LEVEL_STATE::STOP)
{
	for (UINT i = 0; i < MAX_LAYER; ++i)
	{
		m_arrLayer[i] = new CLayer;
		m_arrLayer[i]->m_LayerIdx = i;
	}
}

CLevel::~CLevel()
{
	Safe_Del_Array(m_arrLayer);
}

void CLevel::begin()
{
	for (UINT i = 0; i < MAX_LAYER; ++i)
	{
		m_arrLayer[i]->begin();
	}
}

void CLevel::tick()
{

	for (UINT i = 0; i < MAX_LAYER; ++i)
	{
		m_arrLayer[i]->tick();
	}
}

void CLevel::finaltick()
{
	for (UINT i = 0; i < MAX_LAYER; ++i)
	{
		m_arrLayer[i]->finaltick();
	}
}

void CLevel::ClearRegisteredObjects()
{
	// NOTE: 这里要做的是清理上一帧被注册的对象(Rendering)
	for (UINT i = 0; i < MAX_LAYER; ++i)
	{
		m_arrLayer[i]->Clear();
	}
}


void CLevel::AddObject(UINT _LayerIdx, CGameObject* _Object, bool _ChildMove)
{
	m_arrLayer[_LayerIdx]->AddObject(_Object, _ChildMove);
}

CGameObject* CLevel::FindObjectByName(const wstring& _Name)
{
	for (UINT i = 0; i < MAX_LAYER; ++i)
	{
		const vector<CGameObject*>& vecObjects = m_arrLayer[i]->GetObjects();
		for (size_t j = 0; j < vecObjects.size(); ++j)
		{
			if (vecObjects[j]->GetName() == _Name)
			{
				return vecObjects[j];
			}
		}
	}
	return nullptr;
}

void CLevel::ChangeState(LEVEL_STATE _NextState)
{
	if (LEVEL_STATE::STOP == _NextState || LEVEL_STATE::PAUSE == _NextState)
		CRenderMgr::GetInst()->ChangeRenderMode(RENDER_MODE::EDITOR);

	if(m_State == _NextState)
		return;

	assert(!(LEVEL_STATE::STOP == m_State && LEVEL_STATE::PAUSE == _NextState));
	if (LEVEL_STATE::STOP == m_State && LEVEL_STATE::PLAY == _NextState)
	{
		begin();
	}

	// PAUSE or STOP -> PLAY
	if((LEVEL_STATE::PAUSE == m_State || LEVEL_STATE::STOP == m_State) && LEVEL_STATE::PLAY == _NextState)
	{
		CRenderMgr::GetInst()->ChangeRenderMode(RENDER_MODE::PLAY);
	}

	m_State = _NextState;
}

