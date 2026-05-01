#include "pch.h"
#include "ParameterUI.h"

#include "ImGui/imgui.h"
#include "CImGuiMgr.h"
#include "ListUI.h"

UINT			ParameterUI::g_ParameterUI_ID = 0;
EditorUI* ParameterUI::g_CallerUI = nullptr;
UI_DELEGATE_1	ParameterUI::g_Delegate_1 = nullptr;

int ParameterUI::Param_DragInt(const string& _strName, int* _pInOut, int _Speed)
{
	ImGui::Text(_strName.c_str());
	ImGui::SameLine(100);

	char key[255] = {};
	sprintf_s(key, 255, "##DragInt_%d", g_ParameterUI_ID++);

	if (ImGui::DragInt(key, _pInOut, (float)_Speed))
	{
		return true;
	}


    return false;
}

int ParameterUI::Param_DragFloat(const string& _strName, float* _pInOut, float _Speed)
{
	ImGui::Text(_strName.c_str());
	ImGui::SameLine(100);

	char key[255] = {};
	sprintf_s(key, 255, "##DragFloat_%d", g_ParameterUI_ID++);
	if (ImGui::DragFloat(key, _pInOut, _Speed))
	{
		return true;
	}


	return false;
}

int ParameterUI::Param_DragVec2(const string& _strName, Vec2* _pInOut, float _Speed)
{
	ImGui::Text(_strName.c_str());
	ImGui::SameLine(100);

	char key[255] = {};
	sprintf_s(key, 255, "##DragVec2_%d", g_ParameterUI_ID++);
	float arr[2] = { _pInOut->x, _pInOut->y };
	if (ImGui::DragFloat2(key, arr, _Speed))
	{
		_pInOut->x = arr[0];
		_pInOut->y = arr[1];
		return true;
	}


	return false;
}

int ParameterUI::Param_DragVec4(const string& _strName, Vec4* _pInOut, float _Speed)
{
	ImGui::Text(_strName.c_str());
	ImGui::SameLine(100);

	char key[255] = {};
	sprintf_s(key, 255, "##DragFloat4_%d", g_ParameterUI_ID++);
	if (ImGui::DragFloat4(key, *_pInOut, _Speed))
	{
		return true;
	}


	return false;
}

int ParameterUI::Param_DragMatrix(const string& _strName, Matrix* _pInOut, float _Speed)
{
	return false;
}

int ParameterUI::Param_Checkbox(const string& _strName, int* _pInOut)
{
	ImGui::Text(_strName.c_str());
	ImGui::SameLine(100);

	char key[255] = {};
	sprintf_s(key, 255, "##Checkbox_%d", g_ParameterUI_ID++);

	bool bChecked = (*_pInOut != 0);
	if (ImGui::Checkbox(key, &bChecked))
	{
		*_pInOut = bChecked ? 1 : 0;
		return true;
	}

	return false;
}

int ParameterUI::Param_Texture(const string& _strName, Ptr<CTexture>& _pTex)
{
	int selectBtnClicked = false;

	ImGui::Text(_strName.c_str());
	ImGui::SameLine(100);

	char key[255] = {};
	sprintf_s(key, 255, "##TextureImage%d", g_ParameterUI_ID++);

	// --------------------------
	// Texture Image
	// --------------------------

	// texture name
	string strTextureName;
	if (nullptr != _pTex)
		strTextureName = ToString(_pTex->GetKey());
	ImGui::SetNextItemWidth(200);
	ImGui::InputText(key, (char*)strTextureName.c_str(), strTextureName.capacity(), ImGuiInputTextFlags_ReadOnly);
	ImGui::SameLine();
	// Drag&Drop for texture selection
	// -------- BEGIN Receive Drop ------------------
	if(ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ContentTreeUI");

		if (nullptr != payload)
		{
			DWORD_PTR dwData = 0;
			memcpy(&dwData, payload->Data, payload->DataSize);
			Ptr<CAsset> pAsset = (CAsset*)dwData;

			if (pAsset->GetAssetType() == ASSET_TYPE::TEXTURE)
			{
				_pTex = (CTexture*)pAsset.Get();
			}
		}
		ImGui::EndDragDropTarget();
	}
	// -------- END ------------------

	// ListUI for texture selection
	sprintf_s(key, 255, "##SelectTextureBtn_%d", g_ParameterUI_ID++);
	if (ImGui::Button(key, ImVec2(24, 24)))
	{
		ListUI* pListUI = CImGuiMgr::GetInst()->FindEditorUI<ListUI>("List");
		if (nullptr != pListUI && !pListUI->IsActive())
		{
			pListUI->SetModal(true);
			pListUI->SetActive(true);
			pListUI->SetFocus();

			vector<string> vecTextureNames;
			CAssetMgr::GetInst()->GetAssetNamesByType(ASSET_TYPE::TEXTURE, vecTextureNames);
			pListUI->AddItems(vecTextureNames);
			if (g_CallerUI && g_Delegate_1)
			{
				pListUI->RegisterDBClickDelegate(g_CallerUI, g_Delegate_1);
			}

			g_CallerUI = nullptr;
			g_Delegate_1 = nullptr;
		}

		selectBtnClicked = true;
	}

	// texture image
	ImTextureID texID = (nullptr == _pTex) ? nullptr : _pTex->GetImGuiTextureID();
	ImVec4 BorderColor = (nullptr == _pTex) ? ImVec4(0.f,0.f,0.f,0.f) : ImGui::GetStyleColorVec4(ImGuiCol_Border);
	ImGui::Image(texID,
		ImVec2(100, 100),
		ImVec2(0, 0),
		ImVec2(1, 1),
		ImGui::GetStyleColorVec4(ImGuiCol_Text),
		BorderColor);

	return selectBtnClicked;
}
