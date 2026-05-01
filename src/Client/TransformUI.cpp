#include "pch.h"
#include "TransformUI.h"

#include <CTransform.h>


TransformUI::TransformUI()
	: ComponentUI("TransformUI", "##TransformUI", COMPONENT_TYPE::TRANSFORM)
{
	SetSizeAsChild(ImVec2(0.0f, 200.0f));
}

TransformUI::~TransformUI()
{
}

void TransformUI::render_tick()
{
	render_title();

	Vec3 vPos = GetTargetObject()->Transform()->GetRelativePos();
	Vec3 vScale = GetTargetObject()->Transform()->GetRelativeScale();
	Vec3 vRotation = GetTargetObject()->Transform()->GetRelativeRotation();

	// ----------
	// Position
	// ----------
	ImGui::Text("Position   ");
	ImGui::SameLine();
	if (ImGui::DragFloat3("##Position", vPos))
		GetTargetObject()->Transform()->SetRelativePos(vPos);

	// ----------
	// Scale
	// ----------
	ImGui::Text("Scale       ");
	ImGui::SameLine();
	if (ImGui::DragFloat3("##Scale", vScale))
		GetTargetObject()->Transform()->SetRelativeScale(vScale);

	// ----------
	// Rotation
	// ----------
	ImGui::Text("Rotation  ");
	ImGui::SameLine();
	vRotation = (vRotation / XM_PI) * 180.0f;	// 转换为角度制显示
	if (ImGui::DragFloat3("##Rotation", vRotation))
	{
		vRotation = (vRotation / 180.0f) * XM_PI;	// 转换回弧度制存储
		GetTargetObject()->Transform()->SetRelativeRotation(vRotation);
	}
	// ----------
	// Absolute
	// ----------
	bool Absolute = GetTargetObject()->Transform()->IsAbsolute();
	ImGui::Text("Absolute ");
	ImGui::SameLine();
	if (ImGui::Checkbox("##Absolute", &Absolute))
		GetTargetObject()->Transform()->SetAbsolute(Absolute);

}
