cmake_minimum_required (VERSION 3.8)

project(Symulator)

include_directories(raylib/include)

link_directories(raylib/lib)

add_executable (Symulator "Symulator.cpp" "Symulator.h")
target_link_libraries(Symulator raylib winmm)
