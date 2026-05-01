#include "pch.h"
#include "CAssetMgr.h"
#include "CPathMgr.h"
#include "CTexture.h"


void CAssetMgr::init()
{
	CreateDefaultMesh();
	CreateDefaultTexture();
	CreateDefaultGraphicsShader();
	CreateDefaultMaterial();
	CreateDefaultComputeShader();
}


void CAssetMgr::DeleteAsset(ASSET_TYPE _Type, const wstring& _strKey)
{
	map<wstring, Ptr<CAsset>>::iterator iter = m_mapAsset[(UINT)_Type].find(_strKey);
	assert(iter != m_mapAsset[(UINT)_Type].end());

	m_mapAsset[(UINT)_Type].erase(iter);

	m_bAssetsChanged = true;
}

void CAssetMgr::CreateDefaultMesh()
{
	Ptr<CMesh> pMesh = nullptr;

	vector<Vertex> vecVtx;
	vector<UINT> vecIdx;
	Vertex v;
	// =========
	// Point Mesh
	// =========
	v.Pos = Vec3(0.f, 0.f, 0.f);
	v.Color = Vec4(1.f, 1.f, 1.f, 1.f);
	v.UV = Vec2(0.f, 0.f);
	UINT idx = 0;
	
	pMesh = new CMesh(true);
	pMesh->Create(&v, 1, &idx, 1);
	AddAsset<CMesh>(L"PointMesh", pMesh);
	// =========
	// Rect Mesh
	// =========
	// 0 -- 1
	// | \  |
	// 3 -- 2
	v.Pos = Vec3(-0.5, 0.5f, 0.f);
	v.Color = Vec4(1.f, 0.f, 0.f, 1.f);
	v.UV = Vec2(0.f, 0.f);
	v.vTangent = Vec3(1.f, 0.f, 0.f);
	v.vNormal = Vec3(0.f, 0.f, -1.f);
	v.vBinormal = Vec3(0.f, -1.f, 0.f);
	vecVtx.push_back(v);

	v.Pos = Vec3(0.5f, 0.5f, 0.f);
	v.Color = Vec4(0.f, 1.f, 0.f, 1.f);
	v.UV = Vec2(1.f, 0.f);
	vecVtx.push_back(v);

	v.Pos = Vec3(0.5f, -0.5f, 0.f);
	v.Color = Vec4(0.f, 0.f, 1.f, 1.f);
	v.UV = Vec2(1.f, 1.f);
	vecVtx.push_back(v);

	v.Pos = Vec3(-0.5, -0.5f, 0.f);
	v.Color = Vec4(0.f, 1.f, 0.f, 1.f);
	v.UV = Vec2(0.f, 1.f);
	vecVtx.push_back(v);

	// Index Buffer
	vecIdx.push_back(0);
	vecIdx.push_back(2);
	vecIdx.push_back(3);

	vecIdx.push_back(0);
	vecIdx.push_back(1);
	vecIdx.push_back(2);


	pMesh = new CMesh(true);
	pMesh->Create(vecVtx.data(), (UINT)vecVtx.size(), vecIdx.data(), (UINT)vecIdx.size());
	AddAsset<CMesh>(L"RectMesh", pMesh);
	// =========
	// Debug Rect Mesh
	// =========
	vecIdx.clear();
	vecIdx.push_back(0);
	vecIdx.push_back(1);
	vecIdx.push_back(2);
	vecIdx.push_back(3);
	vecIdx.push_back(0);
	pMesh = new CMesh(true);
	pMesh->Create(vecVtx.data(), (UINT)vecVtx.size(), vecIdx.data(), (UINT)vecIdx.size());
	AddAsset<CMesh>(L"RectMesh_Debug", pMesh);

	// ==========
	// Circle 圆形
	// ==========
	vecVtx.clear();
	vecIdx.clear();
	v.Pos = Vec3(0.0f, 0.0f, 0.0f); // origin 原点/圆心
	v.Color = Vec4(1.0f, 1.0f, 0.0f, 1.0f);
	vecVtx.push_back(v);

	float Radius = 0.5f;
	UINT Slice = 36;
	float AngleStep = (2 * XM_PI) / Slice;

	// Vertices 圆形的顶点
	float Angle = 0.f;
	for (UINT i = 0; i <= Slice; ++i)
	{
		v.Pos = Vec3(cosf(Angle) * Radius, sinf(Angle) * Radius, 0.f);
		v.Color = Vec4(1.0f, 1.0f, 0.0f, 1.0f);

		vecVtx.push_back(v);

		Angle += AngleStep;
	}
	// Indices 圆形顶点的索引
	for (UINT i = 0; i < Slice; ++i)
	{
		vecIdx.push_back(0);
		vecIdx.push_back(i + 2);
		vecIdx.push_back(i + 1);
	}

	pMesh = new CMesh(true);
	pMesh->Create(vecVtx.data(), vecVtx.size(), vecIdx.data(), vecIdx.size());
	AddAsset<CMesh>(L"CircleMesh", pMesh);
	// ==========
	// Debug Circle 圆形
	// ==========
	vecIdx.clear();
	for (int i = 0; i <= (int)Slice; ++i)
	{
		vecIdx.push_back(i + 1);
	}
	pMesh = new CMesh(true);
	pMesh->Create(vecVtx.data(), vecVtx.size(), vecIdx.data(), vecIdx.size());
	AddAsset<CMesh>(L"CircleMesh_Debug", pMesh);


	// ==========
	// Cube Mesh
	// ==========
	Vertex arrCube[24] = {};

	// Top face
	arrCube[0].Pos = Vec3(-0.5f, 0.5f, 0.5f);
	arrCube[0].Color = Vec4(1.f, 1.f, 1.f, 1.f);
	arrCube[0].UV = Vec2(0.f, 0.f);
	arrCube[0].vNormal = Vec3(0.f, 1.f, 0.f);

	arrCube[1].Pos = Vec3(0.5f, 0.5f, 0.5f);
	arrCube[1].Color = Vec4(1.f, 1.f, 1.f, 1.f);
	arrCube[1].UV = Vec2(0.f, 0.f);
	arrCube[1].vNormal = Vec3(0.f, 1.f, 0.f);

	arrCube[2].Pos = Vec3(0.5f, 0.5f, -0.5f);
	arrCube[2].Color = Vec4(1.f, 1.f, 1.f, 1.f);
	arrCube[2].UV = Vec2(0.f, 0.f);
	arrCube[2].vNormal = Vec3(0.f, 1.f, 0.f);

	arrCube[3].Pos = Vec3(-0.5f, 0.5f, -0.5f);
	arrCube[3].Color = Vec4(1.f, 1.f, 1.f, 1.f);
	arrCube[3].UV = Vec2(0.f, 0.f);
	arrCube[3].vNormal = Vec3(0.f, 1.f, 0.f);


	// 
	arrCube[4].Pos = Vec3(-0.5f, -0.5f, -0.5f);
	arrCube[4].Color = Vec4(1.f, 0.f, 0.f, 1.f);
	arrCube[4].UV = Vec2(0.f, 0.f);
	arrCube[4].vNormal = Vec3(0.f, -1.f, 0.f);

	arrCube[5].Pos = Vec3(0.5f, -0.5f, -0.5f);
	arrCube[5].Color = Vec4(1.f, 0.f, 0.f, 1.f);
	arrCube[5].UV = Vec2(0.f, 0.f);
	arrCube[5].vNormal = Vec3(0.f, -1.f, 0.f);

	arrCube[6].Pos = Vec3(0.5f, -0.5f, 0.5f);
	arrCube[6].Color = Vec4(1.f, 0.f, 0.f, 1.f);
	arrCube[6].UV = Vec2(0.f, 0.f);
	arrCube[6].vNormal = Vec3(0.f, -1.f, 0.f);

	arrCube[7].Pos = Vec3(-0.5f, -0.5f, 0.5f);
	arrCube[7].Color = Vec4(1.f, 0.f, 0.f, 1.f);
	arrCube[7].UV = Vec2(0.f, 0.f);
	arrCube[7].vNormal = Vec3(0.f, -1.f, 0.f);

	//  左侧面
	arrCube[8].Pos = Vec3(-0.5f, 0.5f, 0.5f);
	arrCube[8].Color = Vec4(0.f, 1.f, 0.f, 1.f);
	arrCube[8].UV = Vec2(0.f, 0.f);
	arrCube[8].vNormal = Vec3(-1.f, 0.f, 0.f);

	arrCube[9].Pos = Vec3(-0.5f, 0.5f, -0.5f);
	arrCube[9].Color = Vec4(0.f, 1.f, 0.f, 1.f);
	arrCube[9].UV = Vec2(0.f, 0.f);
	arrCube[9].vNormal = Vec3(-1.f, 0.f, 0.f);

	arrCube[10].Pos = Vec3(-0.5f, -0.5f, -0.5f);
	arrCube[10].Color = Vec4(0.f, 1.f, 0.f, 1.f);
	arrCube[10].UV = Vec2(0.f, 0.f);
	arrCube[10].vNormal = Vec3(-1.f, 0.f, 0.f);

	arrCube[11].Pos = Vec3(-0.5f, -0.5f, 0.5f);
	arrCube[11].Color = Vec4(0.f, 1.f, 0.f, 1.f);
	arrCube[11].UV = Vec2(0.f, 0.f);
	arrCube[11].vNormal = Vec3(-1.f, 0.f, 0.f);

	arrCube[12].Pos = Vec3(0.5f, 0.5f, -0.5f);
	arrCube[12].Color = Vec4(0.f, 0.f, 1.f, 1.f);
	arrCube[12].UV = Vec2(0.f, 0.f);
	arrCube[12].vNormal = Vec3(1.f, 0.f, 0.f);

	arrCube[13].Pos = Vec3(0.5f, 0.5f, 0.5f);
	arrCube[13].Color = Vec4(0.f, 0.f, 1.f, 1.f);
	arrCube[13].UV = Vec2(0.f, 0.f);
	arrCube[13].vNormal = Vec3(1.f, 0.f, 0.f);

	arrCube[14].Pos = Vec3(0.5f, -0.5f, 0.5f);
	arrCube[14].Color = Vec4(0.f, 0.f, 1.f, 1.f);
	arrCube[14].UV = Vec2(0.f, 0.f);
	arrCube[14].vNormal = Vec3(1.f, 0.f, 0.f);

	arrCube[15].Pos = Vec3(0.5f, -0.5f, -0.5f);
	arrCube[15].Color = Vec4(0.f, 0.f, 1.f, 1.f);
	arrCube[15].UV = Vec2(0.f, 0.f);
	arrCube[15].vNormal = Vec3(1.f, 0.f, 0.f);

	arrCube[16].Pos = Vec3(0.5f, 0.5f, 0.5f);
	arrCube[16].Color = Vec4(1.f, 1.f, 0.f, 1.f);
	arrCube[16].UV = Vec2(0.f, 0.f);
	arrCube[16].vNormal = Vec3(0.f, 0.f, 1.f);

	arrCube[17].Pos = Vec3(-0.5f, 0.5f, 0.5f);
	arrCube[17].Color = Vec4(1.f, 1.f, 0.f, 1.f);
	arrCube[17].UV = Vec2(0.f, 0.f);
	arrCube[17].vNormal = Vec3(0.f, 0.f, 1.f);

	arrCube[18].Pos = Vec3(-0.5f, -0.5f, 0.5f);
	arrCube[18].Color = Vec4(1.f, 1.f, 0.f, 1.f);
	arrCube[18].UV = Vec2(0.f, 0.f);
	arrCube[18].vNormal = Vec3(0.f, 0.f, 1.f);

	arrCube[19].Pos = Vec3(0.5f, -0.5f, 0.5f);
	arrCube[19].Color = Vec4(1.f, 1.f, 0.f, 1.f);
	arrCube[19].UV = Vec2(0.f, 0.f);
	arrCube[19].vNormal = Vec3(0.f, 0.f, 1.f);

	arrCube[20].Pos = Vec3(-0.5f, 0.5f, -0.5f);;
	arrCube[20].Color = Vec4(1.f, 0.f, 1.f, 1.f);
	arrCube[20].UV = Vec2(0.f, 0.f);
	arrCube[20].vNormal = Vec3(0.f, 0.f, -1.f);

	arrCube[21].Pos = Vec3(0.5f, 0.5f, -0.5f);
	arrCube[21].Color = Vec4(1.f, 0.f, 1.f, 1.f);
	arrCube[21].UV = Vec2(0.f, 0.f);
	arrCube[21].vNormal = Vec3(0.f, 0.f, -1.f);

	arrCube[22].Pos = Vec3(0.5f, -0.5f, -0.5f);
	arrCube[22].Color = Vec4(1.f, 0.f, 1.f, 1.f);
	arrCube[22].UV = Vec2(0.f, 0.f);
	arrCube[22].vNormal = Vec3(0.f, 0.f, -1.f);

	arrCube[23].Pos = Vec3(-0.5f, -0.5f, -0.5f);
	arrCube[23].Color = Vec4(1.f, 0.f, 1.f, 1.f);
	arrCube[23].UV = Vec2(0.f, 0.f);
	arrCube[23].vNormal = Vec3(0.f, 0.f, -1.f);

	// indices
	vecIdx.clear();
	for (int i = 0; i < 12; i += 2)
	{
		vecIdx.push_back(i * 2);
		vecIdx.push_back(i * 2 + 1);
		vecIdx.push_back(i * 2 + 2);

		vecIdx.push_back(i * 2);
		vecIdx.push_back(i * 2 + 2);
		vecIdx.push_back(i * 2 + 3);
	}

	pMesh = new CMesh(true);
	pMesh->Create(arrCube, 24, vecIdx.data(), (UINT)vecIdx.size());
	AddAsset(L"CubeMesh", pMesh);
	vecVtx.clear();
	vecIdx.clear();
	// ============
	// cube mesh debug
	// ============
	UINT arrCubeIdx[] = {0,1,2,3,0,7,6,1,2,5,4,3,0,7,4,5,6};
	pMesh = new CMesh(true);
	pMesh->Create(arrCube, 24, arrCubeIdx, sizeof(arrCubeIdx) / sizeof(UINT));
	AddAsset(L"CubeMesh_Debug", pMesh);
	vecVtx.clear();
	vecIdx.clear();
	
	// ============
	// Sphere Mesh
	// ============
	float fRadius = 0.5f;

	// Top
	v.Pos = Vec3(0.f, fRadius, 0.f);
	v.UV = Vec2(0.5f, 0.f);
	v.Color = Vec4(1.f, 1.f, 1.f, 1.f);
	v.vNormal = v.Pos;
	v.vNormal.Normalize();
	v.vTangent = Vec3(1.f, 0.f, 0.f);
	v.vBinormal = Vec3(0.f, 0.f, -1.f);
	vecVtx.push_back(v);

	// Body
	UINT iStackCount = 40; 
	UINT iSliceCount = 40; 

	float fStackAngle = XM_PI / iStackCount;
	float fSliceAngle = XM_2PI / iSliceCount;

	float fUVXStep = 1.f / (float)iSliceCount;
	float fUVYStep = 1.f / (float)iStackCount;

	for (UINT i = 1; i < iStackCount; ++i)
	{
		float phi = i * fStackAngle;

		for (UINT j = 0; j <= iSliceCount; ++j)
		{
			float theta = j * fSliceAngle;

			v.Pos = Vec3(fRadius * sinf(i * fStackAngle) * cosf(j * fSliceAngle)
				, fRadius * cosf(i * fStackAngle)
				, fRadius * sinf(i * fStackAngle) * sinf(j * fSliceAngle));

			v.UV = Vec2(fUVXStep * j, fUVYStep * i);
			v.Color = Vec4(1.f, 1.f, 1.f, 1.f);
			v.vNormal = v.Pos;
			v.vNormal.Normalize();

			v.vTangent.x = -fRadius * sinf(phi) * sinf(theta);
			v.vTangent.y = 0.f;
			v.vTangent.z = fRadius * sinf(phi) * cosf(theta);
			v.vTangent.Normalize();

			v.vNormal.Cross(v.vTangent, v.vBinormal);
			v.vBinormal.Normalize();

			vecVtx.push_back(v);
		}
	}

	// Bottom
	v.Pos = Vec3(0.f, -fRadius, 0.f);
	v.UV = Vec2(0.5f, 1.f);
	v.Color = Vec4(1.f, 1.f, 1.f, 1.f);
	v.vNormal = v.Pos;
	v.vNormal.Normalize();

	v.vTangent = Vec3(1.f, 0.f, 0.f);
	v.vBinormal = Vec3(0.f, 0.f, -1.f);
	vecVtx.push_back(v);

	for (UINT i = 0; i < iSliceCount; ++i)
	{
		vecIdx.push_back(0);
		vecIdx.push_back(i + 2);
		vecIdx.push_back(i + 1);
	}

	for (UINT i = 0; i < iStackCount - 2; ++i)
	{
		for (UINT j = 0; j < iSliceCount; ++j)
		{
			// + 
			// | \
			// +--+
			vecIdx.push_back((iSliceCount + 1) * (i)+(j)+1);
			vecIdx.push_back((iSliceCount + 1) * (i + 1) + (j + 1) + 1);
			vecIdx.push_back((iSliceCount + 1) * (i + 1) + (j)+1);

			// +--+
			//  \ |
			//    +
			vecIdx.push_back((iSliceCount + 1) * (i)+(j)+1);
			vecIdx.push_back((iSliceCount + 1) * (i)+(j + 1) + 1);
			vecIdx.push_back((iSliceCount + 1) * (i + 1) + (j + 1) + 1);
		}
	}

	UINT iBottomIdx = (UINT)vecVtx.size() - 1;
	for (UINT i = 0; i < iSliceCount; ++i)
	{
		vecIdx.push_back(iBottomIdx);
		vecIdx.push_back(iBottomIdx - (i + 2));
		vecIdx.push_back(iBottomIdx - (i + 1));
	}

	pMesh = new CMesh(true);
	pMesh->Create(vecVtx.data(), (UINT)vecVtx.size(), vecIdx.data(), (UINT)vecIdx.size());
	AddAsset(L"SphereMesh", pMesh);
	vecVtx.clear();
	vecIdx.clear();
}

