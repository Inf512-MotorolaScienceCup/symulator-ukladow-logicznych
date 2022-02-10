:: 32bit build
cmake -G "Visual Studio 17 2022" -A Win32 -S . -B out\build32
cmake --build out\build32 --config Debug

:: 64bit build
::cmake -G "Visual Studio 17 2022" -A x64 -S . -B out\build64
::cmake --build out\build64 --config Debug
