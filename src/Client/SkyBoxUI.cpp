#include "pch.h"
#include "SkyBoxUI.h"

#include <CSkyBox.h>


SkyBoxUI::SkyBoxUI()
	: ComponentUI("SkyBox", "##SkyBoxUI", COMPONENT_TYPE::SKYBOX)
{
}

SkyBoxUI::~SkyBoxUI()
{
}

void SkyBoxUI::render_tick()
{
	render_title();
	
	// ============================
	// SkyBox Type
	// ============================
	ImGui::Text("SkyBox Type");
	ImGui::SameLine(100);

	CSkyBox* pSkyBox = GetTargetObject()->SkyBox();
	SKYBOX_TYPE Type = pSkyBox->GetSkyBoxType();

	const char* arrSkyBoxType[] = { "Sphere", "Cube" };
	if (ImGui::BeginCombo("##SkyBoxType", arrSkyBoxType[(UINT)Type], 0))
	{
		for (int i = 0; i < 2; ++i)
		{
			const bool is_selected = ((UINT)Type == i);
			if (ImGui::Selectable(arrSkyBoxType[i], is_selected))
			{
				pSkyBox->SetSkyBoxType((SKYBOX_TYPE)i);
			}
		}
		ImGui::EndCombo();
	}

	// ============================
	// SkyBox Texture
	// ============================
	ImGui::Text("SkyBox Texture");
	ImGui::SameLine(100);
	
	Ptr<CTexture> pTexture = pSkyBox->GetSkyBoxTexture();
	ImTextureID TextureID = 0;
	string strTextureKey;
	if (nullptr != pTexture.Get())
	{
		TextureID = (ImTextureID)pTexture->GetImGuiTextureID();
		strTextureKey = ToString(pTexture->GetKey().c_str());
	}
	else
	{
		strTextureKey = "None";
	}
	ImGui::InputText("##SkyBoxTexture", (char*)strTextureKey.c_str(), strTextureKey.capacity(), ImGuiInputTextFlags_ReadOnly);

	// Thumnail
	ImVec2 uv_min = ImVec2(0.0f, 0.0f);
	ImVec2 uv_max = ImVec2(1.0f, 1.0f);
	
	ImVec4 border_color = ImGui::GetStyleColorVec4(ImGuiCol_Border);
	ImGui::Image(TextureID, ImVec2(150, 150), uv_min, uv_max, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), border_color);

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
				pSkyBox->SetSkyBoxTexture((CTexture*)pAsset.Get());
			}
		}
		ImGui::EndDragDropTarget();
	}
	// -------- END ------------------

	// ============================
	// Visibility Toggle
	// ============================
	bool bVisible = pSkyBox->IsVisible();
	if (ImGui::Checkbox("##SkyBoxVisible", &bVisible))
		pSkyBox->SetVisible(bVisible);

	// ============================
	// Environment Rotation
	// ============================
	ImGui::Text("Rotation");
	ImGui::SameLine(100);
	float fRotDeg = pSkyBox->GetRotationY() * (180.f / 3.14159265f);
	if (ImGui::SliderFloat("##EnvRotation", &fRotDeg, 0.f, 360.f, "%.1f deg"))
	{
		pSkyBox->SetRotationY(fRotDeg * (3.14159265f / 180.f));
	}

		// ============================
	// [3] Exposure
	// ============================
	ImGui::Text("Exposure");
	ImGui::SameLine(100);
	float fExposure = pSkyBox->GetExposure();
	if (ImGui::DragFloat("##Exposure", &fExposure, 0.01f, 0.0f, 10.0f, "%.2f"))
	{
		pSkyBox->SetExposure(fExposure);
	}

	// ============================
	// [4] Roughness Override
	// ============================
	float fRoughness = pSkyBox->GetRoughnessOverride();
	bool bRoughnessOverride = (fRoughness >= 0.f);
	if (ImGui::Checkbox("Roughness Override##SkyBox", &bRoughnessOverride))
	{
		pSkyBox->SetRoughnessOverride(bRoughnessOverride ? 0.5f : -1.f);
		fRoughness = pSkyBox->GetRoughnessOverride();
	}
	if (bRoughnessOverride)
	{
		ImGui::SameLine();
		ImGui::SetNextItemWidth(150.f);
		if (ImGui::SliderFloat("##RoughnessVal", &fRoughness, 0.f, 1.f, "%.2f"))
		{
			pSkyBox->SetRoughnessOverride(fRoughness);
		}
	}

}
