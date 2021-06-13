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
Currently only Windows is supported. The easiest way you can get started is to open the Visual Studio solution and create a new project in it, then reference the LeopphEngine project from it. This way, you get all the necessary public headers.
*Disclaimer: LeopphEngine uses C++20 features that are on the edge of stability and not yet fully implemented. You should probably use the latest Visual Studio version available (or even the preview) to make sure your compiler has the necessary features.*