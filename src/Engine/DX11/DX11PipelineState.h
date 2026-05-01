#pragma once
#include "../RHI/IRHIPipelineState.h"
#include <d3d11.h>

class DX11PipelineState : public IRHIPipelineState
{
private:
    RHIGraphicsPipelineDesc m_Desc = {};

public:
    // DX11: Create 只存储 desc，不编译任何东西
    // RTVFormats / DSVFormat / NumRenderTargets 是 DX12 专用字段，这里忽略
    int Create(const RHIGraphicsPipelineDesc& desc) override;

    // 按顺序应用全部 DX11 状态：
    // Topology → InputLayout+Shaders → RSSetState → OMSetDepthStencilState → OMSetBlendState
    void Bind() override;

    const RHIGraphicsPipelineDesc& GetDesc() const override { return m_Desc; }

public:
    DX11PipelineState()  = default;
    ~DX11PipelineState() = default;
};