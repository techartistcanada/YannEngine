#pragma once

// ============================================
// CModelImporter
// 
// 将3D模型文件(glTF/FBX/OBJ等)解析为CGameObject层级树
// 每个aiMesh对应一个子CGameObject(含CTransform+CMeshRenderer)
// 材质和贴图会自动注册到CAssetMgr。
// 
// 用法:
//   CGameObject* pSponza = CModelImporter::Load(L"mesh\\sponza\\sponza.gltf");
//   pLevel->AddObject(0, pSponza);
// ============================================

class CGameObject;

class CModelImporter
{
private:
	CModelImporter() = delete;

public:
	static CGameObject* Load(const wstring& _RelativePath);

};

