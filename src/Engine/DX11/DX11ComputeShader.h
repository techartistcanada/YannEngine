#pragma once
#include "../RHI/IRHIComputeShader.h"

class DX11ComputeShader : public IRHIComputeShader
{
private:
    ComPtr<ID3DBlob>            m_CSBlob;
    ComPtr<ID3DBlob>            m_ErrBlob;
    ComPtr<ID3D11ComputeShader> m_CS;

public:
    int  CreateComputeShader(const wstring& _strFilePath, const string& _CSFuncName) override;
    int  Bind() override;
    bool HasCS() const override { return m_CS != nullptr; }
};