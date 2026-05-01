class CGenMipsCubemapCS : public CComputeShader
{
private:
    CTexture* m_pSrcMip = nullptr;  // 独立的 1-mip scratch (by CIBLManager)
    CTexture* m_pDstCubemap = nullptr;
    UINT      m_SrcMip = 0;
    UINT      m_DstMip = 0;

public:
    void SetSrcMipTex(CTexture* _pTex)   { m_pSrcMip = _pTex; }
    void SetDstCubemap(CTexture* _pTex)  { m_pDstCubemap = _pTex; }
    void SetDstMip(UINT _Mip)            { m_DstMip = _Mip; }
	void SetSrcMip(UINT _Mip)            { m_SrcMip = _Mip; }

    virtual int  Binding()            override;
    virtual void CalculateNumGroups() override;
    virtual void Clear()              override;

    CGenMipsCubemapCS();
    ~CGenMipsCubemapCS() = default;
};