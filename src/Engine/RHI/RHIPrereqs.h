#pragma once

// Windows 平台类型 (HWND, UINT, DWORD 等)
// 必须在 global.h 之前 include
#include <windows.h>

// 引擎数学 + STL + ComPtr
// global.h 中 d3d11.h 已注释掉 —— 不会引入任何 DX11/DX12 类型
#include "../global.h"

// RHI 层自己的类型定义
// (RHI_BIND_FLAG 等放这里，所有 IRHIXxx.h 共用)
#include <dxgiformat.h>  // DXGI_FORMAT — DX11/DX12 共用，不属于任何一方

enum class RHI_BIND_FLAG : UINT
{
    NONE             = 0,
    SHADER_RESOURCE  = 1 << 0,  // SRV  t 寄存器
    RENDER_TARGET    = 1 << 1,  // RTV  颜色输出
    DEPTH_STENCIL    = 1 << 2,  // DSV  深度/模板
    UNORDERED_ACCESS = 1 << 3,  // UAV  计算读写
};
DEFINE_ENUM_FLAG_OPERATORS(RHI_BIND_FLAG)

enum class RHI_BUFFER_TYPE : UINT
{
    VERTEX_BUFFER,
    INDEX_BUFFER,
	CONSTANT_BUFFER,
	STRUCTURED_BUFFER,
};

enum class RHI_INDEX_FORMAT : UINT
{
    UINT16,  // 顶点数 <= 65535
    UINT32,  // 顶点数 > 65535
};

// Values match D3D_PRIMITIVE_TOPOLOGY — backends can static_cast<> directly
enum class RHI_PRIMITIVE_TOPOLOGY : UINT
{
    UNDEFINED        = 0,
    POINT_LIST       = 1,
    LINE_LIST        = 2,
    LINE_STRIP       = 3,
    TRIANGLE_LIST    = 4,
    TRIANGLE_STRIP   = 5,
    PATCH_LIST_1     = 33,  // tessellation control points
    PATCH_LIST_2     = 34,
    PATCH_LIST_3     = 35,
    PATCH_LIST_4     = 36,
};