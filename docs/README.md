# 🪄 Sorcery 🪄

Sorcery is my toy game engine that I'm building to learn about rendering architectures, graphics algorithms, and hardware features.

## Features
* DX12 Renderer
  * Built using HLSL Dynamic Resources, Enhanced Barriers, Mesh Shaders, and other modern features
  * Deferred rendering
    * Thin G-buffer for deferred lighting and screen space effects
  * PBR material system using Cook-Torrance BRDF
    * Albedo, metallic, roughness, ao, and normal values/maps, alpha clipping support
  * Directional, spot, and pointlights
  * Cascaded Shadow Maps for directional lights
    * Customizable split distances
    * Stabilized projection
  * Dynamic spot and pointlight shadow resolution
    * Screen-coverage based allocation from shadow atlas
  * PCF Tent filtering on shadow maps
  * Depth and normal shadow bias
  * HDR Skybox
  * Mipmapping
  * Meshlet culling on GPU for cameras and shadows
  * Screen Space Ambient Occlusion (SSAO)
  * Multi-camera support with custom viewports and render targets
  * Skeletal animations with compute skinning
  * Temporal Anti-Aliasing (TAA)
  * Image Based Lighting based on skybox (IBL)
* Entity-Component model
  * Camera, Static Mesh, Light, Skybox, and Transform components
  * Transform hierarchy
  * Support for custom components (scripting API)
  * Support for tickable components (behaviors)
  * Scene system
* Resource Management
  * Custom binary formats for textures, scenes, materials, and meshes
  * On-demand resource loading when switching scenes
* Custom linear algebra library
  * Using x64 intrinsics for better performance
* Editor
  * Full-fledged world editor
    * Transform manipulation
    * Scene traversal
    * Entity-component editing
  * Asset importing, preconditioning, and management
  * Asset creation and editing (materials, scenes)
  * Performance logging
  * Several changable settings (graphics, performance, etc.)

## Building
You'll need Visual Studio with a version of MSVC capable of C++23. Once you have that, just
- Run **setup.bat** from the root directory
- Build the solution in the root directory
