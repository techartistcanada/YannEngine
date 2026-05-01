#include "pch.h"
#include "CModelImporter.h"

#include "CPathMgr.h"
#include "CAssetMgr.h"
#include "CGameObject.h"
#include "components.h"
#include "assets.h"
#include "CMaterialInstance.h"

// ======== ASSIMP ===========
#define ASSIMP_DLL
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#ifdef _DEBUG
	#pragma comment(lib, "assimp-vc145-mtd.lib")
#else
	#pragma comment(lib, "assimp-vc145-mt.lib")
#endif
#ifndef AI_MATKEY_GLTF_ALPHAMODE
	#define AI_MATKEY_GLTF_ALPHAMODE "$mat.gltf.alphaMode", 0, 0
#endif
// ======== ASSIMP ===========

// ============================================
// Forward declarations (file-local helpers)
// ============================================
static wstring  WStringFromUTF8(const char* _szUTF8);
static string   UTF8FromWString(const wstring& _wstr);
static wstring  GetDirectoryFromPath(const wstring& _Path);
static Vec3     ToVec3(const aiVector3D& _v);
static void     DecomposeAiMatrix(const aiMatrix4x4& _aiMat, Vec3& _outPos, Vec3& _outRot, Vec3& _outScale);
static bool     IsDecalMesh(const aiMesh* _pMesh, const aiScene* _pScene);

static void     ProcessNode(
	const aiNode* _pNode,
	const aiScene* _pScene,
	CGameObject* _pParent,
	const vector<Ptr<CMesh>>& _vecMeshes,
	const vector<Ptr<CMaterial>>& _vecMaterials);

static Ptr<CMesh> CreateMesh(
	const aiMesh* _pMesh,
	const wstring& _MeshKey);

static Ptr<CMaterial> CreateMaterial(
	const aiMaterial* _pAiMat,
	const aiScene* _pScene,
	const wstring& _ModelDir,
	const wstring& _MatKey,
	Ptr<CGraphicsShader> _pShader);

static Ptr<CTexture> LoadTexture(
	const aiMaterial* _pAiMat,
	aiTextureType _Type,
	const aiScene* _pScene,
	const wstring& _ModelDir);


