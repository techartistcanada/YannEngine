#include "pch.h"
#include "CGameObject.h"

#include "CLevelMgr.h"
#include "CLevel.h"
#include "CLayer.h"

#include "CComponent.h"
#include "CRenderComponent.h"
#include "CScript.h"
#include "CTaskMgr.h"


CGameObject::CGameObject()
	: m_arrComponents{}
	, m_RenderComponent(nullptr)
	, m_Parent(nullptr)
	, m_LayerIdx(-1)
	, m_IsDead(false)
{
}

CGameObject::CGameObject(const CGameObject& _Origin)
	: CEntity(_Origin)
	, m_arrComponents{}
	, m_RenderComponent(nullptr)
	, m_LayerIdx(-1)
	, m_Parent(nullptr)
	, m_IsDead(false)
{
	// 这里要深拷贝组件(Components)
	for (UINT i = 0; i < (UINT)COMPONENT_TYPE::END; ++i)
	{
		if (nullptr != _Origin.m_arrComponents[i])
		{
			AddComponent(_Origin.m_arrComponents[i]->Clone());
		}
	}
	// 这里要深拷贝脚本(Scripts)
	for (size_t i = 0; i < _Origin.m_vecScripts.size(); ++i)
	{
		AddComponent(_Origin.m_vecScripts[i]->Clone());
	}

	// 这里要深拷贝子物体(Children)
	for (size_t i = 0; i < _Origin.m_vecChildren.size(); ++i)
	{
		AddChild(_Origin.m_vecChildren[i]->Clone());
	}
}

CGameObject::~CGameObject()
{
	Safe_Del_Array(m_arrComponents);
	Safe_Del_Vector(m_vecScripts);
	Safe_Del_Vector(m_vecChildren);
}

void CGameObject::begin()
{
	// components begin
	for (UINT i = 0; i < (UINT)COMPONENT_TYPE::END; ++i)
	{
		if(nullptr != m_arrComponents[i])
			m_arrComponents[i]->begin();
	}

	// scripts begin
	for (size_t i = 0; i < m_vecScripts.size(); ++i)
	{
		m_vecScripts[i]->begin();
	}

	// children begin
	for (size_t i = 0; i < m_vecChildren.size(); ++i)
	{
		m_vecChildren[i]->begin();
	}
}

void CGameObject::tick()
{
	// components tick
	for (UINT i = 0; i < (UINT)COMPONENT_TYPE::END; ++i)
	{
		if(nullptr != m_arrComponents[i])
			m_arrComponents[i]->tick();
	}

	// scripts tick
	for (size_t i = 0; i < m_vecScripts.size(); ++i)
	{
		m_vecScripts[i]->tick();
	}

	// children tick
	for (size_t i = 0; i < m_vecChildren.size(); ++i)
	{
		m_vecChildren[i]->tick();
	}

}

void CGameObject::finaltick()
{
	// Components finaltick
	for (UINT i = 0; i < (UINT)COMPONENT_TYPE::END; ++i)
	{
		if(nullptr != m_arrComponents[i])
			m_arrComponents[i]->finaltick();
	}

	// register self to layer
	CLevel* pCurLevel = CLevelMgr::GetInst()->GetCurrentLevel();
	CLayer* pLayer = pCurLevel->GetLayer(m_LayerIdx);
	pLayer->RegisterObject(this);
	 
	// children finaltick
	for (size_t i = 0; i < m_vecChildren.size(); ++i)
	{
		m_vecChildren[i]->finaltick();
	}
}
void CGameObject::render()
{
	if (m_RenderComponent)
	{
		m_RenderComponent->render();
	}
}

void CGameObject::AddComponent(CComponent* _Component)
{
	COMPONENT_TYPE type = _Component->GetComponentType();

	if (COMPONENT_TYPE::SCRIPT == type)
	{
		m_vecScripts.push_back((CScript*)_Component);
	}
	else
	{
		assert(!m_arrComponents[(UINT)type]);

		CRenderComponent* pRenderCom = dynamic_cast<CRenderComponent*>(_Component);
		if (nullptr != pRenderCom)
		{
			assert(!m_RenderComponent);
			m_RenderComponent = pRenderCom;
		}

		m_arrComponents[(UINT)type] = _Component;

	}

	_Component->m_Owner = this;
	
}

void CGameObject::AddChild(CGameObject* _Child)
{
	if (_Child->GetParent())
	{
		_Child->DetachFromParent();
	}
	else if (-1 != _Child->m_LayerIdx)
	{
		CLevel* pCurLevel = CLevelMgr::GetInst()->GetCurrentLevel();
		CLayer* pCurLayer = pCurLevel->GetLayer(_Child->m_LayerIdx);
		pCurLayer->DeRegisterAsParent(_Child);
	}

	_Child->m_Parent = this;
	m_vecChildren.push_back(_Child);
}

void CGameObject::RegisterAsParentObjectInCurrentLayer()
{
	CLevel* pCurLevel = CLevelMgr::GetInst()->GetCurrentLevel();
	CLayer* pCurLayer = pCurLevel->GetLayer(m_LayerIdx);
	pCurLayer->AddObject(this, false);
}

void CGameObject::DetachFromParent()
{
	if (nullptr == m_Parent)
		return;

	vector<CGameObject*>::iterator iter = m_Parent->m_vecChildren.begin();
	for (; iter != m_Parent->m_vecChildren.end(); ++iter)
	{
		if (*iter == this)
		{
			m_Parent->m_vecChildren.erase(iter);
			m_Parent = nullptr;
			return;
		}
	}

	assert(nullptr);
}

bool CGameObject::IsAncestorOf(CGameObject* _Object)
{
	CGameObject* pAncestor = m_Parent;

	while (pAncestor)
	{
		if (pAncestor == _Object)
			return true;

		pAncestor = pAncestor->GetParent();
	}

	return false;

	return false;
}

void CGameObject::Destroy()
{
	tTask task = {};
	task.Type = TASK_TYPE::DESTROY_OBJECT;
	task.dwParam_0 = (DWORD_PTR)this;

	CTaskMgr::GetInst()->AddTask(task);
}
