#include "pch.h"
#include "Collider2DUI.h"

#include <CCollider2D.h>


Collider2DUI::Collider2DUI()
	: ComponentUI("Collider2D", "##Collider2DUI", COMPONENT_TYPE::COLLIDER2D)
{
	SetSizeAsChild(ImVec2(0.0f, 200.0f));
}

Collider2DUI::~Collider2DUI()
{
}

void Collider2DUI::render_tick()
{
	render_title();

	CCollider2D* pCollider2D = GetTargetObject()->Collider2D();
	assert(pCollider2D);
	
	Vec3 vOffsetPos = pCollider2D->GetOffset();
	Vec3 vScale = pCollider2D->GetScale();
	bool bAbsolute = pCollider2D->IsAbsolute();

	float dragSpeed = 1.f;
	if(!bAbsolute)
		dragSpeed = 0.01f;
	// ----------
	// Offset Position
	// ----------
	ImGui::Text("Offset Position");
	ImGui::SameLine();
	if (ImGui::DragFloat3("##Collider2DOffsetPosition", vOffsetPos, dragSpeed))
	{
		pCollider2D->SetOffset(vOffsetPos);
	}

	// ----------
	// Scale
	// ----------
	ImGui::Text("Scale                ");
	ImGui::SameLine();
	if (ImGui::DragFloat3("##Collider2DScale", vScale, dragSpeed))
	{
		pCollider2D->SetScale(vScale);
	}

	// ----------
	// Absolute
	// ----------
	ImGui::Text("Absolute         ");
	ImGui::SameLine();
	if (ImGui::Checkbox("##Collider2DAbsolute", &bAbsolute))
	{
		pCollider2D->SetAbsolute(bAbsolute);
		if (!bAbsolute)
		{
			pCollider2D->SetOffset(Vec3(0.0f, 0.0f, 0.0f));
			pCollider2D->SetScale(Vec3(1.0f, 1.0f, 1.0f));
		}
	}
}
