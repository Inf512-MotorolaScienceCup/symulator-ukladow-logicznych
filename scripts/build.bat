@echo off
title Build

if exist ..\CMakeLists.txt cd ..
	
mkdir out
cd out
cmake ..
cmake --build .

pause

:: 32bit build
:: cmake -G "Visual Studio 16 2019" -A Win32 -S . -B out\build32
:: cmake --build out\build32 --config Debug

:: 64bit build
::cmake -G "Visual Studio 16 2019" -A x64 -S . -B out\build64
::cmake --build out\build64 --config Debug
