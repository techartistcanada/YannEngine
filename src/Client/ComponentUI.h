#pragma once
#include "EditorUI.h"

#include <CGameObject.h>

class ComponentUI :
    public EditorUI
{
private:
	CGameObject*		 m_TargetObject;
	const COMPONENT_TYPE m_Type;
public:
	void SetTargetObject(CGameObject* _Target);
	CGameObject* GetTargetObject() const { return m_TargetObject; }

	COMPONENT_TYPE GetComponentType() const { return m_Type; }
public:
	virtual void render_tick() = 0;
protected:
	void render_title();

public:
	ComponentUI(const string& _Name, const string& _ID, const COMPONENT_TYPE& _Type);
	~ComponentUI();
};

