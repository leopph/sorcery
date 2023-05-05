# ⚙ LeopphEngine ⚙

## About the project
LeopphEngine (pronounced "lœff") is my solo-project game engine. I started building it to learn about game development technology, as this is my strongest interest. I used it as my CS BSc thesis and now I'm continuously improving it, adding more features to it. I'm planning to use it as my MSc thesis and future reference work. The name, as stupid as it may sound, comes from my last name. No, I couldn't come up with anything better. Yes, seriously.

## History
Originally LeopphEngine was a simple C++ runtime library that one could link against to create simple interactive 3D environments. The current development direction however is to create a fully featured editor capable of building levels, managing projects and resources, and exporting game executables, and to build the runtime layer on top of this.

## Used technologies
Non-exhaustive list of core libraries:
- Win32 for platform basics
- Direct3D 11 for graphics
- Mono for C# scripting
- Dear ImGui for tool interfaces
- Assimp for model asset importing

## Building and Usage
### Build requirements
- Windows SDK 10.0.14393.795 or newer
- MSBuild
- MSVC v143 toolset
### Runtime requirements
- Windows 10, version 1607 or newer, 64-bit
- Mono 6.12.0 or newer
- Feature level 11_0 capable GPU
### Building
- Run **setup.bat** from the root directory
- Build the solution in the root directory
### Usage
The compilation products are a set of runtime libraries and an editor executable. To use the engine, run the editor. The libraries are not meant to be consumed directly, instead they are part of the editor and the games exported through it.
