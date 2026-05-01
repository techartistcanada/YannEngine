#pragma once
#include "EditorUI.h"

class MainMenuBarUI :
    public EditorUI
{
private:
	void FileMenu();
	void LevelMenu();
	void GameObjectMenu();
	void AssetMenu();
	void DebugViewsMenu();
private:
	wstring GetNewAssetDefaultName(wstring _BaseName);
public:
	virtual void tick() override;
	virtual void render_tick() override;
public:
	MainMenuBarUI();
	~MainMenuBarUI();
};

