#include "pch.h"

tTransform g_Trans = {};
tGlobalData g_GlobalData = {};

extern Vec3 XAxis(1.f, 0.f, 0.f);
extern Vec3 YAxis(0.f, 1.f, 0.f);
extern Vec3 ZAxis(0.f, 0.f, 1.f);


extern const char* COMPONENT_TYPE_STRINGS[(UINT)COMPONENT_TYPE::END] =
{
	"TRANSFORM",
	"CAMERA",
	"COLLIDER2D",
	"COLLIDER3D",
	"ANIMATOR2D",
	"ANIMATOR3D",
	"LIGHT2D",
	"LIGHT3D",
	"BOUNDINGBOX",

	"MESHRENDERER",
	"SKYBOX",
	"DECAL",
	"PARTICLESYSTEM",
	"TILEMAP",
	"LANDSCAPE",
};


extern const char* ASSET_TYPE_STRINGS[(UINT)ASSET_TYPE::END] =
{
	"PREFAB",
	"MESH",
	"MESH_DATA",
	"MATERIAL",
	"TEXTURE",
	"SOUND",
	"GRAPHICS_SHADER",
	"COMPUTER_SHADER",
};


