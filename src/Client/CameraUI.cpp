#include "pch.h"
#include "CameraUI.h"

#include <CCamera.h>

#include <CRenderMgr.h>


CameraUI::CameraUI()
	: ComponentUI("Camera", "##CameraUI", COMPONENT_TYPE::CAMERA)
{
	SetSizeAsChild(ImVec2(0.0f, 230.0f));
}

CameraUI::~CameraUI()
{
}

void CameraUI::render_tick()
{
	render_title();

	// ------------------
	// Projection Type
	// ------------------
	ImGui::Text("Projection Type");
	ImGui::SameLine(100);

	CCamera* pCamera = GetTargetObject()->Camera();
	PROJ_TYPE  ProjType = pCamera->GetProjType();
	bool IsPerspective = (PROJ_TYPE::PERSPECTIVE == ProjType);
	bool IsOrthographic = (PROJ_TYPE::ORTHOGRAPHIC == ProjType);
	const char* arrProjType[] = { "Orthographic", "Perspective" };

	if (ImGui::BeginCombo("##CameraProjType", arrProjType[(UINT)ProjType], 0))
	{
		for (int i = 0; i < 2; ++i)
		{
			const bool is_selected = ((UINT)ProjType == i);
			if (ImGui::Selectable(arrProjType[i], is_selected))
			{
				pCamera->SetProjType((PROJ_TYPE)i);
			}
		}
		ImGui::EndCombo();
	}
	// ------------------
	// Current Camera Priority
	// ------------------
	ImGui::Text("Camera Priority");
	ImGui::SameLine(100);

	// change priority
	vector<CCamera*>& vecCams = CRenderMgr::GetInst()->GetRegisteredCameras();
	int CamPriority = pCamera->GetCameraPriority();
	if (ImGui::InputInt("##CameraPriority", &CamPriority))
	{
		int CurPriority = pCamera->GetCameraPriority();
		vecCams[CurPriority] = nullptr; // NOTE: 先把当前优先级位置的相机置空
		pCamera->SetCameraPriority(CamPriority);
	}

	// ------------------
	// List of Cameras in order of priority
	// ------------------
	ImGui::Text("All Cameras");
	ImGui::SameLine(100);
	vector<string> vecCamNames;
	for (size_t i = 0; i < vecCams.size(); ++i)
	{
		char buffer[256] = {};
		sprintf_s(buffer, 256, "Priority: %d: ", (int)i);
		string Name = buffer;

		if (nullptr == vecCams[i])
		{
			vecCamNames.push_back(Name + "None");
		}
		else
		{
			vecCamNames.push_back(Name + ToString(vecCams[i]->GetOwner()->GetName()));
		}
	}

	string CurCamName;
	char buffer[256] = {};
	sprintf_s(buffer, 256, "Priority: %d: ", CamPriority);
	CurCamName = buffer;
	CurCamName += ToString(pCamera->GetOwner()->GetName());
	if (ImGui::BeginCombo("##RegisteredCameras", CurCamName.c_str(), 0))
	{
		for (size_t i = 0; i < vecCamNames.size(); ++i)
		{
			const bool is_selected = (CurCamName == vecCamNames[i]);
			if (ImGui::Selectable(vecCamNames[i].c_str(), is_selected))
			{
				int a = 0;
			}
		}
		ImGui::EndCombo();
	}
	// ------------------
	// FOV
	// ------------------
	ImGui::Text("FOV");
	ImGui::SameLine(100);

	float FOV = pCamera->GetFOV();
	FOV = (FOV * 180.0f) / XM_PI; // NOTE: 把弧度转换成角度，方便用户理解

	ImGui::BeginDisabled(IsOrthographic); // NOTE: 如果是正交投影，就禁用FOV的编辑
	if (ImGui::DragFloat("##CameraFOV", &FOV, 0.1f, 0.1f, 180.f, "%.3f"))
	{
		FOV = (FOV * XM_PI) / 180.0f; // NOTE: 把角度转换成弧度，方便程序使用
		pCamera->SetFOV(FOV);
	}
	ImGui::EndDisabled();
	// ------------------
	// Far Plane
	// ------------------
	ImGui::Text("Far");
	ImGui::SameLine(100);

	float Far = pCamera->GetFar();
	if (ImGui::DragFloat("##CameraFar", &Far, 10.f, 2.f, 10000.f, "%.3f"))
	{
		if (Far < 2.f)
			Far = 2.f;
		pCamera->SetFar(Far);
	}

	// ------------------
	// Width
	// ------------------
	ImGui::Text("Width");
	ImGui::SameLine(100);
	float Width = pCamera->GetWidth();
	if (ImGui::DragFloat("##CameraWidth", &Width, 1.f, 1.f, 5000.f, "%.3f"))
	{
		if (Width < 1.f)
			Width = 1.f;
		pCamera->SetWidth(Width);
	}
	// ------------------
	// Aspect Ratio
	// ------------------
	ImGui::Text("Aspect Ratio");
	ImGui::SameLine(100);
	float AspectRatio = pCamera->GetAspectRatio();
	if (ImGui::DragFloat("##CameraAspectRatio", &AspectRatio, 0.1f, 0.1f, 10.f, "%.3f"))
	{
		if (AspectRatio < 0.1f)
			AspectRatio = 0.1f;
		pCamera->SetAspectRatio(AspectRatio);
	}
	// ------------------
	// Scale
	// ------------------
	ImGui::BeginDisabled(IsPerspective);
	ImGui::Text("Scale");
	ImGui::SameLine(100);
	float Scale = pCamera->GetScale();
	if (ImGui::DragFloat("##CameraScale", &Scale, 0.1f, 0.1f, 100.f, "%.3f"))
	{
		if (Scale < 0.1f)
			Scale = 0.1f;
		pCamera->SetScale(Scale);
	}
	ImGui::EndDisabled();

	// ------------------
	// Layer Check
	// ------------------
	ImGui::Text("Layer Check");
	ImGui::SameLine();
}
