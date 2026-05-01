#pragma once
#include "AssetUI.h"
class MaterialUI :
    public AssetUI
{
private:
	TEX_PARAM m_CurTargetParam;
public:
	virtual void render_tick() override;
	virtual void TargetAssetChanged() override;

	UINT SelectTexture(DWORD_PTR _Selected);
public:
	MaterialUI();
	~MaterialUI();
};

