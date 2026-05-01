#pragma once
#include "RHIPrereqs.h"

class IRHIComputeShader
{
public:
	virtual ~IRHIComputeShader() = default;

	virtual int CreateComputeShader(const wstring& _strFilePath, const string& _CSFuncName) = 0;

	virtual int Bind() = 0;

    virtual bool HasCS() const = 0;
};