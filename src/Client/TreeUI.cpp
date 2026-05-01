#include "pch.h"
#include "TreeUI.h"




// =======================================================
// TreeNode !!! NOT TreeUI
// =======================================================
TreeNode::~TreeNode()
{
	Safe_Del_Vector(m_vecChildrenNodes);
}

void TreeNode::render_tick()
{
	string Name = m_Name;

	UINT iFlag = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;

	if(m_bImGuiFramed)
		iFlag |= ImGuiTreeNodeFlags_Framed;

	if (m_bSelected)
	{
		iFlag |= ImGuiTreeNodeFlags_Selected;
	}

	// Bug: not working
	if (m_Owner->m_bShowFilenameOnly)
	{
		char buffer[256] = {};
		_splitpath_s(Name.c_str(), 0, 0, 0, 0, buffer, 256, 0, 0);
		Name = buffer;
	}

		
	if (m_vecChildrenNodes.empty())
	{
		iFlag |= ImGuiTreeNodeFlags_Leaf;
		if(m_bImGuiFramed)
			Name = "   " + m_Name;
	}

	// render children nodes
	if (ImGui::TreeNodeEx(Name.c_str(), iFlag))
	{
		// ----------------------------
		// Drag & Drop(Begin Dragging)
		// ----------------------------
		if (m_Owner->m_bEnableDrag)
		{
			if (ImGui::BeginDragDropSource())
			{
				m_Owner->SetDragNode(this);

				ImGui::SetDragDropPayload(m_Owner->GetDisplayName().c_str(), &m_Data, sizeof(DWORD_PTR));
				ImGui::Text(m_Name.c_str());
				ImGui::EndDragDropSource();
			}
		}
		// ----------------------------
		// Drag & Drop(Receive Dropping)
		// ----------------------------
		if (m_Owner->m_bEnableDrop)
		{
			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(m_Owner->GetDisplayName().c_str());
				if (nullptr != payload)
				{
					m_Owner->SetDropNode(this);
				}
				ImGui::EndDragDropTarget();
			}
		}
		// ------------------
		// Click to select
		// ------------------
		if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			if(!m_bImGuiFramed)
				m_Owner->SetSelectedNode(this);
		}

		for (size_t i = 0; i < m_vecChildrenNodes.size(); ++i)
		{
			m_vecChildrenNodes[i]->render_tick();
		}
		ImGui::TreePop();
	}
}














// ======================================================
// TreeUI
// ======================================================
TreeUI::TreeUI(const string& _Name)
	: EditorUI(_Name, "##TreeUI")
	, m_RootNode(nullptr)
	, m_bShowRoot(true)
	, m_bShowFilenameOnly(false)
	, m_SelectedNode(nullptr)
	, m_OnNodeSelCallback(nullptr)
	, m_CallerUI(nullptr)
	, m_OnNodeSelDelegate(nullptr)
	, m_bEnableDrag(false)
	, m_bEnableDrop(false)
	, m_DragNode(nullptr)
	, m_DropNode(nullptr)
	, m_DragDropCallerUI(nullptr)
	, m_OnNodeDragDropDelegate(nullptr)
{
}

TreeUI::~TreeUI()
{
	if (nullptr != m_RootNode)
	{
		delete m_RootNode;
		m_RootNode = nullptr;
	}
}

void TreeUI::render_tick()
{
	if (nullptr == m_RootNode)
		return;

	if (m_bShowRoot)
	{
		m_RootNode->render_tick();
	}
	else
	{
		const vector<TreeNode*>& vecChildNodes = m_RootNode->GetChildrenNodes();
		for (size_t i = 0; i < vecChildNodes.size(); ++i)
		{
			vecChildNodes[i]->render_tick();
		}
	}

	// 拖拽到其他节点上 或 在空白处松开
	if (m_DragNode && (m_DropNode || ImGui::IsMouseReleased(ImGuiMouseButton_Left)))
	{
		if (m_DragDropCallerUI && m_OnNodeDragDropDelegate)
		{
			(m_DragDropCallerUI->*m_OnNodeDragDropDelegate)((DWORD_PTR)m_DragNode, (DWORD_PTR)m_DropNode);
		}
		m_DragNode = nullptr;
		m_DropNode = nullptr;
	}
}

TreeNode* TreeUI::AddTreeNode(TreeNode* _Parent, const string& _NodeName, DWORD_PTR _dwData)
{
	TreeNode* pNewNode = new TreeNode(_NodeName, _dwData);

	pNewNode->m_Owner = this;

	if (nullptr == _Parent)
	{
		assert(!m_RootNode);

		m_RootNode = pNewNode;
	}
	else
	{
		_Parent->AddChildNode(pNewNode);
	}

	return pNewNode;
}

void TreeUI::SetSelectedNode(TreeNode* _SelectedNode)
{
	if(m_SelectedNode)
		m_SelectedNode->m_bSelected = false;

	m_SelectedNode = _SelectedNode;

	if(m_SelectedNode)
		m_SelectedNode->m_bSelected = true;

	if (m_OnNodeSelCallback)
	{
		m_OnNodeSelCallback();
	}
	if (m_CallerUI && m_OnNodeSelDelegate)
	{
		(m_CallerUI->*m_OnNodeSelDelegate)((DWORD_PTR)m_SelectedNode);
	}
}


