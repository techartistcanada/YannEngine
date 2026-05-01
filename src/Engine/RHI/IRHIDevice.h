#pragma once

#include "RHIPrereqs.h"

class IRHICommandList;
class IRHIGraphicsShader;
class IRHIComputeShader;
class IRHIPipelineState;
class IRHIBuffer;
class IRHITexture;

class IRHIDevice
{
public:
	virtual ~IRHIDevice() = default;

	virtual int init(HWND _hWnd, Vec2 _Resolution) = 0;
	virtual void Present() = 0;
	virtual void ResizeSwapChain(UINT _Width, UINT _Height) = 0;
	virtual bool IsResizing() const = 0;
	virtual void SetResizing(bool _Resizing) = 0;
	virtual Vec2 GetRenderResolution() const = 0;

	virtual IRHICommandList* GetCommandList() = 0;


	// Factory methods
	virtual IRHIGraphicsShader* CreateGraphicsShader() = 0;
	virtual IRHIComputeShader* CreateComputeShader() = 0;
	virtual IRHIPipelineState* CreatePipelineState() = 0;
	virtual IRHIBuffer* CreateBuffer() = 0;
	virtual IRHITexture* CreateTexture() = 0;
};

extern IRHIDevice* g_pRHIDevice;