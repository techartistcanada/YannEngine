#include "pch.h"
#include "CMesh.h"

//#include "CDX11Device.h"
#include "RHI/IRHIDevice.h"
#include "RHI/IRHICommandList.h"
#include "CGameObject.h"

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
// ======== ASSIMP ===========



CMesh::CMesh(bool _bEngineAsset)
	: CAsset(ASSET_TYPE::MESH, _bEngineAsset)
	, m_pVB(nullptr)
	, m_VtxCount(0)
	, m_VtxSysMem(nullptr)
	, m_pIB(nullptr)
	, m_IdxCount(0)
	, m_IdxSysMem(nullptr)
{

}

CMesh::~CMesh()
{
	if (nullptr != m_VtxSysMem)
		delete[] m_VtxSysMem;
	if (nullptr != m_IdxSysMem)
		delete[] m_IdxSysMem;
	if (nullptr != m_pVB)
		delete m_pVB;
	if (nullptr != m_pIB)
		delete m_pIB;
}

int CMesh::Create(Vertex* _VtxSysMem, size_t _VtxCount, UINT* _IdxSysMem, size_t _IdxCount)
{
	m_VtxCount = (UINT)_VtxCount;
	m_IdxCount = (UINT)_IdxCount;
	// ==========
	// 1.创建顶点缓冲区
	// ==========
	m_pVB = g_pRHIDevice->CreateBuffer();
	if (FAILED(m_pVB->CreateVertex(sizeof(Vertex), m_VtxCount, _VtxSysMem)))
	{
		return E_FAIL;
	}

	// ==========
	// 2.创建索引缓冲区
	// ==========
	m_pIB = g_pRHIDevice->CreateBuffer();
	if (FAILED(m_pIB->CreateIndex(DXGI_FORMAT_R32_UINT, m_IdxCount, _IdxSysMem)))
	{
		return E_FAIL;
	}

	// ==========
	// 3.将数据保存到系统内存
	// ==========
	m_VtxSysMem = new Vertex[m_VtxCount];
	memcpy(m_VtxSysMem, _VtxSysMem, sizeof(Vertex) * m_VtxCount);

	m_IdxSysMem = new UINT[m_IdxCount];
	memcpy(m_IdxSysMem, _IdxSysMem, sizeof(UINT) * m_IdxCount);

	// ==========
	// 4.计算包围球 (Bounding Box/Sphere from AABB)
	// ==========
	if (m_VtxCount > 0)
	{
		Vec3 vMin = _VtxSysMem[0].Pos;
		Vec3 vMax = _VtxSysMem[0].Pos;

		for (UINT i = 1; i < m_VtxCount; ++i)
		{
			Vec3 p = _VtxSysMem[i].Pos;
			if (p.x < vMin.x) vMin.x = p.x;
			if (p.y < vMin.y) vMin.y = p.y;
			if (p.z < vMin.z) vMin.z = p.z;
			if (p.x > vMax.x) vMax.x = p.x;
			if (p.y > vMax.y) vMax.y = p.y;
			if (p.z > vMax.z) vMax.z = p.z;
		}

		m_BoundCenter = (vMin + vMax) * 0.5f;

		// Radius = max distance from center to any vertex
		float maxDistSq = 0.f;
		for (UINT i = 0; i < m_VtxCount; ++i)
		{
			Vec3 diff = Vec3(_VtxSysMem[i].Pos) - m_BoundCenter;
			float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
			if (distSq > maxDistSq)
				maxDistSq = distSq;
		}
		m_BoundRadius = sqrtf(maxDistSq);

		// AABB half-extents = (max - min) * 0.5
		m_BoundHalfExtents = (vMax - vMin) * 0.5f;
	}

	return S_OK;
}

void CMesh::Binding()
{
	IRHICommandList* pCmdList = g_pRHIDevice->GetCommandList();
	pCmdList->SetVertexBuffer(m_pVB, sizeof(Vertex), 0);
	pCmdList->SetIndexBuffer(m_pIB, m_pIB->GetIndexFormat(), 0);
}

void CMesh::render()
{
	Binding();
	g_pRHIDevice->GetCommandList()->DrawIndexed(m_IdxCount, 0, 0);
}

void CMesh::render_submesh(UINT _SubMeshIdx)
{
	if (_SubMeshIdx >= m_vecSubMeshes.size())
		return;

	Binding();

	const tSubMesh& submesh = m_vecSubMeshes[_SubMeshIdx];
	g_pRHIDevice->GetCommandList()->DrawIndexed(submesh.IndexCount, submesh.IndexStart, 0);
}

void CMesh::render_particle(UINT _InstanceCount)
{
	Binding();
	g_pRHIDevice->GetCommandList()->DrawIndexedInstanced(m_IdxCount, _InstanceCount, 0, 0, 0);
}

