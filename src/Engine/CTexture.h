#pragma once
#include "CAsset.h"
#include "RHI/IRHITexture.h"

class CTexture :
    public CAsset
{
private:
	IRHITexture* m_pRHITexture;

public:
	IRHITexture* GetRHITexture() { return m_pRHITexture; }
    float GetWidth() const { return (float)m_pRHITexture->GetWidth(); }
    float GetHeight() const { return (float)m_pRHITexture->GetHeight(); }
private:
    virtual int Load(const wstring& _FilePath) override;
    virtual int Save(const wstring& _FilePath) override;

    int Create(UINT _Width, UINT _Height, DXGI_FORMAT _PixelFormat, RHI_BIND_FLAG _BindFlags);
	int CreateCubemap(UINT _Size, DXGI_FORMAT _PixelFormat, RHI_BIND_FLAG _BindFlags, UINT _MipLevels = 1);
    int CreateFromRHITexture(IRHITexture* _pRHITexture);

    // FIXME: ??
#ifdef USE_DX11
	int Create(ComPtr<ID3D11Texture2D> _Tex2D);
#endif

public:
    void Binding(int _RegisterNum);
	void Binding_CS_SRV(int _RegisterNum);
	void Binding_CS_UAV(int _RegisterNum);
	void Binding_CS_UAV_Mip(UINT _RegisterNum, UINT _MipSlice);
	void Binding_CS_SRV_Mip(UINT _RegisterNum, UINT _MipSlice);

    static void Clear(int _RegisterNum);
	static void Clear_CS_SRV(int _RegisterNum);
	static void Clear_CS_UAV(int _RegisterNum);

    // FIXME: For Imgui
    void* GetImGuiTextureID() const;
	bool HasUAV() const { return m_pRHITexture && m_pRHITexture->CanUnorderedAccess(); }
    UINT GetMipLevels() const { return m_pRHITexture->GetMipLevels(); }



	CLONE_DISABLED(CTexture);
public:
    CTexture(bool _bEngineAsset = false);
    ~CTexture();

    friend class CAssetMgr;
};

