#pragma once
#include "../RHI/IRHIGraphicsShader.h"
#include <d3d11.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;

class DX11GraphicsShader : public IRHIGraphicsShader
{
private:
	ComPtr<ID3DBlob>	m_VSBlob;
	ComPtr<ID3DBlob>	m_HSBlob;
	ComPtr<ID3DBlob>	m_DSBlob;
	ComPtr<ID3DBlob>	m_GSBlob;
	ComPtr<ID3DBlob>	m_PSBlob;
	ComPtr<ID3DBlob>	m_ErrBlob;

	  // Shader objects
    ComPtr<ID3D11VertexShader>  m_VS;
    ComPtr<ID3D11HullShader>    m_HS;
    ComPtr<ID3D11DomainShader>  m_DS;
    ComPtr<ID3D11GeometryShader>m_GS;
    ComPtr<ID3D11PixelShader>   m_PS;

    // InputLayout: created internally from VS blob + hardcoded struct Vertex layout
    // Never exposed to IRHIGraphicsShader — DX11 internal detail only
    ComPtr<ID3D11InputLayout>   m_InputLayout;

    SHADER_DOMAIN               m_Domain;

public:
    int CreateVertexShader(const wstring& filePath, const string& entryFunc) override;
    int CreateHullShader(const wstring& filePath, const string& entryFunc) override;
    int CreateDomainShader(const wstring& filePath, const string& entryFunc) override;
    int CreateGeometryShader(const wstring& filePath, const string& entryFunc) override;
    int CreatePixelShader(const wstring& filePath, const string& entryFunc) override;

    void SetDomain(SHADER_DOMAIN domain) override { m_Domain = domain; }
    SHADER_DOMAIN GetDomain() const override      { return m_Domain; }

    bool HasVS() const override { return m_VS != nullptr; }
    bool HasHS() const override { return m_HS != nullptr; }
    bool HasDS() const override { return m_DS != nullptr; }
    bool HasGS() const override { return m_GS != nullptr; }
    bool HasPS() const override { return m_PS != nullptr; }

    // Binds: IASetInputLayout + VS/HS/DS/GS/PS set
    // RS/DS/BS/Topology are NOT set here → handled by DX11PipelineState
    int Bind() override;

    // Expose VSBlob for DX11PipelineState PSO creation if needed
    ID3DBlob* GetVSBlob() const { return m_VSBlob.Get(); }

public:
    DX11GraphicsShader()  = default;
    ~DX11GraphicsShader() = default;

};