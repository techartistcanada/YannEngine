#ifndef _YANN_ENGINE_ENUM_H_
#define _YANN_ENGINE_ENUM_H_
// ==============================
// Component Type
// ==============================
enum class COMPONENT_TYPE
{
	TRANSFORM,
	CAMERA,
	COLLIDER2D,
	COLLIDER3D,
	ANIMATOR2D,
	ANIMATOR3D,
	LIGHT2D,
	LIGHT3D,
	BOUNDINGBOX,

	MESHRENDERER,
	SKYBOX,
	DECAL,
	PARTICLESYSTEM,
	TILEMAP,
	LANDSCAPE,

	END,

	SCRIPT,
};

extern const char* COMPONENT_TYPE_STRINGS[(UINT)COMPONENT_TYPE::END];


// ==============================
// Light Type
// ==============================
enum class LIGHT_TYPE
{
	DIRECTIONAL,
	POINT,
	SPOT,
};

// ==============================
// Asset Type
// ==============================
enum class ASSET_TYPE
{
	PREFAB,
	MESH,
	MESH_DATA,
	MATERIAL,
	TEXTURE,
	SOUND,
	GRAPHICS_SHADER,
	COMPUTE_SHADER,


	END,
};

extern const char* ASSET_TYPE_STRINGS[(UINT)ASSET_TYPE::END];

// ==============================
// Constant Buffer Type
// ==============================
enum class CB_TYPE
{
	TRANSFORM,
	MATERIAL,
	ANIMATION,
	GLOBAL,
	END, 
};

// ==============================
// Rasterizer State Type
// ==============================
enum class RS_TYPE
{
	CULL_BACK,
	CULL_FRONT,
	CULL_NONE,
	WIRE_FRAME,

	END,

};

// DepthStencil State
enum class DS_TYPE
{
	LESS,
	LESS_EQUAL,

	GREATER,

	NO_WRITE,
	NO_TEST,
	NO_TEST_NO_WRITE,

	// For Volume Mesh for Deferred Point Light
	BACKFACE_CHECK,
	FRONTFACE_CHECK,
	STENCIL_CHECK,

	END,
};

// Blend State
enum class BS_TYPE
{
	DEFAULT,			// 0, 关闭混合
	ALPHA_BLEND,		// 计算公式:  SrcAlpha * SrcColor + (1 - SrcAlpha) * DestColor
	ALPHA_TO_COVERAGE,	// 多重采样时使用
	ONE_ONE,			// 计算公式:  1 * SrcColor + 1 * DestColor

	DECAL_BLEND,        // Target0 - AlphaBlend, Target1 - OneOne
	END,
};

// direction
enum class DIR_TYPE
{
	RIGHT,
	UP,
	FRONT,
};
extern Vec3 XAxis;
extern Vec3 YAxis;
extern Vec3 ZAxis;

// ==============================
// scalar parameter for shader/material
// ==============================
enum SCALAR_PARAM
{
	INT_0,
	INT_1,
	INT_2,
	INT_3,

	FLOAT_0,
	FLOAT_1,
	FLOAT_2,
	FLOAT_3,

	VEC2_0,
	VEC2_1,
	VEC2_2,
	VEC2_3,

	VEC4_0,
	VEC4_1,
	VEC4_2,
	VEC4_3,

	MAT_0,
	MAT_1,
	MAT_2,
	MAT_3,

	SCALAR_END,
};

// ==============================
// Texture Parameter for shader/material
// ==============================
enum TEX_PARAM
{
	TEX_0,
	TEX_1,
	TEX_2,
	TEX_3,
	TEX_4,
	TEX_5,

	TEX_CUBE_0,
	TEX_CUBE_1,
	TEX_CUBE_2,
	TEX_CUBE_3,

	TEX_ARR_0,
	TEX_ARR_1,
	TEX_ARR_2,
	TEX_ARR_3,

	TEX_END,

};

enum class DEBUG_SHAPE
{
	RECT,
	CIRCLE,
	LINE,

	CUBE,
	SPHERE,

};

enum class PROJ_TYPE
{
	ORTHOGRAPHIC,
	PERSPECTIVE,
};

enum class TASK_TYPE
{

	// Param 0: Layer Index
	// Param 1: Object Address
	SPAWN_OBJECT,
	// Param 0: Object Address
	DESTROY_OBJECT,
	// Param 0: Collider2D Address
	COLLIDER_SEMI_DEACTIVATE,
	COLLIDER_DEACTIVATE,

	// Param 0: Level Address Param 1: LEVEL_STATE
	CHANGE_LEVEL,

	CHANGE_LEVEL_STATE,

	// Param 0: Asset Address, :w

	DEL_ASSET
};

enum class SHADER_DOMAIN
{
	// Deffered rendering pipeline domains
	DOMAIN_DEFFERED,
	DOMAIN_DEFERRED_DECAL,
	DOMAIN_DEFFERED_LIGHT,
	DOMAIN_SHADOWMAP,

	// Forward rendering pipeline domains
	DOMAIN_OPAQUE,
	DOMAIN_MASKED,
	DOMAIN_TRANSPARENT,

	DOMAIN_PARTICLE,
	DOMAIN_POSTPROCESS,
};

// particle system modules
enum class PARTICLE_MODULE
{
	SPAWN,
	SPAWN_BURST,
	ADD_VELOCITY,
	SCALE,
	DRAG,
	NOISE_FORCE,
	RENDER,
	END,
};

// level state
enum class LEVEL_STATE
{
	PLAY,
	STOP,
	PAUSE,
};

enum class MRT_TYPE
{
	SWAPCHAIN,

	DEFERRED,
	DEFERRED_DECAL,
	DEFERRED_LIGHT,

	SSAO,
	SSAO_BLUR,

	HDR_SCENE,

	END,
};

#endif
