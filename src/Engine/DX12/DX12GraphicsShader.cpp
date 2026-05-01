#include "dx12pch.h"
#include "DX12GraphicsShader.h"
#include <d3dcompiler.h>


static int CompileShaderDX12(const wstring& _filePath,
    const string& _entryPoint,
    const string& _target,
    ComPtr<ID3DBlob>& _outBlob,
    ComPtr<ID3DBlob>& _errorBlob)
{
    HRESULT hr = D3DCompileFromFile(
        _filePath.c_str(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        _entryPoint.c_str(),
        _target.c_str(),
        D3DCOMPILE_DEBUG,
        0,
        _outBlob.GetAddressOf(),
        _errorBlob.GetAddressOf());

    if (FAILED(hr))
    {
		if (_errorBlob)
        {
            OutputDebugStringA((char*)_errorBlob->GetBufferPointer());
        }
        return E_FAIL;
    }

    return S_OK;
}


int DX12GraphicsShader::CreateVertexShader(const wstring& _strFilePath, const string& _VSFuncName)
{
	return CompileShaderDX12(_strFilePath, _VSFuncName, "vs_5_1", m_VSBlob, m_ErrBlob);
}

int DX12GraphicsShader::CreateHullShader(const wstring& _strFilePath, const string& _HSFuncName)
{
	return CompileShaderDX12(_strFilePath, _HSFuncName, "hs_5_1", m_HSBlob, m_ErrBlob);
}

int DX12GraphicsShader::CreateDomainShader(const wstring& _strFilePath, const string& _DSFuncName)
{
	return CompileShaderDX12(_strFilePath, _DSFuncName, "ds_5_1", m_DSBlob, m_ErrBlob);
}

int DX12GraphicsShader::CreateGeometryShader(const wstring& _strFilePath, const string& _GSFuncName)
{
	return CompileShaderDX12(_strFilePath, _GSFuncName, "gs_5_1", m_GSBlob, m_ErrBlob);
}

int DX12GraphicsShader::CreatePixelShader(const wstring& _strFilePath, const string& _PSFuncName)
{
	return CompileShaderDX12(_strFilePath, _PSFuncName, "ps_5_1", m_PSBlob, m_ErrBlob);
}

D3D12_SHADER_BYTECODE DX12GraphicsShader::GetVSBytecode() const
{
     return m_VSBlob ? D3D12_SHADER_BYTECODE{ m_VSBlob->GetBufferPointer(), m_VSBlob->GetBufferSize() }
     : D3D12_SHADER_BYTECODE{};
}

D3D12_SHADER_BYTECODE DX12GraphicsShader::GetHSBytecode() const
{
    return m_HSBlob ? D3D12_SHADER_BYTECODE{ m_HSBlob->GetBufferPointer(), m_HSBlob->GetBufferSize() }
	: D3D12_SHADER_BYTECODE{};
}

D3D12_SHADER_BYTECODE DX12GraphicsShader::GetDSBytecode() const
{
	return m_DSBlob ? D3D12_SHADER_BYTECODE{ m_DSBlob->GetBufferPointer(), m_DSBlob->GetBufferSize() }
	: D3D12_SHADER_BYTECODE{};
}

D3D12_SHADER_BYTECODE DX12GraphicsShader::GetGSBytecode() const
{
	return m_GSBlob ? D3D12_SHADER_BYTECODE{ m_GSBlob->GetBufferPointer(), m_GSBlob->GetBufferSize() }
    : D3D12_SHADER_BYTECODE{};
}

D3D12_SHADER_BYTECODE DX12GraphicsShader::GetPSBytecode() const
{
	return m_PSBlob ? D3D12_SHADER_BYTECODE{ m_PSBlob->GetBufferPointer(), m_PSBlob->GetBufferSize() }
	: D3D12_SHADER_BYTECODE{};
}
