#include "pch.h"
#include "CMaterial.h"
#include "CGraphicsShader.h"

#ifdef USE_DX11
#include "DX11/DX11Device.h"
#else
#include "DX12/DX12Device.h"
#endif

#include "CConstantBuffer.h"

#include "CLevelMgr.h"
#include "CLevel.h"


CMaterial::CMaterial(bool _bEngineAsset)
	: CAsset(ASSET_TYPE::MATERIAL, _bEngineAsset)
	, m_bDynamic(false)
{
}


CMaterial::~CMaterial()
{
}

void CMaterial::SetShader(Ptr<CGraphicsShader> _Shader)
{
	bool bSave = false;

	CLevel* pCurLevel = CLevelMgr::GetInst()->GetCurrentLevel();
	if (m_Shader != _Shader &&
		!IsEngineAsset() &&
		!m_bDynamic &&
		nullptr != pCurLevel &&
		pCurLevel->GetState() == LEVEL_STATE::STOP)
	{
		bSave = true;
	}

	m_Shader = _Shader;

	if (bSave)
	{
		Save(CPathMgr::GetInst()->GetContentPath() + GetRelativePath());
	}
}

void CMaterial::SetTexParam(TEX_PARAM _Param, Ptr<CTexture> _Tex)
{
	m_arrTex[_Param] = _Tex;
}

void CMaterial::SetTexParamByName(TEX_PARAM _Param, const wstring& _AssetName)
{
	// 不缓存指针,仅记录名字,每次Binding()都通过名字查找(支持Resize后自动更新)
    m_arrTexNameBinding[_Param] = _AssetName;
    m_arrTex[_Param] = nullptr;
}

void* CMaterial::GetScalarParam(SCALAR_PARAM _Param)
{
	switch(_Param)
	{
		case INT_0:
		case INT_1:
		case INT_2:
		case INT_3:
			return &m_Const.iArr[_Param];
			break;
		case FLOAT_0:
		case FLOAT_1:
		case FLOAT_2:
		case FLOAT_3:
			return &m_Const.fArr[_Param - FLOAT_0];
			break;
		case VEC2_0:
		case VEC2_1:
		case VEC2_2:
		case VEC2_3:
			return &m_Const.v2Arr[_Param - VEC2_0];
			break;
		case VEC4_0:
		case VEC4_1:
		case VEC4_2:
		case VEC4_3:
			return &m_Const.v4Arr[_Param - VEC4_0];
			break;
		case MAT_0:
		case MAT_1:
		case MAT_2:
		case MAT_3:
			return &m_Const.matArr[_Param - MAT_0];
			break;
	}

	return nullptr;
}

Ptr<CMaterial> CMaterial::GetDynamicMaterial()
{
	// TODO: 此时关卡应该处于play状态才允许创建动态材质

	Ptr<CMaterial> pDynamicMat = Clone();
	pDynamicMat->m_bDynamic = true;
	return pDynamicMat;
}

void CMaterial::Binding()
{
	// 1. Shader first — sets PSO + root signature before any descriptor staging
    if (nullptr != m_Shader.Get())
    {
        m_Shader->Binding();
    }

    // 2. RootSignature 确定后再 stage 纹理 SRV
    for (UINT i = 0; i < TEX_PARAM::TEX_END; ++i)
    {
        // 按名字每帧重新解析（支持 Resize 后纹理重建自动生效）
        if (!m_arrTexNameBinding[i].empty())
        {
            m_arrTex[i] = CAssetMgr::GetInst()->FindAsset<CTexture>(m_arrTexNameBinding[i]);
        }

        if (nullptr == m_arrTex[i])
        {
            CTexture::Clear(i);
            m_Const.bText[i] = false;
            continue;
        }
        m_arrTex[i]->Binding(i);
        m_Const.bText[i] = true;
    }

    // 3. 材质常量缓冲
    CConstantBuffer* pMaterialCB = RHI_DEVICE->GetConstantBuffer(CB_TYPE::MATERIAL);
    pMaterialCB->SetData(&m_Const);
    pMaterialCB->Binding();
}


int CMaterial::Save(const wstring& _FilePath)
{
	assert(!m_bDynamic && !IsEngineAsset());

	SetRelativePath(CPathMgr::GetInst()->GetRelativePath(_FilePath));

	FILE* pFile = nullptr;
	_wfopen_s(&pFile, _FilePath.c_str(), L"wb");

	// 此处保存材质常量
	fwrite(&m_Const, sizeof(tMaterialConst), 1, pFile);

	// 此处保存shader引用
	SaveAssetRef(m_Shader, pFile);

	// 此处保存纹理引用
	for (UINT i = 0; i < TEX_PARAM::TEX_END; ++i)
	{
		SaveAssetRef(m_arrTex[i], pFile);
	}

	fclose(pFile);

	return S_OK;
}

int CMaterial::Load(const wstring& _FilePath)
{
	assert(!m_bDynamic && !IsEngineAsset());

	FILE* pFile = nullptr;
	_wfopen_s(&pFile, _FilePath.c_str(), L"rb");
	if (nullptr == pFile)
	{
		return E_FAIL;
	}

	// 此处加载材质常量
	fread(&m_Const, sizeof(tMaterialConst), 1, pFile);

	// 此处加载shader引用
	LoadAssetRef(m_Shader, pFile);

	// 此处加载纹理引用
	for (UINT i = 0; i < TEX_PARAM::TEX_END; ++i)
	{
		LoadAssetRef(m_arrTex[i], pFile);
	}

	fclose(pFile);

	return S_OK;
}