void CAssetMgr::CreateDefaultTexture()
{
}


void CAssetMgr::CreateDefaultGraphicsShader()
{
	Ptr<CGraphicsShader> pShader = nullptr;


	wstring strPath = CPathMgr::GetInst()->GetContentPath();

	// ================
	// 创建 2D标准着色器 create 2D standard shader
	// ================
	pShader = new CGraphicsShader;
	pShader->CreateVertexShader(strPath + L"shader\\std2d.fx", "VS_Std2D");
	pShader->CreatePixelShader(strPath + L"shader\\std2d.fx", "PS_Std2D");

	pShader->SetRSType(RS_TYPE::CULL_NONE);
	pShader->SetBSType(BS_TYPE::DEFAULT);
	pShader->SetDSType(DS_TYPE::LESS);

	pShader->SetDomain(SHADER_DOMAIN::DOMAIN_MASKED);

	// default parameters
	pShader->AddScalarParam("Paper Burn Intensity", SCALAR_PARAM::FLOAT_0);
	pShader->AddScalarParam("Test Parameter", SCALAR_PARAM::INT_0);

	pShader->AddTextureParam("Diffuse Texture", TEX_PARAM::TEX_0);


	AddAsset<CGraphicsShader>(L"Std2DShader", pShader);

	// ================
	// 创建 2D半透明着色器
	// ================
	pShader = new CGraphicsShader;
	pShader->CreateVertexShader(strPath + L"shader\\std2d.fx", "VS_Std2D");
	pShader->CreatePixelShader(strPath + L"shader\\std2d.fx", "PS_Std2D_AB");

	pShader->SetRSType(RS_TYPE::CULL_NONE);
	pShader->SetBSType(BS_TYPE::ALPHA_BLEND);
	pShader->SetDSType(DS_TYPE::LESS);

	pShader->SetDomain(SHADER_DOMAIN::DOMAIN_TRANSPARENT);

	AddAsset<CGraphicsShader>(L"Std2DAlphaBlendShader", pShader);
	// ================
	// 创建 Particle 着色器 create Particle Render shader
	// ================
	pShader = new CGraphicsShader;
	pShader->CreateVertexShader(strPath + L"shader\\particle.fx", "VS_Particle");
	pShader->CreateGeometryShader(strPath + L"shader\\particle.fx", "GS_Particle");
	pShader->CreatePixelShader(strPath + L"shader\\particle.fx", "PS_Particle");

	pShader->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	pShader->SetRSType(RS_TYPE::CULL_NONE);
	pShader->SetBSType(BS_TYPE::ALPHA_BLEND);
	pShader->SetDSType(DS_TYPE::NO_WRITE);

	pShader->SetDomain(SHADER_DOMAIN::DOMAIN_TRANSPARENT);

	AddAsset<CGraphicsShader>(L"ParticleRenderShader", pShader);
	// ---------------------------
	// PostProcess Filter Shader
	// ---------------------------
	pShader = new CGraphicsShader;
	pShader->CreateVertexShader(strPath + L"shader\\postprocess.fx", "VS_PostProcess");
	pShader->CreatePixelShader(strPath + L"shader\\postprocess.fx", "PS_PostProcess");

	pShader->SetDSType(DS_TYPE::NO_TEST_NO_WRITE);

	pShader->SetDomain(SHADER_DOMAIN::DOMAIN_POSTPROCESS);

	AddAsset<CGraphicsShader>(L"FilterShader", pShader);
	// ================
	// 创建 Debug 着色器 create Debug shader
	// ================
	pShader = new CGraphicsShader;
	pShader->CreateVertexShader(strPath + L"shader\\debug_shape.fx", "VS_DebugShape");
	pShader->CreatePixelShader(strPath + L"shader\\debug_shape.fx", "PS_DebugShape");

	pShader->SetRSType(RS_TYPE::CULL_BACK);
	pShader->SetDSType(DS_TYPE::NO_TEST_NO_WRITE);
	pShader->SetBSType(BS_TYPE::ALPHA_BLEND);

	pShader->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	AddAsset<CGraphicsShader>(L"DebugShapeShader", pShader);
	// ================
	// 创建 std 3d 标准着色器 create standard 3D shader
	// ================
	pShader = new CGraphicsShader;
	pShader->CreateVertexShader(strPath + L"shader\\std3d.fx", "VS_Std3D");
	pShader->CreatePixelShader(strPath + L"shader\\std3d.fx", "PS_Std3D");
	pShader->SetRSType(RS_TYPE::CULL_BACK);
	pShader->SetDSType(DS_TYPE::LESS);
	pShader->SetBSType(BS_TYPE::DEFAULT);
	pShader->SetDomain(SHADER_DOMAIN::DOMAIN_OPAQUE);

	pShader->AddScalarParam("Use Specular", SCALAR_PARAM::INT_0);
	pShader->AddTextureParam("Diffuse Texture", TEX_PARAM::TEX_0);
	pShader->AddTextureParam("Normal Texture", TEX_PARAM::TEX_1);

	AddAsset<CGraphicsShader>(L"Std3DShader", pShader);
	// ================
	// 创建 skybox shader 天空盒着色器
	// ================
	pShader = new CGraphicsShader;
	pShader->CreateVertexShader(strPath + L"shader\\skybox.fx", "VS_SkyBox");
	pShader->CreatePixelShader(strPath + L"shader\\skybox.fx", "PS_SkyBox");
	pShader->SetRSType(RS_TYPE::CULL_FRONT);
	pShader->SetBSType(BS_TYPE::DEFAULT);
	pShader->SetDSType(DS_TYPE::LESS_EQUAL);
	pShader->SetDomain(SHADER_DOMAIN::DOMAIN_OPAQUE);

	pShader->AddTextureParam("SkyBox Texture", TEX_PARAM::TEX_0);

	AddAsset<CGraphicsShader>(L"SkyBoxShader", pShader);

	// ================
	// 创建 std 3d deferred 标准着色器 create standard 3D deferred shader
	// ================
	pShader = new CGraphicsShader;
	pShader->CreateVertexShader(strPath + L"shader\\std3d_deferred.fx", "VS_Std3D_Deferred");
	pShader->CreatePixelShader(strPath + L"shader\\std3d_deferred.fx", "PS_Std3D_Deferred");
	pShader->SetRSType(RS_TYPE::CULL_BACK);
	pShader->SetBSType(BS_TYPE::DEFAULT);
	pShader->SetDSType(DS_TYPE::LESS);
	pShader->SetDomain(SHADER_DOMAIN::DOMAIN_DEFFERED);

	// NOTE: Dx12
	pShader->SetNumRenderTargets(5);
	pShader->SetRTVFormat(0, DXGI_FORMAT_R8G8B8A8_UNORM);       // Diffuse
	pShader->SetRTVFormat(1, DXGI_FORMAT_R32G32B32A32_FLOAT);   // Normal
	pShader->SetRTVFormat(2, DXGI_FORMAT_R32G32B32A32_FLOAT);   // Position
	pShader->SetRTVFormat(3, DXGI_FORMAT_R32G32B32A32_FLOAT);   // Emissive
	pShader->SetRTVFormat(4, DXGI_FORMAT_R32G32B32A32_FLOAT);   // Custom Data
	

	pShader->AddTextureParam("Diffuse ", TEX_PARAM::TEX_0);
	pShader->AddTextureParam("Normal ", TEX_PARAM::TEX_1);
	pShader->AddTextureParam("Specular ", TEX_PARAM::TEX_2);
	pShader->AddTextureParam("Heightmap ", TEX_PARAM::TEX_3);
	pShader->AddTextureParam("Emmisve ", TEX_PARAM::TEX_4);

	AddAsset<CGraphicsShader>(L"Std3DDeferredShader", pShader);

	// ================
	// deferred directional lighting shader
	// ================
	pShader = new CGraphicsShader;
	pShader->CreateVertexShader(strPath + L"shader\\deferred_lighting.fx", "VS_DirLight");
	pShader->CreatePixelShader(strPath + L"shader\\deferred_lighting.fx", "PS_DirLight");
	pShader->SetRSType(RS_TYPE::CULL_BACK);
	pShader->SetBSType(BS_TYPE::ONE_ONE); // NOTE: additive blending for lighting pass
	pShader->SetDSType(DS_TYPE::NO_TEST_NO_WRITE); // NOTE: no depth test/write for lighting pass
	pShader->SetDomain(SHADER_DOMAIN::DOMAIN_DEFFERED_LIGHT);

	// DX12: must match DEFERRED_LIGHT MRT layout (2 float targets, no depth)
	pShader->SetNumRenderTargets(2);
	pShader->SetRTVFormat(0, DXGI_FORMAT_R32G32B32A32_FLOAT);   // Diffuse
	pShader->SetRTVFormat(1, DXGI_FORMAT_R32G32B32A32_FLOAT);   // Specular
	pShader->SetDSVFormat(DXGI_FORMAT_UNKNOWN);

	pShader->AddTextureParam("Diffuse Texture", TEX_PARAM::TEX_0);

	AddAsset<CGraphicsShader>(L"DeferredDirLightingShader", pShader);

	// ================
	// deferred merging shader
	// ================
	pShader = new CGraphicsShader;
	pShader->CreateVertexShader(strPath + L"shader\\deferred_merging.fx", "VS_MergeDeferred");
	pShader->CreatePixelShader(strPath + L"shader\\deferred_merging.fx", "PS_MergeDeferred");
	pShader->SetRSType(RS_TYPE::CULL_BACK);
	pShader->SetBSType(BS_TYPE::DEFAULT); 
	pShader->SetDSType(DS_TYPE::NO_TEST_NO_WRITE);
	pShader->SetDomain(SHADER_DOMAIN::DOMAIN_DEFFERED_LIGHT);

	// DX12: must match the render target this shader actually writes to
	pShader->SetNumRenderTargets(1);
	pShader->SetRTVFormat(0, DXGI_FORMAT_R16G16B16A16_FLOAT);  // If merging to swapchain RT
	pShader->SetDSVFormat(DXGI_FORMAT_UNKNOWN);

	AddAsset<CGraphicsShader>(L"DeferredMergingShader", pShader);


	// ================
	// deferred point lighting shader
	// ================
	pShader = new CGraphicsShader;
	pShader->CreateVertexShader(strPath + L"shader\\deferred_lighting.fx", "VS_PointLight");
	pShader->CreatePixelShader(strPath + L"shader\\deferred_lighting.fx", "PS_PointLight");
	pShader->SetRSType(RS_TYPE::CULL_FRONT);
	pShader->SetBSType(BS_TYPE::ONE_ONE); // NOTE: additive blending for lighting pass
	pShader->SetDSType(DS_TYPE::NO_TEST_NO_WRITE); // NOTE: no depth test/write for lighting pass
	pShader->SetDomain(SHADER_DOMAIN::DOMAIN_DEFFERED_LIGHT);

	// DX12: must match DEFERRED_LIGHT MRT layout (2 float targets, no depth)
	pShader->SetNumRenderTargets(2);
	pShader->SetRTVFormat(0, DXGI_FORMAT_R32G32B32A32_FLOAT);   // Diffuse
	pShader->SetRTVFormat(1, DXGI_FORMAT_R32G32B32A32_FLOAT);   // Specular
	pShader->SetDSVFormat(DXGI_FORMAT_UNKNOWN);

	pShader->AddTextureParam("Diffuse Texture", TEX_PARAM::TEX_0);

	AddAsset<CGraphicsShader>(L"DeferredPointLightingShader", pShader);

	// ================
	// deferred decal shader
	// ================
	pShader = new CGraphicsShader;
	pShader->CreateVertexShader(strPath + L"shader\\deferred_decal.fx", "VS_DeferredDecal");
	pShader->CreatePixelShader(strPath + L"shader\\deferred_decal.fx", "PS_DeferredDecal");
	pShader->SetRSType(RS_TYPE::CULL_FRONT);
	pShader->SetBSType(BS_TYPE::DECAL_BLEND); 
	pShader->SetDSType(DS_TYPE::NO_TEST_NO_WRITE); 
	pShader->SetDomain(SHADER_DOMAIN::DOMAIN_DEFERRED_DECAL);

	// DX12: must match DEFERRED_DECAL MRT layout (Color + Emissive, no depth)
	pShader->SetNumRenderTargets(2);
	pShader->SetRTVFormat(0, DXGI_FORMAT_R8G8B8A8_UNORM);       // Color
	pShader->SetRTVFormat(1, DXGI_FORMAT_R32G32B32A32_FLOAT);   // Emissive
	pShader->SetDSVFormat(DXGI_FORMAT_UNKNOWN);

	pShader->AddTextureParam("Decal Texture", TEX_PARAM::TEX_0);

	AddAsset<CGraphicsShader>(L"DeferredDecalShader", pShader);

	// * ===========================
	// * PBR
	// * ===========================
	pShader = new CGraphicsShader;
	pShader->CreateVertexShader(strPath + L"shader\\std3d_deferred_pbr.fx", "VS_Std3D_Deferred_PBR");
	pShader->CreatePixelShader(strPath + L"shader\\std3d_deferred_pbr.fx", "PS_Std3D_Deferred_PBR");
	pShader->SetRSType(RS_TYPE::CULL_BACK);
	pShader->SetBSType(BS_TYPE::DEFAULT);
	pShader->SetDSType(DS_TYPE::LESS);
	pShader->SetDomain(SHADER_DOMAIN::DOMAIN_DEFFERED);

	pShader->SetNumRenderTargets(5);
	pShader->SetRTVFormat(0, DXGI_FORMAT_R16G16B16A16_FLOAT);
	pShader->SetRTVFormat(1, DXGI_FORMAT_R32G32B32A32_FLOAT);
	pShader->SetRTVFormat(2, DXGI_FORMAT_R32G32B32A32_FLOAT);
	pShader->SetRTVFormat(3, DXGI_FORMAT_R32G32B32A32_FLOAT);
	pShader->SetRTVFormat(4, DXGI_FORMAT_R32G32B32A32_FLOAT);

	pShader->AddScalarParam("Alpha Cutout",              SCALAR_PARAM::INT_0);
	pShader->AddScalarParam("Flip Normal Y",			 SCALAR_PARAM::INT_1);
	pShader->AddScalarParam("ORM Format (0=ORM 1=glTF)", SCALAR_PARAM::INT_2);
	pShader->AddScalarParam("Metallic Factor",           SCALAR_PARAM::FLOAT_0);
	pShader->AddScalarParam("Roughness Factor",          SCALAR_PARAM::FLOAT_1);
	pShader->AddScalarParam("Base Color Factor",         SCALAR_PARAM::VEC4_0);
	pShader->AddScalarParam("Emissive Factor",           SCALAR_PARAM::VEC4_1);
	pShader->AddTextureParam("Base Color",               TEX_PARAM::TEX_0);
	pShader->AddTextureParam("Normal Map",               TEX_PARAM::TEX_1);
	pShader->AddTextureParam("ORM or MetallicRoughness", TEX_PARAM::TEX_2);
	pShader->AddTextureParam("Emissive",				 TEX_PARAM::TEX_3);

	AddAsset<CGraphicsShader>(L"Std3DDeferredPBRShader", pShader);



	// * ===========================
	// * Forward Transparent PBR Glass
	// * ===========================
	pShader = new CGraphicsShader;
	pShader->CreateVertexShader(strPath + L"shader\\std3d_forward_pbr_glass.fx", "VS_Glass");
	pShader->CreatePixelShader(strPath + L"shader\\std3d_forward_pbr_glass.fx", "PS_Glass");
	pShader->SetRSType(RS_TYPE::CULL_NONE);        // glass is visible from both sides
	pShader->SetBSType(BS_TYPE::ALPHA_BLEND);       // SrcAlpha blending
	pShader->SetDSType(DS_TYPE::NO_WRITE);          // read depth (occluded by walls) but don't write (show things behind glass)
	pShader->SetDomain(SHADER_DOMAIN::DOMAIN_TRANSPARENT);

	pShader->SetNumRenderTargets(1);
	pShader->SetRTVFormat(0, DXGI_FORMAT_R16G16B16A16_FLOAT);

	pShader->AddScalarParam("Opacity",          SCALAR_PARAM::FLOAT_0);
	pShader->AddScalarParam("Roughness Factor", SCALAR_PARAM::FLOAT_1);
	pShader->AddScalarParam("Tint Color",       SCALAR_PARAM::VEC4_0);
	pShader->AddTextureParam("Base Color",  TEX_PARAM::TEX_0);
	pShader->AddTextureParam("Normal Map",  TEX_PARAM::TEX_1);
	pShader->AddTextureParam("Roughness",   TEX_PARAM::TEX_2);
	pShader->AddTextureParam("Occlusion",   TEX_PARAM::TEX_3);

	AddAsset<CGraphicsShader>(L"Std3DForwardPBRGlassShader", pShader);

	// * ===========================
	// * Shadow Map Shader
	// * ===========================
	pShader = new CGraphicsShader;
	pShader->CreateVertexShader(strPath + L"shader\\shadowmap.fx", "VS_ShadowMap");
	pShader->CreatePixelShader(strPath + L"shader\\shadowmap.fx", "PS_ShadowMap");
	pShader->SetRSType(RS_TYPE::CULL_BACK);        
	pShader->SetBSType(BS_TYPE::DEFAULT);       
	pShader->SetDSType(DS_TYPE::LESS);          
	pShader->SetDomain(SHADER_DOMAIN::DOMAIN_SHADOWMAP);

	// DX12: must match ShadowMapTargetTex format (R32_FLOAT) + DSV format
	pShader->SetNumRenderTargets(1);
	pShader->SetRTVFormat(0, DXGI_FORMAT_R32_FLOAT);
	pShader->SetDSVFormat(DXGI_FORMAT_D24_UNORM_S8_UINT);

	AddAsset<CGraphicsShader>(L"ShadowMapShader", pShader);

	// * ===========================
	// * Tonemapping Shader
	// * ===========================
	pShader = new CGraphicsShader;
	pShader->CreateVertexShader(strPath + L"shader\\tonemapping.fx", "VS_ToneMapping");
	pShader->CreatePixelShader(strPath + L"shader\\tonemapping.fx", "PS_ToneMapping");
	pShader->SetRSType(RS_TYPE::CULL_BACK);        
	pShader->SetBSType(BS_TYPE::DEFAULT);       
	pShader->SetDSType(DS_TYPE::NO_TEST_NO_WRITE);          
	pShader->SetDomain(SHADER_DOMAIN::DOMAIN_DEFFERED_LIGHT);

	// DX12: must match ShadowMapTargetTex format (R32_FLOAT) + DSV format
	pShader->SetNumRenderTargets(1);
	pShader->SetRTVFormat(0, DXGI_FORMAT_R8G8B8A8_UNORM);
	pShader->SetDSVFormat(DXGI_FORMAT_UNKNOWN);

	pShader->AddScalarParam("Exposure", SCALAR_PARAM::FLOAT_0);
	pShader->AddScalarParam("Tone Map Operator", SCALAR_PARAM::INT_0);

	AddAsset<CGraphicsShader>(L"ToneMappingShader", pShader);

	// * ===========================
	// * SSAO Generation Shader
	// * ===========================
	pShader = new CGraphicsShader;
	pShader->CreateVertexShader(strPath + L"shader\\ssao.fx", "VS_SSAO");
	pShader->CreatePixelShader(strPath + L"shader\\ssao.fx", "PS_SSAO");
	pShader->SetRSType(RS_TYPE::CULL_BACK);
	pShader->SetBSType(BS_TYPE::DEFAULT);
	pShader->SetDSType(DS_TYPE::NO_TEST_NO_WRITE);
	pShader->SetDomain(SHADER_DOMAIN::DOMAIN_DEFFERED_LIGHT);

	pShader->SetNumRenderTargets(1);
	pShader->SetRTVFormat(0, DXGI_FORMAT_R8_UNORM);
	pShader->SetDSVFormat(DXGI_FORMAT_UNKNOWN);

	AddAsset<CGraphicsShader>(L"SSAOShader", pShader);

	// * ===========================
	// * SSAO Blur Shader
	// * ===========================
	pShader = new CGraphicsShader;
	pShader->CreateVertexShader(strPath + L"shader\\ssao_blur.fx", "VS_SSAOBlur");
	pShader->CreatePixelShader(strPath + L"shader\\ssao_blur.fx", "PS_SSAOBlur");
	pShader->SetRSType(RS_TYPE::CULL_BACK);
	pShader->SetBSType(BS_TYPE::DEFAULT);
	pShader->SetDSType(DS_TYPE::NO_TEST_NO_WRITE);
	pShader->SetDomain(SHADER_DOMAIN::DOMAIN_DEFFERED_LIGHT);

	pShader->SetNumRenderTargets(1);
	pShader->SetRTVFormat(0, DXGI_FORMAT_R8_UNORM);
	pShader->SetDSVFormat(DXGI_FORMAT_UNKNOWN);

	AddAsset<CGraphicsShader>(L"SSAOBlurShader", pShader);
}

