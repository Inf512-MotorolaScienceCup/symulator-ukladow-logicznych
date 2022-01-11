cmake_minimum_required(VERSION 3.13)    # CMake version check

project(SUL)   # Create project "Symulator ukladow logicznych"

# check if raylib is installed
find_package(raylib 3.0 QUIET)
if (NOT raylib_FOUND)
  include(FetchContent)

  FetchContent_Declare(
     raylib
     GIT_REPOSITORY https://github.com/raysan5/raylib.git
     GIT_TAG 3.0.0
  )

  FetchContent_GetProperties(raylib)
  if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(raylib)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
    add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
  endif()
endif()

add_subdirectory(src)
target_include_directories(${PROJECT_NAME} PUBLIC include)


#add_subdirectory(external)
