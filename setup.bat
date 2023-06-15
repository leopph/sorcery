@echo off

rem Clone all submodules
call git submodule update --init

rem Bootstrap vcpkg
call .\vcpkg\bootstrap-vcpkg.bat -disableMetrics
