#include "pch.h"
#include "OutlinerUI.h"

#include "TreeUI.h"

#include <CLevelMgr.h>
#include <CLevel.h>
#include <CLayer.h>
#include <CGameObject.h>
#include <CTaskMgr.h>

#include "InspectorUI.h"



OutlinerUI::OutlinerUI()
	: EditorUI("Outliner", "##OutlinerUI")
	, m_TreeUI(nullptr)
{
	m_TreeUI = new TreeUI("OutlinerTreeUI");
	m_TreeUI->SetShowRoot(false);
	AddChildUI(m_TreeUI);
	
	m_TreeUI->RegisterOnNodeSelDelegate(this, (UI_DELEGATE_1)&OutlinerUI::SelectGameObject);

	m_TreeUI->SetEnableDragDrop(true);
	m_TreeUI->RegisterOnNodeDragDropDelegate(this, (UI_DELEGATE_2)&OutlinerUI::OnNodeDragDrop);

	UpdateContent();
}

OutlinerUI::~OutlinerUI()
{
}

void OutlinerUI::render_tick()
{
	if (CTaskMgr::GetInst()->HasAnyObjectChangeEvent())
	{
		UpdateContent();
	}
}

void OutlinerUI::AddGameObjectToTree(TreeNode* _Node, CGameObject* _GameObject)
{
	TreeNode* pNewNode = m_TreeUI->AddTreeNode(_Node, ToString(_GameObject->GetName()), (DWORD_PTR)_GameObject);

	const vector<CGameObject*>& vecChildren = _GameObject->GetChildren();
	for (size_t i = 0; i < vecChildren.size(); ++i)
	{
		AddGameObjectToTree(pNewNode, vecChildren[i]);
	}
}

UINT OutlinerUI::SelectGameObject(DWORD_PTR _dwData)
{
	int a = 100;
	TreeNode* pSelectedNode = (TreeNode*)_dwData;
	CGameObject* pSelectedObject = (CGameObject*)pSelectedNode->GetData();
	
	InspectorUI* pInspector = CImGuiMgr::GetInst()->FindEditorUI<InspectorUI>("Inspector");

	pInspector->SetTargetObject(pSelectedObject);
	return 0;
}

UINT OutlinerUI::OnNodeDragDrop(DWORD_PTR _DragNode, DWORD_PTR _DropNode)
{
	TreeNode* pDragNode = (TreeNode*)_DragNode;
	TreeNode* pDropNode = (TreeNode*)_DropNode;

	CGameObject* pDragObject = (CGameObject*)pDragNode->GetData();
	CGameObject* pDropObject = pDropNode ? (CGameObject*)pDropNode->GetData() : nullptr;
	
	if (pDropObject)
	{
		// CASE 1: 拖到另一个物体上了, 那就把拖动的物体放到目标物体下面
		if (false == pDropObject->IsAncestorOf(pDragObject))
		{
			pDropObject->AddChild(pDragObject);
		}

	}
	else
	{
		// CASE 2: 拖到空白处了, 那就把拖动的物体放到根节点下面
		if (nullptr != pDragObject->GetParent())
		{
			pDragObject->DetachFromParent();
			pDragObject->RegisterAsParentObjectInCurrentLayer();
		}
	}

	UpdateContent();

	return 0;
}

void OutlinerUI::UpdateContent()
{
	m_TreeUI->Clear();
	TreeNode* pRootNode = m_TreeUI->AddTreeNode(nullptr, "DummyRoot");

	CLevel* pCurLevel = CLevelMgr::GetInst()->GetCurrentLevel();

	if(nullptr == pCurLevel)
		return;

	for (UINT i = 0; i < MAX_LAYER; ++i)
	{
		CLayer* pLayer = pCurLevel->GetLayer(i);

		const vector<CGameObject*>& vecParents = pLayer->GetParentObjects();

		for (size_t j = 0; j < vecParents.size(); ++j)
		{
			AddGameObjectToTree(pRootNode, vecParents[j]);
		}
	}
}

