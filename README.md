# ⚙ LeopphEngine ⚙

## About the project
LeopphEngine is a WIP game engine written in C++. It's my BSc thesis, so its main purpose is to get me through university (lol). It's name, as stupid as it may sound, is derived from my last name. No, I couldn't come up with anything better. Yes, seriously.

## Features
These are the planned features and their current states:  
✅ - implemented  
〽️ - work in progress  
❌ - planned  
- 〽️ Entity-Component Model and Scene Hierarchy 
  - ✅ Fundamental Entity-Component structure
  - ✅ Necessary engine-provided components
  - ✅ API to make functionality extension possible
  - ❌ Parent-child relation between Entities
- 〽️ OpenGL Renderer
  - ✅ Blinn-Phong model
    - ✅ Directional, point, and spotlights
    - ✅ Ambient, diffuse and specular vertex colors
    - ✅ Ambient, diffuse and specular maps
  - ✅ Instancing of models for fast rendering
  - ❌ Normal mapping
  - ❌ Parallax mapping
  - 〽️ Cascaded shadow mapping for directional lights 
  - ❌ Shadow mapping for spotlights 
  - ❌ Shadow mapping for pointlights
  - ❌ Screen space reflections
  - ❌ FXAA
  - ❌ Skeletal animation system
- 〽️ Resource Management API
- ✅ Event System
- ❌ Sound Management API
- ❌ 3D Physics Engine
- ✅ Keyboard and Mouse Input Handling
- 〽️ Several utilites to further help game development
  - 〽️ Extensive math library mainly focusing on linear algebra and trigonometry
  - ❌ Raycast and debug visuals system

### Scene hierarchy
LeopphEngine uses a flexible Entity-Component model. Entities are skeletons that can be decorated by components. Components are custom pieces of code that give Entities additional property and may also give them per-frame behavior.

### Rendering
LeopphEngine uses OpenGL for 3D rendering. It is currently capable of parsing several model data formats and render them according to the Blinn-Phong shading model. The engine also uses the standard trio of light types and will be calculating shadow maps on all of them in the future. LeopphEngine is created in a way that the programmer does not and can not modify rendering parameters for individual models. All data should be defined in a professional modelling software and only scene specific information (mainly lighting) should be and can be set up in-engine. This is to enforce the philosophy that every graphical object has its own set of properties that is independent of the scene it is part of, and that the environments are what should be dynamic and changing.

### Resource Management
LeopphEngine puts a lot of emphasis on abstracting away data-lifetime management. In practice, this means that even though it's API is written in C++, almost all data objects coming from the engine that the developers may handle are internally managed and never require allocations and deallocations from the game side. These management procedures are designed in a way to minimize overhead so the speeds of C++ can be absolutely harvested.

### Event System
LeopphEngine provides a centrally managed event system that is shared between internal and external code. This means the engine uses the same dataflow path that the developers might use so that CPU time is shared equally among all dispatches, and the developers only need to specify the data that comes with the events, send out those events, and define event handlers using the provided APIs without having to worry about class-codependence, breaking data visibility guidelines, or accidentally creating "spaghetti code".

### Sound Management API
There will be one I promise.

### 3D Physics Engine
I sure hope I can build one of these...

### Input handling
The engine provides APIs to check key press states and mouse positions. At the time of writing the English language keyboards are fully implemented.

### Other utilites
At this point LeopphEngine provides basic trigonometry through its mathematics library, as well as vector and matrix operations to aid in computations.

## Documentation
I'm trying to place as much info into header/module files as I can. For now, developers can use these to get more info on specific behaviors. I will eventually create a standalone document about all the different APIs LeopphEngine provides.

## Support and Usage
Currently only Windows is supported.  
To get started, pull all the submodules and build them. Then compile LeopphEngine using the provided VS solution.
You should now include **Leopph.hpp** from **LeopphEngine/include** in your project, link against **leopph.lib**, and copy **leopph.dll** (both found in **LeopphEngine/*<target_config>***) to your output directory, and everything should work!  
*Disclaimer: LeopphEngine uses Microsoft's WIP C++ 20 implementation. It is recommended that you use the latest Visual Studio release and set your project language version to C++ 20 to make sure your compiler has the necessary features.*