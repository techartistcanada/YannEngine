#include "pch.h"
#include "ComponentUI.h"

const char* COMPONENT_TYPE_ICONS[(UINT)COMPONENT_TYPE::END] =
{
    ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT, // TRANSFORM
    ICON_FA_CAMERA,                    // CAMERA
    ICON_FA_SQUARE,                    // COLLIDER2D
    ICON_FA_CUBE,                      // COLLIDER3D
    ICON_FA_FILM,                      // ANIMATOR2D
    ICON_FA_FILM,                      // ANIMATOR3D
    ICON_FA_LIGHTBULB,                 // LIGHT2D
    ICON_FA_LIGHTBULB,                 // LIGHT3D
	ICON_FA_VECTOR_SQUARE, 		       // BOUNDINGBOX

    ICON_FA_CUBE,                      // MESHRENDERER
    ICON_FA_CLOUD,                      // SKYBOX
    ICON_FA_STAMP,                     // DECAL
    ICON_FA_STAMP,                     // PARTICLESYSTEM
    ICON_FA_TABLE_CELLS,               // TILEMAP
    ICON_FA_MOUNTAIN,                  // LANDSCAPE
};

ComponentUI::ComponentUI(const string& _Name, const string& _ID, const COMPONENT_TYPE& _Type)
	: EditorUI(_Name, _ID)
	, m_TargetObject(nullptr)
	, m_Type(_Type)
{
}

ComponentUI::~ComponentUI()
{
}

void ComponentUI::SetTargetObject(CGameObject* _Target)
{

	m_TargetObject = _Target;
	if (nullptr == m_TargetObject)
	{
		SetActive(false);
	}
	else
	{
		if (nullptr != m_TargetObject->GetComponent(m_Type))
		{

			SetActive(true);
		}
		else
		{
			SetActive(false);
		}
	}
}

void ComponentUI::render_tick()
{
}

void ComponentUI::render_title()
{
	ImGui::PushID(0);
	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(2.f / 7.f, 0.6f, 0.6f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(2.f / 7.f, 0.6f, 0.6f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(2.f / 7.f, 0.6f, 0.6f));

	//ImGui::Button(COMPONENT_TYPE_STRINGS[(UINT)GetComponentType()]);
    const UINT type = (UINT)GetComponentType();
	const char* name = (type < (UINT)COMPONENT_TYPE::END)
		? COMPONENT_TYPE_STRINGS[type]
        : "UNKNOWN";

    const char* icon = (type < (UINT)COMPONENT_TYPE::END)
        ? COMPONENT_TYPE_ICONS[type]
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
