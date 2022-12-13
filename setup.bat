@echo off

rem Clone all submodules
call git submodule update --init

rem Bootstrap vcpkg
call .\vcpkg\bootstrap-vcpkg.bat -disableMetrics

rem Build Mono runtime using MSBuild
setlocal enabledelayedexpansion
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
  call "%%i" Engine/deps/vendor/mono/msvc/mono.sln -p:Platform=x64 -p:Configuration=Debug
  call "%%i" Engine/deps/vendor/mono/msvc/mono.sln -p:Platform=x64 -p:Configuration=Release
  exit /b !errorlevel!
)