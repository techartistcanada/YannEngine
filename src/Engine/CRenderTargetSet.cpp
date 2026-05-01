#include "pch.h"
#include "CRenderTargetSet.h"

//#include "CDX11Device.h"
#include "CTexture.h"
#include "RHI/IRHIDevice.h"
#include "RHI/IRHICommandList.h"

CRenderTargetSet::CRenderTargetSet()
	: m_RTTextures{}
	, m_ClearColors{}
	, m_RTCount(0)
	, m_ViewportHeight(0.0f)
	, m_ViewportWidth(0.0f)
{
}

CRenderTargetSet::~CRenderTargetSet()
{
}


void CRenderTargetSet::OMSet()
{
	IRHITexture* pRTVs[8] = {};
	for (UINT i = 0; i < m_RTCount; ++i)
	{
		pRTVs[i] = m_RTTextures[i]->GetRHITexture();
	}

	IRHITexture* pDSV = m_DSTexture.Get() ? m_DSTexture->GetRHITexture() : nullptr;

	IRHICommandList* pCmdList = g_pRHIDevice->GetCommandList();
	pCmdList->SetRenderTargets(pRTVs, m_RTCount, pDSV);
	pCmdList->SetViewport(0.0f, 0.0f, m_ViewportWidth, m_ViewportHeight);
}

void CRenderTargetSet::ClearTargets()
{
	IRHICommandList* pCmdList = g_pRHIDevice->GetCommandList();
	for (UINT i = 0; i < m_RTCount; ++i)
	{
		if (m_RTTextures[i].Get())
		{
			pCmdList->ClearRenderTarget(m_RTTextures[i]->GetRHITexture(), &m_ClearColors[i].x);
		}
	}
}

void CRenderTargetSet::ClearDepthStencil()
{
	if (nullptr == m_DSTexture)
		return;

	g_pRHIDevice->GetCommandList()->ClearDepthStencil(m_DSTexture->GetRHITexture(), 1.0f, 0);
}

void CRenderTargetSet::Init(Ptr<CTexture>* _RTTextures, UINT _RTCount, Ptr<CTexture> _DSTexture)
{
	for (UINT i = 0; i < _RTCount; ++i)
	{
		m_RTTextures[i] = _RTTextures[i];
	}

	m_RTCount = _RTCount;
	m_DSTexture = _DSTexture;

	// Derive viewport size from the first available texture
	if (_RTCount > 0 && _RTTextures[0].Get())
	{
		m_ViewportWidth  = (float)_RTTextures[0]->GetWidth();
		m_ViewportHeight = (float)_RTTextures[0]->GetHeight();
	}
	else if (_DSTexture.Get())
	{
		m_ViewportWidth  = (float)_DSTexture->GetWidth();
		m_ViewportHeight = (float)_DSTexture->GetHeight();
	}
}
