#pragma once
#include "RHIPrereqs.h"
#include "../enum.h"

class IRHIGraphicsShader;

struct RHIGraphicsPipelineDesc
{
	IRHIGraphicsShader*    pShader = nullptr;

	RS_TYPE				   RasterizerState = RS_TYPE::CULL_BACK;
	DS_TYPE				   DepthStencilState = DS_TYPE::LESS;
	BS_TYPE				   BlendState = BS_TYPE::DEFAULT;
	RHI_PRIMITIVE_TOPOLOGY PrimitiveTopology = RHI_PRIMITIVE_TOPOLOGY::TRIANGLE_LIST;

	// DX12 only
	UINT				   NumRenderTargets = 1;
	DXGI_FORMAT            RTVFormats[8] = { DXGI_FORMAT_R8G8B8A8_UNORM };
	DXGI_FORMAT            DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
};


class IRHIPipelineState
{
public:
	virtual ~IRHIPipelineState() = default;

	virtual int Create(const RHIGraphicsPipelineDesc& _desc) = 0;
	virtual void Bind() = 0;

	virtual const RHIGraphicsPipelineDesc& GetDesc() const = 0;
};

