# ⚙ LeopphEngine ⚙

## About the project
LeopphEngine is a WIP game engine written in C++. It's my BSc thesis, so its main purpose is to get me through university (lol). It's name, as stupid as it may sound, is derived from my last name. No, I couldn't come up with anything better. Yes, seriously.

## Features
These are the planned features and their current states:
- Object-Component Model and Scene Hierarchy (WIP)
- OpenGL Renderer (WIP)
- Sound Management API (TBD)
- 3D Physics Engine (TBD)
- Keyboard and Mouse Input Handling (WIP)
- Several utilites to further help game development (WIP)

### Scene hierarchy
LeopphEngine currently uses a severly underdeveloped Object-Component model, that lets users create game objects and attach components to them. These components can be predefined by the API, or user-defined to provide the game logics and other custom behaviors. The objects have spatial position, rotation, and scaling attributes to describe their states.

### Rendering
LeopphEngine has a very basic OpenGL renderer, currently capable of displaying simple models with ambient, diffuse, and specular mappings and lighting them with directional- and spotlights. Plans are to extend these with shadow mappings, screen-space reflections, and -eventually- animation handling. I may implement another, more modern graphics API (probably Vulkan) later, if I have the time and energy.

### Sound Management API
There will be one I promise.

### 3D Physics Engine
I sure hope I can build one of these...

### Input handling
The engine provides APIs to check key press states and mouse positions. Right now only a couple main buttons are supported.

### Other utilites
At this point LeopphEngine provides basic utilities to aid in mathematical computations.

## Documentation
I'm trying to place as much info into header/module files as I can. For now, users can use these to get more info on specific behaviors. I will eventually create a standalone document about all the different APIs LeopphEngine provides.

## Support and Usage
Currently only Windows is supported.  
To get started, pull all the submodules and compile Assimp, glad, GLFW, and spdlog. Then compile LeopphEngine using the provided VS solution.
You should now include **leopph.h** from **LeopphEngine/include** in your project, link against **leopph.lib**, and copy **leopph.dll** (both found in **LeopphEngine/*<target_config>***) to your output directory, and everything should work!  
*Disclaimer: LeopphEngine uses Microsoft's WIP C++20 implementation. It is recommended to use the latest Visual Studio release and set your project to std:c++latest to make sure your compiler has the necessary features.*