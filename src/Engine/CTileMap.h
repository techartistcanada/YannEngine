#pragma once
#include "CRenderComponent.h"
class CTileMap :
	public CRenderComponent
{

private:
	UINT m_Row;
	UINT m_Col;
public:
	virtual void finaltick() override;
	virtual void render() override;
public:
	CTileMap();
	~CTileMap();
};

