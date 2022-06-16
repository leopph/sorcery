@echo off

::set vswhere=%programfiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe
::set msbuild=MSBuild

::if not exist "%vswhere%" (
::    echo Can't find vswhere at "%vswhere%".
::    exit
::)

::for /f "delims=" %%p in ('"%vswhere%" -property installationPath') do (
::    call "%%p"\Common7\Tools\VsMSBuildCmd.bat
::)

set CONFIG_DEBUG=Debug
set CONFIG_RELEASE=Release
set CMAKE_BUILD_CONFIG=cmake --build . --config

git submodule update --init Engine/deps/vendor/gl3w
git submodule update --init Engine/deps/vendor/glfw

::cd Engine\deps\vendor\physx
:: :: call :PhysX
cd Engine\deps\vendor\assimp
cd ..\assimp
call :Assimp
cd ..\gl3w
call :Gl3w
cd ..\glfw
call :GLFW
cd ..\spdlog
call :Spdlog
cd ..\zlib
call :Zlib
exit /B 0

:Assimp
cmake -DBUILD_SHARED_LIBS=0 -DASSIMP_NO_EXPORT=1 -DASSIMP_BUILD_ZLIB=0 -DASSIMP_BUILD_ASSIMP_TOOLS=0 -DASSIMP_BUILD_TESTS=0 -DASSIMP_INJECT_DEBUG_POSTFIX=0 -DCMAKE_CONFIGURATION_TYPES="%CONFIG_DEBUG%;%CONFIG_RELEASE%" -DLIBRARY_SUFFIX="" .
%CMAKE_BUILD_CONFIG% %CONFIG_DEBUG%
%CMAKE_BUILD_CONFIG% %CONFIG_RELEASE%
exit /B 0

:GLFW
cmake -DCMAKE_CONFIGURATION_TYPES="%CONFIG_DEBUG%;%CONFIG_RELEASE%" -DGLFW_BUILD_DOCS=0 -DGLFW_BUILD_EXAMPLES=0 -DGLFW_BUILD_TESTS=0 -DGLFW_USE_HYBRID_HPG=1 .
%CMAKE_BUILD_CONFIG% %CONFIG_DEBUG%
%CMAKE_BUILD_CONFIG% %CONFIG_RELEASE%
exit /B 0

:Gl3w
cmake -DCMAKE_CONFIGURATION_TYPES="%CONFIG_DEBUG%;%CONFIG_RELEASE%" .
%CMAKE_BUILD_CONFIG% %CONFIG_DEBUG%
%CMAKE_BUILD_CONFIG% %CONFIG_RELEASE%
exit /B 0

:::PhysX
::cd physx
::call generate_projects.bat "vc17win64"
::cd compiler/vc17win64
::start "Building PhysX Checked" "%msbuild%" PhysXSDK.sln /p:Configuration=checked
::start "Building PhysX Profile" "%msbuild%" PhysXSDK.sln /p:Configuration=profile
::start "Building PhysX Release" "%msbuild%" PhysXSDK.sln /p:Configuration=release
::cd ../../..
::exit /B 0

:Spdlog
cmake -DCMAKE_CONFIGURATION_TYPES="%CONFIG_DEBUG%;%CONFIG_RELEASE%" -DSPDLOG_BUILD_EXAMPLE=0 .
%CMAKE_BUILD_CONFIG% %CONFIG_DEBUG%
%CMAKE_BUILD_CONFIG% %CONFIG_RELEASE%
exit /B 0

:Zlib
cmake -DCMAKE_CONFIGURATION_TYPES="%CONFIG_DEBUG%;%CONFIG_RELEASE%"
%CMAKE_BUILD_CONFIG% %CONFIG_DEBUG%
%CMAKE_BUILD_CONFIG% %CONFIG_RELEASE%
exit /B 0