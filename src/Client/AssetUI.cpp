#include "pch.h"
#include "AssetUI.h"

static const char* ASSET_TYPE_ICONS[(int)ASSET_TYPE::END] =
{
    ICON_FA_CUBES,      // PREFAB
    ICON_FA_CUBE,       // MESH
    ICON_FA_CUBE,       // MESH_DATA
    ICON_FA_PALETTE,    // MATERIAL
    ICON_FA_IMAGE,      // TEXTURE
    ICON_FA_FILE_AUDIO,  // SOUND
    ICON_FA_BOLT,       // GRAPHICS_SHADER
    ICON_FA_MICROCHIP,  // COMPUTE_SHADER
};


AssetUI::AssetUI(const string& _Name, const string& _ID, const ASSET_TYPE& _Type)
	: EditorUI(_Name, _ID)
	, m_TargetAsset(nullptr)
	, m_AssetType(_Type)
{
}

AssetUI::~AssetUI()
{
}

void AssetUI::SetTargetAsset(Ptr<CAsset> _Target)
{
    if (m_TargetAsset == _Target)
        return;

	m_TargetAsset = _Target;
}

void AssetUI::render_title()
{
	ImGui::PushID(0);
	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(2.f / 7.f, 0.6f, 0.6f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(2.f / 7.f, 0.6f, 0.6f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(2.f / 7.f, 0.6f, 0.6f));

    const UINT type = (UINT)GetAssetType();
	const char* name = (type < (UINT)ASSET_TYPE::END)
		? ASSET_TYPE_STRINGS[type]
        : "UNKNOWN";

    const char* icon = (type < (UINT)ASSET_TYPE::END)
        ? ASSET_TYPE_ICONS[type]
        : ICON_FA_PUZZLE_PIECE;

	std::string label;
    label.reserve(strlen(icon) + 1 + strlen(name));
    label += icon;
    label += " ";
    label += name;

    ImGui::Button(label.c_str());

	ImGui::PopStyleColor(3);
	ImGui::PopID();
}