// ============================================================
//  CModelImporter::Load
// ============================================================
CGameObject* CModelImporter::Load(const wstring& _RelativePath)
{
	// =====================
	// 1. Build full path
	// =====================
	wstring strFullPath = CPathMgr::GetInst()->GetContentPath();
	strFullPath += _RelativePath;

	string strUTF8Path = UTF8FromWString(strFullPath);

	// =====================
	// 2. Import via Assimp
	// =====================
	Assimp::Importer importer;
	const aiScene* pScene = importer.ReadFile(strUTF8Path,
		  aiProcess_Triangulate
		| aiProcess_CalcTangentSpace
		| aiProcess_JoinIdenticalVertices
		| aiProcess_ConvertToLeftHanded
		| aiProcess_GenSmoothNormals
		// NOTE: 不使用 aiProcess_PreTransformVertices, 保留节点层级
	);

	if (nullptr == pScene
		|| (pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		|| nullptr == pScene->mRootNode)
	{
		string err = importer.GetErrorString();
		wstring wErr = WStringFromUTF8(err.c_str());
		MessageBox(nullptr, wErr.c_str(), L"CModelImporter: Failed to load model", MB_OK);
		return nullptr;
	}

	// 模型文件所在目录（用于定位贴图的相对路径）
	wstring strModelDir = GetDirectoryFromPath(strFullPath);

	// 取得用于生成唯一 Key 的模型名称前缀
	// e.g. "mesh\\sponza\\sponza.gltf" → "sponza"
	wstring strModelName;
	{
		path p(_RelativePath);
		strModelName = p.stem().wstring();
	}

	// =====================
	// 3. Get PBR gltf material
	// =====================
	Ptr<CMaterial> pDefaultMat = CAssetMgr::GetInst()->FindAsset<CMaterial>(L"Std3DDeferredPBRgltfMaterial");
	Ptr<CGraphicsShader> pDeferredShader = nullptr;
	if (nullptr != pDefaultMat.Get())
	{
		pDeferredShader = pDefaultMat->GetShader();
	}

	// =====================
	// 4. Pre-create all CMesh assets (one per aiMesh)
	// =====================
	vector<Ptr<CMesh>> vecMeshes;
	vecMeshes.resize(pScene->mNumMeshes);

	for (UINT i = 0; i < pScene->mNumMeshes; ++i)
	{
		const aiMesh* pAiMesh = pScene->mMeshes[i];

		// TODO: Phase 1: 跳过 decal mesh, 留 nullptr
		if (IsDecalMesh(pAiMesh, pScene))
			continue;

		// Key: "Sponza_Mesh_0_walls"
		wstring strMeshName = WStringFromUTF8(pAiMesh->mName.C_Str());
		wstring strKey = strModelName + L"_Mesh_" + std::to_wstring(i) + L"_" + strMeshName;

		// 如果 Asset 已存在（重复加载同一模型），直接复用
		Ptr<CMesh> pExisting = CAssetMgr::GetInst()->FindAsset<CMesh>(strKey);
		if (nullptr != pExisting.Get())
		{
			vecMeshes[i] = pExisting;
			continue;
		}

		vecMeshes[i] = CreateMesh(pAiMesh, strKey);
	}

	// =====================
	// 5. Pre-create all CMaterial assets (one per aiMaterial)
	// =====================
	vector<Ptr<CMaterial>> vecMaterials;
	vecMaterials.resize(pScene->mNumMaterials);

	for (UINT i = 0; i < pScene->mNumMaterials; ++i)
	{
		const aiMaterial* pAiMat = pScene->mMaterials[i];

		aiString aiMatName;
		pAiMat->Get(AI_MATKEY_NAME, aiMatName);
		wstring strMatName = WStringFromUTF8(aiMatName.C_Str());
		wstring strKey = strModelName + L"_Mat_" + std::to_wstring(i) + L"_" + strMatName;

		Ptr<CMaterial> pExisting = CAssetMgr::GetInst()->FindAsset<CMaterial>(strKey);
		if (nullptr != pExisting.Get())
		{
			vecMaterials[i] = pExisting;
			continue;
		}

		vecMaterials[i] = CreateMaterial(pAiMat, pScene, strModelDir, strKey, pDeferredShader);
	}

	// =====================
	// 6. Build CGameObject hierarchy by walking the aiNode tree
	// =====================
	CGameObject* pRoot = new CGameObject;
	pRoot->SetName(strModelName);
	pRoot->AddComponent(new CTransform);
	pRoot->Transform()->SetRelativeScale(Vec3(1.f, 1.f, 1.f));

	ProcessNode(pScene->mRootNode, pScene, pRoot, vecMeshes, vecMaterials);

	return pRoot;
}



// ============================================================
//  CreateMesh — 从 aiMesh 提取顶点/索引, 创建 CMesh 并注册到 CAssetMgr
// ============================================================
static Ptr<CMesh> CreateMesh(const aiMesh* _pMesh, const wstring& _MeshKey)
{
	// ---- 提取顶点 ----
	vector<Vertex> vecVtx;
	vecVtx.reserve(_pMesh->mNumVertices);

	for (UINT v = 0; v < _pMesh->mNumVertices; ++v)
	{
		Vertex vtx = {};

		vtx.Pos = ToVec3(_pMesh->mVertices[v]);
		vtx.Color = Vec4(1.f, 1.f, 1.f, 1.f);

		if (_pMesh->HasTextureCoords(0))
			vtx.UV = Vec2(_pMesh->mTextureCoords[0][v].x,
						  _pMesh->mTextureCoords[0][v].y);

		if (_pMesh->HasNormals())
			vtx.vNormal = ToVec3(_pMesh->mNormals[v]);

		if (_pMesh->HasTangentsAndBitangents())
		{
			vtx.vTangent  = ToVec3(_pMesh->mTangents[v]);
			vtx.vBinormal = ToVec3(_pMesh->mBitangents[v]);
		}

		vecVtx.push_back(vtx);
	}

	// ---- 提取索引 ----
	vector<UINT> vecIdx;
	for (UINT f = 0; f < _pMesh->mNumFaces; ++f)
	{
		const aiFace& face = _pMesh->mFaces[f];
		for (UINT i = 0; i < face.mNumIndices; ++i)
			vecIdx.push_back(face.mIndices[i]);
	}

	if (vecVtx.empty() || vecIdx.empty())
		return nullptr;

	// ---- 创建 CMesh 并注册 ----
	CMesh* pMesh = new CMesh;
	if (FAILED(pMesh->Create(vecVtx.data(), vecVtx.size(), vecIdx.data(), vecIdx.size())))
	{
		delete pMesh;
		return nullptr;
	}

	CAssetMgr::GetInst()->AddAsset<CMesh>(_MeshKey, pMesh);
	return CAssetMgr::GetInst()->FindAsset<CMesh>(_MeshKey);
}


// ============================================================
//  CreateMaterial — 从 aiMaterial 创建 CMaterialInstance, 绑定贴图并注册
// ============================================================
static Ptr<CMaterial> CreateMaterial(
	const aiMaterial* _pAiMat,
	const aiScene* _pScene,
	const wstring& _ModelDir,
	const wstring& _MatKey,
	Ptr<CGraphicsShader> _pShader)
{
	// Get the shared parent material (shader + default params)
	Ptr<CMaterial> pParent = CAssetMgr::GetInst()->FindAsset<CMaterial>(L"Std3DDeferredPBRMaterial");
	if (nullptr == pParent.Get())
		return nullptr;

	CMaterialInstance* pMI = new CMaterialInstance;
	pMI->SetParentMaterial(pParent);
	pMI->SetScalarOverride(SCALAR_PARAM::INT_2, 1); // glTF MR format (no AO in R channel)

	// --- TEX_0: Base Color(Albedo) ---
	Ptr<CTexture> pAlbedo = LoadTexture(_pAiMat, aiTextureType_BASE_COLOR, _pScene, _ModelDir);
	if (nullptr == pAlbedo.Get())
		pAlbedo = LoadTexture(_pAiMat, aiTextureType_DIFFUSE, _pScene, _ModelDir);
	if (nullptr != pAlbedo.Get())
		pMI->SetTexOverride(TEX_PARAM::TEX_0, pAlbedo);

	// --- TEX_1: Normal Map ---
	Ptr<CTexture> pNormal = LoadTexture(_pAiMat, aiTextureType_NORMALS, _pScene, _ModelDir);
	if (nullptr == pNormal.Get())
		pNormal = LoadTexture(_pAiMat, aiTextureType_HEIGHT, _pScene, _ModelDir);
	if (nullptr != pNormal.Get())
		pMI->SetTexOverride(TEX_PARAM::TEX_1, pNormal);

	// --- TEX_2: glTF Metallic/Roughness (R=unused, G=Roughness, B=Metallic) ---
	Ptr<CTexture> pMetalRough = LoadTexture(_pAiMat, aiTextureType_UNKNOWN, _pScene, _ModelDir);
	if (nullptr == pMetalRough.Get())
		pMetalRough = LoadTexture(_pAiMat, aiTextureType_METALNESS, _pScene, _ModelDir);
	if (nullptr == pMetalRough.Get())
		pMetalRough = LoadTexture(_pAiMat, aiTextureType_DIFFUSE_ROUGHNESS, _pScene, _ModelDir);
	if (nullptr != pMetalRough.Get())
		pMI->SetTexOverride(TEX_PARAM::TEX_2, pMetalRough);

	// --- TEX_3: Emissive (glTF shader reads emissive from TEX_3) ---
	Ptr<CTexture> pEmissive = LoadTexture(_pAiMat, aiTextureType_EMISSIVE, _pScene, _ModelDir);
	if (nullptr != pEmissive.Get())
		pMI->SetTexOverride(TEX_PARAM::TEX_3, pEmissive);

	// --- Scalar PBR factors (fallback when no texture, also used as multiplier) ---
	float metallicFactor  = 0.f;    // 安全默认值
	float roughnessFactor = 0.5f;

	// 不管读取是否成功，都要override，防止继承危险的parent默认值
	_pAiMat->Get(AI_MATKEY_METALLIC_FACTOR,  metallicFactor);
	_pAiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughnessFactor);
	pMI->SetScalarOverride(SCALAR_PARAM::FLOAT_0, metallicFactor);
	pMI->SetScalarOverride(SCALAR_PARAM::FLOAT_1, roughnessFactor);

	// --- Base Color Factor (glTF: baseColorFactor, used as fallback or multiplier) ---
	aiColor4D baseColorFactor(1.f, 1.f, 1.f, 1.f);
	if (AI_SUCCESS == _pAiMat->Get(AI_MATKEY_BASE_COLOR, baseColorFactor))
	{
		Vec4 vBaseColor(baseColorFactor.r, baseColorFactor.g, baseColorFactor.b, baseColorFactor.a);
		pMI->SetScalarOverride(SCALAR_PARAM::VEC4_0, vBaseColor);
	}
	else
	{
		// fallback: try diffuse color (older glTF or non-PBR materials)
		aiColor4D diffuseColor(1.f, 1.f, 1.f, 1.f);
		if (AI_SUCCESS == _pAiMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor))
		{
			Vec4 vBaseColor(diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a);
			pMI->SetScalarOverride(SCALAR_PARAM::VEC4_0, vBaseColor);
		}
		else
		{
			pMI->SetScalarOverride(SCALAR_PARAM::VEC4_0, Vec4(1.f, 1.f, 1.f, 1.f));
		}
	}

	// --- Emissive Factor (glTF: emissiveFactor, used as fallback or multiplier) ---
	aiColor3D emissiveFactor(0.f, 0.f, 0.f);
	if (AI_SUCCESS == _pAiMat->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveFactor))
	{
		Vec4 vEmissive(emissiveFactor.r, emissiveFactor.g, emissiveFactor.b, 0.f);
		pMI->SetScalarOverride(SCALAR_PARAM::VEC4_1, vEmissive);
	}

	// --- Alpha mode ---
	aiString alphaMode;
	if (AI_SUCCESS == _pAiMat->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode))
	{
		if (strcmp(alphaMode.C_Str(), "MASK") == 0)
			pMI->SetScalarOverride(SCALAR_PARAM::INT_0, 1);
	}

	CAssetMgr::GetInst()->AddAsset<CMaterial>(_MatKey, pMI);
	return CAssetMgr::GetInst()->FindAsset<CMaterial>(_MatKey);
}