int CMesh::Load(const wstring& _FilePath)
{
		// wstring → UTF-8 string (Assimp 使用 char*)
	int iLen = WideCharToMultiByte(CP_UTF8, 0, _FilePath.c_str(), -1, nullptr, 0, nullptr, nullptr);
	string strPath(iLen - 1, '\0');
	WideCharToMultiByte(CP_UTF8, 0, _FilePath.c_str(), -1, strPath.data(), iLen, nullptr, nullptr);

	Assimp::Importer importer;
	const aiScene* pScene = importer.ReadFile(strPath,
		  aiProcess_Triangulate            // 强制三角面
		| aiProcess_CalcTangentSpace       // 计算切线/副切线
		| aiProcess_JoinIdenticalVertices  // 合并重复顶点 (per-aiMesh, uses ALL attributes)
		| aiProcess_ConvertToLeftHanded    // DX 左手坐标系 + 翻转 UV + 翻转绕序
		| aiProcess_GenSmoothNormals       // 若无法线则生成平滑法线
		| aiProcess_SortByPType);          // 移除退化图元, 不合并mesh

	// NOTE: aiProcess_PreTransformVertices 已移除!
	// 该标志会将共享 mMaterialIndex 的多个 aiMesh 合并为一个,
	// 导致 Unreal 导出的 FBX (多section共享同一材质) 的UV和submesh边界被破坏。
	// 改为手动遍历节点树并应用变换。

	if (nullptr == pScene
		|| (pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		|| nullptr == pScene->mRootNode)
	{
		string err = importer.GetErrorString();
		int wLen = MultiByteToWideChar(CP_UTF8, 0, err.c_str(), -1, nullptr, 0);
		wstring wErr(wLen - 1, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, err.c_str(), -1, wErr.data(), wLen);
		MessageBox(nullptr, wErr.c_str(), L"CMesh::Load failed", MB_OK);
		return E_FAIL;
	}
	// Debug: dump all aiMeshes and their material info
	{
		wchar_t buf[512];
		swprintf_s(buf, L"[CMesh::Load] Scene has %u meshes, %u materials\n",
				   pScene->mNumMeshes, pScene->mNumMaterials);
		OutputDebugString(buf);

		for (UINT m = 0; m < pScene->mNumMeshes; ++m)
		{
			aiMesh* pM = pScene->mMeshes[m];
			swprintf_s(buf, L"  aiMesh[%u]: \"%hs\" verts=%u faces=%u matIdx=%u\n",
					   m, pM->mName.C_Str(), pM->mNumVertices, pM->mNumFaces, pM->mMaterialIndex);
			OutputDebugString(buf);
		}

		for (UINT m = 0; m < pScene->mNumMaterials; ++m)
		{
			aiString name;
			pScene->mMaterials[m]->Get(AI_MATKEY_NAME, name);
			swprintf_s(buf, L"  aiMaterial[%u]: \"%hs\"\n", m, name.C_Str());
			OutputDebugString(buf);
		}
	}

	// =============================
	// 递归遍历节点树, 手动应用变换, 合并所有子 Mesh 的顶点/索引
	// =============================
	vector<Vertex> vecVtx;
	vector<UINT>   vecIdx;
	m_vecSubMeshes.clear();

	std::function<void(aiNode*, const aiMatrix4x4&)> ProcessNode;
	ProcessNode = [&](aiNode* pNode, const aiMatrix4x4& parentTransform)
	{
		aiMatrix4x4 globalTransform = parentTransform * pNode->mTransformation;

		// Normal matrix = inverse-transpose of upper-left 3x3
		aiMatrix3x3 normalMat(globalTransform);
		normalMat.Inverse();
		normalMat.Transpose();

		for (UINT i = 0; i < pNode->mNumMeshes; ++i)
		{
			aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[i]];
			UINT uBase    = (UINT)vecVtx.size();
			UINT idxStart = (UINT)vecIdx.size();

			for (UINT v = 0; v < pMesh->mNumVertices; ++v)
			{
				Vertex vtx = {};

				// 应用节点全局变换到位置
				aiVector3D pos = globalTransform * pMesh->mVertices[v];
				vtx.Pos = Vec3(pos.x, pos.y, pos.z);

				vtx.Color = Vec4(1.f, 1.f, 1.f, 1.f);

				if (pMesh->HasTextureCoords(0))
					vtx.UV = Vec2(pMesh->mTextureCoords[0][v].x,
								  pMesh->mTextureCoords[0][v].y);

				if (pMesh->HasNormals())
				{
					aiVector3D n = normalMat * pMesh->mNormals[v];
					n.Normalize();
					vtx.vNormal = Vec3(n.x, n.y, n.z);
				}

				if (pMesh->HasTangentsAndBitangents())
				{
					aiVector3D t = normalMat * pMesh->mTangents[v];
					t.Normalize();
					vtx.vTangent = Vec3(t.x, t.y, t.z);

					aiVector3D b = normalMat * pMesh->mBitangents[v];
					b.Normalize();
					vtx.vBinormal = Vec3(b.x, b.y, b.z);
				}

				vecVtx.push_back(vtx);
			}

			for (UINT f = 0; f < pMesh->mNumFaces; ++f)
			{
				aiFace& face = pMesh->mFaces[f];
				for (UINT j = 0; j < face.mNumIndices; ++j)
					vecIdx.push_back(uBase + face.mIndices[j]);
			}

			// 每个 aiMesh 保持独立的 submesh, 不因共享 materialIndex 而合并
			tSubMesh submesh = {};
			submesh.IndexStart    = idxStart;
			submesh.IndexCount    = (UINT)vecIdx.size() - idxStart;
			submesh.MaterialIndex = pMesh->mMaterialIndex;
			m_vecSubMeshes.push_back(submesh);
		}

		for (UINT c = 0; c < pNode->mNumChildren; ++c)
			ProcessNode(pNode->mChildren[c], globalTransform);
	};

	ProcessNode(pScene->mRootNode, aiMatrix4x4());

	if (vecVtx.empty() || vecIdx.empty())
		return E_FAIL;

	return Create(vecVtx.data(), vecVtx.size(), vecIdx.data(), vecIdx.size());
}



