cmake_minimum_required (VERSION 3.13)

target_include_directories(${PROJECT_NAME} PUBLIC raylib/include)

set(raylib ${CMAKE_SOURCE_DIR}/lib/raylib/lib/raylib.lib)
target_link_libraries(${PROJECT_NAME} PUBLIC ${raylib} winmm)
