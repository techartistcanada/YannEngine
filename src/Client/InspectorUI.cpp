#include "pch.h"
#include "InspectorUI.h"

#include <CLevelMgr.h>
#include <CLevel.h>
#include <CGameObject.h>
#include <components.h>

// Component UIs
#include "TransformUI.h"
#include "MeshRendererUI.h"
#include "CameraUI.h"
#include "ParticleSystemUI.h"
#include "Collider2DUI.h"
#include "Light2DUI.h"
#include "Light3DUI.h"
#include "Animator2DUI.h"
#include "SkyBoxUI.h"

// Asset UIs
#include "TextureUI.h"
#include "MeshUI.h"
#include "MeshDataUI.h"
#include "MaterialUI.h"
#include "PrefabUI.h"
#include "SoundUI.h"
#include "GraphicShaderUI.h"
#include "ComputeShaderUI.h"

#include "ScriptUI.h"

InspectorUI::InspectorUI()
	: EditorUI("Inspector", "##Inspector")
	, m_TargetObject(nullptr)
	, m_arrComponentUI{}
{
	CreateComponentUIs();

	CreateAssetUIs();
}


InspectorUI::~InspectorUI()
{
}

void InspectorUI::render_tick()
{
}

void InspectorUI::SetTargetObject(CGameObject* _Target)
{
	if (nullptr != m_TargetAsset)
	{
		m_arrAssetUI[(UINT)m_TargetAsset->GetAssetType()]->SetActive(false);
		m_TargetAsset = nullptr;
	}

	m_TargetObject = _Target;
	for (UINT i = 0; i < (UINT)COMPONENT_TYPE::END; ++i)
	{
		if (nullptr == m_arrComponentUI[i])
			continue;

		m_arrComponentUI[i]->SetTargetObject(_Target);
	}

	// -------------------------
	// ScriptUIs
	// -------------------------
	if (nullptr == m_TargetObject)
	{
		for (size_t i = 0; i < m_vecScriptUIs.size(); ++i)
		{
			m_vecScriptUIs[i]->SetActive(false);
		}
	}
	else
	{
		const vector<CScript*>& vecScripts = m_TargetObject->GetScripts();

		if (m_vecScriptUIs.size() < vecScripts.size())
		{
			size_t iAddCount = vecScripts.size() - m_vecScriptUIs.size();
			for (size_t i = 0; i < iAddCount; ++i)
			{
				ScriptUI* pScriptUI = nullptr;
				pScriptUI = new ScriptUI;
				pScriptUI->SetActive(false);
				pScriptUI->SetSeperator(true);
				m_vecScriptUIs.push_back(pScriptUI);
				AddChildUI(pScriptUI);
			}
		}

		for (size_t i = 0; i < vecScripts.size(); ++i)
		{
			m_vecScriptUIs[i]->SetTargetScript(vecScripts[i]);
			m_vecScriptUIs[i]->SetActive(true);
		}
	}

}

void InspectorUI::SetTargetAsset(Ptr<CAsset> _Target)
{
	SetTargetObject(nullptr);

	m_TargetAsset = _Target;

	for (UINT i = 0; i < (UINT)ASSET_TYPE::END; ++i)
	{
		if (nullptr == m_arrAssetUI[i])
			continue;

		m_arrAssetUI[i]->SetActive(false);
	}

	if (nullptr == m_TargetAsset)
		return;

	ASSET_TYPE AssetType = m_TargetAsset->GetAssetType();
	m_arrAssetUI[(UINT)AssetType]->SetActive(true);
	m_arrAssetUI[(UINT)AssetType]->SetTargetAsset(_Target);
}

void InspectorUI::CreateComponentUIs()
{
	m_arrComponentUI[(UINT)COMPONENT_TYPE::TRANSFORM] = new TransformUI;
	m_arrComponentUI[(UINT)COMPONENT_TYPE::COLLIDER2D] = new Collider2DUI;
	m_arrComponentUI[(UINT)COMPONENT_TYPE::ANIMATOR2D] = new Animator2DUI;
	m_arrComponentUI[(UINT)COMPONENT_TYPE::CAMERA] = new CameraUI;
	m_arrComponentUI[(UINT)COMPONENT_TYPE::LIGHT2D] = new Light2DUI;
	m_arrComponentUI[(UINT)COMPONENT_TYPE::LIGHT3D] = new Light3DUI;
	m_arrComponentUI[(UINT)COMPONENT_TYPE::MESHRENDERER] = new MeshRendererUI;
	m_arrComponentUI[(UINT)COMPONENT_TYPE::SKYBOX] = new SkyBoxUI;
	m_arrComponentUI[(UINT)COMPONENT_TYPE::PARTICLESYSTEM] = new ParticleSystemUI;

	for (UINT i = 0; i < (UINT)COMPONENT_TYPE::END; ++i)
	{
		if (nullptr == m_arrComponentUI[i])
			continue;

		m_arrComponentUI[i]->SetActive(false);
		m_arrComponentUI[i]->SetSeperator(true);
		AddChildUI(m_arrComponentUI[i]);
	}

	// ---------------------
	// Script UIs
	// ---------------------
	ScriptUI* pScriptUI = nullptr;
	pScriptUI = new ScriptUI;
	pScriptUI->SetActive(false);
	pScriptUI->SetSeperator(true);

	m_vecScriptUIs.push_back(pScriptUI);
	AddChildUI(pScriptUI);
}


void InspectorUI::CreateAssetUIs()
{
	m_arrAssetUI[(UINT)ASSET_TYPE::MATERIAL] = new MaterialUI;
	m_arrAssetUI[(UINT)ASSET_TYPE::GRAPHICS_SHADER] = new GraphicShaderUI;
	m_arrAssetUI[(UINT)ASSET_TYPE::COMPUTE_SHADER] = new ComputeShaderUI;
	m_arrAssetUI[(UINT)ASSET_TYPE::MESH] = new MeshUI;
	m_arrAssetUI[(UINT)ASSET_TYPE::TEXTURE] = new TextureUI;
	m_arrAssetUI[(UINT)ASSET_TYPE::MESH_DATA] = new MeshDataUI;
	m_arrAssetUI[(UINT)ASSET_TYPE::PREFAB] = new PrefabUI;
	m_arrAssetUI[(UINT)ASSET_TYPE::SOUND] = new SoundUI;

	for (UINT i = 0; i < (UINT)ASSET_TYPE::END; ++i)
	{
		if (nullptr == m_arrAssetUI[i])
			continue;

		m_arrAssetUI[i]->SetActive(false);
		m_arrAssetUI[i]->SetSeperator(true);
		AddChildUI(m_arrAssetUI[i]);
	}
}
