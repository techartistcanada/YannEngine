#include "pch.h"
#include "EditorUI.h"



EditorUI::EditorUI(const string& _Name, const string& _ID)
	: m_DisplayName(_Name), m_ID(_ID)
	, m_bActive(true)
	, m_ParentUI(nullptr)
	, m_bSeperator(false)
	, m_bModal(false)
{
}

EditorUI::~EditorUI()
{
}

bool EditorUI::SetActive(bool _bActive)
{
	if (_bActive == m_bActive)
		return false;

	m_bActive = _bActive;

	if (m_bActive)
		Activate();
	else
		Deactivate();

	return true;
}

void EditorUI::SetFocus()
{
	string fullname = m_DisplayName + m_ID;
	ImGui::SetWindowFocus(fullname.c_str());
}

void EditorUI::tick()
{
	if (m_bActive == false)
		return;

	string fullname = m_DisplayName + m_ID;

	if (IsRootUI())
	{
		bool bActive = m_bActive;
		// -------------
		// 非modal模式
		// -------------
		if (!m_bModal)
		{
			ImGui::Begin(fullname.c_str(), &bActive);
			SetActive(bActive);


			render_tick();
			for (size_t i = 0; i < m_vecChildrenUIs.size(); ++i)
			{

				m_vecChildrenUIs[i]->tick();
			}

			ImGui::End();
		}
		// -------------
		// Modal模式
		// -------------
		else
		{
			ImGui::OpenPopup(fullname.c_str());
			if (ImGui::BeginPopupModal(fullname.c_str(), &bActive))
			{
				SetActive(bActive);
				render_tick();

				for (size_t i = 0; i < m_vecChildrenUIs.size(); ++i)
				{
					m_vecChildrenUIs[i]->tick();
				}

				ImGui::EndPopup();
			}
			else
			{
				SetActive(bActive);
			}

		}
	}
	else
	{
		ImGui::BeginChild(fullname.c_str(), m_SizeAsChild);

		render_tick();
		for (size_t i = 0; i < m_vecChildrenUIs.size(); ++i)
		{
			m_vecChildrenUIs[i]->tick();
		}

		ImGui::EndChild();
	}
}


