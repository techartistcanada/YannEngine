#include "pch.h"
#include "MeshDataUI.h"


MeshDataUI::MeshDataUI()
	: AssetUI("MeshData", "##MeshDataUI", ASSET_TYPE::MESH_DATA)
{
}

MeshDataUI::~MeshDataUI()
{
}

void MeshDataUI::render_tick()
{
	render_title();
}
