#pragma once
#include "../RHI/IRHIGraphicsShader.h"
#include <d3d12.h>
#include <d3dcompiler.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

class DX12GraphicsShader : public IRHIGraphicsShader
{
private:
	ComPtr<ID3DBlob> m_VSBlob;
	ComPtr<ID3DBlob> m_HSBlob;
	ComPtr<ID3DBlob> m_DSBlob;
	ComPtr<ID3DBlob> m_GSBlob;
	ComPtr<ID3DBlob> m_PSBlob;
	ComPtr<ID3DBlob> m_ErrBlob;

	SHADER_DOMAIN	 m_Domain = SHADER_DOMAIN::DOMAIN_OPAQUE;

public:
	int CreateVertexShader(const wstring& _strFilePath, const string& _VSFuncName) override;
	int CreateHullShader(const wstring& _strFilePath, const string& _HSFuncName) override;
	int CreateDomainShader(const wstring& _strFilePath, const string& _DSFuncName) override;
	int CreateGeometryShader(const wstring& _strFilePath, const string& _GSFuncName) override;
	int CreatePixelShader(const wstring& _strFilePath, const string& _PSFuncName) override;

	void SetDomain(SHADER_DOMAIN _Domain) override { m_Domain = _Domain; }
	SHADER_DOMAIN GetDomain() const override { return m_Domain; }

	bool HasVS() const override { return m_VSBlob != nullptr; }
	bool HasHS() const override { return m_HSBlob != nullptr; }
	bool HasDS() const override { return m_DSBlob != nullptr; }
	bool HasGS() const override { return m_GSBlob != nullptr; }
	bool HasPS() const override { return m_PSBlob != nullptr; }

	// ! NOTE: dx12 doesn't need this function, but we need it to satisfy the interface. 
	int Bind() override { return S_OK; }

	D3D12_SHADER_BYTECODE GetVSBytecode() const;
	D3D12_SHADER_BYTECODE GetHSBytecode() const;
	D3D12_SHADER_BYTECODE GetDSBytecode() const;
	D3D12_SHADER_BYTECODE GetGSBytecode() const;
	D3D12_SHADER_BYTECODE GetPSBytecode() const;

	DX12GraphicsShader() = default;
	~DX12GraphicsShader() = default;
};