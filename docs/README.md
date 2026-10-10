# 🪄 Sorcery 🪄

**A C++23 / DirectX 12 rendering and game-engine playground.**

Sorcery is my hobby game engine I'm building to explore real-time graphics, modern GPU features, and game engine architecture. I built it from scratch around a custom renderer, with a scene editor and asset pipeline to make experimentation easier.

Sorcery isn't meant to be a production-ready engine. It is my playground to learn, experiment with, and implement graphics and engine systems.

## Highlights

### Modern GPU rendering
- **DirectX 12 / Shader Model 6.6** - using mesh shaders, amplification shaders, HLSL dynamic resources (bindless resource access), and enhanced barriers.
- **Meshlet-based rendering** - GPU-side meshlet frustum and normal-cone culling, with amplification shaders compacting visible meshlets before dispatching mesh shaders.
- **Deferred PBR** - a compact G-buffer and Cook–Torrance-based lighting with metallic/roughness materials, normal mapping, and alpha clipping.
- **Dynamic shadows** - cascaded shadow maps for directional lights, screen-coverage-based shadow atlas allocation for spot and point lights, and configurable PCF filtering.
- **Image-based lighting** - skybox-derived irradiance and prefiltered environment maps for diffuse and specular lighting.
- **Screen-space and temporal effects** - SSAO, TAA with motion vectors and history reprojection, and experimental screen-space reflections.
- **Skeletal animation** - animated meshes with compute-shader skinning.

### Editor and asset pipeline
Sorcery includes **Mage**, a custom editor built with Dear ImGui.

- Scene hierarchy, entity inspection, and transform manipulation.
- Editable materials, lights, cameras, and scene properties.
- Model and texture import, with configurable import settings.
- Asset management with resource identifiers, metadata, and serialized resource formats.
- Interactive rendering settings, shadow visualization, and performance inspection.

### Engine architecture
Beyond rendering, Sorcery is also a place to experiment with the systems that make an engine work:

- **Entity-component model** - hierarchical transforms, custom components, updateable behaviors, and scene serialization.
- **Resource management** - custom binary asset formats, resource references, and on-demand loading.
- **Rendering infrastructure** - frame scheduling, GPU command submission, frame-scoped uploads, and GPU resource lifetime management.
- **Custom maths** - a linear algebra implementation using x86-64 SIMD intrinsics.
- **Supporting infrastructure** - a job system, reflection, serialization, and object lifetime tracking.

The low-level graphics abstraction / RHI is implemented in [Wand](https://github.com/leopph/wand), a separate project developed alongside Sorcery.

## Building

Sorcery currently targets Windows x64 with a modern DirectX 12-capable GPU supporting mesh shaders and Shader Model 6.6.

Requirements:
- Visual Studio with an MSVC toolset supporting C++23.
- Windows SDK and DirectX 12 Agility SDK support.

Clone the project, including submodules:

<code>git clone --recursive https://github.com/leopph/sorcery.git</code>

Run <code>setup.bat</code> to bootstrap the included vcpkg installation, then open Sorcery.sln in Visual Studio and build the solution.

Dependencies are managed through vcpkg.
