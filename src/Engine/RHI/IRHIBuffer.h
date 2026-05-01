#pragma once
#include "RHIPrereqs.h"


class IRHIBuffer
{
public:
	virtual ~IRHIBuffer() = default;

	virtual int CreateVertex(UINT _stride, UINT _count, const void* _pData = nullptr) = 0;
	virtual int CreateIndex(DXGI_FORMAT _format, UINT _count, const void* _pData = nullptr) = 0;

	virtual int CreateConstant(UINT _byteSize) = 0;
	// _bUAV      : 是否需要无序访问视图 (SB_TYPE::SRV_UAV)
	// _bCPUReadback : 是否需要 CPU 回读 (SetData / GetData)
	virtual int CreateStructured(UINT _elementSize, UINT _elementCount, bool _bCPUReadback, bool _bUAV, const void* _pData = nullptr) = 0;

	virtual void SetData(const void* _pData, UINT _byteSize) = 0;
	virtual void GetData(void* _pDest, UINT _byteSize) = 0;

	virtual RHI_BUFFER_TYPE GetType()         const = 0;
    virtual UINT            GetByteSize()     const = 0;
    virtual UINT            GetElementSize()  const = 0;
    virtual UINT            GetElementCount() const = 0;
    virtual DXGI_FORMAT     GetIndexFormat()  const = 0;  // 仅 INDEX 类型有效

};