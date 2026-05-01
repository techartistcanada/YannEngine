#pragma once
#include "EditorUI.h"

class TreeUI;

class ContentUI :
    public EditorUI
{
private:
	TreeUI* m_TreeUI;
	vector<wstring> m_vecContentAssetPaths;
public:
	void UpdateContent();
	void ReloadContentToRAM();
private:
	void ScanAssetsInFolder(const wstring& _strFolderPath);
	UINT SelectAsset(DWORD_PTR _dwData);
	ASSET_TYPE GetAssetTypeFromExtension(const path& _strAssetPath);
public:
	virtual void render_tick() override;
public:
	ContentUI();
	~ContentUI();
};

