#pragma once
#include "ComponentUI.h"
class Light3DUI :
    public ComponentUI
{
private:

public:
	virtual void render_tick() override;

public:
	Light3DUI();
	~Light3DUI();
};

