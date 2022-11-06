@echo off

REM Clone and bootstrap vcpkg
git submodule update --init vcpkg
.\vcpkg\bootstrap-vcpkg.bat -disableMetrics


REM Build Mono runtime using MSBuild
setlocal enabledelayedexpansion
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
  "%%i" Engine/deps/vendor/mono/msvc/mono.sln
  exit /b !errorlevel!
)


REM set CONFIG_DEBUG=Debug
REM set CONFIG_RELEASE=Release
REM set CMAKE_BUILD_CONFIG=cmake --build . --config
REM 
REM git submodule update --init Engine/deps/vendor/glfw
REM 
REM cd Engine\deps\vendor\assimp
REM cd ..\assimp
REM call :Assimp
REM cd ..\glfw
REM call :GLFW
REM cd ..\spdlog
REM call :Spdlog
REM cd ..\zlib
REM call :Zlib
REM exit /B 0
REM 
REM :Assimp
REM cmake -DBUILD_SHARED_LIBS=0 -DASSIMP_NO_EXPORT=1 -DASSIMP_BUILD_ZLIB=0 -DASSIMP_BUILD_ASSIMP_TOOLS=0 -DASSIMP_BUILD_TESTS=0 -DASSIMP_INJECT_DEBUG_POSTFIX=0 -DCMAKE_CONFIGURATION_TYPES="%CONFIG_DEBUG%;%CONFIG_RELEASE%" -DLIBRARY_SUFFIX="" .
REM %CMAKE_BUILD_CONFIG% %CONFIG_DEBUG%
REM %CMAKE_BUILD_CONFIG% %CONFIG_RELEASE%
REM exit /B 0
REM 
REM :GLFW
REM cmake -DCMAKE_CONFIGURATION_TYPES="%CONFIG_DEBUG%;%CONFIG_RELEASE%" -DGLFW_BUILD_DOCS=0 -DGLFW_BUILD_EXAMPLES=0 -DGLFW_BUILD_TESTS=0 -DGLFW_USE_HYBRID_HPG=1 .
REM %CMAKE_BUILD_CONFIG% %CONFIG_DEBUG%
REM %CMAKE_BUILD_CONFIG% %CONFIG_RELEASE%
REM exit /B 0
REM 
REM :Spdlog
REM cmake -DCMAKE_CONFIGURATION_TYPES="%CONFIG_DEBUG%;%CONFIG_RELEASE%" -DSPDLOG_BUILD_EXAMPLE=0 .
REM %CMAKE_BUILD_CONFIG% %CONFIG_DEBUG%
REM %CMAKE_BUILD_CONFIG% %CONFIG_RELEASE%
REM exit /B 0
REM 
REM :Zlib
REM cmake -DCMAKE_CONFIGURATION_TYPES="%CONFIG_DEBUG%;%CONFIG_RELEASE%"
REM %CMAKE_BUILD_CONFIG% %CONFIG_DEBUG%
REM %CMAKE_BUILD_CONFIG% %CONFIG_RELEASE%
REM exit /B 0