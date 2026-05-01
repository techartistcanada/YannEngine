# YannEngine

A personal DirectX 12 rendering engine built in C++17.

## Features

- DirectX 12 rendering backend
- Asset loading via [Assimp](https://github.com/assimp/assimp)
- Texture processing via [DirectXTex](https://github.com/microsoft/DirectXTex)
- HLSL shader pipeline
- Modular architecture: Engine / Scripts / CodeGen / Client

## Prerequisites

| Tool | Version |
|------|---------|
| Windows 10/11 | 64-bit |
| Visual Studio 2022 | with "Desktop development with C++" workload |
| CMake | ≥ 3.29 |
| vcpkg | latest ([install guide](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started)) |

Make sure the `VCPKG_ROOT` environment variable is set to your vcpkg installation directory.

```bat
set VCPKG_ROOT=C:\vcpkg
```

## Build

```bat
# Configure (installs dependencies automatically via vcpkg manifest mode)
cmake --preset windows-x64-debug

# Build
cmake --build --preset debug
```

For a release build:

```bat
cmake --preset windows-x64-release
cmake --build --preset release
```

The compiled executable and shaders are placed in:

```
build/debug/OutputFile/bin/Client.exe
build/debug/OutputFile/content/shader/
```

## Project Structure

```
YannEngine/
├── src/
│   ├── Engine/       # Core rendering (DX12, shaders)
│   ├── Scripts/      # Game/scene logic
│   ├── CodeGen/      # Code generation utilities
│   └── Client/       # Application entry point
├── CMakeLists.txt
├── CMakePresets.json
├── vcpkg.json        # Dependency manifest
└── vcpkg-configuration.json
```

## Dependencies

| Library | Purpose |
|---------|---------|
| [Assimp](https://github.com/assimp/assimp) | 3D asset import |
| [DirectXTex](https://github.com/microsoft/DirectXTex) | Texture loading & processing |
| d3d12 / dxgi / dxguid | DirectX 12 system libraries |
