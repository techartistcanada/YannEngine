#include "pch.h"
#include "CEditorObject.h"


#include <CComponent.h>

void CEditorObject::finaltick()
{
	// Components finaltick
	for (UINT i = 0; i < (UINT)COMPONENT_TYPE::END; ++i)
	{
		if (nullptr != GetComponent(COMPONENT_TYPE(i)))
			GetComponent(COMPONENT_TYPE(i))->finaltick();
	}

	 
	// children finaltick
	const vector<CGameObject*>& vecChildren = GetChildren();
	for (size_t i = 0; i < vecChildren.size(); ++i)
	{
		vecChildren[i]->finaltick();
	}
}
