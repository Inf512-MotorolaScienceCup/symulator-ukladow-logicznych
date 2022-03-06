cmake_minimum_required (VERSION 3.8)

project(Symulator)

if (MSVC)
   add_compile_options(/MT)
endif()

include_directories(raylib/include)
link_directories(raylib/lib)

set(game_resources)
file(GLOB resources "./res/*")
list(APPEND game_resources ${resources})

add_executable (Symulator "Symulator.cpp" "Symulator.h")
target_link_libraries(Symulator raylib winmm)

file(COPY ${game_resources} DESTINATION "res/")
make_directory("projects")
