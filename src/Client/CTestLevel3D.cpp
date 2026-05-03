#include "pch.h"
#include "CTestLevel3D.h"


#include <CLevel.h>
#include <CLayer.h>
#include <CCollisionManager.h>
#include <CGameObject.h>
#include <components.h>

#include <CPlayerScript.h>
#include <CMonsterScript.h>
#include <CCameraMoveScript.h>


#include <CSetColorShader.h>

#include <CPrefab.h>

#include <CModelImporter.h>

#include <CMaterialInstance.h>


void CTestLevel3D::CreateTestLevel()
{
	CLevel* pLevel = nullptr;
	pLevel = new CLevel;
	pLevel->GetLayer(0)->SetName(L"Default");
	pLevel->GetLayer(1)->SetName(L"Player");
	pLevel->GetLayer(2)->SetName(L"Monster");

	// Get the shared parent material (shader + default params)
	Ptr<CMaterial> pPBRParent = CAssetMgr::GetInst()->FindAsset<CMaterial>(L"Std3DDeferredPBRMaterial");

	// =====================
	// Directional Light
	// =====================
	CGameObject* pLight = new CGameObject;
	pLight->SetName(L"Dir Light");
	pLight->AddComponent(new CTransform);
	pLight->AddComponent(new CLight3D);

	pLight->Light3D()->SetLightType(LIGHT_TYPE::DIRECTIONAL);
	pLight->Light3D()->SetDiffuse(Vec3(1.f, 1.f, 1.f));
	pLight->Light3D()->SetSpecular(Vec3(.3f, .3f, .3f));
	pLight->Light3D()->SetAmbient(Vec3(.0f, .0f, .0f));
	pLight->Light3D()->SetIsRenderShadow(true);

	pLight->Transform()->SetRelativePos(Vec3(-157.f, 1916.f, -88.f));
	pLight->Transform()->SetRelativeRotation(Vec3(XM_PI / 3.f, 0.f, 0.f));

	pLevel->AddObject(0, pLight);
	// =====================
	// Gothic tracery ruins scene
	// =====================
	/*
	CGameObject* pTraceryRuins = new CGameObject;
	pTraceryRuins->SetName(L"TraceryRuins");
	pTraceryRuins->AddComponent(new CTransform);
	pTraceryRuins->AddComponent(new CMeshRenderer);
	pTraceryRuins->AddComponent(new CBoundingBox);

	pTraceryRuins->Transform()->SetRelativePos(Vec3(0.f, 0.f, 0.f));
	pTraceryRuins->Transform()->SetRelativeScale(Vec3(1.f, 1.f, 1.f));
	
	// Load FBX mesh
	pTraceryRuins->MeshRenderer()->SetMesh(CAssetMgr::GetInst()->Load<CMesh>(L"TraceryRuins", L"mesh\\tracery_gothic.fbx"));

	// Materials
	CMaterialInstance* pTraceryRuinsMI_FloorTiles = new CMaterialInstance;
	pTraceryRuinsMI_FloorTiles->SetParentMaterial(pPBRParent);
	pTraceryRuinsMI_FloorTiles->SetScalarOverride(SCALAR_PARAM::INT_2, 1); // gltf no AO, only MR
	pTraceryRuinsMI_FloorTiles->SetTexOverride(TEX_PARAM::TEX_0,
		CAssetMgr::GetInst()->Load<CTexture>(L"tracery_floortiles_basecolor", L"texture\\tracery\\tracery_floor_tiles_BC.jpg"));
	pTraceryRuinsMI_FloorTiles->SetTexOverride(TEX_PARAM::TEX_1,
		CAssetMgr::GetInst()->Load<CTexture>(L"tracery_floortiles_normal", L"texture\\tracery\\tracery_floor_tiles_N.jpg"));
	pTraceryRuinsMI_FloorTiles->SetTexOverride(TEX_PARAM::TEX_2,
		CAssetMgr::GetInst()->Load<CTexture>(L"tracery_floortiles_ORM", L"texture\\tracery\\tracery_floor_tiles_ORM.png"));

	// glass
	Ptr<CMaterial> pGlassParent = CAssetMgr::GetInst()->FindAsset<CMaterial>(L"Std3DForwardPBRGlassMaterial");
	CMaterialInstance* pTraceryRuinsMI_Glass = new CMaterialInstance;
	pTraceryRuinsMI_Glass->SetParentMaterial(pGlassParent);
	pTraceryRuinsMI_Glass->SetScalarOverride(SCALAR_PARAM::FLOAT_0, 0.35f); // opacity
	pTraceryRuinsMI_Glass->SetTexOverride(TEX_PARAM::TEX_0,
		CAssetMgr::GetInst()->Load<CTexture>(L"tracery_glass_basecolor", L"texture\\tracery\\tracery_A_stained_glass_BC.jpg"));
	pTraceryRuinsMI_Glass->SetTexOverride(TEX_PARAM::TEX_2,
		CAssetMgr::GetInst()->Load<CTexture>(L"tracery_glass_roughness", L"texture\\tracery\\tracery_A_stained_glass_R.jpg"));
	pTraceryRuinsMI_Glass->SetTexOverride(TEX_PARAM::TEX_3,
		CAssetMgr::GetInst()->Load<CTexture>(L"tracery_glass_opacity", L"texture\\tracery\\tracery_A_stained_glass_O.jpg"));


	CMaterialInstance* pTraceryRuinsMI_Bricks = new CMaterialInstance;
	pTraceryRuinsMI_Bricks->SetParentMaterial(pPBRParent);
	pTraceryRuinsMI_Bricks->SetScalarOverride(SCALAR_PARAM::INT_2, 1); // gltf no AO, only MR
	pTraceryRuinsMI_Bricks->SetTexOverride(TEX_PARAM::TEX_0,
		CAssetMgr::GetInst()->Load<CTexture>(L"tracery_bricks_basecolor", L"texture\\tracery\\tracery_bricks_new_BC.jpg"));
	pTraceryRuinsMI_Bricks->SetTexOverride(TEX_PARAM::TEX_1,
		CAssetMgr::GetInst()->Load<CTexture>(L"tracery_bricks_normal", L"texture\\tracery\\tracery_bricks_new_N.jpg"));
	pTraceryRuinsMI_Bricks->SetTexOverride(TEX_PARAM::TEX_2,
		CAssetMgr::GetInst()->Load<CTexture>(L"tracery_bricks_ORM", L"texture\\tracery\\tracery_bricks_new_ORM.png"));

	CMaterialInstance* pTraceryRuinsMI_TilesStone = new CMaterialInstance;
	pTraceryRuinsMI_TilesStone->SetParentMaterial(pPBRParent);
	pTraceryRuinsMI_TilesStone->SetScalarOverride(SCALAR_PARAM::INT_2, 1); // glTF no AO, only MR
	pTraceryRuinsMI_TilesStone->SetTexOverride(TEX_PARAM::TEX_0,
		CAssetMgr::GetInst()->Load<CTexture>(L"tracery_tilesstone_basecolor", L"texture\\tracery\\tracery_tiles_stone_BC.jpg"));
	pTraceryRuinsMI_TilesStone->SetTexOverride(TEX_PARAM::TEX_1,
		CAssetMgr::GetInst()->Load<CTexture>(L"tracery_tilesstone_normal", L"texture\\tracery\\tracery_tiles_stone_N.jpg"));
	pTraceryRuinsMI_TilesStone->SetTexOverride(TEX_PARAM::TEX_2,
		CAssetMgr::GetInst()->Load<CTexture>(L"tracery_tilesstone_ORM", L"texture\\tracery\\tracery_tiles_stone_ORM.png"));

	CMaterialInstance* pTraceryRuinsMI_WallVariant = new CMaterialInstance;
	pTraceryRuinsMI_WallVariant->SetParentMaterial(pPBRParent);
	pTraceryRuinsMI_WallVariant->SetScalarOverride(SCALAR_PARAM::INT_2, 1); // glTF MR format
	pTraceryRuinsMI_WallVariant->SetTexOverride(TEX_PARAM::TEX_0,
		CAssetMgr::GetInst()->Load<CTexture>(L"tracery_wallvariant_basecolor", L"texture\\tracery\\tracery_wall_variant_C_BC.jpg"));
	pTraceryRuinsMI_WallVariant->SetTexOverride(TEX_PARAM::TEX_1,
		CAssetMgr::GetInst()->Load<CTexture>(L"tracery_wallvariant_normal", L"texture\\tracery\\tracery_wall_variant_N.jpg"));
	pTraceryRuinsMI_WallVariant->SetTexOverride(TEX_PARAM::TEX_2,
		CAssetMgr::GetInst()->Load<CTexture>(L"tracery_wallvariant_ORM", L"texture\\tracery\\tracery_wall_variant_ORM.png"));

	CMaterialInstance* pTraceryRuinsMI_Trims = new CMaterialInstance;
	pTraceryRuinsMI_Trims->SetParentMaterial(pPBRParent);
	pTraceryRuinsMI_Trims->SetScalarOverride(SCALAR_PARAM::INT_2, 1); // glTF MR format
	pTraceryRuinsMI_Trims->SetTexOverride(TEX_PARAM::TEX_0,
		CAssetMgr::GetInst()->Load<CTexture>(L"tracery_trims_basecolor", L"texture\\tracery\\tracery_trims_A_BC.jpg"));
	pTraceryRuinsMI_Trims->SetTexOverride(TEX_PARAM::TEX_1,
		CAssetMgr::GetInst()->Load<CTexture>(L"tracery_trims_normal", L"texture\\tracery\\tracery_trims_A_N.jpg"));
	pTraceryRuinsMI_Trims->SetTexOverride(TEX_PARAM::TEX_2,
		CAssetMgr::GetInst()->Load<CTexture>(L"tracery_trims_ORM", L"texture\\tracery\\tracery_trims_A_ORM.png"));

	CMaterialInstance* pTraceryRuinsMI_LightMounts = new CMaterialInstance;
	pTraceryRuinsMI_LightMounts->SetParentMaterial(pPBRParent);
	pTraceryRuinsMI_LightMounts->SetScalarOverride(SCALAR_PARAM::INT_2, 1); // glTF MR format
	// * emissive
	pTraceryRuinsMI_LightMounts->SetScalarOverride(SCALAR_PARAM::VEC4_1, Vec4(1.f, 1.f, 1.f, 1.f)); 
	pTraceryRuinsMI_LightMounts->SetTexOverride(TEX_PARAM::TEX_0,
		CAssetMgr::GetInst()->Load<CTexture>(L"tracery_lightmounts_basecolor", L"texture\\tracery\\tracery_light_mounts_BC.jpg"));
	pTraceryRuinsMI_LightMounts->SetTexOverride(TEX_PARAM::TEX_1,
		CAssetMgr::GetInst()->Load<CTexture>(L"tracery_lightmounts_normal", L"texture\\tracery\\tracery_lights_mounts_N.jpg"));
	pTraceryRuinsMI_LightMounts->SetTexOverride(TEX_PARAM::TEX_2,
		CAssetMgr::GetInst()->Load<CTexture>(L"tracery_lightmounts_ORM", L"texture\\tracery\\tracery_lights_mount_ORM.png"));
	pTraceryRuinsMI_LightMounts->SetTexOverride(TEX_PARAM::TEX_3,
		CAssetMgr::GetInst()->Load<CTexture>(L"tracery_lightmounts_Emissive", L"texture\\tracery\\tracery_Light-mounts_EM.jpg"));

	// * 0 is trims
	pTraceryRuins->MeshRenderer()->SetMaterial(pTraceryRuinsMI_Trims, 0);
	// * 1 is glass
	pTraceryRuins->MeshRenderer()->SetMaterial(pTraceryRuinsMI_Glass, 1);
	// * 2 is tiles stone
	pTraceryRuins->MeshRenderer()->SetMaterial(pTraceryRuinsMI_TilesStone, 2);
	// * 3 is floor tiles
	pTraceryRuins->MeshRenderer()->SetMaterial(pTraceryRuinsMI_FloorTiles, 3);
	// * 4 is wall variant
	pTraceryRuins->MeshRenderer()->SetMaterial(pTraceryRuinsMI_WallVariant, 4);
	// * 5 is light mounts
	pTraceryRuins->MeshRenderer()->SetMaterial(pTraceryRuinsMI_LightMounts, 5);
	// * 6 is bricks
	pTraceryRuins->MeshRenderer()->SetMaterial(pTraceryRuinsMI_Bricks, 6);

	//pTraceryRuins->MeshRenderer()->SetMaterial(pTraceryRuinsMI_WallVariant, 7);
	//pTraceryRuins->MeshRenderer()->SetMaterial(pTraceryRuinsMI_WallVariant, 8);

	pTraceryRuins->BoundingBox()->SetAbsolute(true);
	pTraceryRuins->BoundingBox()->SetOffsetPos(Vec3(0.f, 0.f, 0.f));
	pTraceryRuins->BoundingBox()->SetRadius(600.f);

	pLevel->AddObject(0, pTraceryRuins);
	*/
	// =====================
	// Sponza Knight
	// =====================
	/*
	CGameObject* pSponzaKnight = new CGameObject;
	pSponzaKnight->SetName(L"Sponza Knight");
	pSponzaKnight->AddComponent(new CTransform);
	pSponzaKnight->AddComponent(new CMeshRenderer);
	pSponzaKnight->AddComponent(new CBoundingBox);
	pSponzaKnight->Transform()->SetRelativePos(Vec3(0.f, 0.f, 0.f));
	pSponzaKnight->Transform()->SetRelativeScale(Vec3(1.f, 1.f, 1.f));


	
	// Load FBX mesh
	pSponzaKnight->MeshRenderer()->SetMesh(CAssetMgr::GetInst()->Load<CMesh>(L"SponzaKnight", L"mesh\\Knight_USD_002.fbx"));
	Ptr<CMesh> pSponzaKnightMesh = pSponzaKnight->MeshRenderer()->GetMesh();
	for (UINT i = 0; i < pSponzaKnightMesh->GetSubMeshCount(); ++i)
	{
		const CMesh::tSubMesh& sm = pSponzaKnightMesh->GetSubMesh(i);
		wchar_t buf[256];
		swprintf_s(buf, L"SubMesh[%u]: IndexStart=%u, IndexCount=%u, MaterialIndex=%u\n",
				   i, sm.IndexStart, sm.IndexCount, sm.MaterialIndex);
		OutputDebugString(buf);
	}

	// Shared metal textures
	auto texMetalD  = CAssetMgr::GetInst()->Load<CTexture>(L"knight_metal_D", L"texture\\sponza_knight\\brushed_metal_rough_D.png");
	auto texMetalF  = CAssetMgr::GetInst()->Load<CTexture>(L"knight_metal_F", L"texture\\sponza_knight\\brushed_metal_rough_F.png");
	// H = Height map (can be used for TEX parallax or skip for now)
	// auto texMetalH = CAssetMgr::GetInst()->Load<CTexture>(L"knight_metal_H", L"texture\\sponza_knight\\brushed_metal_rough_H.png");

	// Slot 0: Armor — D map is the Base Color (metal surface color = F0 in metallic workflow)
	CMaterialInstance* pMI_Armor = new CMaterialInstance;
	pMI_Armor->SetParentMaterial(pPBRParent);
	// D map for brushed metal is already linear (metal textures usually stored linear)
	pMI_Armor->SetScalarOverride(SCALAR_PARAM::INT_3, 1);       // TEX_0 is linear
	pMI_Armor->SetTexOverride(TEX_PARAM::TEX_0, texMetalF);     // D = Base Color (NOT the F map)
	pMI_Armor->SetScalarOverride(SCALAR_PARAM::FLOAT_0, 1.0f);  // metallic=1 (pure metal), F0 = albedo
	pMI_Armor->SetScalarOverride(SCALAR_PARAM::FLOAT_1, 0.3f);  // roughness

	// Slot 1: Brass — D map + gold tint via VEC4_0
	CMaterialInstance* pMI_Brass = new CMaterialInstance;
	pMI_Brass->SetParentMaterial(pPBRParent);
	pMI_Brass->SetScalarOverride(SCALAR_PARAM::INT_3, 1);       // TEX_0 is linear
	pMI_Brass->SetTexOverride(TEX_PARAM::TEX_0, texMetalF);     // D = Base Color
	pMI_Brass->SetScalarOverride(SCALAR_PARAM::VEC4_0, Vec4(1.0f, 0.77f, 0.34f, 1.f)); // gold tint factor
	pMI_Brass->SetScalarOverride(SCALAR_PARAM::FLOAT_0, 1.0f);  // metallic=1
	pMI_Brass->SetScalarOverride(SCALAR_PARAM::FLOAT_1, 0.35f);

	// Slot 7: Sword — use D map as base color
	CMaterialInstance* pMI_Sword = new CMaterialInstance;
	pMI_Sword->SetParentMaterial(pPBRParent);
	pMI_Sword->SetScalarOverride(SCALAR_PARAM::INT_3, 1);
	pMI_Sword->SetTexOverride(TEX_PARAM::TEX_0,
		CAssetMgr::GetInst()->Load<CTexture>(L"knight_sword_D", L"texture\\sponza_knight\\sword_diffuse_C.png"));
	pMI_Sword->SetScalarOverride(SCALAR_PARAM::FLOAT_0, 1.0f);  // metallic
	pMI_Sword->SetScalarOverride(SCALAR_PARAM::FLOAT_1, 0.2f);  // polished sword

	// Slot 2: Leather
	CMaterialInstance* pMI_Leather = new CMaterialInstance;
	pMI_Leather->SetParentMaterial(pPBRParent);
	pMI_Leather->SetTexOverride(TEX_PARAM::TEX_0,
		CAssetMgr::GetInst()->Load<CTexture>(L"knight_leather_D", L"texture\\sponza_knight\\leather_diffuse_D.png"));
	pMI_Leather->SetScalarOverride(SCALAR_PARAM::FLOAT_0, 0.0f);
	pMI_Leather->SetScalarOverride(SCALAR_PARAM::FLOAT_1, 0.7f);

	// Slot 5: Cloth
	CMaterialInstance* pMI_Cloth = new CMaterialInstance;
	pMI_Cloth->SetParentMaterial(pPBRParent);
	pMI_Cloth->SetTexOverride(TEX_PARAM::TEX_0,
		CAssetMgr::GetInst()->Load<CTexture>(L"knight_cloth_D", L"texture\\sponza_knight\\cloth_diffuse_D.png"));
	pMI_Cloth->SetTexOverride(TEX_PARAM::TEX_1,
		CAssetMgr::GetInst()->Load<CTexture>(L"knight_cloth_N", L"texture\\sponza_knight\\cloth_normal_D.png"));
	pMI_Cloth->SetScalarOverride(SCALAR_PARAM::FLOAT_0, 0.0f);
	pMI_Cloth->SetScalarOverride(SCALAR_PARAM::FLOAT_1, 0.8f);


	// Don't bind TEX_2 with roughness-only texture
	pMI_Sword->SetScalarOverride(SCALAR_PARAM::FLOAT_0, 0.95f);  // metallic
	pMI_Sword->SetScalarOverride(SCALAR_PARAM::FLOAT_1, 0.25f);  // polished sword
	pLevel->AddObject(0, pSponzaKnight);

	// Assign
	pSponzaKnight->MeshRenderer()->SetMaterial(pMI_Armor, 0);
	pSponzaKnight->MeshRenderer()->SetMaterial(pMI_Brass, 1);
	pSponzaKnight->MeshRenderer()->SetMaterial(pMI_Leather, 2);
	// Chestplate & Shoulder can reuse Armor MI or create variants
	pSponzaKnight->MeshRenderer()->SetMaterial(pMI_Armor, 3);
	pSponzaKnight->MeshRenderer()->SetMaterial(pMI_Armor, 4);
	pSponzaKnight->MeshRenderer()->SetMaterial(pMI_Cloth, 5);
	//pSponzaKnight->MeshRenderer()->SetMaterial(pMI_Shield, 6);
	pSponzaKnight->MeshRenderer()->SetMaterial(pMI_Sword, 7);
	*/

	// =====================
	// Unreal Knight
	// =====================
	/*
	CGameObject* pKnight = new CGameObject;
	pKnight->SetName(L"Knight");
	pKnight->AddComponent(new CTransform);
	pKnight->AddComponent(new CMeshRenderer);
	pKnight->AddComponent(new CBoundingBox);
	pKnight->Transform()->SetRelativePos(Vec3(122.f, 0.f, -95.f));
	pKnight->Transform()->SetRelativeScale(Vec3(1.f, 1.f, 1.f));


	
	// Load FBX mesh
	pKnight->MeshRenderer()->SetMesh(CAssetMgr::GetInst()->Load<CMesh>(L"Knight", L"mesh\\SK_CrusaderKnight_UE4.FBX"));
	Ptr<CMesh> pKnightMesh = pKnight->MeshRenderer()->GetMesh();
	for (UINT i = 0; i < pKnightMesh->GetSubMeshCount(); ++i)
	{
		const CMesh::tSubMesh& sm = pKnightMesh->GetSubMesh(i);
		wchar_t buf[256];
		swprintf_s(buf, L"SubMesh[%u]: IndexStart=%u, IndexCount=%u, MaterialIndex=%u\n",
				   i, sm.IndexStart, sm.IndexCount, sm.MaterialIndex);
		OutputDebugString(buf);
	}

	// Materials
	CMaterialInstance* pKnight_Leather = new CMaterialInstance;
	pKnight_Leather->SetParentMaterial(pPBRParent);
	pKnight_Leather->SetTexOverride(TEX_PARAM::TEX_0,
		CAssetMgr::GetInst()->Load<CTexture>(L"knight_leather_basecolor", L"texture\\knight\\T_CrusaderKnight_Leather_BaseColor.PNG"));
	pKnight_Leather->SetTexOverride(TEX_PARAM::TEX_1,
		CAssetMgr::GetInst()->Load<CTexture>(L"knight_leather_normal", L"texture\\knight\\T_CrusaderKnight_Leather_Normal.PNG"));
	pKnight_Leather->SetTexOverride(TEX_PARAM::TEX_2,
		CAssetMgr::GetInst()->Load<CTexture>(L"knight_leather_ORM", L"texture\\knight\\T_CrusaderKnight_Leather_OcclusionRoughnessMetallic.PNG"));

	CMaterialInstance* pKnight_Legs = new CMaterialInstance;
	pKnight_Legs->SetParentMaterial(pPBRParent);
	pKnight_Legs->SetScalarOverride(SCALAR_PARAM::INT_0, 1); // alpha cutout
	pKnight_Legs->SetTexOverride(TEX_PARAM::TEX_0,
		CAssetMgr::GetInst()->Load<CTexture>(L"knight_legs_basecolor", L"texture\\knight\\T_CrusaderKnight_Legs_BaseColor.PNG"));
	pKnight_Legs->SetTexOverride(TEX_PARAM::TEX_1,
		CAssetMgr::GetInst()->Load<CTexture>(L"knight_legs_normal", L"texture\\knight\\T_CrusaderKnight_Legs_Normal.PNG"));
	pKnight_Legs->SetTexOverride(TEX_PARAM::TEX_2,
		CAssetMgr::GetInst()->Load<CTexture>(L"knight_legs_ORM", L"texture\\knight\\T_CrusaderKnight_Legs_OcclusionRoughnessMetallic.PNG"));

	CMaterialInstance* pKnight_Cape = new CMaterialInstance;
	pKnight_Cape->SetParentMaterial(pPBRParent);
	pKnight_Cape->SetScalarOverride(SCALAR_PARAM::INT_0, 1); // alpha cutout
	pKnight_Cape->SetScalarOverride(SCALAR_PARAM::VEC4_0, Vec4(1.f, 1.f, 1.f, 1.f)); // base color factor
	pKnight_Cape->SetTexOverride(TEX_PARAM::TEX_0,
		CAssetMgr::GetInst()->Load<CTexture>(L"knight_cape_basecolor", L"texture\\knight\\T_CrusaderKnight_Cape_BaseColor.PNG"));
	pKnight_Cape->SetTexOverride(TEX_PARAM::TEX_1,
		CAssetMgr::GetInst()->Load<CTexture>(L"knight_cape_normal", L"texture\\knight\\T_CrusaderKnight_Cape_Normal.PNG"));
	pKnight_Cape->SetTexOverride(TEX_PARAM::TEX_2,
		CAssetMgr::GetInst()->Load<CTexture>(L"knight_cape_ORM", L"texture\\knight\\T_CrusaderKnight_Cape_OcclusionRoughnessMetallic.PNG"));

	CMaterialInstance* pKnight_Body = new CMaterialInstance;
	pKnight_Body->SetParentMaterial(pPBRParent);
	pKnight_Body->SetScalarOverride(SCALAR_PARAM::INT_0, 1); // alpha cutout
	pKnight_Body->SetTexOverride(TEX_PARAM::TEX_0,
		CAssetMgr::GetInst()->Load<CTexture>(L"knight_body_basecolor", L"texture\\knight\\T_CrusaderKnight_Body_BaseColor.PNG"));
	pKnight_Body->SetTexOverride(TEX_PARAM::TEX_1,
		CAssetMgr::GetInst()->Load<CTexture>(L"knight_body_normal", L"texture\\knight\\T_CrusaderKnight_Body_Normal.PNG"));
	pKnight_Body->SetTexOverride(TEX_PARAM::TEX_2,
		CAssetMgr::GetInst()->Load<CTexture>(L"knight_body_ORM", L"texture\\knight\\T_CrusaderKnight_Body_OcclusionRoughnessMetallic.PNG"));

	CMaterialInstance* pKnight_ChainBody = new CMaterialInstance;
	pKnight_ChainBody->SetParentMaterial(pPBRParent);
	pKnight_ChainBody->SetScalarOverride(SCALAR_PARAM::INT_0, 1); // alpha cutout
	pKnight_ChainBody->SetTexOverride(TEX_PARAM::TEX_0,
		CAssetMgr::GetInst()->Load<CTexture>(L"knight_chainbody_basecolor", L"texture\\knight\\T_CrusaderKnight_ChainBody_BaseColor.PNG"));
	pKnight_ChainBody->SetTexOverride(TEX_PARAM::TEX_1,
		CAssetMgr::GetInst()->Load<CTexture>(L"knight_chainbody_normal", L"texture\\knight\\T_CrusaderKnight_ChainBody_Normal.PNG"));
	pKnight_ChainBody->SetTexOverride(TEX_PARAM::TEX_2,
		CAssetMgr::GetInst()->Load<CTexture>(L"knight_chainbody_ORM", L"texture\\knight\\T_CrusaderKnight_ChainBody_OcclusionRoughnessMetallic.PNG"));

	CMaterialInstance* pKnight_Armor = new CMaterialInstance;
	pKnight_Armor->SetParentMaterial(pPBRParent);
	pKnight_Armor->SetScalarOverride(SCALAR_PARAM::INT_0, 1); // alpha cutout
	pKnight_Armor->SetScalarOverride(SCALAR_PARAM::VEC4_0, Vec4(1.f, 1.f, 1.f, 1.f));
	pKnight_Armor->SetTexOverride(TEX_PARAM::TEX_0,
		CAssetMgr::GetInst()->Load<CTexture>(L"knight_armor_basecolor", L"texture\\knight\\T_CrusaderKnight_Armor_BaseColor.PNG"));
	pKnight_Armor->SetTexOverride(TEX_PARAM::TEX_1,
		CAssetMgr::GetInst()->Load<CTexture>(L"knight_armor_normal", L"texture\\knight\\T_CrusaderKnight_Armor_Normal.PNG"));
	pKnight_Armor->SetTexOverride(TEX_PARAM::TEX_2,
		CAssetMgr::GetInst()->Load<CTexture>(L"knight_armor_ORM", L"texture\\knight\\T_CrusaderKnight_Armor_OcclusionRoughnessMetallic.PNG"));

	// * 0 is leather
	pKnight->MeshRenderer()->SetMaterial(pKnight_Leather, 0);
	// * 1 is legs
	pKnight->MeshRenderer()->SetMaterial(pKnight_Legs, 1);
	// * 2 is cape
	pKnight->MeshRenderer()->SetMaterial(pKnight_ChainBody, 2);
	// * 3 is body
	pKnight->MeshRenderer()->SetMaterial(pKnight_Body, 3);
	// * 4 is chain body
	pKnight->MeshRenderer()->SetMaterial(pKnight_Cape, 4);
	//pKnight->MeshRenderer()->SetMaterial(pKnight_Cape, 5);
	//pKnight->MeshRenderer()->SetMaterial(pKnight_Leather, 6);
	//pKnight->MeshRenderer()->SetMaterial(pKnight_Leather, 7);
	//pKnight->MeshRenderer()->SetMaterial(pKnight_Leather, 8);
	//pKnight->MeshRenderer()->SetMaterial(pKnight_Armor, 9);

	pKnight->MeshRenderer()->SetMaterial(pKnight_Armor, 11);

	pKnight->BoundingBox()->SetAbsolute(true);
	pKnight->BoundingBox()->SetOffsetPos(Vec3(0.f, 0.f, 0.f));
	pKnight->BoundingBox()->SetRadius(600.f);

	pLevel->AddObject(0, pKnight);
	*/

	// =====================
	// SkyBox
	// =====================
	CGameObject* pSkyBox = new CGameObject;
	pSkyBox->SetName(L"SkyBox");
	pSkyBox->AddComponent(new CTransform);
	pSkyBox->AddComponent(new CSkyBox);

	pSkyBox->Transform()->SetRelativePos(Vec3(0.f, 0.f, 0.f));
	pSkyBox->Transform()->SetRelativeScale(Vec3(10000.f, 10000.f, 10000.f));

	//pSkyBox->SkyBox()->SetSkyBoxType(SPHERE);
	//pSkyBox->SkyBox()->SetSkyBoxTexture(CAssetMgr::GetInst()->FindAsset<CTexture>(L"texture\\SkyBox\\Sky02.jpg"));

	pSkyBox->SkyBox()->SetSkyBoxType(SKYBOX_TYPE::SPHERE);
	pSkyBox->SkyBox()->SetSkyBoxTexture(CAssetMgr::GetInst()->Load<CTexture>(L"plains_sunset_4k", L"texture\\SkyBox\\venice_sunset_4k.hdr"));
	pSkyBox->SkyBox()->SetVisible(false);

	pLevel->AddObject(0, pSkyBox);

	// =====================
	// Player
	// =====================
	/*
	CGameObject* pCutFish = new CGameObject;
	pCutFish->SetName(L"Fish");

	pCutFish->AddComponent(new CTransform);
	pCutFish->AddComponent(new CMeshRenderer);
	pCutFish->AddComponent(new CBoundingBox);

	pCutFish->Transform()->SetRelativePos(Vec3(-400.f, 600.f, -1000.f));
	pCutFish->Transform()->SetRelativeScale(Vec3(200.f, 200.f, 200.f));
	
	// Load FBX mesh
	pCutFish->MeshRenderer()->SetMesh(CAssetMgr::GetInst()->Load<CMesh>(L"Cut_Fish", L"mesh\\cut_fish.obj"));
	pCutFish->MeshRenderer()->SetMaterial(
		CAssetMgr::GetInst()->FindAsset<CMaterial>(L"Std3DDeferredPBRMaterial"));
	pCutFish->MeshRenderer()->GetMaterial()->SetTexParam(
		TEX_PARAM::TEX_0,
		CAssetMgr::GetInst()->Load<CTexture>(L"texture\\fish_basecolor.png", L"texture\\Fish_Albedo.png"));
	pCutFish->MeshRenderer()->GetMaterial()->SetTexParam(
		TEX_PARAM::TEX_1,
		CAssetMgr::GetInst()->Load<CTexture>(L"texture\\fish_normal.png", L"texture\\Fish_Normalmap.png"));
	pCutFish->MeshRenderer()->GetMaterial()->SetTexParam(
		TEX_PARAM::TEX_2,
		CAssetMgr::GetInst()->Load<CTexture>(L"texture\\fish_ORM.png", L"texture\\Fish_ORM.png"));

	pCutFish->BoundingBox()->SetAbsolute(true);
	pCutFish->BoundingBox()->SetOffsetPos(Vec3(0.f, 0.f, 0.f));
	pCutFish->BoundingBox()->SetRadius(600.f);

	pLevel->AddObject(0, pCutFish);
	*/

	// =====================
	// Gameboy
	// =====================

	CGameObject* pGameboy = new CGameObject;
	pGameboy->SetName(L"Gameboy");

	pGameboy->AddComponent(new CTransform);
	pGameboy->AddComponent(new CMeshRenderer);
	pGameboy->AddComponent(new CBoundingBox);

	pGameboy->Transform()->SetRelativePos(Vec3(470.f, 437.f, -986.f));
	pGameboy->Transform()->SetRelativeScale(Vec3(1.f, 1.f, 1.f));
	pGameboy->Transform()->SetRelativeRotation(Vec3(ToRadians(2.f), ToRadians(186.f), ToRadians(-76.f)));
	
	// Load FBX mesh
	pGameboy->MeshRenderer()->SetMesh(CAssetMgr::GetInst()->Load<CMesh>(L"Gameboy", L"mesh\\Gameboy.fbx"));
	CMaterialInstance* pGameboyMI = new CMaterialInstance;
	pGameboyMI->SetParentMaterial(pPBRParent);
	pGameboyMI->SetTexOverride(TEX_PARAM::TEX_0,
		CAssetMgr::GetInst()->Load<CTexture>(L"texture\\gameboy_basecolor.png", L"texture\\Gameboy_BaseColor.png"));
	pGameboyMI->SetTexOverride(TEX_PARAM::TEX_1,
		CAssetMgr::GetInst()->Load<CTexture>(L"texture\\gameboy_normal.png", L"texture\\Gameboy_Normal.png"));
	pGameboyMI->SetTexOverride(TEX_PARAM::TEX_2,
		CAssetMgr::GetInst()->Load<CTexture>(L"texture\\gameboy_ORM.png", L"texture\\Gameboy_ORM.png"));
	pGameboy->MeshRenderer()->SetMaterial(pGameboyMI);

	pGameboy->BoundingBox()->SetAbsolute(true);
	pGameboy->BoundingBox()->SetOffsetPos(Vec3(0.f, 0.f, 0.f));
	pGameboy->BoundingBox()->SetRadius(600.f);

	pLevel->AddObject(0, pGameboy);
	// =====================
	// Sofa table lamp
	// =====================
	CGameObject* pSetChairTableLamp = new CGameObject;
	pSetChairTableLamp->SetName(L"SofaTableLamp");

	pSetChairTableLamp->AddComponent(new CTransform);
	pSetChairTableLamp->AddComponent(new CMeshRenderer);
	pSetChairTableLamp->AddComponent(new CBoundingBox);

	pSetChairTableLamp->Transform()->SetRelativePos(Vec3(0.f, 0.f, 0.f));
	pSetChairTableLamp->Transform()->SetRelativeScale(Vec3(10.f, 10.f, 10.f));
	
	// Load FBX mesh
	pSetChairTableLamp->MeshRenderer()->SetMesh(CAssetMgr::GetInst()->Load<CMesh>(L"SofaTableLamp", L"mesh\\Set_chair_table_lamp.fbx"));
	Ptr<CMesh> pSofaMesh = pSetChairTableLamp->MeshRenderer()->GetMesh();
	for (UINT i = 0; i < pSofaMesh->GetSubMeshCount(); ++i)
	{
		const CMesh::tSubMesh& sm = pSofaMesh->GetSubMesh(i);
		wchar_t buf[256];
		swprintf_s(buf, L"SofaTableLamp SubMesh[%u]: IndexStart=%u, IndexCount=%u, MaterialIndex=%u\n",
				   i, sm.IndexStart, sm.IndexCount, sm.MaterialIndex);
		OutputDebugString(buf);
	}


	CMaterialInstance* pSofaTableLampMI = new CMaterialInstance;
	pSofaTableLampMI->SetParentMaterial(pPBRParent);
	// Assign material to ALL submesh slots
	for (UINT i = 0; i < pSofaMesh->GetSubMeshCount(); ++i)
	{
		pSetChairTableLamp->MeshRenderer()->SetMaterial(pSofaTableLampMI, i);
	}
	pSofaTableLampMI->SetTexOverride(TEX_PARAM::TEX_0,
		CAssetMgr::GetInst()->Load<CTexture>(L"texture\\sofa_table_lamp_basecolor.png", L"texture\\SetChair_Table_Lamp_Basecolor.png"));
	pSofaTableLampMI->SetTexOverride(TEX_PARAM::TEX_1,
		CAssetMgr::GetInst()->Load<CTexture>(L"texture\\sofa_table_lamp_normal.png", L"texture\\SetChair_Table_Lamp_Normal_OpenGL.png"));
	pSofaTableLampMI->SetTexOverride(TEX_PARAM::TEX_2,
		CAssetMgr::GetInst()->Load<CTexture>(L"texture\\sofa_table_lamp_ORM.png", L"texture\\SetChair_Table_Lamp_ORM.png"));
	pSofaTableLampMI->SetTexOverride(TEX_PARAM::TEX_3,
		CAssetMgr::GetInst()->Load<CTexture>(L"texture\\sofa_table_lamp_emissive.png", L"texture\\SetChair_Table_Lamp_Emissive.png"));
	pSofaTableLampMI->SetScalarOverride(SCALAR_PARAM::VEC4_1, Vec4(1.f, 1.f, 1.f, 1.f));
	pSetChairTableLamp->MeshRenderer()->SetMaterial(pSofaTableLampMI);

	pSetChairTableLamp->BoundingBox()->SetAbsolute(true);
	pSetChairTableLamp->BoundingBox()->SetOffsetPos(Vec3(0.f, 0.f, 0.f));
	pSetChairTableLamp->BoundingBox()->SetRadius(600.f);
	pLevel->AddObject(0, pSetChairTableLamp);
	// =====================
	// Sponza
	// =====================
	/*
	CGameObject* pSponza = CModelImporter::Load(L"mesh\\sponza\\NewSponza_Main_glTF_003.gltf");
	pSponza->SetName(L"Sponza");
	pSponza->Transform()->SetRelativePos(Vec3(0.f, 0.f, 0.f));
	pSponza->Transform()->SetRelativeScale(Vec3(100.f, 100.f, 100.f));
	if (nullptr != pSponza)
	{
		pLevel->AddObject(0, pSponza);
	}

	// =====================
	// Sponza Curtains
	// =====================
	CGameObject* pSponzaCurtains = CModelImporter::Load(L"mesh\\sponza_curtains\\NewSponza_Curtains_glTF.gltf");
	pSponzaCurtains->SetName(L"SponzaCurtains");
	pSponzaCurtains->Transform()->SetRelativePos(Vec3(0.f, 0.f, 0.f));
	pSponzaCurtains->Transform()->SetRelativeScale(Vec3(100.f, 100.f, 100.f));
	if (nullptr != pSponzaCurtains)
	{
		pLevel->AddObject(0, pSponzaCurtains);
	}


	// =====================
	// Decal
	// =====================
	/*
	CGameObject* pDecalObj = new CGameObject;
	pDecalObj->SetName(L"Decal");

	pDecalObj->AddComponent(new CTransform);
	pDecalObj->AddComponent(new CDecal);

	pDecalObj->Transform()->SetRelativeScale(Vec3(100.f, 100.f, 200.f));

	pDecalObj->Decal()->SetDecalTexture(CAssetMgr::GetInst()->FindAsset<CTexture>(L"texture\\Character.png"));
	pDecalObj->Decal()->SetAsEmissive(true);
	pDecalObj->Decal()->SetEmissiveIntensity(3.f);

	pLevel->AddObject(0, pDecalObj);
	*/
	
	ChangeLevel(pLevel, LEVEL_STATE::STOP);

}

void CTestLevel3D::CreatePrefab()
{
}
