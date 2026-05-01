#include "pch.h"
#include "TextureUI.h"


TextureUI::TextureUI()
	: AssetUI("Texture", "##TextureUI", ASSET_TYPE::TEXTURE)
{
}

TextureUI::~TextureUI()
{
}

void TextureUI::render_tick()
{
	render_title();

	Ptr<CTexture> pTexture = dynamic_cast<CTexture*>(GetTargetAsset().Get());
	assert(pTexture.Get());

	// Name
	ImGui::Text("Name");
	ImGui::SameLine(100);

	string strTexKey = ToString(GetTargetAsset()->GetKey());
	ImGui::InputText("##TextureNameTextureUI", (char*)strTexKey.c_str(), strTexKey.capacity(), ImGuiInputTextFlags_ReadOnly);

	// Thumnail
	ImVec2 uv_min = ImVec2(0.0f, 0.0f);
	ImVec2 uv_max = ImVec2(1.0f, 1.0f);
	
	ImVec4 border_color = ImGui::GetStyleColorVec4(ImGuiCol_Border);
	ImGui::Image((ImTextureID)pTexture->GetImGuiTextureID(), ImVec2(256, 256), uv_min, uv_max, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), border_color);

	// Resolution
	ImGui::Text("Resolution");
	int Width = (int)pTexture->GetWidth();
	int Height = (int)pTexture->GetHeight();

	ImGui::Text("Width");
	ImGui::SameLine(100);
	ImGui::InputInt("##TextureWidthTextureUI", &Width, 1, 100, ImGuiInputTextFlags_ReadOnly);

	ImGui::Text("Height");
	ImGui::SameLine(100);
	ImGui::InputInt("##TextureHeightTextureUI", &Height, 1, 100, ImGuiInputTextFlags_ReadOnly);
}
