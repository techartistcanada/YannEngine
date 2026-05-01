#include "../pch.h"
#ifdef USE_DX11

#include "DX11PipelineState.h"
#include "DX11GraphicsShader.h"
#include "DX11Device.h"

int DX11PipelineState::Create(const RHIGraphicsPipelineDesc& desc)
{
    m_Desc = desc;
    return S_OK;
}

void DX11PipelineState::Bind()
{
    // 1. Primitive Topology
    CONTEXT->IASetPrimitiveTopology(
        static_cast<D3D_PRIMITIVE_TOPOLOGY>(m_Desc.PrimitiveTopology));

    // 2. InputLayout + Shader stages (VS/HS/DS/GS/PS)
    if (m_Desc.pShader)
        static_cast<DX11GraphicsShader*>(m_Desc.pShader)->Bind();

    // 3. Rasterizer State
    CONTEXT->RSSetState(
        DX11Device::GetInst()->GetRS(m_Desc.RasterizerState).Get());

    // 4. Depth Stencil State  (stencil ref = 0)
    CONTEXT->OMSetDepthStencilState(
        DX11Device::GetInst()->GetDS(m_Desc.DepthStencilState).Get(), 0);

    // 5. Blend State  (factor = {1,1,1,1}, sampleMask = 0xFFFFFFFF)
    CONTEXT->OMSetBlendState(
        DX11Device::GetInst()->GetBS(m_Desc.BlendState).Get(), nullptr, 0xFFFFFFFF);
}

#endif // USE_DX11
