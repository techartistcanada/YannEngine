#pragma once
#include "CShader.h"
#include "RHI/IRHIComputeShader.h"

class CComputeShader :
    public CShader
{
private:
	IRHIComputeShader*			  m_pRHIShader;

protected:
	tMaterialConst				  m_MaterialConst;

protected:
	UINT						  m_NumThreadPerGroupX;
	UINT						  m_NumThreadPerGroupY;
	UINT						  m_NumThreadPerGroupZ;
	
	UINT						  m_NumGroupX;
	UINT						  m_NumGroupY;
	UINT						  m_NumGroupZ;
public:
	virtual int Binding() = 0;
	virtual void CalculateNumGroups() = 0;
	virtual void Clear() = 0;
public:
    int Execute();
	int CreateComputeShader(const wstring& _strFilePath, const string& _CSFuncName);
public:
    CComputeShader(UINT _NumThreadPerGroupX, UINT _NumThreadPerGroupY, UINT _NumThreadPerGroupZ);
    ~CComputeShader();
};

