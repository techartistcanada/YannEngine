#pragma once
#include "RHIPrereqs.h"
#include "../enum.h"

class IRHIGraphicsShader
{
public:
    virtual ~IRHIGraphicsShader() = default;
	virtual int CreateVertexShader(const wstring& _strFilePath, const string& _VSFuncName) = 0;
	virtual int CreateHullShader(const wstring& _strFilePath, const string& _HSFuncName) = 0;
	virtual int CreateDomainShader(const wstring& _strFilePath, const string& _DSFuncName) = 0;
	virtual int CreateGeometryShader(const wstring& _strFilePath, const string& _GSFuncName) = 0;
	virtual int CreatePixelShader(const wstring& _strFilePath, const string& _PSFuncName) = 0;

	virtual void SetDomain(SHADER_DOMAIN _Domain) = 0;
	virtual SHADER_DOMAIN GetDomain() const = 0;

	// Stage presence queries (needed by IRHIPipelineState)
    virtual bool HasVS() const = 0;
    virtual bool HasHS() const = 0;
    virtual bool HasDS() const = 0;
    virtual bool HasGS() const = 0;
    virtual bool HasPS() const = 0;

    // Bind shader stages to the pipeline (no RS/DS/BS/Topology here)
    virtual int Bind() = 0;
};