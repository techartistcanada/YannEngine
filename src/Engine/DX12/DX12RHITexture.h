#pragma once
#include "../RHI/IRHITexture.h"
#include "DX12Texture.h"
#include "DX12DescriptorAllocation.h"

class DX12RHITexture : public IRHITexture
{
private:
    DX12Texture     m_Texture;
    RHI_BIND_FLAG   m_BindFlags = RHI_BIND_FLAG::NONE;
    UINT            m_Width     = 0;
    UINT            m_Height    = 0;
    DXGI_FORMAT     m_Format    = DXGI_FORMAT_UNKNOWN;

public:
    int  Create(UINT _Width, UINT _Height, DXGI_FORMAT _Format, RHI_BIND_FLAG _BindFlags) override;
    int  CreateCubemap(UINT _Size, DXGI_FORMAT _Format, RHI_BIND_FLAG _BindFlags, UINT _MipLevels = 1) override;
    int  Load(const wstring& _FilePath) override;

    UINT          GetWidth()      const override { return m_Width;     }
    UINT          GetHeight()     const override { return m_Height;    }
    DXGI_FORMAT   GetDXGIFormat() const override { return m_Format;    }
    RHI_BIND_FLAG GetBindFlags()  const override { return m_BindFlags; }
    UINT GetMipLevels() const override
    {
        return static_cast<UINT>(m_Texture.GetD3D12ResourceDesc().MipLevels);
	}

    // DX12-specific accessors
    DX12Texture&       GetDX12Texture()       { return m_Texture; }
    const DX12Texture& GetDX12Texture() const { return m_Texture; }

	D3D12_GPU_DESCRIPTOR_HANDLE GetImGuiGpuHandle() const { return m_Texture.GetImGuiGpuHandle(); }

    D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const { return m_Texture.GetRenderTargetView();  }
    D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() const { return m_Texture.GetDepthStencilView();  }
    D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const
    { 
		auto desc = m_Texture.GetD3D12ResourceDesc();
		// Cubemaps: ArraySize >= 6 and loaded as Albedo (not RenderTarget/Depth)
		if (desc.DepthOrArraySize >= 6
			&& desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D
			&& (m_BindFlags & RHI_BIND_FLAG::RENDER_TARGET) == RHI_BIND_FLAG::NONE
			&& (m_BindFlags & RHI_BIND_FLAG::DEPTH_STENCIL) == RHI_BIND_FLAG::NONE)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = desc.Format;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MostDetailedMip = 0;
			srvDesc.TextureCube.MipLevels = desc.MipLevels;
			return m_Texture.GetShaderResourceView(&srvDesc);
		}
		return m_Texture.GetShaderResourceView();
    }
    D3D12_CPU_DESCRIPTOR_HANDLE GetUAV() const 
    { 
        OutputDebugStringA(("[DX12RHITexture::GetUAV] this=0x" + std::to_string((uintptr_t)this)
            + " &m_Texture=0x" + std::to_string((uintptr_t)&m_Texture)
            + " resource=0x" + std::to_string((uintptr_t)m_Texture.GetD3D12Resource().Get()) + "\n").c_str());
        return m_Texture.GetUnorderedAccessView();
    }

    // Create from existing swap chain back buffer
    int CreateFromExisting(Microsoft::WRL::ComPtr<ID3D12Resource> resource, RHI_BIND_FLAG bindFlags);

public:
    DX12RHITexture()  = default;
    ~DX12RHITexture() = default;
};