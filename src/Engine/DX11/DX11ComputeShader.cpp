#include "../pch.h"
#ifdef USE_DX11

#include "DX11ComputeShader.h"
#include "DX11Device.h"

int DX11ComputeShader::CreateComputeShader(const wstring& _strFilePath, const string& _CSFuncName)
{
    if (FAILED(D3DCompileFromFile(
        _strFilePath.c_str(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        _CSFuncName.c_str(),
        "cs_5_0",
        D3DCOMPILE_DEBUG, 0,
        m_CSBlob.GetAddressOf(),
        m_ErrBlob.GetAddressOf())))
    {
        if (m_ErrBlob)
            OutputDebugStringA((char*)m_ErrBlob->GetBufferPointer());
        return E_FAIL;
    }

    if (FAILED(DEVICE->CreateComputeShader(
        m_CSBlob->GetBufferPointer(),
        m_CSBlob->GetBufferSize(),
        nullptr,
        m_CS.GetAddressOf())))
    {
        return E_FAIL;
    }

    return S_OK;
}

int DX11ComputeShader::Bind()
{
    CONTEXT->CSSetShader(m_CS.Get(), nullptr, 0);
    return S_OK;
}

#endif // USE_DX11