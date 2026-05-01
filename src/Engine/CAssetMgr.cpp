#include "pch.h"
#include "CAssetMgr.h"


CAssetMgr::CAssetMgr()
	: m_bAssetsChanged(false)
{
}

CAssetMgr::~CAssetMgr()
{

}

// **********************************************************************************************
// Get Asset Names
// 获取指定类型的所有资源名称 get all asset names of the specified type
// **********************************************************************************************
void CAssetMgr::GetAssetNamesByType(ASSET_TYPE _Type, vector<string>& _vecNames)
{
	for (const auto& pair : m_mapAsset[(UINT)_Type])
	{
		_vecNames.push_back(string(pair.first.begin(), pair.first.end()));
	}
}

Ptr<CTexture> CAssetMgr::CreateTexture(const wstring& _strKey, UINT _Width, UINT _Height, DXGI_FORMAT _PixelFormat, RHI_BIND_FLAG _BindFlag)
{
	Ptr<CTexture> pTexture = FindAsset<CTexture>(_strKey);
	if (nullptr != pTexture)
	{
		return pTexture;
	}

	pTexture = new CTexture(true); // true = engine asset
	pTexture->Create(_Width, _Height, _PixelFormat, _BindFlag);
	
	pTexture->m_Key = _strKey;
	m_mapAsset[(UINT)ASSET_TYPE::TEXTURE].insert(make_pair(_strKey, pTexture.Get()));

	return pTexture;
}

Ptr<CTexture> CAssetMgr::CreateTexture(const wstring& _strKey, IRHITexture* _pRHITexture)
{
	Ptr<CTexture> pTexture = FindAsset<CTexture>(_strKey);
	if (nullptr != pTexture)
	{
		delete _pRHITexture; // caller-created, we must clean up if not used
		return pTexture;
	}

	pTexture = new CTexture(true);
	pTexture->CreateFromRHITexture(_pRHITexture); // takes ownership

	pTexture->m_Key = _strKey;
	m_mapAsset[(UINT)ASSET_TYPE::TEXTURE].insert(make_pair(_strKey, pTexture.Get()));

	return pTexture;
}

Ptr<CTexture> CAssetMgr::CreateCubemapTexture(const wstring& _strKey, UINT _Size, DXGI_FORMAT _PixelFormat, RHI_BIND_FLAG _BindFlag, UINT _MipLevels)
{
	Ptr<CTexture> pTex = FindAsset<CTexture>(_strKey);
    if (pTex.Get()) return pTex;

    pTex = new CTexture(true);
    if (FAILED(pTex->CreateCubemap(_Size, _PixelFormat, _BindFlag, _MipLevels)))
        return nullptr;

    AddAsset<CTexture>(_strKey, pTex);
    m_bAssetsChanged = true;
    return pTex;
}

#ifdef USE_DX11
Ptr<CTexture> CAssetMgr::CreateTexture(const wstring& _strKey, ComPtr<ID3D11Texture2D> _Tex2D)
{
	Ptr<CTexture> pTexture = FindAsset<CTexture>(_strKey);
	if (nullptr != pTexture)
	{
		return pTexture;
	}
	
	pTexture = new CTexture(true); // true = engine asset
	pTexture->Create(_Tex2D);

	pTexture->m_Key = _strKey;
	m_mapAsset[(UINT)ASSET_TYPE::TEXTURE].insert(make_pair(_strKey, pTexture.Get()));

	return pTexture;
}
#endif
