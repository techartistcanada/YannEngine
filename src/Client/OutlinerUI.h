#pragma once
#include "EditorUI.h"

class TreeUI;
class TreeNode;

class OutlinerUI :
    public EditorUI
{
private:
	TreeUI* m_TreeUI;
public:
	void UpdateContent();
public:
	virtual void render_tick() override;
private:
	void AddGameObjectToTree(TreeNode* _Node, CGameObject* _GameObject);
	UINT SelectGameObject(DWORD_PTR _dwData);
	UINT OnNodeDragDrop(DWORD_PTR _DragNode, DWORD_PTR _DropNode);
public:
	OutlinerUI();
	~OutlinerUI();
};

