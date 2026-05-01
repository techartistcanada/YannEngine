#include "pch.h"
#include "CLevelSaveLoad.h"

#include <CLevelMgr.h>
#include <CLevel.h>
#include <CLayer.h>
#include <CGameObject.h>
#include <components.h>
#include <CCollisionManager.h>
#include <CScript.h>
#include <CCollisionManager.h>

#include <CScriptMgr.h>

void CLevelSaveLoad::SaveLevel(CLevel* _Level, const wstring& _FilePath)
{
	FILE* pFile = nullptr;
	_wfopen_s(&pFile, _FilePath.c_str(), L"wb");

	if (nullptr == pFile)
	{
		MessageBox(nullptr, L"Failed to open file for saving level.", L"Error", MB_OK | MB_ICONERROR);
	}

	// 1. save level name
	SaveWString(_Level->GetName(), pFile);

	// 2. save layers
	for (UINT i = 0; i < MAX_LAYER; ++i)
	{
		CLayer* pLayer = _Level->GetLayer(i);

		// 1. save layer name
		SaveWString(pLayer->GetName(), pFile);
		// 2. save all parent objects in this layer
		const vector<CGameObject*>& vecParents = pLayer->GetParentObjects();
		size_t NumObjects = vecParents.size();
		fwrite(&NumObjects, sizeof(size_t), 1, pFile); // save number of parent objects

		for (size_t j = 0; j < NumObjects; ++j)
		{
			SaveGameObject(vecParents[j],pFile);
		}
	}

	// 3. save collision infos
	CCollisionManager::GetInst()->SaveCollisionInfosToLevelFile(pFile);
	fclose(pFile);
}


CLevel* CLevelSaveLoad::LoadLevel(const wstring& _FilePath)
{
	FILE* pFile = nullptr;
	_wfopen_s(&pFile, _FilePath.c_str(), L"rb");

	if (nullptr == pFile)
	{
		MessageBox(nullptr, L"Failed to open file for loading level.", L"Error", MB_OK | MB_ICONERROR);
		return nullptr;
	}

	CLevel* pLevel = new CLevel;
	// 1. load level name
	wstring LevelName; 
	LoadWString(LevelName, pFile);
	pLevel->SetName(LevelName);
	// 2. load layers
	for (UINT i = 0; i < MAX_LAYER; ++i)
	{
		CLayer* pLayer = pLevel->GetLayer(i);
		// 1. load layer name
		wstring LayerName;
		LoadWString(LayerName, pFile);
		pLayer->SetName(LayerName);
		// 2. load all parent objects in this layer
		size_t NumObjects = 0;
		fread(&NumObjects, sizeof(size_t), 1, pFile); // load number of parent objects
		for (size_t j = 0; j < NumObjects; ++j)
		{
			CGameObject* pParentObj = LoadGameObject(pFile);
			pLayer->AddObject(pParentObj);
		}
	}

	// 3. load collision infos
	CCollisionManager::GetInst()->LoadCollisionInfosFromLevelFile(pFile);

	fclose(pFile);
	return pLevel;
}