void CAssetMgr::CreateDefaultMaterial()
{
	// ===============
	// 创建 2D标准材质 create 2D standard material
	// ===============
	Ptr<CMaterial> pMaterial = nullptr;
	pMaterial = new CMaterial(true);
	pMaterial->SetName(L"Std2DMaterial");
	pMaterial->SetShader(FindAsset<CGraphicsShader>(L"Std2DShader"));
	AddAsset<CMaterial>(pMaterial->GetName(), pMaterial);

	// ===============
	// create bg material
	// ===============
	pMaterial = new CMaterial(true);
	pMaterial->SetName(L"BgMaterial");
	pMaterial->SetShader(FindAsset<CGraphicsShader>(L"Std2DShader"));
	AddAsset<CMaterial>(pMaterial->GetName(), pMaterial);
	// ===============
	// 创建粒子材质 create particle material
	// ===============
	pMaterial = new CMaterial(true);
	pMaterial->SetName(L"ParticleMaterial");
	pMaterial->SetShader(FindAsset<CGraphicsShader>(L"ParticleRenderShader"));
	AddAsset<CMaterial>(pMaterial->GetName(), pMaterial);

	// ===============
	// 创建postprocess材质 create postprocess material
	// ===============
	pMaterial = new CMaterial(true);
	pMaterial->SetName(L"FilterMaterial");
	pMaterial->SetShader(FindAsset<CGraphicsShader>(L"FilterShader"));
	AddAsset<CMaterial>(pMaterial->GetName(), pMaterial);

	// ===============
	// 创建 Debug 材质 create Debug material
	// ===============
	pMaterial = new CMaterial(true);
	pMaterial->SetName(L"DebugShapeMaterial");
	pMaterial->SetShader(FindAsset<CGraphicsShader>(L"DebugShapeShader"));

	AddAsset<CMaterial>(pMaterial->GetName(), pMaterial);
	// ===============
	// 创建 3D标准材质 create 3D standard material
	// ===============
	pMaterial = new CMaterial(true);
	pMaterial->SetName(L"Std3DMaterial");
	pMaterial->SetShader(FindAsset<CGraphicsShader>(L"Std3DShader"));
	AddAsset<CMaterial>(pMaterial->GetName(), pMaterial);

	// ===============
	// 创建 skybox material 天空盒材质
	// ===============
	pMaterial = new CMaterial(true);
	pMaterial->SetName(L"SkyBoxMaterial");
	pMaterial->SetShader(FindAsset<CGraphicsShader>(L"SkyBoxShader"));
	AddAsset<CMaterial>(pMaterial->GetName(), pMaterial);

	// ===============
	// 创建 std 3d deferred 材质 create standard 3D deferred material
	// ===============
	pMaterial = new CMaterial(true);
	pMaterial->SetName(L"Std3DDeferredMaterial");
	pMaterial->SetShader(FindAsset<CGraphicsShader>(L"Std3DDeferredShader"));
	AddAsset<CMaterial>(pMaterial->GetName(), pMaterial);

	// ===============
	// 创建 deferred directional lighting 材质 create deferred directional lighting material
	// ===============
	pMaterial = new CMaterial(true);
	pMaterial->SetName(L"DeferredDirLightingMaterial");
	pMaterial->SetShader(FindAsset<CGraphicsShader>(L"DeferredDirLightingShader"));
	pMaterial->SetTexParamByName(TEX_PARAM::TEX_0, L"GBuffer_Position");
	pMaterial->SetTexParamByName(TEX_PARAM::TEX_1, L"GBuffer_Normal");
	pMaterial->SetTexParamByName(TEX_PARAM::TEX_2, L"GBuffer_Color");      // Albedo for PBR
	pMaterial->SetTexParamByName(TEX_PARAM::TEX_3, L"GBuffer_CustomData"); // Metallic/Roughness/AO
	AddAsset<CMaterial>(pMaterial->GetName(), pMaterial);

	// ===============
	// 创建 deferred merging 材质 create deferred merging material
	// ===============
	pMaterial = new CMaterial(true);
	pMaterial->SetName(L"DeferredMergingMaterial");
	pMaterial->SetShader(FindAsset<CGraphicsShader>(L"DeferredMergingShader"));
	AddAsset<CMaterial>(pMaterial->GetName(), pMaterial);

	// ===============
	// 创建 deferred point lighting 材质 create deferred point lighting material
	// ===============
	pMaterial = new CMaterial(true);
	pMaterial->SetName(L"DeferredPointLightingMaterial");
	pMaterial->SetShader(FindAsset<CGraphicsShader>(L"DeferredPointLightingShader"));
	pMaterial->SetTexParamByName(TEX_PARAM::TEX_0, L"GBuffer_Position");
	pMaterial->SetTexParamByName(TEX_PARAM::TEX_1, L"GBuffer_Normal");
	pMaterial->SetTexParamByName(TEX_PARAM::TEX_2, L"GBuffer_Color");      // Albedo for PBR
	pMaterial->SetTexParamByName(TEX_PARAM::TEX_3, L"GBuffer_CustomData"); // Metallic/Roughness/AO
	AddAsset<CMaterial>(pMaterial->GetName(), pMaterial);

	// ===============
	// 创建 deferred decal 材质 create deferred decal material
	// ===============
	pMaterial = new CMaterial(true);
	pMaterial->SetName(L"DeferredDecalMaterial");
	pMaterial->SetShader(FindAsset<CGraphicsShader>(L"DeferredDecalShader"));
	pMaterial->SetTexParamByName(TEX_PARAM::TEX_0, L"GBuffer_Position");
	AddAsset<CMaterial>(pMaterial->GetName(), pMaterial);

	// * ===============
	// * PBR material
	// * ===============
	pMaterial = new CMaterial(true);
	pMaterial->SetName(L"Std3DDeferredPBRMaterial");
	pMaterial->SetShader(FindAsset<CGraphicsShader>(L"Std3DDeferredPBRShader"));
	pMaterial->SetScalarParam(SCALAR_PARAM::VEC4_0, Vec4(1.f, 1.f, 1.f, 1.f));  // Base Color Factor = white
	pMaterial->SetScalarParam(SCALAR_PARAM::FLOAT_0, 1.f);                       // Metallic Factor
	pMaterial->SetScalarParam(SCALAR_PARAM::FLOAT_1, 1.f);                       // Roughness Factor
	AddAsset<CMaterial>(pMaterial->GetName(), pMaterial);


	// * ===============
	// * Forward Transparent PBR Glass material
	// * ===============
	pMaterial = new CMaterial(true);
	pMaterial->SetName(L"Std3DForwardPBRGlassMaterial");
	pMaterial->SetShader(FindAsset<CGraphicsShader>(L"Std3DForwardPBRGlassShader"));
	pMaterial->SetScalarParam(SCALAR_PARAM::FLOAT_0, 0.f);    // Metallic = 0（非金属安全默认值）
	pMaterial->SetScalarParam(SCALAR_PARAM::FLOAT_1, 0.5f);   // Roughness = 0.5
	pMaterial->SetScalarParam(SCALAR_PARAM::VEC4_0, Vec4(1.f, 1.f, 1.f, 1.f));
	AddAsset<CMaterial>(pMaterial->GetName(), pMaterial);

	// * ===============
	// * Shadow Map material
	// * ===============
	pMaterial = new CMaterial(true);
	pMaterial->SetName(L"ShadowMapMaterial");
	pMaterial->SetShader(FindAsset<CGraphicsShader>(L"ShadowMapShader"));
	AddAsset<CMaterial>(pMaterial->GetName(), pMaterial);

	// * ===============
	// * Tonemapping material
	// * ===============
	pMaterial = new CMaterial(true);
	pMaterial->SetName(L"ToneMappingMaterial");
	pMaterial->SetShader(FindAsset<CGraphicsShader>(L"ToneMappingShader"));
	AddAsset<CMaterial>(pMaterial->GetName(), pMaterial);

	// * ===============
	// * SSAO material
	// * ===============
	pMaterial = new CMaterial(true);
	pMaterial->SetName(L"SSAOMaterial");
	pMaterial->SetShader(FindAsset<CGraphicsShader>(L"SSAOShader"));
	AddAsset<CMaterial>(pMaterial->GetName(), pMaterial);

	// * ===============
	// * SSAO Blur material
	// * ===============
	pMaterial = new CMaterial(true);
	pMaterial->SetName(L"SSAOBlurMaterial");
	pMaterial->SetShader(FindAsset<CGraphicsShader>(L"SSAOBlurShader"));
	AddAsset<CMaterial>(pMaterial->GetName(), pMaterial);

}

