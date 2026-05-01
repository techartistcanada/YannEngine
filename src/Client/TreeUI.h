#pragma once
#include "EditorUI.h"

class TreeUI;

class TreeNode
{
private:
	TreeUI*				m_Owner;
	TreeNode*			m_ParentNode;
	vector<TreeNode*>   m_vecChildrenNodes;
	string				m_Name;
	DWORD_PTR			m_Data;

	bool                m_bImGuiFramed;
	bool                m_bSelected;

public:
	void render_tick();
public:
	void SetName(const string& _Name) { m_Name = _Name; }
	void SetData(DWORD_PTR _Data) { m_Data = _Data; }
	void SetImGuiFramed(bool _bImGuiFramed) { m_bImGuiFramed = _bImGuiFramed; }

	const vector<TreeNode*>& GetChildrenNodes() const { return m_vecChildrenNodes; }
	void AddChildNode(TreeNode* _ChildNode)
	{
		m_vecChildrenNodes.push_back(_ChildNode);
		_ChildNode->m_ParentNode = this;
	}

	const string& GetName() const { return m_Name; }
	DWORD_PTR GetData() { return m_Data; }
public:
	TreeNode()
		: m_Owner(nullptr)
		, m_ParentNode(nullptr)
		, m_Data(0)
		, m_bImGuiFramed(false)
		, m_bSelected(false)
	{
	}
	TreeNode(const string& _Name, DWORD_PTR _Data)
		: m_Owner(nullptr)
		, m_ParentNode(nullptr)
		, m_Name(_Name)
		, m_Data(_Data)
		, m_bImGuiFramed(false)
		, m_bSelected(false)
	{
	}
	~TreeNode();

	friend class TreeUI;
};


class TreeUI :
    public EditorUI
{
private:
	TreeNode*		m_RootNode;
	TreeNode*		m_SelectedNode;

	TreeNode*		m_DragNode;
	TreeNode*		m_DropNode;

	bool			m_bShowRoot;
	bool			m_bShowFilenameOnly;
	bool            m_bEnableDrag;
	bool            m_bEnableDrop;

	UI_CALLBACK		m_OnNodeSelCallback;
	EditorUI*		m_CallerUI;
	UI_DELEGATE_1	m_OnNodeSelDelegate;
	EditorUI*		m_DragDropCallerUI;
	UI_DELEGATE_2	m_OnNodeDragDropDelegate;
public:
	virtual void render_tick() override;
public:
	void SetShowRoot(bool _bShow) { m_bShowRoot = _bShow; }
	void SetShowFilenameOnly(bool _bShow) { m_bShowFilenameOnly = _bShow; }

	void SetEnableDrag(bool _bEnable) { m_bEnableDrag = _bEnable; }
	void SetEnableDragDrop(bool _bEnable) { m_bEnableDrop = _bEnable; m_bEnableDrag = _bEnable; }

	void RegisterOnNodeSelCallback(UI_CALLBACK _Callback) { m_OnNodeSelCallback = _Callback; }
	void RegisterOnNodeSelDelegate(EditorUI* _CallerUI, UI_DELEGATE_1 _Delegate) { m_OnNodeSelDelegate = _Delegate; m_CallerUI = _CallerUI; }
	void RegisterOnNodeDragDropDelegate(EditorUI* _CallerUI, UI_DELEGATE_2 _Delegate) { m_OnNodeDragDropDelegate = _Delegate; m_DragDropCallerUI = _CallerUI; }

	TreeNode* AddTreeNode(TreeNode* _Parent, const string& _NodeName, DWORD_PTR _dwData = 0);
	void Clear()
	{
		if (nullptr != m_RootNode)
		{
			delete m_RootNode;
			m_RootNode = nullptr;
		}

	}
private:
	void SetSelectedNode(TreeNode* _SelectedNode);
	void SetDragNode(TreeNode* _DragNode) { m_DragNode = _DragNode; }
	void SetDropNode(TreeNode* _DropNode) { m_DropNode = _DropNode; }
public:
	TreeUI(const string& _Name);
	~TreeUI();

	friend class TreeNode;
};

