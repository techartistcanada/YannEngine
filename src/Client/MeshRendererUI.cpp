#include "pch.h"
#include "MeshRendererUI.h"

#include <CMeshRenderer.h>
#include <CMesh.h>
#include <CMaterial.h>
#include <CAssetMgr.h>

#include "CImGuiMgr.h"
#include "ListUI.h"
#include "InspectorUI.h"




MeshRendererUI::MeshRendererUI()
	: ComponentUI("MeshRendererUI", "##MeshRendererUI", COMPONENT_TYPE::MESHRENDERER)
{
	SetSizeAsChild(ImVec2(0.0f, 200.0f));
}

MeshRendererUI::~MeshRendererUI()
{
}



void MeshRendererUI::render_tick()
{
	render_title();

	CMeshRenderer* pMeshRenderer = GetTargetObject()->MeshRenderer();

	Ptr<CMesh> pMesh = pMeshRenderer->GetMesh();
	Ptr<CMaterial> pMaterial = pMeshRenderer->GetMaterial();

	// ---------------------------------
	// Mesh
	// ---------------------------------
	string strMesh;
	if (nullptr != pMesh)
	{
		strMesh = string(pMesh->GetKey().begin(), pMesh->GetKey().end());
	}
	ImGui::Text("Mesh     ");
	ImGui::SameLine();
	ImGui::InputText("##MeshName", (char*)strMesh.c_str(), 255, ImGuiInputTextFlags_ReadOnly);
	ImGui::SameLine();

	// -------- BEGIN Receive Drop ------------------
	if(ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ContentTreeUI");

		if (nullptr != payload)
		{
			DWORD_PTR dwData = 0;
			memcpy(&dwData, payload->Data, payload->DataSize);
			Ptr<CAsset> pAsset = (CAsset*)dwData;

			if (pAsset->GetAssetType() == ASSET_TYPE::MESH)
			{
				Ptr<CMesh> pMesh = (CMesh*)pAsset.Get();
				GetTargetObject()->GetRenderComponent()->SetMesh(pMesh);
			}
		}
		ImGui::EndDragDropTarget();
	}
	// -------- END ------------------

	if (ImGui::Button(ICON_FA_FOLDER_OPEN "##MeshBtn", ImVec2(32, 32)))
	{
		ListUI* pListUI = CImGuiMgr::GetInst()->FindEditorUI<ListUI>("List");
		if (nullptr != pListUI && !pListUI->IsActive())
		{
			if(pListUI->SetActive(true))
			{
				pListUI->SetModal(true);
				pListUI->SetFocus();

				vector<string> vecMeshNames;
				CAssetMgr::GetInst()->GetAssetNamesByType(ASSET_TYPE::MESH, vecMeshNames);
				pListUI->AddItems(vecMeshNames);
				pListUI->RegisterDBClickDelegate(this, (UI_DELEGATE_1)&MeshRendererUI::SelectMesh);
			}
		}
	}

	// -----------
	// Material
	// -----------
	string strMaterial;
	if (nullptr != pMaterial)
	{
		strMaterial = string(pMaterial->GetKey().begin(), pMaterial->GetKey().end());
	}
	ImGui::Text("Material ");
	ImGui::SameLine();
	ImGui::InputText("##MaterialName", (char*)strMaterial.c_str(), 255, ImGuiInputTextFlags_ReadOnly);
	ImGui::SameLine();

	// -------- BEGIN Receive Drop ------------------
	if(ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ContentTreeUI");

		if (nullptr != payload)
		{
			DWORD_PTR dwData = 0;
			memcpy(&dwData, payload->Data, payload->DataSize);
			Ptr<CAsset> pAsset = (CAsset*)dwData;

			if (pAsset->GetAssetType() == ASSET_TYPE::MATERIAL)
			{
				Ptr<CMaterial> pMaterial = (CMaterial*)pAsset.Get();
				GetTargetObject()->GetRenderComponent()->SetMaterial(pMaterial);
			}
		}

		ImGui::EndDragDropTarget();
	}
	// -------- END ------------------
	if (ImGui::Button(ICON_FA_FOLDER_OPEN "##MaterialBtn", ImVec2(32, 32)))
	{
		ListUI* pListUI = CImGuiMgr::GetInst()->FindEditorUI<ListUI>("List");
		if (nullptr != pListUI)
		{
			if (pListUI->SetActive(true))
			{
				pListUI->SetFocus();

				vector<string> vecMaterialNames;
				CAssetMgr::GetInst()->GetAssetNamesByType(ASSET_TYPE::MATERIAL, vecMaterialNames);
				pListUI->AddItems(vecMaterialNames);
				pListUI->RegisterDBClickDelegate(this, (UI_DELEGATE_1)&MeshRendererUI::SelectMaterial);
			}
		}
	}
}

// *****************************************
// Select Material
// *****************************************
UINT MeshRendererUI::SelectMaterial(DWORD_PTR _Selected)
{
	// Get Selected Material
	string* pStrSelected = (string*)_Selected;
	wstring strSelected = ToWString(*pStrSelected);
	Ptr<CMaterial> pMaterial = CAssetMgr::GetInst()->FindAsset<CMaterial>(strSelected);

	assert(nullptr != pMaterial && nullptr != GetTargetObject());
	GetTargetObject()->GetRenderComponent()->SetMaterial(pMaterial);

	SetFocus();
	return 0;
}

// *****************************************
// Select Mesh
// *****************************************
UINT MeshRendererUI::SelectMesh(DWORD_PTR _Selected)
{
	// -----------------
	// Get Selected Mesh
	// -----------------
	string* pStrSelected = (string*)_Selected;
	wstring strSelected = ToWString(*pStrSelected);
	Ptr<CMesh> pMesh = CAssetMgr::GetInst()->FindAsset<CMesh>(strSelected);

	assert(nullptr != GetTargetObject() && nullptr != pMesh);

	GetTargetObject()->GetRenderComponent()->SetMesh(pMesh);

	SetFocus();
	return 0;
}
