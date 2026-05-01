#pragma once
#include "AssetUI.h"

class MeshUI :
    public AssetUI
{
private:
public:
	virtual void render_tick() override;

public:
	MeshUI();
	~MeshUI();
};