#include "CSetColorShader.h"
#include "CParticleTickCS.h"
#include "CEquirectToCubeCS.h"
#include "CIBLIrradianceCS.h"
#include "CIBLPrefilterCS.h"
#include "CBRDFLutCS.h"
#include "CGenMipsCubemapCS.h"
#include "CGenMipsTexture2DCS.h"

void CAssetMgr::CreateDefaultComputeShader()
{
	wstring strPath = CPathMgr::GetInst()->GetContentPath();
	Ptr<CComputeShader> pComputerShader = nullptr;

	// set color compute shader
	pComputerShader = new CSetColorShader;
	pComputerShader->CreateComputeShader(strPath + L"shader\\compute.fx", "CS_Test");

	AddAsset<CComputeShader>(L"SetColorCS", pComputerShader);
	
	// particle tick compute shader
	pComputerShader = new CParticleTickCS;
	pComputerShader->CreateComputeShader(strPath + L"shader\\particletick.fx", "CS_ParticleTick");
	AddAsset<CComputeShader>(L"ParticleTickCS", pComputerShader);

	// ================================
	// IBL Compute Shaders
	// ================================

	// Equirectangular -> Cubemap
	pComputerShader = new CEquirectToCubeCS;
	pComputerShader->CreateComputeShader(strPath + L"shader\\equirect_to_cubemap_cs.fx", "CS_EquirectToCube");
	AddAsset<CComputeShader>(L"EquirectToCubeCS", pComputerShader);

	// Irradiance convolution
	pComputerShader = new CIBLIrradianceCS;
	pComputerShader->CreateComputeShader(strPath + L"shader\\ibl_irradiance_cs.fx", "CS_Irradiance");
	AddAsset<CComputeShader>(L"IBLIrradianceCS", pComputerShader);

	// Specular prefilter
	pComputerShader = new CIBLPrefilterCS;
	pComputerShader->CreateComputeShader(strPath + L"shader\\ibl_prefilter_cs.fx", "CS_Prefilter");
	AddAsset<CComputeShader>(L"IBLPrefilterCS", pComputerShader);

	// BRDF LUT
	pComputerShader = new CBRDFLutCS;
	pComputerShader->CreateComputeShader(strPath + L"shader\\brdf_lut_cs.fx", "CS_BRDFLut");
	AddAsset<CComputeShader>(L"BRDFLutCS", pComputerShader);
	
	// Cubemap mip-chain generator (Karis-weighted, HDR-safe)
	pComputerShader = new CGenMipsCubemapCS;
	pComputerShader->CreateComputeShader(strPath + L"shader\\generate_mips_cubemap_cs.fx", "CS_GenerateMipsCubemap");
	AddAsset<CComputeShader>(L"GenMipsCubemapCS", pComputerShader);

	// Generic Texture2D mip-chain generator
	pComputerShader = new CGenMipsTexture2DCS;
	pComputerShader->CreateComputeShader(strPath + L"shader\\generate_mips_cs.fx", "CS_GenerateMips");
	AddAsset<CComputeShader>(L"GenMipsTexture2DCS", pComputerShader);
}
