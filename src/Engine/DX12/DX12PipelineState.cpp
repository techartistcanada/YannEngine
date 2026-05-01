#include "dx12pch.h"
#include "DX12PipelineState.h"
#include "DX12GraphicsShader.h"
#include "DX12Device.h"
#include "DX12RootSignature.h"
#include "DX12CommandList.h"
#include "DX12CommandQueue.h"

static D3D12_PRIMITIVE_TOPOLOGY_TYPE ToPrimitiveTopologyType(RHI_PRIMITIVE_TOPOLOGY topo);

int DX12PipelineState::Create(const RHIGraphicsPipelineDesc& _Desc)
{
	auto* pDevice = DX12Device::GetInst();

    // Auto-derive RTV/DSV formats from currently-bound render targets.
    // Shaders no longer need to call SetRTVFormat/SetDSVFormat manually,
    // and PSO format always matches what OMSetRenderTargets just bound.
    RHIGraphicsPipelineDesc desc = _Desc;
    desc.NumRenderTargets = pDevice->GetBoundNumRTVs();
    for (UINT i = 0; i < 8; ++i)
        desc.RTVFormats[i] = (i < desc.NumRenderTargets) ? pDevice->GetBoundRTVFormat(i) : DXGI_FORMAT_UNKNOWN;
    desc.DSVFormat = pDevice->GetBoundDSVFormat();

    // Cache: same shader+state+RT-format-set => reuse PSO
    if (m_PipelineState && memcmp(&m_Desc, &desc, sizeof(RHIGraphicsPipelineDesc)) == 0)
        return S_OK;

    m_Desc = desc;

    auto* pShader = static_cast<DX12GraphicsShader*>(desc.pShader);

	// Input Layout
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 60, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // pso desc
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = pDevice->GetGraphicsRootSignature().GetRootSignature().Get();
	psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    psoDesc.PrimitiveTopologyType = ToPrimitiveTopologyType(desc.PrimitiveTopology);

    // Shaders
    if (pShader)
    {
        psoDesc.VS = pShader->GetVSBytecode();
        psoDesc.HS = pShader->GetHSBytecode();
        psoDesc.DS = pShader->GetDSBytecode();
        psoDesc.GS = pShader->GetGSBytecode();
		psoDesc.PS = pShader->GetPSBytecode();
    }

	// Rasterizer State / Depth Stencil State / Blend State
	psoDesc.RasterizerState   = pDevice->GetRS(desc.RasterizerState);
	psoDesc.DepthStencilState = pDevice->GetDS(desc.DepthStencilState);
	psoDesc.BlendState        = pDevice->GetBS(desc.BlendState);

	// Render Target / Depth Stencil formats (auto-derived above)
	psoDesc.NumRenderTargets = desc.NumRenderTargets;
	for (UINT i = 0; i < desc.NumRenderTargets; ++i)
    {
        psoDesc.RTVFormats[i] = desc.RTVFormats[i];
    }
	psoDesc.DSVFormat = desc.DSVFormat;

    // Sampler
	psoDesc.SampleDesc = { 1, 0 };
	psoDesc.SampleMask = UINT_MAX;

    // Create PSO
	HRESULT hr = pDevice->GetD3D12Device()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineState));

    // handle error
    if (FAILED(hr))
    {
        OutputDebugStringA("DX12PipelineState::Create — CreateGraphicsPipelineState FAILED\n");
        return E_FAIL;
    }

    return S_OK;
}

void DX12PipelineState::Bind()
{
    auto* pDevice = DX12Device::GetInst();
    auto  cmdList = pDevice->GetCurrentCommandList();

    // Set PSO
    cmdList->SetPipelineState(m_PipelineState);

    // Set root signature
    cmdList->SetGraphicsRootSignature(pDevice->GetGraphicsRootSignature());

    // Set primitive topology (PSO only stores the TYPE, the actual topology is set separately)
    cmdList->SetPrimitiveTopology(
        static_cast<D3D_PRIMITIVE_TOPOLOGY>(m_Desc.PrimitiveTopology));
}

// Helper: map RHI_PRIMITIVE_TOPOLOGY to D3D12_PRIMITIVE_TOPOLOGY_TYPE
D3D12_PRIMITIVE_TOPOLOGY_TYPE ToPrimitiveTopologyType(RHI_PRIMITIVE_TOPOLOGY topo)
{
    switch (topo)
    {
		case RHI_PRIMITIVE_TOPOLOGY::POINT_LIST:       return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		case RHI_PRIMITIVE_TOPOLOGY::LINE_LIST:
		case RHI_PRIMITIVE_TOPOLOGY::LINE_STRIP:       return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		case RHI_PRIMITIVE_TOPOLOGY::TRIANGLE_LIST:
		case RHI_PRIMITIVE_TOPOLOGY::TRIANGLE_STRIP:   return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		case RHI_PRIMITIVE_TOPOLOGY::PATCH_LIST_1:
		case RHI_PRIMITIVE_TOPOLOGY::PATCH_LIST_2:
		case RHI_PRIMITIVE_TOPOLOGY::PATCH_LIST_3:
		case RHI_PRIMITIVE_TOPOLOGY::PATCH_LIST_4:     return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
		default:                                       return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    }
}
