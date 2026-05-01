#include "pch.h"
#include "ScriptUI.h"

#include <CScriptMgr.h>
#include <CScript.h>

#include "ParameterUI.h"


ScriptUI::ScriptUI()
	: ComponentUI("Script", "##ScriptUI", COMPONENT_TYPE::SCRIPT)
	, m_TargetScript(nullptr)
{
}

ScriptUI::~ScriptUI()
{
}

void ScriptUI::SetTargetScript(CScript* _Target)
{
	m_TargetScript = _Target;

	string strScriptName = ToString(CScriptMgr::GetScriptName(m_TargetScript));

	SetDisplayName(strScriptName.c_str());
}


void ScriptUI::render_scriptname()
{
	ImGui::PushID(0);
	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(2.f / 7.f, 0.6f, 0.6f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(2.f / 7.f, 0.6f, 0.6f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(2.f / 7.f, 0.6f, 0.6f));


	wstring ScriptName = CScriptMgr::GetScriptName(m_TargetScript);
	string strScriptName = ToString(ScriptName);
	std::string label = std::string(ICON_FA_FILE_CODE) + "  " + strScriptName;
	ImGui::Button(label.c_str());

	ImGui::PopStyleColor(3);
	ImGui::PopID();
}

void ScriptUI::render_tick()
{
	render_scriptname();

	const vector<tScriptParam>& vecParams = m_TargetScript->GetScriptParams();
	for (size_t i = 0; i < vecParams.size(); ++i)
	{
		const tScriptParam& Param = vecParams[i];
		switch (Param.Type)
		{
		case SCRIPT_PARAM_TYPE::INT:
			break;
		case SCRIPT_PARAM_TYPE::FLOAT:
			ParameterUI::Param_DragFloat(Param.Desc.c_str(), (float*)Param.pData, 1);
			break;
		case SCRIPT_PARAM_TYPE::VEC2:
			break;
		case SCRIPT_PARAM_TYPE::VEC3:
			break;
		case SCRIPT_PARAM_TYPE::VEC4:
			break;
		case SCRIPT_PARAM_TYPE::TEXTURE:
			break;
		default:
			break;
		}
	}
}