// ============================================================
//  LoadTexture — 从 aiMaterial 中读取指定类型的贴图路径并加载
// ============================================================
static Ptr<CTexture> LoadTexture(
	const aiMaterial* _pAiMat,
	aiTextureType _Type,
	const aiScene* _pScene,
	const wstring& _ModelDir)
{
	if (_pAiMat->GetTextureCount(_Type) == 0)
		return nullptr;

	aiString aiPath;
	if (AI_SUCCESS != _pAiMat->GetTexture(_Type, 0, &aiPath))
		return nullptr;

	// --- 检查是否为嵌入贴图 (glb) ---
	const aiTexture* pEmbedded = _pScene->GetEmbeddedTexture(aiPath.C_Str());
	if (nullptr != pEmbedded)
	{
		// TODO: 支持从内存加载嵌入纹理 (glb 格式)
		// 目前先跳过, 大多数 Sponza 发行版使用外部贴图文件
		return nullptr;
	}

	// --- 外部贴图文件 ---
	wstring strTexRelPath = WStringFromUTF8(aiPath.C_Str());

	// Assimp 返回的路径可能使用 '/' 或 '\\'
	for (auto& ch : strTexRelPath)
	{
		if (ch == L'/') ch = L'\\';
	}

	// 构建完整磁盘路径: 模型目录 + 贴图相对路径
	wstring strFullTexPath = _ModelDir + strTexRelPath;

	// 转换为 Content 目录下的相对路径 (CAssetMgr::Load 需要)
	wstring strContentRelPath = CPathMgr::GetInst()->GetRelativePath(strFullTexPath);
	if (strContentRelPath.empty())
	{
		// 贴图不在 Content 目录下, 尝试直接用模型目录下的相对路径作为 key
		strContentRelPath = strTexRelPath;
	}

	// 用相对路径作为 Key（确保同一贴图不重复加载）
	Ptr<CTexture> pTex = CAssetMgr::GetInst()->Load<CTexture>(strContentRelPath, strContentRelPath);
	return pTex;
}


