#pragma once
#include "CEntity.h"
#include "RHI/IRHIBuffer.h"

class CConstantBuffer :
    public CEntity
{
private:
	IRHIBuffer*             m_pBuffer;
    CB_TYPE                 m_Type;
    UINT                    m_ByteSize;
public:
    int Create(size_t _bufferSize, CB_TYPE _Type);
    void SetData(void* _pData);
    void Binding();
    void Binding_CS();
    void Clear();

	CLONE_DISABLED(CConstantBuffer);
public:
    CConstantBuffer();
    ~CConstantBuffer();
};

