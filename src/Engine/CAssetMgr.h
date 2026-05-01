#pragma once
#include "Singleton.h"
#include "CPathMgr.h"

#include "assets.h"
#include "RHI/RHIPrereqs.h"

class CAssetMgr :
    public CSingleton<CAssetMgr>
{
    SINGLE(CAssetMgr)
private:
    map<wstring, Ptr<CAsset>>   m_mapAsset[(UINT)ASSET_TYPE::END];
    bool                        m_bAssetsChanged;

public:
    void init();
	//void tick() { m_bAssetsChanged = false; }
	void tick() { }
	bool IsAssetsChanged() { return m_bAssetsChanged; }
	void ClearAssetsChangedFlag() { m_bAssetsChanged = false;  }
	void GetAssetNamesByType(ASSET_TYPE _Type, _Out_ vector<string>& _vecNames);
	const map<wstring, Ptr<CAsset>>& GetAssets(ASSET_TYPE _Type) { return m_mapAsset[(UINT)_Type]; }

	Ptr<CTexture> CreateTexture(const wstring& _strKey, UINT _Width, UINT _Height, DXGI_FORMAT _PixelFormat, RHI_BIND_FLAG _BindFlag);
    // ! NOTE:
    Ptr<CTexture> CreateTexture(const wstring& _strKey, IRHITexture* _pRHITexture);
	Ptr<CTexture> CreateCubemapTexture(const wstring& _strKey, UINT _Size, DXGI_FORMAT _PixelFormat, RHI_BIND_FLAG _BindFlag, UINT _MipLevels = 1);

#ifdef USE_DX11
    // TODO: fix this
	Ptr<CTexture> CreateTexture(const wstring& _strKey, ComPtr<ID3D11Texture2D> _Tex2D);
#endif

    template<typename T>
    Ptr<T> Load(const wstring& _strKey, const wstring& _strRelativePath);
    template<typename T>
    Ptr<T> FindAsset(const wstring& _strKey);

    template<typename T>
    void AddAsset(const wstring& _strKey, Ptr<T> pAsset);

	// TODO: temprary, delete this function after implementing asset reference counting
	template<typename T>
    void DeleteMyAsset(const wstring& _strKey);
private:
	void DeleteAsset(ASSET_TYPE _Type, const wstring& _strKey);


private:
    void CreateDefaultMesh();
    void CreateDefaultTexture();
    void CreateDefaultMaterial();
    void CreateDefaultGraphicsShader();
    void CreateDefaultComputeShader();

    friend class CTaskMgr;
};


template<typename T>
ASSET_TYPE GetAssetType()
{
    if constexpr (std::is_same_v<T, CMesh>)
    {
        return ASSET_TYPE::MESH;
    }
    if constexpr (std::is_same_v<T, CGraphicsShader>)
    {
        return ASSET_TYPE::GRAPHICS_SHADER;
    }
    if constexpr (std::is_same_v<T, CGraphicsShader>)
    {
        return ASSET_TYPE::COMPUTE_SHADER;
    }
    if constexpr (std::is_same_v<T, CTexture>)
    {
        return ASSET_TYPE::TEXTURE;
    }
    if constexpr (std::is_same_v<T, CMaterial>)
    {
        return ASSET_TYPE::MATERIAL;
    }
    if constexpr (std::is_same_v<T, CPrefab>)
    {
        return ASSET_TYPE::PREFAB;
    }

    // BUG:
    return ASSET_TYPE::MESH;
}

template<typename T>
inline Ptr<T> CAssetMgr::Load(const wstring& _strKey, const wstring& _strRelativePath)
{
    Ptr<CAsset> pAsset = FindAsset<T>(_strKey).Get();
    if (nullptr != pAsset.Get())
    {
        return (T*)pAsset.Get();
    }

    if constexpr (std::is_same_v <T, CComputeShader> || std::is_same_v<T,CGraphicsShader>)
    {
        return nullptr;
    }
    else
    {
		wstring strFullPath = CPathMgr::GetInst()->GetContentPath();
		strFullPath += _strRelativePath;

		pAsset = new T;
		if (FAILED(pAsset->Load(strFullPath)))
		{
			MessageBox(nullptr, strFullPath.c_str(), L"Failed to load asset", MB_OK);
			return nullptr;
		}

		pAsset->m_RelativePath = _strRelativePath;
		AddAsset<T>(_strKey, (T*)pAsset.Get());

		m_bAssetsChanged = true;

		return (T*)pAsset.Get();
    }
}

template<typename T>
inline Ptr<T> CAssetMgr::FindAsset(const wstring& _strKey)
{
    ASSET_TYPE Type = GetAssetType<T>();

    map <wstring, Ptr<CAsset>>::iterator iter = m_mapAsset[(UINT)Type].find(_strKey);
    if (iter == m_mapAsset[(UINT)Type].end())
    {
        return nullptr;
    }

#ifdef _DEBUG
    T* pAsset = dynamic_cast<T*>(iter->second.Get());
    return pAsset;
#else
    return (T*)iter->second.Get();
#endif
}

template<typename T>
inline void CAssetMgr::AddAsset(const wstring& _strKey, Ptr<T> _pAsset)
{
    Ptr<T> pFindAsset = FindAsset<T>(_strKey);

    assert(pFindAsset.Get() == nullptr);

    ASSET_TYPE type = GetAssetType<T>();

    m_mapAsset[(UINT)type].insert(make_pair(_strKey, _pAsset.Get()));
    _pAsset->m_Key = _strKey;

	m_bAssetsChanged = true;
}

template<typename T>
inline void CAssetMgr::DeleteMyAsset(const wstring& _strKey)
{
    ASSET_TYPE Type = GetAssetType<T>();
    m_mapAsset[(UINT)Type].erase(_strKey);
    m_bAssetsChanged = true;
}
