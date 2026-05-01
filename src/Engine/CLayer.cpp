#include "pch.h"
#include "CLayer.h"


#include "CGameObject.h"


CLayer::CLayer()
{
}

CLayer::~CLayer()
{
	Safe_Del_Vector(m_vecParents);
}


void CLayer::begin()
{
	for (size_t i = 0; i < m_vecParents.size(); ++i)
	{
		m_vecParents[i]->begin();
	}

}

void CLayer::tick()
{
	for (size_t i = 0; i < m_vecParents.size(); ++i)
	{
		m_vecParents[i]->tick();
	}
}

void CLayer::finaltick()
{
	vector<CGameObject*>::iterator iter = m_vecParents.begin();

	for (; iter != m_vecParents.end();)
	{
		(*iter)->finaltick();
		if ((*iter)->IsDead())
		{
			iter = m_vecParents.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

void CLayer::AddObject(CGameObject* _Object, bool _ChildMove)
{
	if (!_Object->GetParent())
	{
		m_vecParents.push_back(_Object);
	}

	// NOTE: BFS 设置层级, 为什么要用BFS呢，因为有可能子物体有自己的子物体
	static list<CGameObject*> queue;
	queue.clear();
	queue.push_back(_Object);

	while (!queue.empty())
	{
		CGameObject* pObject = queue.front();
		queue.pop_front();

		if(_ChildMove)
		{
			pObject->m_LayerIdx = m_LayerIdx;
		}
		else
		{
			// NOTE: 
			// 但有条件：
			// - 要么是根对象本身
			// - 要么之前还没被放进任何 Layer（m_LayerIdx == -1）
			if(pObject == _Object || -1 == pObject->m_LayerIdx)
			{
				pObject->m_LayerIdx = m_LayerIdx;
			}
		}

		vector<CGameObject*> vecChildren = pObject->GetChildren();
		for (size_t i = 0; i < vecChildren.size(); ++i)
		{
			queue.push_back(vecChildren[i]);
		}
	}
}

void CLayer::DeRegisterAsParent(CGameObject* _Object)
{
	vector<CGameObject*>::iterator iter = m_vecParents.begin();
	for (; iter != m_vecParents.end(); ++iter)
	{
		if (*iter == _Object)
		{
			m_vecParents.erase(iter);
			return;
		}
	}

	assert(nullptr);
}
