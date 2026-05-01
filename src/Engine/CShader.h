#pragma once
#include "CAsset.h"

struct tShaderScalarParam
{
	string          strDesc;
    SCALAR_PARAM    Param;
};

struct tShaderTextureParam
{
    string          strDesc;
	TEX_PARAM       Param;
};



class CShader :
    public CAsset
{
protected:
    ComPtr<ID3DBlob>                m_ErrBlob;
	vector<tShaderScalarParam>      m_vecScalarParams;
	vector<tShaderTextureParam>     m_vecTextureParams;

public:
	void AddScalarParam(const string& _Desc, SCALAR_PARAM _Param) { m_vecScalarParams.push_back(tShaderScalarParam{ _Desc, _Param }); }
	void AddTextureParam(const string& _Desc, TEX_PARAM _Param) { m_vecTextureParams.push_back(tShaderTextureParam{ _Desc, _Param }); }

	const vector<tShaderScalarParam>& GetScalarParams() { return m_vecScalarParams; }
	const vector<tShaderTextureParam>& GetTextureParams() { return m_vecTextureParams; }
public:
    virtual int Binding() = 0;
    virtual int Load(const wstring& _FilePath) override { return S_OK; }
    virtual int Save(const wstring& _FilePath) override { return S_OK; }

	CLONE_DISABLED(CShader);
public:
    CShader(ASSET_TYPE _Type);
    ~CShader();
};

