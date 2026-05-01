#pragma once
#include "../RHI/IRHIComputeShader.h"
#include <d3d12.h>
#include <d3dcompiler.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

class RootSignature;

class DX12ComputeShader : public IRHIComputeShader
{
private:
	ComPtr<ID3DBlob>		    m_CSBlob;
	ComPtr<ID3DBlob>            m_ErrBlob;
	ComPtr<ID3D12PipelineState> m_PipelineState;

public:
	int CreateComputeShader(const wstring& _strFilePath, const string& _CSFuncName) override;
	int Bind() override;
	bool HasCS() const override { return m_CSBlob != nullptr; }

	D3D12_SHADER_BYTECODE GetCSBytecode() const;
	ID3D12PipelineState* GetPipelineState() const { return m_PipelineState.Get(); }

	int BuildPSO();

public:
	DX12ComputeShader() = default;
	~DX12ComputeShader() = default;
};
