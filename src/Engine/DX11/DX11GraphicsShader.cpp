#include "../pch.h"
#ifdef USE_DX11

#include "DX11GraphicsShader.h"
#include "DX11Device.h"

static int CompileShader(const wstring&    filePath,
                         const string&     entryFunc,
                         const string&     target,
                         ComPtr<ID3DBlob>& outBlob,
                         ComPtr<ID3DBlob>& outErr)
{
    HRESULT hr = D3DCompileFromFile(
        filePath.c_str(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryFunc.c_str(),
        target.c_str(),
        D3DCOMPILE_DEBUG, 0,
        outBlob.GetAddressOf(),
        outErr.GetAddressOf());

    if (FAILED(hr))
    {
        if (outErr)
            OutputDebugStringA((char*)outErr->GetBufferPointer());
        return E_FAIL;
    }
    return S_OK;
}

// ----------------------------------------------------------------
// Vertex Shader + InputLayout
// ----------------------------------------------------------------
int DX11GraphicsShader::CreateVertexShader(const wstring& filePath,
                                           const string&  entryFunc)
{
    if (FAILED(CompileShader(filePath, entryFunc, "vs_5_0", m_VSBlob, m_ErrBlob)))
        return E_FAIL;

    if (FAILED(DEVICE->CreateVertexShader(
        m_VSBlob->GetBufferPointer(), m_VSBlob->GetBufferSize(),
        nullptr, m_VS.GetAddressOf())))
        return E_FAIL;

    D3D11_INPUT_ELEMENT_DESC layout[6] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 60, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    if (FAILED(DEVICE->CreateInputLayout(
        layout, 6,
        m_VSBlob->GetBufferPointer(), m_VSBlob->GetBufferSize(),
        m_InputLayout.GetAddressOf())))
        return E_FAIL;

    return S_OK;
}

int DX11GraphicsShader::CreateHullShader(const wstring& filePath,
                                         const string&  entryFunc)
{
    if (FAILED(CompileShader(filePath, entryFunc, "hs_5_0", m_HSBlob, m_ErrBlob)))
        return E_FAIL;
    return FAILED(DEVICE->CreateHullShader(
        m_HSBlob->GetBufferPointer(), m_HSBlob->GetBufferSize(),
        nullptr, m_HS.GetAddressOf())) ? E_FAIL : S_OK;
}

int DX11GraphicsShader::CreateDomainShader(const wstring& filePath,
                                           const string&  entryFunc)
{
    if (FAILED(CompileShader(filePath, entryFunc, "ds_5_0", m_DSBlob, m_ErrBlob)))
        return E_FAIL;
    return FAILED(DEVICE->CreateDomainShader(
        m_DSBlob->GetBufferPointer(), m_DSBlob->GetBufferSize(),
        nullptr, m_DS.GetAddressOf())) ? E_FAIL : S_OK;
}

int DX11GraphicsShader::CreateGeometryShader(const wstring& filePath,
                                             const string&  entryFunc)
{
    if (FAILED(CompileShader(filePath, entryFunc, "gs_5_0", m_GSBlob, m_ErrBlob)))
        return E_FAIL;
    return FAILED(DEVICE->CreateGeometryShader(
        m_GSBlob->GetBufferPointer(), m_GSBlob->GetBufferSize(),
        nullptr, m_GS.GetAddressOf())) ? E_FAIL : S_OK;
}

int DX11GraphicsShader::CreatePixelShader(const wstring& filePath,
                                          const string&  entryFunc)
{
    if (FAILED(CompileShader(filePath, entryFunc, "ps_5_0", m_PSBlob, m_ErrBlob)))
        return E_FAIL;
    return FAILED(DEVICE->CreatePixelShader(
        m_PSBlob->GetBufferPointer(), m_PSBlob->GetBufferSize(),
        nullptr, m_PS.GetAddressOf())) ? E_FAIL : S_OK;
}

int DX11GraphicsShader::Bind()
{
    CONTEXT->IASetInputLayout(m_InputLayout.Get());

    CONTEXT->VSSetShader(m_VS.Get(), nullptr, 0);
    CONTEXT->HSSetShader(m_HS.Get(), nullptr, 0);   
    CONTEXT->DSSetShader(m_DS.Get(), nullptr, 0);
    CONTEXT->GSSetShader(m_GS.Get(), nullptr, 0);
    CONTEXT->PSSetShader(m_PS.Get(), nullptr, 0);

    return S_OK;
}

#endif // USE_DX11
