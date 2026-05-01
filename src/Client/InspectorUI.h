#pragma once
#include "EditorUI.h"

class CGameObject;
class ComponentUI;

class CAsset;
class AssetUI;
class ScriptUI;

class InspectorUI :
    public EditorUI
{
private:
	CGameObject*		m_TargetObject;
	ComponentUI*		m_arrComponentUI[(UINT)COMPONENT_TYPE::END];
	vector<ScriptUI*>	m_vecScriptUIs;

	Ptr<CAsset> 	m_TargetAsset;
	AssetUI*		m_arrAssetUI[(UINT)ASSET_TYPE::END];

public:
	virtual void render_tick() override;

public:
	void SetTargetObject(CGameObject* _Target);
	CGameObject* GetTargetObject() const { return m_TargetObject; }

	void SetTargetAsset(Ptr<CAsset> _Target);
	Ptr<CAsset> GetTargetAsset() { return m_TargetAsset; }

private:
	void CreateComponentUIs();
	void CreateAssetUIs();


public:
	InspectorUI();
	~InspectorUI();
};

