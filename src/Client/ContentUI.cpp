#include "pch.h"
#include "ContentUI.h"

#include "TreeUI.h"
#include <CAssetMgr.h>

#include "InspectorUI.h"


ContentUI::ContentUI()
	: EditorUI("Content", "##ContentUI")
{
	m_TreeUI = new TreeUI("ContentTreeUI");
	m_TreeUI->SetShowRoot(false);
	m_TreeUI->SetShowFilenameOnly(true);
	m_TreeUI->SetEnableDrag(true);
	AddChildUI(m_TreeUI);

	m_TreeUI->RegisterOnNodeSelDelegate(this, (UI_DELEGATE_1)&ContentUI::SelectAsset);

	ReloadContentToRAM();

	UpdateContent();
}

ContentUI::~ContentUI()
{
}

void ContentUI::UpdateContent()
{
	m_TreeUI->Clear();
	TreeNode* pRootNode = m_TreeUI->AddTreeNode(nullptr, "DummyRoot");

	for (UINT i = 0; i < (UINT)ASSET_TYPE::END; ++i)
	{
		TreeNode* pCategoryNode = m_TreeUI->AddTreeNode(pRootNode, ASSET_TYPE_STRINGS[i]);
		pCategoryNode->SetImGuiFramed(true);

		const map<wstring, Ptr<CAsset>>& mapAssets = CAssetMgr::GetInst()->GetAssets((ASSET_TYPE)i);
		for (const auto& pair : mapAssets)
		{
			m_TreeUI->AddTreeNode(pCategoryNode, ToString(pair.first).c_str(), (DWORD_PTR)pair.second.Get());
		}
	}
}

void ContentUI::ReloadContentToRAM()
{
	ScanAssetsInFolder(CPathMgr::GetInst()->GetContentPath());

	for (size_t i = 0; i < m_vecContentAssetPaths.size(); ++i)
	{
		ASSET_TYPE type = GetAssetTypeFromExtension(m_vecContentAssetPaths[i]);
		switch (type)
		{
			case ASSET_TYPE::PREFAB:
			{
				//CAssetMgr::GetInst()->Load<CPrefab>(m_vecContentAssetPaths[i], m_vecContentAssetPaths[i]);
			}
			break;
			case ASSET_TYPE::MESH:
			{
				CAssetMgr::GetInst()->Load<CMesh>(m_vecContentAssetPaths[i], m_vecContentAssetPaths[i]);
			}
			break;
			case ASSET_TYPE::MESH_DATA:
			{

			}
			break;
			case ASSET_TYPE::MATERIAL:
			{
				CAssetMgr::GetInst()->Load<CMaterial>(m_vecContentAssetPaths[i], m_vecContentAssetPaths[i]);
			}
			break;
			case ASSET_TYPE::TEXTURE:
			{
				CAssetMgr::GetInst()->Load<CTexture>(m_vecContentAssetPaths[i], m_vecContentAssetPaths[i]);
			}
			break;
			case ASSET_TYPE::SOUND:
			{
			}
			break;
			case ASSET_TYPE::END:
				continue;
				break;
		}
	}

	m_vecContentAssetPaths.clear();

	wstring strContentPath = CPathMgr::GetInst()->GetContentPath();
	for (UINT i = 0; i < (UINT)ASSET_TYPE::END; ++i)
	{
		// shader 不算"资产"，不需要扫描加载
		if ((UINT)ASSET_TYPE::GRAPHICS_SHADER == i || (UINT)ASSET_TYPE::COMPUTE_SHADER == i)
		{
			continue;
		}

		const map<wstring, Ptr<CAsset>>& mapAssets = CAssetMgr::GetInst()->GetAssets((ASSET_TYPE)i);

		vector<Ptr<CAsset>> vecToDelete;
		for (const auto& pair : mapAssets)
		{
			if (pair.second->IsEngineAsset())
				continue;
			if (!exists(strContentPath + pair.second->GetRelativePath()))
			{
				vecToDelete.push_back(pair.second);
				//DeleteAsset(pair.second);
			}
		}
		for (auto& pAsset : vecToDelete)
		{
			DeleteAsset(pAsset);
		}
	}
}

void ContentUI::ScanAssetsInFolder(const wstring& _strFolderPath)
{
	wstring strFindPath = _strFolderPath + L"\\*.*";

	WIN32_FIND_DATA FindData = {};
	HANDLE hFind = FindFirstFile(strFindPath.c_str(), &FindData);

	wstring ErrMsg = L"Failed to find assets in folder." + _strFolderPath;
	if (INVALID_HANDLE_VALUE == hFind)
	{
		MessageBox(nullptr, ErrMsg.c_str(), L"Error", MB_OK | MB_ICONERROR);
	}

	while (FindNextFile(hFind, &FindData))
	{
		if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (!wcscmp(FindData.cFileName, L".."))
				continue;

			ScanAssetsInFolder(_strFolderPath + FindData.cFileName + L"\\");
		}
		else
		{
			wstring RelativePath = CPathMgr::GetInst()->GetRelativePath(_strFolderPath + FindData.cFileName);
			m_vecContentAssetPaths.push_back(RelativePath);
		}
	}

	FindClose(hFind);
}

UINT ContentUI::SelectAsset(DWORD_PTR _dwData)
{
	TreeNode* pSelectedNode = (TreeNode*)_dwData;
	pSelectedNode->GetName();
	Ptr<CAsset> pAsset = (CAsset*)pSelectedNode->GetData();

	InspectorUI* pInspector = CImGuiMgr::GetInst()->FindEditorUI<InspectorUI>("Inspector");

	pInspector->SetTargetAsset(pAsset);

	return 0;
}

ASSET_TYPE ContentUI::GetAssetTypeFromExtension(const path& _strAssetPath)
{
	path extension = _strAssetPath.extension();
	if (extension == L".prefab" || extension == L".PREFAB")
	{
		return ASSET_TYPE::PREFAB;
	}
	else if (extension == L".png" || extension == L".PNG" ||
			extension == L".jpg" || extension == L".JPG" ||
			extension == L".bmp" || extension == L".BMP" ||
			extension == L".tga" || extension == L".TGA" ||
			extension == L".dds" || extension == L".DDS")
	{
		return ASSET_TYPE::TEXTURE;
	}
	else if (extension == L".mesh" || extension == L".MESH")
	{
		return ASSET_TYPE::MESH;
	}
	else if (extension == L".mdat" || extension == L".MDAT")
	{
		return ASSET_TYPE::MESH_DATA;
	}
	else if (extension == L".mp3" || extension == L".MP3" ||
			extension == L".wav" || extension == L".WAV" ||
			extension == L".ogg" || extension == L".OGG")
	{
		return ASSET_TYPE::SOUND;
	}
	else if (extension == L".mat" || extension == L".MAT")
	{
		return ASSET_TYPE::MATERIAL;
	}

	return ASSET_TYPE::END;
}

void ContentUI::render_tick()
{
	if (CAssetMgr::GetInst()->IsAssetsChanged())
	{
		UpdateContent();
	}
}
