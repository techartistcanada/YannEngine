#include "pch.h"
#include "MaterialUI.h"

#include "ParameterUI.h"

MaterialUI::MaterialUI()
	: AssetUI("Material", "##Material", ASSET_TYPE::MATERIAL)
{
}

MaterialUI::~MaterialUI()
{
}

void MaterialUI::render_tick()
{
	render_title();

	string strName = ToString(GetTargetAsset()->GetName());

	Ptr<CMaterial> pMaterial = dynamic_cast<CMaterial*>(GetTargetAsset().Get());
	assert(pMaterial.Get());

	// --------------------
	// Material Name
	// --------------------
	ImGui::Text("Material Name");
	ImGui::SameLine(100);
	ImGui::InputText("##MaterialNameMaterialUI", (char*)strName.c_str(), strName.capacity(), ImGuiInputTextFlags_ReadOnly);

	// --------------------
	// Shader Name
	// --------------------
	string strShaderName = (nullptr == pMaterial->GetShader() ? "" : ToString(pMaterial->GetShader()->GetKey()));

	ImGui::Text("Shader Name");
	ImGui::SameLine(100);
	ImGui::InputText("##MaterialShaderNameMaterialUI", (char*)strShaderName.c_str(), strShaderName.capacity(), ImGuiInputTextFlags_ReadOnly);
	// -------------------------------
	// Drag and Drop Shader
	// -------------------------------

	// -------- BEGIN Receive Drop ------------------
	if(ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ContentTreeUI");

		if (nullptr != payload)
		{
			DWORD_PTR dwData = 0;
			memcpy(&dwData, payload->Data, payload->DataSize);
			Ptr<CAsset> pAsset = (CAsset*)dwData;

			if (pAsset->GetAssetType() == ASSET_TYPE::GRAPHICS_SHADER)
			{
				Ptr<CGraphicsShader> pShader = (CGraphicsShader*)pAsset.Get();
				pMaterial->SetShader(pShader);
			}
		}
		ImGui::EndDragDropTarget();
	}
	// -------- END ------------------

	// -------------------------------
	// Shader Scalar Parameters
	// -------------------------------
	ImGui::Separator();
	ImGui::Text("Shader Parameters");
	ImGui::Text("");

	if (nullptr == pMaterial->GetShader())
		return;

	Ptr<CGraphicsShader> pShader = pMaterial->GetShader();
	vector<tShaderScalarParam> vecScalarParams = pShader->GetScalarParams();

	for(size_t i = 0; i < vecScalarParams.size(); i++)
	{
		// SCALAR_PARAM TYPE
		switch (vecScalarParams[i].Param)
		{
			case SCALAR_PARAM::INT_0:
			case SCALAR_PARAM::INT_1:
			case SCALAR_PARAM::INT_2:
			case SCALAR_PARAM::INT_3:
			{
				const string& desc = vecScalarParams[i].strDesc;
				bool bUseCheckbox = (desc.find("Flip") != string::npos
								  || desc.find("Enable") != string::npos
								  || desc.find("Use") != string::npos
								  || desc.find("Cutout") != string::npos);

				if (bUseCheckbox)
				{
					ParameterUI::Param_Checkbox(vecScalarParams[i].strDesc, (int*)pMaterial->GetScalarParam(vecScalarParams[i].Param));
				}
				else
				{
					ParameterUI::Param_DragInt(vecScalarParams[i].strDesc, (int*)pMaterial->GetScalarParam(vecScalarParams[i].Param));
				}
			}
				break;
			case SCALAR_PARAM::FLOAT_0:
			case SCALAR_PARAM::FLOAT_1:
			case SCALAR_PARAM::FLOAT_2:
			case SCALAR_PARAM::FLOAT_3:
				if (ParameterUI::Param_DragFloat(vecScalarParams[i].strDesc, (float*)pMaterial->GetScalarParam(vecScalarParams[i].Param), 0.01f))
				{
				}
				break;
			case SCALAR_PARAM::VEC2_0:
			case SCALAR_PARAM::VEC2_1:
			case SCALAR_PARAM::VEC2_2:
			case SCALAR_PARAM::VEC2_3:
				if (ParameterUI::Param_DragVec2(vecScalarParams[i].strDesc, (Vec2*)pMaterial->GetScalarParam(vecScalarParams[i].Param), 0.01f))
				{
				}
				break;
			case SCALAR_PARAM::VEC4_0:
			case SCALAR_PARAM::VEC4_1:
			case SCALAR_PARAM::VEC4_2:
			case SCALAR_PARAM::VEC4_3:
				if (ParameterUI::Param_DragVec4(vecScalarParams[i].strDesc, (Vec4*)pMaterial->GetScalarParam(vecScalarParams[i].Param), 0.01f))
				{
				}
				break;
			case SCALAR_PARAM::MAT_0:
			case SCALAR_PARAM::MAT_1:
			case SCALAR_PARAM::MAT_2:
			case SCALAR_PARAM::MAT_3:
				if (ParameterUI::Param_DragMatrix(vecScalarParams[i].strDesc, (Matrix*)pMaterial->GetScalarParam(vecScalarParams[i].Param), 0.01f))
				{
				}
				break;
		}
	}
	// -------------------------------
	// Shader Texture Parameters
	// -------------------------------
	vector<tShaderTextureParam> vecTextureParams = pShader->GetTextureParams();
	for (size_t i = 0; i < vecTextureParams.size(); i++)
	{
		ParameterUI::RegisterSelTexDelegate(this, (UI_DELEGATE_1)&MaterialUI::SelectTexture);

		if (ParameterUI::Param_Texture(vecTextureParams[i].strDesc, pMaterial->GetTextureParam(vecTextureParams[i].Param)))
		{
			m_CurTargetParam = vecTextureParams[i].Param;
		}
		ImGui::Text("");
	}

}

void MaterialUI::TargetAssetChanged()
{

}

UINT MaterialUI::SelectTexture(DWORD_PTR _Selected)
{
	string* pStrSelected = (string*)_Selected;
	wstring strTextureKey = ToWString(*pStrSelected);

	Ptr<CTexture> pTexture = CAssetMgr::GetInst()->FindAsset<CTexture>(strTextureKey);
	Ptr<CMaterial> pMaterial = dynamic_cast<CMaterial*>(GetTargetAsset().Get());

	pMaterial->SetTexParam(m_CurTargetParam, pTexture);
	return 0;
}
