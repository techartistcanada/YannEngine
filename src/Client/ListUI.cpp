#include "pch.h"
#include "ListUI.h"
#include "ImGui/IconsFontAwesome6.h"


ListUI::ListUI()
	: EditorUI("List", "##ListUI")
	, m_iSelectedIndex(0)
{
}

ListUI::~ListUI()
{
}

void ListUI::render_tick()
{
	ImVec2 vWindowSize = ImGui::GetWindowContentRegionMax();
	vWindowSize.y -= ImGui::GetTextLineHeightWithSpacing() * 2.0f;
	if (ImGui::BeginListBox("##ListBox", ImVec2(vWindowSize.x, vWindowSize.y)))
	{
		for (size_t i = 0; i < m_vecItems.size(); ++i)
		{
			bool bSelected = (i == m_iSelectedIndex);
			if (ImGui::Selectable(m_vecItems[i].c_str(), bSelected))
			{
				m_iSelectedIndex = (int)i;
			}
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if(m_Callback)
					m_Callback();

				if (m_CallerUI && m_Delegate)
					(m_CallerUI->*m_Delegate)((DWORD_PTR)&m_vecItems[m_iSelectedIndex]);

				SetActive(false);
			}
		}
		ImGui::EndListBox();
	}
}

void ListUI::Activate()
{
	m_vecItems.insert(m_vecItems.begin(), "None");
}

void ListUI::Deactivate()
{
	m_vecItems.clear();
	m_iSelectedIndex = -1;

	m_Callback = nullptr;
	m_CallerUI = nullptr;
	m_Delegate = nullptr;

	SetModal(false);
}
