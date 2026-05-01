#pragma once
#include "CEntity.h"
#include "RHI/IRHIBuffer.h"

enum class SB_TYPE
{
    SRV_ONLY,
	SRV_UAV,
};

class CStructuredBuffer :
    public CEntity
{
private:
	IRHIBuffer*	m_pBuffer;

    UINT        m_ElementSize;
    UINT        m_ElementCount;

	SB_TYPE     m_Type;
	bool		m_bSysMemMove;

	UINT        m_SRV_RegisterNum;
	UINT        m_UAV_RegisterNum;

public:
    UINT GetElementSize() { return m_ElementSize; }
    UINT GetElementCount() { return m_ElementCount; }

    int Create(UINT _ElementSize, UINT _ElementCount, SB_TYPE _Type, bool _SysMemMove, void* _InitialData = nullptr);
    void SetData(void* _pData, UINT _ElementCount = 0);
	void GetData(void* _pDest, UINT _ElementCount = 0);

    void Binding(int _RegisterNum);
	void Binding_CS_SRV(int _RegisterNum);
	void Binding_CS_UAV(int _RegisterNum);

	void Clear_SRV();
	void Clear_CS_SRV();
	void Clear_UAV();


    CLONE(CStructuredBuffer);
public:
    CStructuredBuffer();
	CStructuredBuffer(const CStructuredBuffer& _Origin);
    ~CStructuredBuffer();
};

