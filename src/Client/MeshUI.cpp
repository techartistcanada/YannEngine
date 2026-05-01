#include "pch.h"
#include "MeshUI.h"


MeshUI::MeshUI()
	: AssetUI("Mesh", "##MeshUI", ASSET_TYPE::MESH)
{
}

MeshUI::~MeshUI()
{
}

void MeshUI::render_tick()
{
	render_title();

	string strName = ToString(GetTargetAsset()->GetName());

	Ptr<CMesh> pMesh = dynamic_cast<CMesh*>(GetTargetAsset().Get());
	assert(pMesh.Get());
	
	// Mesh Name
	ImGui::Text("Mesh Name");
	ImGui::SameLine(100);
	ImGui::InputText("##MeshNameMeshUI", (char*)strName.c_str(), strName.capacity(), ImGuiInputTextFlags_ReadOnly);

	// Vertex Count
	int VtxCount = pMesh->GetVertexCount();
	ImGui::Text("Vertex Count");
	ImGui::SameLine(100);
	ImGui::InputInt("##MeshVertexCountMeshUI", &VtxCount, 1, 100, ImGuiInputTextFlags_ReadOnly);

	// Index Count
	int IdxCount = pMesh->GetIndexCount();
	ImGui::Text("Index Count");
	ImGui::SameLine(100);
	ImGui::InputInt("##MeshIndexCountMeshUI", &IdxCount, 1, 100, ImGuiInputTextFlags_ReadOnly);
}
