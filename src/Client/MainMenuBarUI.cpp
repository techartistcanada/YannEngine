#include "pch.h"
#include "MainMenuBarUI.h"

#include <CEngine.h>

#include <CGameObject.h>
#include <CLevelMgr.h>
#include <CLevel.h>
#include <CLayer.h>
#include <components.h>
#include <CPathMgr.h>
#include <CRenderMgr.h>
#include <CScript.h>

#include <CScriptMgr.h>

#include "InspectorUI.h"
#include "ContentUI.h"

#include "CLevelSaveLoad.h"


MainMenuBarUI::MainMenuBarUI()
	: EditorUI("MenuBar", "##MenuBar")
{
}

MainMenuBarUI::~MainMenuBarUI()
{
}


void MainMenuBarUI::tick()
{
	if (ImGui::BeginMainMenuBar())
	{

		FileMenu();
		LevelMenu();
		GameObjectMenu();
		AssetMenu();
		DebugViewsMenu();

		ImGui::EndMainMenuBar();
	}
}

void MainMenuBarUI::render_tick() {}

void MainMenuBarUI::FileMenu()
{
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("Exit", "Alt+F4"))
		{
			int a = 100;
			DestroyWindow(CEngine::GetInst()->GetMainWnd());
		}

		ImGui::EndMenu();
	}
}

void MainMenuBarUI::LevelMenu()
{
	if (ImGui::BeginMenu("Level"))
	{
		// ---------------------------------------------
		// Save Level
		// ---------------------------------------------
		if (ImGui::MenuItem("Save Level", ""))
		{
			wchar_t Buffer[255] = {};
			wstring strInitialDir = CPathMgr::GetInst()->GetContentPath();
			wcscpy_s(Buffer, 255, strInitialDir.c_str());

			OPENFILENAME ofn = {};
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = nullptr;
			ofn.lpstrFile = Buffer;
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = 255;
			ofn.lpstrFilter = L"Level\0*.lv\0All\0*.*";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;

			//ofn.lpstrInitialDir = strInitialDir.c_str();
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
			if (GetSaveFileName(&ofn))
			{
				CLevel* pCurrentLevel = CLevelMgr::GetInst()->GetCurrentLevel();
				if (nullptr != pCurrentLevel)
				{
					CLevelSaveLoad::SaveLevel(pCurrentLevel, Buffer);
				}
			}


		}

		// ---------------------------------------------
		// Load Level
		// ---------------------------------------------
		if (ImGui::MenuItem("Load Level", ""))
		{

				wchar_t Buffer[255] = {};
				wstring strInitialDir = CPathMgr::GetInst()->GetContentPath();
				wcscpy_s(Buffer, 255, strInitialDir.c_str());

				OPENFILENAME ofn = {};
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = nullptr;
				ofn.lpstrFile = Buffer;
				ofn.lpstrFile[0] = '\0';
				ofn.nMaxFile = 255;
				ofn.lpstrFilter = L"Level\0*.lv\0All\0*.*";
				ofn.nFilterIndex = 1;
				ofn.lpstrFileTitle = NULL;
				ofn.nMaxFileTitle = 0;

				//ofn.lpstrInitialDir = strInitialDir.c_str();
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
				if (GetOpenFileName(&ofn))
				{
					CLevel* pLoadedLevel = CLevelSaveLoad::LoadLevel(Buffer);
					ChangeLevel(pLoadedLevel, LEVEL_STATE::STOP);
				}
		}

		ImGui::Separator();

		CLevel* pCurrentLevel = CLevelMgr::GetInst()->GetCurrentLevel();
		bool IsPlayState = pCurrentLevel->GetState() == LEVEL_STATE::PLAY;
		bool IsPauseState = pCurrentLevel->GetState() == LEVEL_STATE::PAUSE;
		bool IsStopState = pCurrentLevel->GetState() == LEVEL_STATE::STOP;

		// ---------------------------------------------
		// Play
		// ---------------------------------------------
		ImGui::BeginDisabled(IsPlayState);
		if (ImGui::MenuItem("Play"))
		{
			ChangeLevelState(LEVEL_STATE::PLAY);
			if (IsStopState)
			{
				CLevel* pLevel = CLevelMgr::GetInst()->GetCurrentLevel();
				wstring LevelPath = CPathMgr::GetInst()->GetContentPath();
				LevelPath += L"level//temp.lv";
				CLevelSaveLoad::SaveLevel(pLevel, LevelPath);
			}
		}
		ImGui::EndDisabled();

		// ---------------------------------------------
		// Pause
		// ---------------------------------------------
		ImGui::BeginDisabled(!IsPlayState);
		if (ImGui::MenuItem("Pause"))
		{
			ChangeLevelState(LEVEL_STATE::PAUSE);
		}
		ImGui::EndDisabled();

		// ---------------------------------------------
		// Stop
		// ---------------------------------------------
		ImGui::BeginDisabled(IsStopState);
		if (ImGui::MenuItem("Stop"))
		{
			wstring LevelPath = CPathMgr::GetInst()->GetContentPath();
			LevelPath += L"level//temp.lv";
			CLevel* pLoadedLevel = CLevelSaveLoad::LoadLevel(LevelPath);
			ChangeLevel(pLoadedLevel, LEVEL_STATE::STOP);
		}
		ImGui::EndDisabled();

		ImGui::EndMenu();
	}
}

