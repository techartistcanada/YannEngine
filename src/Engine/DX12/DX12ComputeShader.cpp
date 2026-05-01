#include "dx12pch.h"
#include "DX12ComputeShader.h"
#include "DX12Device.h"
#include "DX12RootSignature.h"
#include "DX12CommandList.h"

#pragma comment(lib, "d3dcompiler.lib")

int DX12ComputeShader::CreateComputeShader(const wstring& _strFilePath, const string& _CSFuncName)
{
	HRESULT hr = D3DCompileFromFile(
	_strFilePath.c_str(),
	nullptr,
	D3D_COMPILE_STANDARD_FILE_INCLUDE,
	_CSFuncName.c_str(),
	"cs_5_1",
	D3DCOMPILE_DEBUG, 0,
	m_CSBlob.GetAddressOf(),
	m_ErrBlob.GetAddressOf());

    if (FAILED(hr))
    {
        if (m_ErrBlob)
            OutputDebugStringA((char*)m_ErrBlob->GetBufferPointer());
        return E_FAIL;
    }

    // Build the compute PSO immediately
    return BuildPSO();
}

int DX12ComputeShader::Bind()
{
    if (!m_PipelineState)
        return E_FAIL;

    auto pCmdList = DX12Device::GetInst()->GetCurrentCommandList();
    if (!pCmdList)
        return E_FAIL;

    pCmdList->GetGraphicsCommandList()->SetPipelineState(m_PipelineState.Get());
    return S_OK;
}

D3D12_SHADER_BYTECODE DX12ComputeShader::GetCSBytecode() const
{
    return m_CSBlob ? D3D12_SHADER_BYTECODE{ m_CSBlob->GetBufferPointer(), m_CSBlob->GetBufferSize() }
                    : D3D12_SHADER_BYTECODE{ nullptr, 0 };
}

int DX12ComputeShader::BuildPSO()
{
    if (!m_CSBlob) return E_FAIL;

    auto* pDevice = DX12Device::GetInst();

    D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
    desc.pRootSignature = pDevice->GetComputeRootSignature().GetRootSignature().Get();
    desc.CS = GetCSBytecode();

    HRESULT hr = pDevice->GetD3D12Device()->CreateComputePipelineState(
        &desc, IID_PPV_ARGS(&m_PipelineState));

	OutputDebugStringA(FAILED(hr) ? "Failed to create compute pipeline state.\n" : "Successfully created compute pipeline state.\n");

    return FAILED(hr) ? E_FAIL : S_OK;
}
