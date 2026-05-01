#include "pch.h"
#include "CTexture.h"

#ifdef USE_DX11
#include "DX11/DX11Device.h"
#include "DX11/DX11Texture.h"
#else
#include "DX12/DX12Device.h"
#include "DX12/DX12RHITexture.h"
#endif

#include "RHI/IRHIDevice.h"
#include "RHI/IRHICommandList.h"


CTexture::CTexture(bool _bEngineAsset)
	: CAsset(ASSET_TYPE::TEXTURE, _bEngineAsset)
	, m_pRHITexture(nullptr)
{
}

CTexture::~CTexture()
{
	delete m_pRHITexture;
}

int CTexture::Load(const wstring& _FilePath)
{
	delete m_pRHITexture;
	m_pRHITexture = g_pRHIDevice->CreateTexture();
	return m_pRHITexture->Load(_FilePath);
}

int CTexture::Save(const wstring& _FilePath)
{
	return S_OK;
}

int CTexture::Create(UINT _Width, UINT _Height, DXGI_FORMAT _PixelFormat, RHI_BIND_FLAG _BindFlags)
{
	delete m_pRHITexture;
	m_pRHITexture = g_pRHIDevice->CreateTexture();
	return m_pRHITexture->Create(_Width, _Height, _PixelFormat, _BindFlags);
}

int CTexture::CreateFromRHITexture(IRHITexture* _pRHITexture)
{
	delete m_pRHITexture;
	m_pRHITexture = _pRHITexture;
	return S_OK;
}

#ifdef USE_DX11
int CTexture::Create(ComPtr<ID3D11Texture2D> _Tex2D)
{
	delete m_pRHITexture;
	m_pRHITexture = g_pRHIDevice->CreateTexture();
	return static_cast<DX11Texture*>(m_pRHITexture)->CreateFromExisting(_Tex2D);
}
#endif

int CTexture::CreateCubemap(UINT _Size, DXGI_FORMAT _PixelFormat, RHI_BIND_FLAG _BindFlags, UINT _MipLevels)
{
    m_pRHITexture = g_pRHIDevice->CreateTexture();
    if (FAILED(m_pRHITexture->CreateCubemap(_Size, _PixelFormat, _BindFlags, _MipLevels)))
        return E_FAIL;
    return S_OK;
}

void CTexture::Binding(int _RegisterNum)
{
	g_pRHIDevice->GetCommandList()->SetTextureSRV(_RegisterNum, m_pRHITexture);
}

void CTexture::Binding_CS_SRV(int _RegisterNum)
{
	g_pRHIDevice->GetCommandList()->SetTextureSRV_CS(_RegisterNum, m_pRHITexture);
}

void CTexture::Binding_CS_UAV(int _RegisterNum)
{
	g_pRHIDevice->GetCommandList()->SetTextureUAV_CS(_RegisterNum, m_pRHITexture);
}

void CTexture::Binding_CS_UAV_Mip(UINT _RegisterNum, UINT _MipSlice)
{
	g_pRHIDevice->GetCommandList()->SetTextureUAV_CS_Mip(_RegisterNum, m_pRHITexture, _MipSlice);
}

void CTexture::Binding_CS_SRV_Mip(UINT _RegisterNum, UINT _MipSlice)
{
	g_pRHIDevice->GetCommandList()->SetTextureSRV_CS_Mip(_RegisterNum, m_pRHITexture, _MipSlice);
}

void CTexture::Clear(int _RegisterNum)
{
	g_pRHIDevice->GetCommandList()->ClearTextureSRV(_RegisterNum);
}

void CTexture::Clear_CS_SRV(int _RegisterNum)
{
	g_pRHIDevice->GetCommandList()->ClearTextureSRV_CS(_RegisterNum);
}

void CTexture::Clear_CS_UAV(int _RegisterNum)
{
	g_pRHIDevice->GetCommandList()->ClearTextureUAV_CS(_RegisterNum);
}

void* CTexture::GetImGuiTextureID() const
{
    // DX11: ImGui 需要 ID3D11ShaderResourceView* 转成 void*
    // 未来换 DX12 只需改这一行
	if (!m_pRHITexture)
        return nullptr;
#ifdef USE_DX11
    return static_cast<DX11Texture*>(m_pRHITexture)->GetSRV();
#else
	// DX12: ImGui uses D3D12_GPU_DESCRIPTOR_HANDLE.ptr as ImTextureID
    DX12RHITexture* pDX12Tex = static_cast<DX12RHITexture*>(m_pRHITexture);
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = pDX12Tex->GetImGuiGpuHandle();
    if (gpuHandle.ptr == 0)
        return nullptr;
    return (void*)gpuHandle.ptr;
#endif

}
