# ⚙ LeopphEngine ⚙

## About the project
LeopphEngine (pronounced "lœff") is my C++ game engine. I originally started building it as my CS BSc thesis with a limited feature set, and now I'm planning to improve it, add more features, and eventually use it as my MSc thesis and future reference work. The name, as stupid as it may sound, comes from my last name. No, I couldn't come up with anything better. Yes, seriously.

## Ideology
Originally LeopphEngine was supposed to build around the idea of ease of use with an API that was designed in a way that resembles higher-level languages, like managed object lifetimes and implicit pointer correctness while keeping performance relatively high. This is starting to fall somewhat apart now, as I'm digging deeper and deeper into the guts of performance-oriented programming and engine design. I'm still aiming to maintain a beginner-friendly subset of the API, with the focus being on fast development, but most new features I add these days are more geared towards fine tuning and customizability, which inherently adds some complexity. We'll see what turns out of this big mess.

## Features
This list provides an overview of the capabalities and development goals of LeopphEngine.

| Symbol | Feature State | Detailed Meaning |
| --- | --- | --- |
| ✅ | Implemented | All planned functionality is available for use. Mainline development is complete, but improvements may be delivered at any time, as well as feature extensions. This does not mean that the feature is stable and never changes. |
| 🟣 | Work In Progress | The planned feature is in active development. Some or most of the functionality may not yet be available for use. The specification, implementation, and feature goals can get significant changes from one version to the next. |
| ❌ | Planned | The feature is in planning phase. Mainline development has not yet started. The feature idea might be greatly altered before any part of it gets implemented. |

### General API
- Entity-Component system
  - ✅ fundamental entity structure
    - ✅ parental relationships and hierarchy
    - ✅ component access API
    - ✅ node based 3D position, orientation, and scale
  - ✅ component extension API
  - ✅ built-in components
    - ✅ rendering components
    - ✅ camera components
    - ✅ light components
- Scene management API
  - ❌ centralized scene manager
  - ❌ scene descriptor files
  - ❌ background scene loading
  - ❌ fast scene switching
- Math library targeted at game development
  - ✅ vectors
  - ✅ matrices
  - ✅ quaternions
  - ✅ several miscellaneous linear algebraic operations

### Graphical elements
- OpenGL renderer
  - ✅ Blinn-Phong lighting model
  - 🟣 material system
    - ✅ diffuse and specular colors
    - ✅ diffuse, specular, and opacity mapping
    - ✅ transparency
    - ❌ normal mapping
    - ❌ parallax mapping
    - ❌ customizable material properties
  - ✅ lighting system
    - ✅ ambient, directional, point-, and spotlights
    - ✅ cascaded shadow mapping for directional lights (CSM)
    - ✅ shadow mapping for spot- and pointlights
  - 🟣 forward rendering pipeline
  - ✅ deferred rendering pipeline
  - ✅ instanced rendering
  - ❌ dynamic batching based on materials
  - ❌ screen space ambient occlusion (SSAO)
  - ❌ screen space reflections (SSR)
  - ❌ gamma correction
  - ❌ bloom
  - ❌ skeletal animation system
  - ❌ custom shaders
- Vulkan renderer
  - ❌ new renderer using the Vulkan API with all features of the OpenGL renderer
- Custom model format and conversion toolset
  - ✅ leopph3d file format for optimal 3D model storage
  - ✅ Leopphverter conversion tool from many different formats
- Integrated graphical interface API
  - ❌ Dear ImGui integration

### Physics simulation
- Full 3D physics system
  - ❌ Nvidia PhysX integration

### Input management
- Centralized input handling
  - ✅ frame-based keyboard key state querying
  - ✅ cursor position queries
  - ❌ controller support

### Multithreading
- Load-balancing job system
  - 🟣 basic task system and execution
  - ❌ scaling to an arbitrary number of CPU cores
  - ❌ work stealing
  - ❌ jobification of other engine systems

### Sound
- Spatial audio system
  - ❌ no concrete plans yet

### Utility
- Resource and memory management system
  - 🟣 data caching and reuse
  - 🟣 full internal lifetime and memory management
- Event system
  - ✅ centralized observer pattern
  - ✅ custom event type creation
  - ✅ inheritance based event handlers
  - ✅ composition based event handlers

### Development
- Code structure
  - 🟣 fully modular architecture with major feature groups and systems in separate libraries
- Consumer integration
  - ❌ moving from headers to C++20 modules where applicable
- Platform support
  - ❌ support for Linux and macOS on x86-64
  - ❌ moving from MSBuild to CMake


## Documentation
Currently there is no standalone documentation or tutorial on how to use LeopphEngine. The API headers contain helpful information to get a better picture of the concepts in comment form. Functions and types are annotated with their most important properties.

## Support and Usage
### Build requirements
- Windows
- MSBuild
- MSVC v143 toolset
- CMake
- Python 2.7 or newer
### Runtime requirements
- Windows
- OpenGL 4.5 compatible GPU
### Building
- Run **setup.bat** from the root directory
  - This pulls all submodules, then configures and builds all dependencies under *Engine/deps/vendor*
- Build the solution in the root directory  
### Consumption
To use LeopphEngine in your application:
- Add **Engine/include** to your compiler's include path
- Link against the necessary libraries under **Engine/bin/*<target_config>***
LeopphEngine is highly modular. Depending on the feature set you use, you can leave some libraries out of the linking process to save time. You can also pick and choose from the vast number of headers available to reduce your compilation times, or just include the combined **Leopph.hpp** header to get all the features.

## Branches and Releases
Development of LeopphEngine takes place on two branches.  
- The **master** branch always contains the latest code that is verified to compile and be stable, and can be used for game development purposes. Features may disappear from it, so it is in no way a "release" channel, but it is the most stable, slowest branch that never contains code that hasn't passed the dev branch.  
- The **dev** branch is where all the new stuff goes. As soon as anything changes, it is reflected in this channel. Features are highly volatile here, code may be heavily buggy, incomplete, or even non-compiling. If you happen to check this branch out and spend some time exploring it, your comments and suggestions would be very welcome.
- There may appear other branches outside of the standard 2 branch setup. These can safely be ignored by developers as more than likely I just set them up for organizing development of a feature according to my personal taste.  

Every now and then I release "PB" builds. What does PB stand for? It may be "Public Beta". It could be "Published Build". It just might be "Product Breakthrough". Who knows? The versioning is simple: it is the date in the format **yymmdd** with an optional letter at the end in case there are multiple releases a day. Letters are added in alphabetic order.  
Example: LeopphEngine ***PB211223b***. This means, that the build came out on December 23rd, 2021, and it is the third release that day (since no letter would mean first and "a" would mean second).  
You may also find releases using an earlier versioning scheme. In the case of those builds, you can rely on higher numbers meaning later releases.