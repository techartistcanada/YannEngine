#include "pch.h"
#include "Light3DUI.h"
#include <CLight3D.h>

Light3DUI::Light3DUI()
	: ComponentUI("Light3D", "##Light3DUI", COMPONENT_TYPE::LIGHT3D)
{
	SetSizeAsChild(ImVec2(0.0f, 200.0f));
}

Light3DUI::~Light3DUI()
{
}

void Light3DUI::render_tick()
{
	render_title();

	CLight3D* pLight3D = GetTargetObject()->Light3D();
	const tLightInfo& LightInfo = pLight3D->GetLightInfo();
	// ------------------
	// Light Type
	// ------------------
	ImGui::Text("Light Type");
	ImGui::SameLine(100);

	const char* arrLightType[] = { "Directional", "Point", "Spot" };
	if (ImGui::BeginCombo("##Light3DType", arrLightType[LightInfo.LightType], 0))
	{
		for (UINT i = 0; i < 3; ++i)
		{
			bool bSelected = (LightInfo.LightType == i);
			if (ImGui::Selectable(arrLightType[i], bSelected))
			{
				pLight3D->SetLightType((LIGHT_TYPE)i);
			}
		}
		ImGui::EndCombo();
	}
	// ------------------
	// Diffuse
	// ------------------
	ImGui::Text("Light Diffuse");
	ImGui::SameLine(100);
	if (ImGui::ColorEdit3("##Light3DDiffuse", (float*)&LightInfo.Light.vDiffuse, 0))
	{
		pLight3D->SetDiffuse(Vec3(LightInfo.Light.vDiffuse.x, LightInfo.Light.vDiffuse.y, LightInfo.Light.vDiffuse.z));
	}

	ImGui::Text("Light Ambient");
	ImGui::SameLine(100);
	if (ImGui::ColorEdit3("##Light3DAmbient", (float*)&LightInfo.Light.vAmbient, 0))
	{
		pLight3D->SetAmbient(Vec3(LightInfo.Light.vAmbient.x, LightInfo.Light.vAmbient.y, LightInfo.Light.vAmbient.z));
	}

	// ------------------
	// Range
	// ------------------
	ImGui::Text("Light Range");
	ImGui::SameLine(100);

	bool IsDirLight = (LightInfo.LightType == (UINT)LIGHT_TYPE::DIRECTIONAL);
	float fRange = LightInfo.Range;

	ImGui::BeginDisabled(IsDirLight);
	if (ImGui::DragFloat("##Light3DRange", &fRange, 1.f, 0.f, FLT_MAX))
	{
		pLight3D->SetRange(fRange);
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
	if (ImGui::DragFloat("##Light3DAngle", &fAngle, 1.f, 0.f, 180.f))
	{
		pLight3D->SetAngle(fAngle);
	}
	ImGui::EndDisabled();
	// ------------------
	// Draw Gizmo
	// ------------------
	ImGui::Text("Draw Gizmo");
	ImGui::SameLine(100);
	bool bDrawGizmo = pLight3D->GetDrawGizmo();
	if (ImGui::Checkbox("##Light3DDrawGizmo", &bDrawGizmo))
	{
		pLight3D->SetDrawGizmo(bDrawGizmo);
	}
}
