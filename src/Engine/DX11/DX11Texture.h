#pragma once
#include "../RHI/IRHITexture.h"
#include <d3d11.h>
#include <wrl.h>
#include <DirectXTex.h>

using Microsoft::WRL::ComPtr;

class DX11Texture : public IRHITexture
{
private:
    ScratchImage                        m_Image;      // CPU 侧图像数据（文件加载时使用）
    ComPtr<ID3D11Texture2D>             m_Tex2D;

    ComPtr<ID3D11RenderTargetView>      m_RTV;
    ComPtr<ID3D11DepthStencilView>      m_DSV;
    ComPtr<ID3D11ShaderResourceView>    m_SRV;
    ComPtr<ID3D11UnorderedAccessView>   m_UAV;

    D3D11_TEXTURE2D_DESC                m_Desc      = {};
    RHI_BIND_FLAG                       m_BindFlags = RHI_BIND_FLAG::NONE;
	bool 							    m_bIsCubemap = false;

public:
    int           Create(UINT width, UINT height,
                         DXGI_FORMAT format, RHI_BIND_FLAG bindFlags) override;
    int 	      CreateCubemap(UINT _size, DXGI_FORMAT _format,
		RHI_BIND_FLAG _bindFlags, UINT _mipLevels = 1) override;
    int           Load(const wstring& filePath) override;

    UINT          GetWidth()      const override { return m_Desc.Width;  }
    UINT          GetHeight()     const override { return m_Desc.Height; }
	UINT          GetMipLevels()  const override { return m_Desc.MipLevels; }
    DXGI_FORMAT   GetDXGIFormat() const override { return m_Desc.Format; }
    RHI_BIND_FLAG GetBindFlags()  const override { return m_BindFlags;   }
	bool          IsCubemap()     const { return m_bIsCubemap; }

    ID3D11RenderTargetView*    GetRTV()      const { return m_RTV.Get();  }
    ID3D11DepthStencilView*    GetDSV()      const { return m_DSV.Get();  }
    ID3D11ShaderResourceView*  GetSRV()      const { return m_SRV.Get();  }
    ID3D11UnorderedAccessView* GetUAV()      const { return m_UAV.Get();  }
    ID3D11Resource*            GetResource() const { return m_Tex2D.Get();}

    int CreateFromExisting(ComPtr<ID3D11Texture2D> tex2D);

public:
    DX11Texture()  = default;
    ~DX11Texture() = default;
};