#pragma once
#include "EditorUI.h"
class ListUI :
    public EditorUI
{
private:
	vector<string> m_vecItems;
	int			   m_iSelectedIndex;

	UI_CALLBACK    m_Callback;
	EditorUI*      m_CallerUI;
	UI_DELEGATE_1  m_Delegate;

public:
	const string& GetSelectedItemStr() const { return m_vecItems[m_iSelectedIndex]; }

	void AddItem(const string& _Item) { m_vecItems.push_back(_Item); }
	void AddItems(const vector<string>& _vecItems) { m_vecItems.insert(m_vecItems.end(), _vecItems.begin(), _vecItems.end()); }

	void RegisterDBClickCallback(UI_CALLBACK _Callback) { m_Callback = _Callback; }
	void RegisterDBClickDelegate(EditorUI* _CallerUI, UI_DELEGATE_1 _Func)
	{
		m_CallerUI = _CallerUI;
		m_Delegate = _Func;
	}

public:
	virtual void render_tick() override;
	virtual void Activate() override;
	virtual void Deactivate() override;
public:
	ListUI();
	~ListUI();
};

