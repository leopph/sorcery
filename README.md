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
- Windows
- MSBuild
- MSVC v143 toolset
### Runtime requirements
- Windows 10, version 1809 or newer, 64-bit
- Mono 6.12.0 or newer
- Feature level 11_0 capable GPU
### Building
- Run **setup.bat** from the root directory
- Build the solution in the root directory
### Usage
The compilation products are a set of runtime libraries and an editor executable. To use the engine, run the editor. The libraries are not meant to be consumed directly, instead they are part of the editor and the games exported through it.

## Follow the development
### Branches
Development of LeopphEngine takes place on two branches.  
- The **master** branch always contains the latest code that is verified to compile and be stable. Features may disappear from it of course, so it is in no way a "release" channel, but it is the most stable, slowest branch that never contains code that hasn't passed the dev branch.
- The **dev** branch is where all the new stuff goes. As soon as anything changes, it is reflected in this channel. Features are highly volatile here, code may be heavily buggy, incomplete, or even non-compiling. If you happen to check this branch out and spend some time exploring it, your comments and suggestions would be very welcome.
- There may appear other branches outside of the standard 2 branch setup. These can safely be ignored as more than likely I just set them up for organizing development of a feature according to my personal taste.
