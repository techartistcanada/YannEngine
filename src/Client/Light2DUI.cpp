#include "pch.h"
#include "Light2DUI.h"

#include <CLight2D.h>


Light2DUI::Light2DUI()
	: ComponentUI("Light2D", "##Light2DUI", COMPONENT_TYPE::LIGHT2D)
{
	SetSizeAsChild(ImVec2(0.0f, 200.0f));
}

Light2DUI::~Light2DUI()
{
}

void Light2DUI::render_tick()
{
	render_title();

	CLight2D* pLight2D = GetTargetObject()->Light2D();
	const tLightInfo& LightInfo = pLight2D->GetLightInfo();

	// ------------------
	// Light Type
	// ------------------
	ImGui::Text("Light Type");
	ImGui::SameLine(100);

	const char* arrLightType[] = { "Directional", "Point", "Spot" };
	if (ImGui::BeginCombo("##LightType", arrLightType[LightInfo.LightType], 0))
	{
		for (UINT i = 0; i < 3; ++i)
		{
			bool bSelected = (LightInfo.LightType == i);
			if (ImGui::Selectable(arrLightType[i], bSelected))
			{
				pLight2D->SetLightType((LIGHT_TYPE)i);
			}
		}
		ImGui::EndCombo();
	}
	// ------------------
	// Color
	// ------------------
	ImGui::Text("Light Color");
	ImGui::SameLine(100);
	if (ImGui::ColorEdit3("##Light2DColor", (float*)&LightInfo.Light.vDiffuse, 0))
	{
		pLight2D->SetDiffuse(Vec3(LightInfo.Light.vDiffuse.x, LightInfo.Light.vDiffuse.y, LightInfo.Light.vDiffuse.z));
	}

	// ------------------
	// Range
	// ------------------
	ImGui::Text("Light Range");
	ImGui::SameLine(100);

	bool IsDirLight = (LightInfo.LightType == (UINT)LIGHT_TYPE::DIRECTIONAL);
	float fRange = LightInfo.Range;

	ImGui::BeginDisabled(IsDirLight);
	if (ImGui::DragFloat("##Light2DRange", &fRange, 1.f, 0.f, FLT_MAX))
	{
		pLight2D->SetRange(fRange);
	}
	ImGui::EndDisabled();

	// ------------------
	// Angle
	// ------------------
	ImGui::Text("SpotLight Angle");
	ImGui::SameLine(100);

	bool IsSpotLight = (LightInfo.LightType == (UINT)LIGHT_TYPE::SPOT);

	ImGui::BeginDisabled(!IsSpotLight);
	float fAngle = LightInfo.Angle;
	if (ImGui::DragFloat("##Light2DAngle", &fAngle, 1.f, 0.f, 180.f))
	{
		pLight2D->SetAngle(fAngle);
	}
	ImGui::EndDisabled();

}