// *************************************************
// Name: Save GameObject to Level File
//
// Description: This function saves a game object's name, components, and scripts to a level file. 
// It writes the data in a structured format so that it can be correctly loaded later.
// The function iterates through all components of the game object, saves their type and data,
// and then saves the attached scripts. 
//
// Parma: CGameObject* _Object - the game object to save
// Param: FILE* _pFile - the level file pointer to write to
// *************************************************
void CLevelSaveLoad::SaveGameObject(CGameObject* _Object, FILE* _File)
{
	// 1. save name
	SaveWString(_Object->GetName(), _File);


	// 2. save components
	for (UINT i = 0; i < (UINT)COMPONENT_TYPE::END; ++i)
	{
		CComponent* pComponent = _Object->GetComponent(COMPONENT_TYPE(i));

		if (nullptr == pComponent)
			continue;

		// 1. save component type
		COMPONENT_TYPE Type = pComponent->GetComponentType();
		fwrite(&Type, sizeof(UINT), 1, _File); // write component type
		// 2. save component data
		SaveWString(pComponent->GetName(), _File); 
		pComponent->SaveToLevelFile(_File);
	}

	// 3. mark the end of components
	COMPONENT_TYPE EndType = COMPONENT_TYPE::END;
	fwrite(&EndType, sizeof(UINT), 1, _File);

	// 4. save scripts
	const vector<CScript*>& vecScripts = _Object->GetScripts();
	size_t NumScripts = vecScripts.size();
	fwrite(&NumScripts, sizeof(size_t), 1, _File); // save number of scripts
	for (size_t i = 0; i < NumScripts; ++i)
	{
		wstring ScriptName = CScriptMgr::GetScriptName(vecScripts[i]);
		SaveWString(ScriptName, _File); // save script name
		vecScripts[i]->SaveToLevelFile(_File); // save script data
	}

	// 5. recursively save child objects
	const vector<CGameObject*>& vecChildren = _Object->GetChildren();
	size_t NumChildren = vecChildren.size();
	fwrite(&NumChildren, sizeof(size_t), 1, _File); // save number of child objects
	for (size_t i = 0; i < NumChildren; ++i)
	{
		SaveGameObject(vecChildren[i], _File);
	}
}

CGameObject* CLevelSaveLoad::LoadGameObject(FILE* _File)
{
	CGameObject* pObject = new CGameObject;
	// 1. load name
	wstring ObjectName;
	LoadWString(ObjectName, _File);
	pObject->SetName(ObjectName);

	// 2. load components
	COMPONENT_TYPE Type = COMPONENT_TYPE::END;
	CComponent* pComponent = nullptr;
	while (true)
	{
		fread(&Type, sizeof(UINT), 1, _File);

		if (COMPONENT_TYPE::END == Type)
			break; // end marker for components

		switch (Type)
		{
			case COMPONENT_TYPE::TRANSFORM:
			{
				pComponent = new CTransform;
			}
			break;
			case COMPONENT_TYPE::CAMERA:
			{
				pComponent = new CCamera;
			}
			break;
			case COMPONENT_TYPE::COLLIDER2D:
			{
				pComponent = new CCollider2D;
			}
			break;
			case COMPONENT_TYPE::COLLIDER3D:
			{
			}
			break;
			case COMPONENT_TYPE::ANIMATOR2D:
			{
				pComponent = new CAnimator2D;
			}
			break;
			case COMPONENT_TYPE::ANIMATOR3D:
			{
			}
			break;
			case COMPONENT_TYPE::LIGHT2D:
			{
				pComponent = new CLight2D;
			}
			break;
			case COMPONENT_TYPE::LIGHT3D:
			{
			}
			break;
			case COMPONENT_TYPE::MESHRENDERER:
			{
				pComponent = new CMeshRenderer;
			}
			break;
			case COMPONENT_TYPE::DECAL:
			{
			}
			break;
			case COMPONENT_TYPE::PARTICLESYSTEM:
			{
				pComponent = new CParticleSystem;
			}
			break;
			case COMPONENT_TYPE::TILEMAP:
			{
			}
			break;
			case COMPONENT_TYPE::LANDSCAPE:
			{
			}
			break;
		}
		wstring ComponentName;
		LoadWString(ComponentName, _File); 
		pComponent->SetName(ComponentName);
		pObject->AddComponent(pComponent);
		pComponent->LoadFromLevelFile(_File);
	}

	// 3. load scripts
	size_t NumScripts = 0;
	fread(&NumScripts, sizeof(size_t), 1, _File); // load number of scripts
	for (size_t i = 0; i < NumScripts; ++i)
	{
		wstring ScriptName;
		LoadWString(ScriptName, _File); // load script name
		CScript* pScript = CScriptMgr::GetScript(ScriptName);
		pObject->AddComponent(pScript);
		pScript->LoadFromLevelFile(_File);
	}

	// 4. recursively load child objects
	size_t NumChildren = 0;
	fread(&NumChildren, sizeof(size_t), 1, _File); // load number of child objects
	for (size_t i = 0; i < NumChildren; ++i)
	{
		CGameObject* pChild = LoadGameObject(_File);
		pObject->AddChild(pChild);
	}

	return pObject;
}
