#pragma once

#ifndef USE_DX11
#include <d3d12.h>
#include <wrl.h>
#endif

typedef void (*UI_CALLBACK)(void);
class EditorUI;
typedef UINT(EditorUI::* UI_DELEGATE)(void);
typedef UINT(EditorUI::* UI_DELEGATE_1)(DWORD_PTR);
typedef UINT(EditorUI::* UI_DELEGATE_2)(DWORD_PTR, DWORD_PTR);

class CImGuiMgr :
	public CSingleton<CImGuiMgr>
{
	SINGLE(CImGuiMgr)
private:
	HWND					m_hMainWnd;
	map<string, EditorUI*>  m_mapUI;
	bool                    m_bShowDemo; // imgui demo
	HANDLE					m_hNotify;

public:
	template<typename T>
	T* FindEditorUI(const string& _strName);
public:
	int init(HWND _hwnd);
	void tick();

private:
	static void ApplyEditorDarkTheme();
	void LoadCustomResources();
	void CreateEditorUI();
	void ObserveContentFolderChanges();
};

template<typename T>
inline T* CImGuiMgr::FindEditorUI(const string& _strName)
{
	map<string, EditorUI*>::iterator iter = m_mapUI.find(_strName);
	if (iter == m_mapUI.end())
		return nullptr;
	
	T* pUI = dynamic_cast<T*>(iter->second);

	return pUI;
}