void MainMenuBarUI::GameObjectMenu()
{
	if (ImGui::BeginMenu("GameObject"))
	{
		// --------------------------------------
		// Create Empty Object
		// --------------------------------------
		if (ImGui::MenuItem("Create Empty GameObject"))
		{
			CGameObject* pNewObj = new CGameObject;
			pNewObj->AddComponent(new CTransform);
			pNewObj->SetName(L"Empty GameObject");
			SpawnObject(0, pNewObj);

			InspectorUI* pInspector = CImGuiMgr::GetInst()->FindEditorUI<InspectorUI>("Inspector");
			pInspector->SetTargetObject(pNewObj);
		}

		if (ImGui::MenuItem("Save As Prefab", "Ctrl+N"))
		{
		}

		// --------------------------------------
		// Add Component
		// --------------------------------------
		if (ImGui::BeginMenu("Add Component"))
		{
			InspectorUI* pInspector = CImGuiMgr::GetInst()->FindEditorUI<InspectorUI>("Inspector");
			CGameObject* pTargetObject = pInspector->GetTargetObject();

			ImGui::BeginDisabled(nullptr == pTargetObject);
			for (UINT i = 0; i < (UINT)COMPONENT_TYPE::END; i++)
			{
				if (ImGui::MenuItem(COMPONENT_TYPE_STRINGS[i]))
				{
					if (nullptr != pTargetObject)
					{
						switch ((COMPONENT_TYPE)i)
						{
							case COMPONENT_TYPE::ANIMATOR2D:
							{
							}
							break;
							case COMPONENT_TYPE::ANIMATOR3D:
							{
							}
							break;
							case COMPONENT_TYPE::CAMERA:
							{
								pTargetObject->AddComponent(new CCamera);
							}
							break;
							case COMPONENT_TYPE::COLLIDER2D:
							{
								pTargetObject->AddComponent(new CCollider2D);
							}
							break;
							case COMPONENT_TYPE::COLLIDER3D:
							{
							}
							break;
							case COMPONENT_TYPE::DECAL:
							{
							}
							break;
							case COMPONENT_TYPE::LANDSCAPE:
							{
							}
							break;
							case COMPONENT_TYPE::LIGHT2D:
							{
								pTargetObject->AddComponent(new CLight2D);
							}
							break;
							case COMPONENT_TYPE::LIGHT3D:
							{
								pTargetObject->AddComponent(new CLight3D);
							}
							break;
							case COMPONENT_TYPE::MESHRENDERER:
							{
								pTargetObject->AddComponent(new CMeshRenderer);
							}
							break;
						}
					}
				}
			}
			ImGui::EndDisabled();

			pInspector->SetTargetObject(pTargetObject);
			ImGui::EndMenu();
		}
		// --------------------------------------
		// Add Scripts
		// --------------------------------------
		if (ImGui::BeginMenu("Add Script"))
		{
			InspectorUI* pInspector = CImGuiMgr::GetInst()->FindEditorUI<InspectorUI>("Inspector");
			CGameObject* pTargetObj = pInspector->GetTargetObject();

			ImGui::BeginDisabled(nullptr == pTargetObj);
			vector<wstring> vecScriptNames;
			CScriptMgr::GetAllScriptsNames(vecScriptNames);

			for (size_t i = 0; i < vecScriptNames.size(); ++i)
			{
				string scriptName = ToString(vecScriptNames[i]);
				if (ImGui::MenuItem(scriptName.c_str()))
				{
					CScript* pNewScript = CScriptMgr::GetScript(vecScriptNames[i]);
					if (nullptr != pNewScript)
					{
						pTargetObj->AddComponent(pNewScript);

						pInspector->SetTargetObject(pTargetObj);
						pInspector->SetFocus();
					}
				}
			}
			ImGui::EndDisabled();
			ImGui::EndMenu();
		}

		ImGui::EndMenu();
	}

}

