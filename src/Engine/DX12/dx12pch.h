// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef DX12_PCH_H
#define DX12_PCH_H

// add headers that you want to pre-compile here
#define NOMINMAX
#include <Windows.h>
#include <filesystem>

// 引擎通用工具（g_GlobalData / Vec2 / Singleton 等）
#include "../global.h"

// DX12 专属
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "d3dx12.h"
#include "DX12Helpers.h"

#include <d3dcompiler.h>

#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3dcompiler")

namespace fs = std::filesystem;

#include <algorithm>

#endif //DX12_PCH_H
