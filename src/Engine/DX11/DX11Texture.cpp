#include "../pch.h"
#ifdef USE_DX11


#include "DX11Texture.h"
#include "DX11Device.h"

static UINT ToDX11BindFlags(RHI_BIND_FLAG flags)
{
    UINT result = 0;
    if ((flags & RHI_BIND_FLAG::SHADER_RESOURCE)  != RHI_BIND_FLAG::NONE)
        result |= D3D11_BIND_SHADER_RESOURCE;
    if ((flags & RHI_BIND_FLAG::RENDER_TARGET)    != RHI_BIND_FLAG::NONE)
        result |= D3D11_BIND_RENDER_TARGET;
    if ((flags & RHI_BIND_FLAG::DEPTH_STENCIL)    != RHI_BIND_FLAG::NONE)
        result |= D3D11_BIND_DEPTH_STENCIL;
    if ((flags & RHI_BIND_FLAG::UNORDERED_ACCESS) != RHI_BIND_FLAG::NONE)
        result |= D3D11_BIND_UNORDERED_ACCESS;
    return result;
}

static void CreateViews(ID3D11Device* pDevice,
                        ID3D11Texture2D* pTex,
                        const D3D11_TEXTURE2D_DESC& desc,
                        ComPtr<ID3D11RenderTargetView>&    outRTV,
                        ComPtr<ID3D11DepthStencilView>&    outDSV,
                        ComPtr<ID3D11ShaderResourceView>&  outSRV,
                        ComPtr<ID3D11UnorderedAccessView>& outUAV)
{
    if (desc.BindFlags & D3D11_BIND_DEPTH_STENCIL)
    {
        pDevice->CreateDepthStencilView(pTex, nullptr, outDSV.GetAddressOf());
        return; // 深度模板纹理不创建其他 View
    }

    if (desc.BindFlags & D3D11_BIND_RENDER_TARGET)
        pDevice->CreateRenderTargetView(pTex, nullptr, outRTV.GetAddressOf());

    if (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels       = desc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        pDevice->CreateShaderResourceView(pTex, &srvDesc, outSRV.GetAddressOf());
    }

    if (desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
    {
        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        pDevice->CreateUnorderedAccessView(pTex, &uavDesc, outUAV.GetAddressOf());
    }
}

int DX11Texture::Create(UINT width, UINT height,
                        DXGI_FORMAT format, RHI_BIND_FLAG bindFlags)
{
    m_BindFlags = bindFlags;

    m_Desc              = {};
    m_Desc.Width        = width;
    m_Desc.Height       = height;
    m_Desc.Format       = format;
    m_Desc.ArraySize    = 1;
    m_Desc.MipLevels    = 1;
    m_Desc.Usage        = D3D11_USAGE_DEFAULT;
    m_Desc.BindFlags    = ToDX11BindFlags(bindFlags);
    m_Desc.SampleDesc   = { 1, 0 };

    if (FAILED(DEVICE->CreateTexture2D(&m_Desc, nullptr, m_Tex2D.GetAddressOf())))
        return E_FAIL;

    CreateViews(DEVICE, m_Tex2D.Get(), m_Desc,
                m_RTV, m_DSV, m_SRV, m_UAV);
    return S_OK;
}

int DX11Texture::CreateCubemap(UINT _size, DXGI_FORMAT _format, RHI_BIND_FLAG _bindFlags, UINT _mipLevels)
{
        m_BindFlags  = _bindFlags;
    m_bIsCubemap = true;

    // If mipLevels is 0, compute full chain
    if (_mipLevels == 0)
    {
        UINT s = _size;
        _mipLevels = 1;
        while (s > 1) { s >>= 1; ++_mipLevels; }
    }

    m_Desc = {};
    m_Desc.Width       = _size;
    m_Desc.Height      = _size;
    m_Desc.Format      = _format;
    m_Desc.ArraySize   = 6;
    m_Desc.MipLevels   = _mipLevels;
    m_Desc.Usage       = D3D11_USAGE_DEFAULT;
    m_Desc.BindFlags   = ToDX11BindFlags(_bindFlags);
    m_Desc.SampleDesc  = { 1, 0 };
    m_Desc.MiscFlags   = D3D11_RESOURCE_MISC_TEXTURECUBE;
    // REMOVED: D3D11_RESOURCE_MISC_GENERATE_MIPS + D3D11_BIND_RENDER_TARGET
    // These are for ID3D11DeviceContext::GenerateMips() — incompatible with
    // D3D11_BIND_UNORDERED_ACCESS (DX11 spec violation), corrupts CS UAV mip writes.

    if (FAILED(DEVICE->CreateTexture2D(&m_Desc, nullptr, m_Tex2D.GetAddressOf())))
        return E_FAIL;

    // SRV — full cubemap view across all mips
    if (m_Desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format                        = _format;
        srvDesc.ViewDimension                 = D3D11_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.TextureCube.MostDetailedMip   = 0;
        srvDesc.TextureCube.MipLevels         = _mipLevels;
        if (FAILED(DEVICE->CreateShaderResourceView(m_Tex2D.Get(), &srvDesc, m_SRV.GetAddressOf())))
            return E_FAIL;
    }

    // UAV — mip 0 default view; per-mip UAVs are created on demand in SetTextureUAV_CS_Mip
    if (m_Desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
    {
        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format                         = _format;
        uavDesc.ViewDimension                  = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
        uavDesc.Texture2DArray.MipSlice        = 0;
        uavDesc.Texture2DArray.FirstArraySlice = 0;
        uavDesc.Texture2DArray.ArraySize       = 6;
        if (FAILED(DEVICE->CreateUnorderedAccessView(m_Tex2D.Get(), &uavDesc, m_UAV.GetAddressOf())))
            return E_FAIL;
    }

    return S_OK;
}

int DX11Texture::Load(const wstring& filePath)
{
	wstring ext = filePath.substr(filePath.find_last_of(L'.'));

    HRESULT hr = E_FAIL;
    if      (ext == L".dds" || ext == L".DDS")
        hr = LoadFromDDSFile(filePath.c_str(), DDS_FLAGS_NONE, nullptr, m_Image);
    else if (ext == L".tga" || ext == L".TGA")
        hr = LoadFromTGAFile(filePath.c_str(), nullptr, m_Image);
    else if (ext == L".hdr" || ext == L".HDR")
		hr = LoadFromHDRFile(filePath.c_str(), nullptr, m_Image);
    else
        hr = LoadFromWICFile(filePath.c_str(), WIC_FLAGS_NONE, nullptr, m_Image);

    if (FAILED(hr))
    {
        MessageBox(nullptr, filePath.c_str(), L"DX11Texture: Load failed", MB_OK);
        return E_FAIL;
    }

    // --- Generate full mip chain on CPU if only 1 level exists ---
    if (m_Image.GetMetadata().mipLevels <= 1)
    {
        ScratchImage mipChain;
        hr = GenerateMipMaps(m_Image.GetImages(), m_Image.GetImageCount(),
                             m_Image.GetMetadata(),
                             TEX_FILTER_DEFAULT, 0,  // 0 = full chain
                             mipChain);
        if (SUCCEEDED(hr))
        {
            m_Image = std::move(mipChain);
        }
    }

    hr = CreateShaderResourceView(DEVICE,
                                  m_Image.GetImages(),
                                  m_Image.GetImageCount(),
                                  m_Image.GetMetadata(),
                                  m_SRV.GetAddressOf());
    if (FAILED(hr))
        return E_FAIL;

    m_SRV->GetResource(reinterpret_cast<ID3D11Resource**>(m_Tex2D.GetAddressOf()));
    m_Tex2D->GetDesc(&m_Desc);

    m_BindFlags = RHI_BIND_FLAG::SHADER_RESOURCE;
    return S_OK;
}

int DX11Texture::CreateFromExisting(ComPtr<ID3D11Texture2D> tex2D)
{
    m_Tex2D = tex2D;
    m_Tex2D->GetDesc(&m_Desc);

    m_BindFlags = RHI_BIND_FLAG::NONE;
    if (m_Desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
        m_BindFlags = m_BindFlags | RHI_BIND_FLAG::SHADER_RESOURCE;
    if (m_Desc.BindFlags & D3D11_BIND_RENDER_TARGET)
        m_BindFlags = m_BindFlags | RHI_BIND_FLAG::RENDER_TARGET;
    if (m_Desc.BindFlags & D3D11_BIND_DEPTH_STENCIL)
        m_BindFlags = m_BindFlags | RHI_BIND_FLAG::DEPTH_STENCIL;
    if (m_Desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
        m_BindFlags = m_BindFlags | RHI_BIND_FLAG::UNORDERED_ACCESS;

    CreateViews(DEVICE, m_Tex2D.Get(), m_Desc,
                m_RTV, m_DSV, m_SRV, m_UAV);
    return S_OK;
}

#endif // USE_DX11