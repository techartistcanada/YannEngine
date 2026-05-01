#include "pch.h"
#include "CConstantBuffer.h"

#include "RHI/IRHIDevice.h"
#include "RHI/IRHICommandList.h"

CConstantBuffer::CConstantBuffer()
	: m_Type(CB_TYPE::END)
	, m_pBuffer(nullptr)
	, m_ByteSize(0)
{

}

CConstantBuffer::~CConstantBuffer()
{
	delete m_pBuffer;
}

int CConstantBuffer::Create(size_t _bufferSize, CB_TYPE _Type)
{
	m_Type = _Type;
	m_ByteSize = (UINT)_bufferSize;
	m_pBuffer = g_pRHIDevice->CreateBuffer();
	return m_pBuffer->CreateConstant(m_ByteSize);
}

void CConstantBuffer::SetData(void* _pData)
{
	m_pBuffer->SetData(_pData, m_ByteSize);
}

void CConstantBuffer::Binding()
{
	g_pRHIDevice->GetCommandList()->SetConstantBuffer((UINT)m_Type, m_pBuffer);
}

void CConstantBuffer::Binding_CS()
{
	g_pRHIDevice->GetCommandList()->SetConstantBuffer_CS((UINT)m_Type, m_pBuffer);
}

void CConstantBuffer::Clear()
{
	g_pRHIDevice->GetCommandList()->SetConstantBuffer((UINT)m_Type, nullptr);
}
