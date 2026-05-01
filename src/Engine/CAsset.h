#pragma once
#include "CEntity.h"

#include "Ptr.h"

class CAsset :
    public CEntity
{
private:
    wstring             m_Key;
    wstring             m_RelativePath;
    const ASSET_TYPE    m_Type;

    int                 m_RefCount; // 引用计数器
    bool                m_bEngineAsset;

public:
    const wstring& GetKey() { return m_Key; }
    const wstring& GetRelativePath() { return m_RelativePath; }
    ASSET_TYPE GetAssetType() { return m_Type; }
	bool IsEngineAsset() { return m_bEngineAsset; }
	int GetRefCount() { return m_RefCount; }

protected:
	void SetRelativePath(const wstring& _RelativePath) { m_RelativePath = _RelativePath; }


private:
    virtual int Load(const wstring& _FilePath) = 0;
    virtual int Save(const wstring& _FilePath) = 0;

private:
    void AddRef() { m_RefCount++; }
    void Release()
    {
        m_RefCount--;
        if (m_RefCount <= 0)
        {
            delete this;
        }
    }
public:
	virtual CAsset* Clone() = 0;
public:
    CAsset(ASSET_TYPE _Type, bool _bEngineAsset);
    CAsset(const CAsset& _Origin);
    ~CAsset();

    template<typename T>
    friend class Ptr;

    friend class CAssetMgr;
};

