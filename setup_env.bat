set msbuild=C:\Program Files\Microsoft Visual Studio\2022\Community\Msbuild\Current\Bin\MSBuild.exe

git submodule update --init --remote
cd LeopphEngine\vendor\physx
call :PhysX
cd ../assimp
call :Assimp
cd ../glad
call :GLAD
cd ../glfw
call :GLFW
cd ../spdlog
call :Spdlog
exit /B 0

:Assimp
cmake -DASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT=0 -DASSIMP_BUILD_ASSIMP_TOOLS=0 -DASSIMP_BUILD_TESTS=0 -DASSIMP_INJECT_DEBUG_POSTFIX=0 -DCMAKE_CONFIGURATION_TYPES="Debug;Release" -DLIBRARY_SUFFIX="" .
start "Building Assimp Debug" "%msbuild%" Assimp.sln /p:Configuration=Debug
start "Building Assimp Release" "%msbuild%" Assimp.sln /p:Configuration=Release
exit /B 0

:GLAD
cmake -DCMAKE_CONFIGURATION_TYPES="Debug;Release" -DGLAD_PROFILE="core" .
start "Building GLAD Debug" "%msbuild%" GLAD.sln /p:Configuration=Debug
start "Building GLAD Release" "%msbuild%" GLAD.sln /p:Configuration=Release
exit /B 0

:GLFW
cmake -DCMAKE_CONFIGURATION_TYPES="Debug;Release" -DGLFW_BUILD_DOCS=0 -DGLFW_BUILD_EXAMPLES=0 -DGLFW_BUILD_TESTS=0 -DGLFW_USE_HYBRID_HPG=1 .
start "Building GLFW Debug" "%msbuild%" GLFW.sln /p:Configuration=Debug
start "Building GLFW Release" "%msbuild%" GLFW.sln /p:Configuration=Release
exit /B 0

:PhysX
cd physx
call generate_projects.bat "vc16win64"
cd compiler/vc16win64
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