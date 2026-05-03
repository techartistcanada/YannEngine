# YannEngine

A custom C++ rendering engine built on Direct3D 12, with a parallel D3D11 path behind a shared RHI interface.

## Highlights

- **Custom RHI abstraction** — `IRHIDevice / IRHICommandList / IRHITexture / …` with DX12 (primary) and DX11 (reference) back-ends switchable at compile time. DX12 backend includes hand-rolled descriptor heap allocators, dynamic shader-visible heap, per-resource state tracker with pending barrier resolution, and triple-buffered swap chain.
- **Deferred PBR renderer** — 5-target G-Buffer (color, normal, position, emissive, metal/rough/AO), light-volume per-light draw with Cook-Torrance BRDF (GGX + Smith + Schlick), PCF shadow maps with texel-snapped orthographic projection, and forward + transparency pass layered on the HDR target.
- **GPU-compute IBL pipeline** — equirectangular HDR → cubemap, diffuse irradiance convolution, split-sum specular prefilter (importance-sampled GGX, 5 mips), and BRDF LUT, all baked on the GPU at load time via compute shaders.
- **Material instance system** — parent `CMaterial` + `CMaterialInstance` overrides (UE-style), glTF and Unreal ORM packing supported in the same PBR shader, asset hot-reload.
- **GPU particle system** — per-frame compute-shader tick updates the particle buffer in place; render pass draws from the same buffer inside the HDR pipeline.

## Build

**Requirements:** Windows 10/11 64-bit, Visual Studio 2022 or above (Desktop C++ workload), CMake ≥ 3.29.

```bat
git clone <repo-url>
External\vcpkg\bootstrap-vcpkg.bat
cmake --preset windows-x64-debug
```

Then open the generated Visual Studio solution in the `build/` directory.

For release: use `--preset windows-x64-release` instead.

## Dependencies

| Library | Purpose |
|---------|---------|
| [Assimp](https://github.com/assimp/assimp) | FBX / glTF mesh import |
| [DirectXTex](https://github.com/microsoft/DirectXTex) | Texture loading (DDS / PNG / HDR) |
| d3d12 / dxgi / dxguid | DirectX 12 system libraries |
