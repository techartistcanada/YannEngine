#pragma once
#include "../RHI/IRHIBuffer.h"
#include <d3d11.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

class DX11Buffer : public IRHIBuffer
{
private:
	ComPtr<ID3D11Buffer> m_Buffer;
	ComPtr<ID3D11Buffer> m_WriteBuffer;
	ComPtr<ID3D11Buffer> m_ReadBuffer;

	ComPtr<ID3D11ShaderResourceView>  m_SRV;
	ComPtr<ID3D11UnorderedAccessView> m_UAV;

	RHI_BUFFER_TYPE m_Type = RHI_BUFFER_TYPE::VERTEX_BUFFER;
	UINT			m_ByteSize = 0;
	UINT   			m_ElementSize = 0;
	UINT  			m_ElementCount = 0;
	DXGI_FORMAT     m_IndexFormat = DXGI_FORMAT_UNKNOWN;


public:
	int CreateVertex(UINT _stride, UINT _count, const void* _pData = nullptr) override;
	int CreateIndex(DXGI_FORMAT _format, UINT _count, const void* _pData = nullptr) override;
	int CreateConstant(UINT _byteSize) override;
	int CreateStructured(UINT _elementSize, UINT _elementCount, bool _bCPUReadback, bool _bUAV, const void* _pData = nullptr) override;

	void SetData(const void* _pData, UINT _byteSize = 0) override;
	void GetData(void* _pDest, UINT _byteSize = 0) override;

	RHI_BUFFER_TYPE GetType()         const override { return m_Type;        }
    UINT            GetByteSize()     const override { return m_ByteSize;    }
    UINT            GetElementSize()  const override { return m_ElementSize;    }
    UINT            GetElementCount() const override { return m_ElementCount;   }
    DXGI_FORMAT     GetIndexFormat()  const override { return m_IndexFormat; }

    ID3D11Buffer*              GetBuffer() const { return m_Buffer.Get(); }
    ID3D11ShaderResourceView*  GetSRV()    const { return m_SRV.Get();    }
    ID3D11UnorderedAccessView* GetUAV()    const { return m_UAV.Get();    }

public:
    DX11Buffer()  = default;
    ~DX11Buffer() = default;
};
