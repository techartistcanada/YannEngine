#pragma once

#include "ImGui/imgui.h"
#include "ImGui/IconsFontAwesome6.h"
#include "CImGuiMgr.h"

class EditorUI
{
private:
	string				m_DisplayName;
	const string		m_ID;

	EditorUI*			m_ParentUI;
	vector<EditorUI*>	m_vecChildrenUIs;

	ImVec2			    m_SizeAsChild;
	bool				m_bActive;
	bool                m_bSeperator;
	bool                m_bModal;

public:
	void SetDisplayName(const string& _Name) { m_DisplayName = _Name; }
	const string& GetDisplayName() const { return m_DisplayName; }

	void SetModal(bool _bModal) { m_bModal = _bModal; }
	void SetSeperator(bool _bSeperator) { m_bSeperator = _bSeperator; }
	bool SetActive(bool _bActive);
	bool IsActive() const { return m_bActive; }

	EditorUI* GetParentUI() const { return m_ParentUI; }
	const vector<EditorUI*>& GetChildrenUIs() const { return m_vecChildrenUIs; }

	void SetSizeAsChild(const ImVec2& _Size) { m_SizeAsChild = _Size; }

	void AddChildUI(EditorUI* _ChildUI)
	{ 
		_ChildUI->m_ParentUI = this;
		m_vecChildrenUIs.push_back(_ChildUI);
	}
	 
	bool IsRootUI() { return !m_ParentUI; }
	void SetFocus();

public:
	virtual void tick();
	virtual void render_tick() = 0;

	virtual void Activate() {}
	virtual void Deactivate() {}
public:
	//EditorUI();
	EditorUI(const string& _Name, const string& _ID);
	~EditorUI();
};

