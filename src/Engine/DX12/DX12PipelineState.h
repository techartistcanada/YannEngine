#pragma once
#include "../RHI/IRHIPipelineState.h"
#include <d3d12.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

class DX12PipelineState : public IRHIPipelineState
{
private:
	RHIGraphicsPipelineDesc m_Desc = {};
	ComPtr<ID3D12PipelineState> m_PipelineState = nullptr;

public:
	int Create(const RHIGraphicsPipelineDesc& _Desc) override;
	void Bind() override;

	const RHIGraphicsPipelineDesc& GetDesc() const override { return m_Desc; }
	ID3D12PipelineState* GetD3D12PipelineState() const { return m_PipelineState.Get(); }

public:
	DX12PipelineState() = default;
	~DX12PipelineState() = default;
};
