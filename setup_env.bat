@echo off

set vswhere=%programfiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe
set msbuild=MSBuild

if not exist "%vswhere%" (
    echo Can't find vswhere at "%vswhere%".
    exit
)

for /f "delims=" %%p in ('"%vswhere%" -property installationPath') do (
    call "%%p"\Common7\Tools\VsMSBuildCmd.bat
)

git submodule update --init --remote LeopphEngine/vendor/glfw
git submodule update --init --remote LeopphEngine/vendor/gl3w

cd LeopphEngine\vendor\physx
:: call :PhysX
cd ../assimp
call :Assimp
cd ../gl3w
call :Gl3w
cd ../glfw
call :GLFW
cd ../spdlog
call :Spdlog
exit /B 0

:Assimp
cmake -DBUILD_SHARED_LIBS=0 -DASSIMP_NO_EXPORT=1 -DASSIMP_BUILD_ZLIB=1 -DASSIMP_BUILD_ASSIMP_TOOLS=0 -DASSIMP_BUILD_TESTS=0 -DASSIMP_INJECT_DEBUG_POSTFIX=0 -DCMAKE_CONFIGURATION_TYPES="Debug;Release" -DLIBRARY_SUFFIX="" .
start "Building Assimp Debug" "%msbuild%" Assimp.sln /p:Configuration=Debug
start "Building Assimp Release" "%msbuild%" Assimp.sln /p:Configuration=Release
exit /B 0

:GLFW
cmake -DCMAKE_CONFIGURATION_TYPES="Debug;Release" -DGLFW_BUILD_DOCS=0 -DGLFW_BUILD_EXAMPLES=0 -DGLFW_BUILD_TESTS=0 -DGLFW_USE_HYBRID_HPG=1 .
start "Building GLFW Debug" "%msbuild%" GLFW.sln /p:Configuration=Debug
start "Building GLFW Release" "%msbuild%" GLFW.sln /p:Configuration=Release
exit /B 0

:Gl3w
cmake -DCMAKE_CONFIGURATION_TYPES="Debug;Release" .
start "Building Gl3w Debug" "%msbuild%" gl3w.sln /p:Configuration=Debug
start "Building Gl3w Release" "%msbuild%" gl3w.sln /p:Configuration=Release
exit /B 0

:PhysX
cd physx
call generate_projects.bat "vc17win64"
cd compiler/vc17win64
start "Building PhysX Checked" "%msbuild%" PhysXSDK.sln /p:Configuration=checked
start "Building PhysX Profile" "%msbuild%" PhysXSDK.sln /p:Configuration=profile
start "Building PhysX Release" "%msbuild%" PhysXSDK.sln /p:Configuration=release
cd ../../..
exit /B 0

:Spdlog
cmake -DCMAKE_CONFIGURATION_TYPES="Debug;Release" -DSPDLOG_BUILD_EXAMPLE=0 .
start "Building Spdlog Debug" "%msbuild%" spdlog.sln /p:Configuration=Debug
start "Building Spdlog Release" "%msbuild%" spdlog.sln /p:Configuration=Release
exit /B 0