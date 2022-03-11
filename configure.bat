@echo off
REM Invoke conan
conan install . --build missing -s build_type=Debug
REM Run premake
premake5 vs2022
