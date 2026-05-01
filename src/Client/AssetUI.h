#pragma once
#include "EditorUI.h"

class AssetUI :
    public EditorUI
{
private:
	Ptr<CAsset> 	m_TargetAsset;
	ASSET_TYPE      m_AssetType;
public:
	void SetTargetAsset(Ptr<CAsset> _Target);
	Ptr<CAsset> GetTargetAsset() const { return m_TargetAsset; }

	virtual void TargetAssetChanged() {}

	ASSET_TYPE GetAssetType() const { return m_AssetType; }
public:
	virtual void render_tick() = 0;
protected:
	void render_title();

public:
	AssetUI(const string& _Name, const string& _ID, const ASSET_TYPE& _Type);
	~AssetUI();
};

