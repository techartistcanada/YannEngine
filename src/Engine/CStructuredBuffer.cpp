#include "pch.h"
#include "CStructuredBuffer.h"

#include "RHI/IRHIDevice.h"
#include "RHI/IRHICommandList.h"


CStructuredBuffer::CStructuredBuffer()
	: m_pBuffer(nullptr)
	, m_ElementSize(0)
	, m_ElementCount(0)
	, m_Type(SB_TYPE::SRV_ONLY)
	, m_bSysMemMove(false)
	, m_SRV_RegisterNum(0)
	, m_UAV_RegisterNum(0)
{
}

CStructuredBuffer::CStructuredBuffer(const CStructuredBuffer& _Origin)
	: CEntity(_Origin)
	, m_pBuffer(nullptr)
	, m_ElementSize(_Origin.m_ElementSize)
	, m_ElementCount(_Origin.m_ElementCount)
	, m_Type(_Origin.m_Type)
	, m_bSysMemMove(_Origin.m_bSysMemMove)
	, m_SRV_RegisterNum(0)
	, m_UAV_RegisterNum(0)
{
	Create(m_ElementSize, m_ElementCount, m_Type, m_bSysMemMove);
	if (nullptr != _Origin.m_pBuffer)
	{
		g_pRHIDevice->GetCommandList()->CopyBuffer(m_pBuffer, _Origin.m_pBuffer);
	}
}

CStructuredBuffer::~CStructuredBuffer()
{
	delete m_pBuffer;
}

int CStructuredBuffer::Create(UINT _ElementSize, UINT _ElementCount, SB_TYPE _Type, bool _SysMemMove, void* _InitialData)
{
	// 结构化缓冲区元素必须16字节对齐
	assert(!(_ElementSize % 16));

	delete m_pBuffer;
	m_pBuffer = g_pRHIDevice->CreateBuffer();

	m_ElementSize  = _ElementSize;
	m_ElementCount = _ElementCount;
	m_Type         = _Type;
	m_bSysMemMove  = _SysMemMove;

	const bool bUAV        = (m_Type == SB_TYPE::SRV_UAV);
	const bool bCPUReadback = m_bSysMemMove;

	return m_pBuffer->CreateStructured(m_ElementSize, m_ElementCount, bCPUReadback, bUAV, _InitialData);
}

void CStructuredBuffer::Binding(int _RegisterNum)
{
	m_SRV_RegisterNum = _RegisterNum;
	g_pRHIDevice->GetCommandList()->SetStructuredBufferSRV(_RegisterNum, m_pBuffer);
}

void CStructuredBuffer::Binding_CS_SRV(int _RegisterNum)
{
	m_SRV_RegisterNum = _RegisterNum;
	g_pRHIDevice->GetCommandList()->SetStructuredBufferSRV_CS(_RegisterNum, m_pBuffer);
}

void CStructuredBuffer::Binding_CS_UAV(int _RegisterNum)
{
	m_UAV_RegisterNum = _RegisterNum;
	g_pRHIDevice->GetCommandList()->SetStructuredBufferUAV_CS(_RegisterNum, m_pBuffer);
}


void CStructuredBuffer::SetData(void* _pData, UINT _ElementCount)
{
	if (!_ElementCount)
	{
		_ElementCount = m_ElementCount;
	}

	m_pBuffer->SetData(_pData, _ElementCount * m_ElementSize);
}

void CStructuredBuffer::GetData(void* _pDest, UINT _ElementCount)
{
	if (!_ElementCount)
	{
		_ElementCount = m_ElementCount;
	}
	
	m_pBuffer->GetData(_pDest, _ElementCount * m_ElementSize);
}

void CStructuredBuffer::Clear_SRV()
{
	g_pRHIDevice->GetCommandList()->ClearStructuredBufferSRV(m_SRV_RegisterNum);
}

void CStructuredBuffer::Clear_CS_SRV()
{
	g_pRHIDevice->GetCommandList()->ClearStructuredBufferSRV_CS(m_SRV_RegisterNum);
}

void CStructuredBuffer::Clear_UAV()
{
	g_pRHIDevice->GetCommandList()->ClearStructuredBufferUAV_CS(m_UAV_RegisterNum);
}
