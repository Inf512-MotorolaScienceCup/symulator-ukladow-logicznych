cmake_minimum_required(VERSION 3.13)    # CMake version check

project(SUL)   

# exe output path (change DEBUG to RELEASE when release version):
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}) 

add_subdirectory(src)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_subdirectory(lib)
