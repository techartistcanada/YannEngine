#include "pch.h"
#include "ComputeShaderUI.h"


ComputeShaderUI::ComputeShaderUI()
	: AssetUI("ComputeShader", "##ComputeShaderUI", ASSET_TYPE::COMPUTE_SHADER)
{
}

ComputeShaderUI::~ComputeShaderUI()
{
}

void ComputeShaderUI::render_tick()
{
	render_title();
}
