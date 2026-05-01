#include "pch.h"
#include "CMaterialInstance.h"
#include "CGraphicsShader.h"
#include "CConstantBuffer.h"

#ifdef USE_DX11
#include "DX11/DX11Device.h"
#else
#include "DX12/DX12Device.h"
#endif

CMaterialInstance::CMaterialInstance()
    : CMaterial(false)
{
}

CMaterialInstance::~CMaterialInstance()
{
}

void CMaterialInstance::Binding()
{
    if (nullptr == m_ParentMaterial.Get())
        return;

    // 1. Bind shader from parent
    Ptr<CGraphicsShader> pShader = m_ParentMaterial->GetShader();
    if (nullptr == pShader.Get())
        return;
    pShader->Binding();

    // 2. Resolve textures: instance override > parent default
    tMaterialConst finalConst = m_ParentMaterial->GetConst();

    for (UINT i = 0; i < TEX_PARAM::TEX_END; ++i)
    {
        Ptr<CTexture> pTex = m_bOverrideTex[i]
            ? m_arrOverrideTex[i]
            : m_ParentMaterial->GetTextureParam((TEX_PARAM)i);

        if (nullptr == pTex.Get())
        {
            CTexture::Clear(i);
            finalConst.bText[i] = false;
        }
        else
        {
            pTex->Binding(i);
            finalConst.bText[i] = true;
        }
    }

    // 3. Merge scalar params: override where flagged
    for (UINT i = 0; i < 4; ++i)
    {
        if (m_bOverrideScalar[INT_0 + i])
            finalConst.iArr[i] = m_OverrideConst.iArr[i];
        if (m_bOverrideScalar[FLOAT_0 + i])
            finalConst.fArr[i] = m_OverrideConst.fArr[i];
        if (m_bOverrideScalar[VEC2_0 + i])
            finalConst.v2Arr[i] = m_OverrideConst.v2Arr[i];
        if (m_bOverrideScalar[VEC4_0 + i])
            finalConst.v4Arr[i] = m_OverrideConst.v4Arr[i];
        if (m_bOverrideScalar[MAT_0 + i])
            finalConst.matArr[i] = m_OverrideConst.matArr[i];
    }

    // 4. Upload and bind material constant buffer
    CConstantBuffer* pMaterialCB = RHI_DEVICE->GetConstantBuffer(CB_TYPE::MATERIAL);
    pMaterialCB->SetData(&finalConst);
    pMaterialCB->Binding();
}