// ============================================================
//  Utility functions
// ============================================================
static wstring WStringFromUTF8(const char* _szUTF8)
{
	if (nullptr == _szUTF8 || '\0' == _szUTF8[0])
		return L"";

	int iLen = MultiByteToWideChar(CP_UTF8, 0, _szUTF8, -1, nullptr, 0);
	wstring wstr(iLen - 1, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, _szUTF8, -1, wstr.data(), iLen);
	return wstr;
}

static string UTF8FromWString(const wstring& _wstr)
{
	if (_wstr.empty())
		return "";

	int iLen = WideCharToMultiByte(CP_UTF8, 0, _wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
	string str(iLen - 1, '\0');
	WideCharToMultiByte(CP_UTF8, 0, _wstr.c_str(), -1, str.data(), iLen, nullptr, nullptr);
	return str;
}

static wstring GetDirectoryFromPath(const wstring& _Path)
{
	path p(_Path);
	return p.parent_path().wstring() + L"\\";
}

static Vec3 ToVec3(const aiVector3D& _v)
{
	return Vec3(_v.x, _v.y, _v.z);
}

static void DecomposeAiMatrix(const aiMatrix4x4& _aiMat, Vec3& _outPos, Vec3& _outRot, Vec3& _outScale)
{
	// aiMatrix4x4 uses column-vector convention (translation in column 4)
	// XMMATRIX uses row-vector convention (translation in row 4)
	// → must transpose
	XMMATRIX mat(
		_aiMat.a1, _aiMat.b1, _aiMat.c1, _aiMat.d1,
		_aiMat.a2, _aiMat.b2, _aiMat.c2, _aiMat.d2,
		_aiMat.a3, _aiMat.b3, _aiMat.c3, _aiMat.d3,
		_aiMat.a4, _aiMat.b4, _aiMat.c4, _aiMat.d4
	);

	XMVECTOR vScale, vRotQuat, vTrans;
	XMMatrixDecompose(&vScale, &vRotQuat, &vTrans, mat);

	_outPos   = Vec3(XMVectorGetX(vTrans), XMVectorGetY(vTrans), XMVectorGetZ(vTrans));
	_outScale = Vec3(XMVectorGetX(vScale), XMVectorGetY(vScale), XMVectorGetZ(vScale));

	// Quaternion → Euler (Pitch=X, Yaw=Y, Roll=Z)
	float qx = XMVectorGetX(vRotQuat);
	float qy = XMVectorGetY(vRotQuat);
	float qz = XMVectorGetZ(vRotQuat);
	float qw = XMVectorGetW(vRotQuat);

	// Roll (X)
	float sinr_cosp = 2.f * (qw * qx + qy * qz);
	float cosr_cosp = 1.f - 2.f * (qx * qx + qy * qy);
	float rotX = atan2f(sinr_cosp, cosr_cosp);

	// Pitch (Y)
	float sinp = 2.f * (qw * qy - qz * qx);
	float rotY = (fabsf(sinp) >= 1.f) ? copysignf(XM_PIDIV2, sinp) : asinf(sinp);

	// Yaw (Z)
	float siny_cosp = 2.f * (qw * qz + qx * qy);
	float cosy_cosp = 1.f - 2.f * (qy * qy + qz * qz);
	float rotZ = atan2f(siny_cosp, cosy_cosp);

	_outRot = Vec3(rotX, rotY, rotZ);
}

// ============================================================
//  HasAnyMeshInSubtree — 检查子树中是否存在至少一个 mesh
// ============================================================
static bool HasAnyMeshInSubtree(const aiNode* _pNode)
{
	if (_pNode->mNumMeshes > 0)
		return true;

	for (UINT i = 0; i < _pNode->mNumChildren; ++i)
	{
		if (HasAnyMeshInSubtree(_pNode->mChildren[i]))
			return true;
	}
	return false;
}

// ============================================================
//  IsDecalMesh — 判断 aiMesh 是否属于 decal（贴花）
//  Phase 1 跳过 decal, 后续阶段再实现 alpha-blended decal 渲染
// ============================================================
static bool IsDecalMesh(const aiMesh* _pMesh, const aiScene* _pScene)
{
	// 检查 mesh 名称
	string strMeshName(_pMesh->mName.C_Str());
	if (strMeshName.find("decal") != string::npos
		|| strMeshName.find("Decal") != string::npos
		|| strMeshName.find("DECAL") != string::npos)
	{
		return true;
	}

	// 检查对应 material 名称
	if (_pMesh->mMaterialIndex < _pScene->mNumMaterials)
	{
		aiString aiMatName;
		_pScene->mMaterials[_pMesh->mMaterialIndex]->Get(AI_MATKEY_NAME, aiMatName);
		string strMatName(aiMatName.C_Str());

		if (strMatName.find("decal") != string::npos
			|| strMatName.find("Decal") != string::npos
			|| strMatName.find("DECAL") != string::npos)
		{
			return true;
		}
	}

	return false;
}
// ============================================================
//  IsAuxiliaryNode — 判断是否为建模软件导出的辅助节点
//  (Camera Target, Light Target, etc.)
// ============================================================
static bool IsAuxiliaryNode(const aiNode* _pNode)
{
	// 没有 mesh 的节点才可能是辅助节点
	if (_pNode->mNumMeshes > 0)
		return false;

	string name(_pNode->mName.C_Str());

	// 3ds Max / modeling tool camera & light targets
	if (name.find("Camera") != string::npos
		|| name.find("Target") != string::npos
		|| name.find("SUN") != string::npos
		|| name.find("PhysCamera") != string::npos)
	{
		// 确认子树中也没有任何 mesh
		if (!HasAnyMeshInSubtree(_pNode))
			return true;
	}

	return false;
}

// ============================================================
//  ProcessNode — 递归遍历 aiNode 树, 构建 CGameObject 层级
//  改进: 跳过辅助节点, 折叠空的中间节点
// ============================================================
static void ProcessNode(
	const aiNode* _pNode,
	const aiScene* _pScene,
	CGameObject* _pParent,
	const vector<Ptr<CMesh>>& _vecMeshes,
	const vector<Ptr<CMaterial>>& _vecMaterials)
{
	// --- 跳过辅助节点 (Camera Target, Light Target 等) ---
	if (IsAuxiliaryNode(_pNode))
		return;

	// --- 跳过没有 mesh 且子树中也没有 mesh 的空节点 ---
	if (_pNode->mNumMeshes == 0 && !HasAnyMeshInSubtree(_pNode))
		return;


	// --- 为当前 aiNode 创建一个 CGameObject ---
	CGameObject* pNodeObj = new CGameObject;
	wstring strNodeName = WStringFromUTF8(_pNode->mName.C_Str());
	if (strNodeName.empty())
		strNodeName = L"Node";
	pNodeObj->SetName(strNodeName);

	// Transform: 分解 aiNode 的局部变换矩阵
	pNodeObj->AddComponent(new CTransform);
	Vec3 vPos, vRot, vScale;
	DecomposeAiMatrix(_pNode->mTransformation, vPos, vRot, vScale);
	pNodeObj->Transform()->SetRelativePos(vPos);
	pNodeObj->Transform()->SetRelativeRotation(vRot);
	pNodeObj->Transform()->SetRelativeScale(vScale);

	_pParent->AddChild(pNodeObj);

	// --- 处理该节点引用的所有 Mesh ---
	if (_pNode->mNumMeshes == 1)
	{
		// 只有一个 mesh: 直接把 CMeshRenderer 挂在当前节点上
		UINT meshIdx = _pNode->mMeshes[0];
		// 跳过被过滤的 mesh (decal 等)
		if (nullptr != _vecMeshes[meshIdx].Get())
		{
			const aiMesh* pAiMesh = _pScene->mMeshes[meshIdx];
			UINT matIdx = pAiMesh->mMaterialIndex;

			pNodeObj->AddComponent(new CMeshRenderer);
			pNodeObj->MeshRenderer()->SetMesh(_vecMeshes[meshIdx]);
			if (matIdx < _vecMaterials.size() && nullptr != _vecMaterials[matIdx].Get())
			{
				pNodeObj->MeshRenderer()->SetMaterial(_vecMaterials[matIdx]);
			}
		}
	}
	else if (_pNode->mNumMeshes > 1)
	{
		// 多个 mesh: 每个 mesh 创建一个子 CGameObject
		for (UINT i = 0; i < _pNode->mNumMeshes; ++i)
		{
			UINT meshIdx = _pNode->mMeshes[i];

			// 跳过被过滤的 mesh (decal 等)
			if (nullptr == _vecMeshes[meshIdx].Get())
				continue;

			const aiMesh* pAiMesh = _pScene->mMeshes[meshIdx];
			UINT matIdx = pAiMesh->mMaterialIndex;

			CGameObject* pMeshObj = new CGameObject;
			wstring strMeshName = WStringFromUTF8(pAiMesh->mName.C_Str());
			pMeshObj->SetName(strMeshName.empty() ? L"SubMesh_" + std::to_wstring(i) : strMeshName);

			pMeshObj->AddComponent(new CTransform);
			pMeshObj->Transform()->SetRelativeScale(Vec3(1.f, 1.f, 1.f));

			pMeshObj->AddComponent(new CMeshRenderer);
			pMeshObj->MeshRenderer()->SetMesh(_vecMeshes[meshIdx]);
			if (matIdx < _vecMaterials.size() && nullptr != _vecMaterials[matIdx].Get())
			{
				pMeshObj->MeshRenderer()->SetMaterial(_vecMaterials[matIdx]);
			}

			pNodeObj->AddChild(pMeshObj);
		}
	}

	// --- 递归处理子节点 ---
	for (UINT i = 0; i < _pNode->mNumChildren; ++i)
	{
		ProcessNode(_pNode->mChildren[i], _pScene, pNodeObj, _vecMeshes, _vecMaterials);
	}
}