void MainMenuBarUI::AssetMenu()
{
	if (ImGui::BeginMenu("Asset"))
	{
		if (ImGui::MenuItem("Create New Material"))
		{
			Ptr<CMaterial> pNewMaterial = new CMaterial;
			wstring strMaterialKey = GetNewAssetDefaultName(L"material//Default Material");
			strMaterialKey += L".mat";

			CAssetMgr::GetInst()->AddAsset<CMaterial>(strMaterialKey, pNewMaterial);

			wstring strFilePath = CPathMgr::GetInst()->GetContentPath();
			strFilePath += strMaterialKey;
			pNewMaterial->Save(strFilePath);


			ContentUI* pContentUI = CImGuiMgr::GetInst()->FindEditorUI<ContentUI>("Content");
			pContentUI->UpdateContent();
		}

		if (ImGui::MenuItem("Save Material"))
		{
		}

		ImGui::EndMenu();
	}
}

void MainMenuBarUI::DebugViewsMenu()
{
	if (ImGui::BeginMenu("Debug Views"))
	{
		if (ImGui::BeginMenu("GBuffer Views"))
		{
			Ptr<CTexture> pDebugViewRTTexture = nullptr;
			bool IsDebugView = false;

			if (CRenderMgr::GetInst()->IsDeferredDebugView())
			{
				pDebugViewRTTexture = CRenderMgr::GetInst()->GetDebugViewRTTexture();
			}

			// ===============================
			// GBuffer Color
			// ===============================
			if (nullptr != pDebugViewRTTexture && pDebugViewRTTexture->GetKey() == L"GBuffer_Color")
				IsDebugView = true;
			else
				IsDebugView = false;

			if (ImGui::MenuItem("GBuffer Color", "", &IsDebugView))
			{
				if(IsDebugView)
					CRenderMgr::GetInst()->SetDeferredDebugView(true, CAssetMgr::GetInst()->FindAsset<CTexture>(L"GBuffer_Color"));
				else
					CRenderMgr::GetInst()->SetDeferredDebugView(false, nullptr);
			}

			// ===============================
			// GBuffer Normal
			// ===============================
			if (nullptr != pDebugViewRTTexture && pDebugViewRTTexture->GetKey() == L"GBuffer_Normal")
				IsDebugView = true;
			else
				IsDebugView = false;

			if (ImGui::MenuItem("GBuffer Normal", "", &IsDebugView))
			{
				if(IsDebugView)
						CRenderMgr::GetInst()->SetDeferredDebugView(true, CAssetMgr::GetInst()->FindAsset<CTexture>(L"GBuffer_Normal"));
				else
						CRenderMgr::GetInst()->SetDeferredDebugView(false, nullptr);
			}

			// ===============================
			// GBuffer Position
			// ===============================
			if (nullptr != pDebugViewRTTexture && pDebugViewRTTexture->GetKey() == L"GBuffer_Position")
				IsDebugView = true;
			else
				IsDebugView = false;

			if (ImGui::MenuItem("GBuffer Position", "", &IsDebugView))
			{
				if(IsDebugView)
						CRenderMgr::GetInst()->SetDeferredDebugView(true, CAssetMgr::GetInst()->FindAsset<CTexture>(L"GBuffer_Position"));
				else
						CRenderMgr::GetInst()->SetDeferredDebugView(false, nullptr);
			}

			// ===============================
			// GBuffer Emissive
			// ===============================
			if (nullptr != pDebugViewRTTexture && pDebugViewRTTexture->GetKey() == L"GBuffer_Emmisive")
				IsDebugView = true;
			else
				IsDebugView = false;

			if (ImGui::MenuItem("GBuffer Emissive", "", &IsDebugView))
			{
				if(IsDebugView)
						CRenderMgr::GetInst()->SetDeferredDebugView(true, CAssetMgr::GetInst()->FindAsset<CTexture>(L"GBuffer_Emmisive"));
				else
						CRenderMgr::GetInst()->SetDeferredDebugView(false, nullptr);
			}
			// ===============================
			// GBuffer Specular
			// ===============================
			if (nullptr != pDebugViewRTTexture && pDebugViewRTTexture->GetKey() == L"GBuffer_Specular")
				IsDebugView = true;
			else
				IsDebugView = false;

			if (ImGui::MenuItem("GBuffer Specular", "", &IsDebugView))
			{
				if(IsDebugView)
						CRenderMgr::GetInst()->SetDeferredDebugView(true, CAssetMgr::GetInst()->FindAsset<CTexture>(L"GBuffer_Specular"));
				else
						CRenderMgr::GetInst()->SetDeferredDebugView(false, nullptr);
			}

			// ===============================
			// GBuffer Diffuse
			// ===============================
			if (nullptr != pDebugViewRTTexture && pDebugViewRTTexture->GetKey() == L"GBuffer_Diffuse")
				IsDebugView = true;
			else
				IsDebugView = false;

			if (ImGui::MenuItem("GBuffer Diffuse", "", &IsDebugView))
			{
				if(IsDebugView)
						CRenderMgr::GetInst()->SetDeferredDebugView(true, CAssetMgr::GetInst()->FindAsset<CTexture>(L"GBuffer_Diffuse"));
				else
						CRenderMgr::GetInst()->SetDeferredDebugView(false, nullptr);
			}

			// ===============================
			// GBuffer CustomData
			// ===============================
			if (nullptr != pDebugViewRTTexture && pDebugViewRTTexture->GetKey() == L"GBuffer_CustomData")
				IsDebugView = true;
			else
				IsDebugView = false;

			if (ImGui::MenuItem("GBuffer CustomData", "", &IsDebugView))
			{
				if(IsDebugView)
						CRenderMgr::GetInst()->SetDeferredDebugView(true, CAssetMgr::GetInst()->FindAsset<CTexture>(L"GBuffer_CustomData"));
				else
						CRenderMgr::GetInst()->SetDeferredDebugView(false, nullptr);
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenu();
	}

}

wstring MainMenuBarUI::GetNewAssetDefaultName(wstring _BaseName)
{
	_BaseName += L" %d";

	wchar_t szKey[255] = {};
	int i = 0;
	while (true)
	{
		wcscpy_s(szKey, 255, _BaseName.c_str());
		swprintf_s(szKey, 255, szKey, i++);
		if (nullptr == CAssetMgr::GetInst()->FindAsset<CMaterial>(szKey))
		{
			break;
		}
	}

	return szKey;
}
