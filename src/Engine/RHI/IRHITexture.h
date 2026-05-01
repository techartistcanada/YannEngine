#pragma once

#include "RHIPrereqs.h"

class IRHITexture
{
public:
	virtual ~IRHITexture() = default;

	virtual int Create(UINT _Width, UINT _Height, DXGI_FORMAT _Format, RHI_BIND_FLAG _BindFlags) = 0;
	virtual int CreateCubemap(UINT _Size, DXGI_FORMAT _Format, RHI_BIND_FLAG _BindFlags, UINT _MipLevels = 1) = 0;

	virtual int Load(const wstring& _FilePath) = 0;
	
	virtual UINT GetWidth() const = 0;
	virtual UINT GetHeight() const = 0;
	virtual UINT GetMipLevels() const = 0;
	virtual DXGI_FORMAT GetDXGIFormat() const = 0;
	virtual RHI_BIND_FLAG GetBindFlags() const = 0;

	bool CanRenderTarget()    const { return !!(UINT(GetBindFlags()) & UINT(RHI_BIND_FLAG::RENDER_TARGET));    }
    bool CanDepthStencil()    const { return !!(UINT(GetBindFlags()) & UINT(RHI_BIND_FLAG::DEPTH_STENCIL));    }
    bool CanShaderResource()  const { return !!(UINT(GetBindFlags()) & UINT(RHI_BIND_FLAG::SHADER_RESOURCE));  }
    bool CanUnorderedAccess() const { return !!(UINT(GetBindFlags()) & UINT(RHI_BIND_FLAG::UNORDERED_ACCESS)); }
};