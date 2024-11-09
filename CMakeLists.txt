cmake_minimum_required(VERSION 3.5)
project(AdvancedWebserver)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries or static ones")
set(ENABLE_WARNINGS_ON_DEBUG_BUILD OFF)

if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND ENABLE_WARNINGS_ON_DEBUG_BUILD)
  # add_compile_options(-Wall -Wextra -Wpedantic -Werror)
  add_compile_options(-Wall)
endif()

add_subdirectory(external/GNetworking)
add_subdirectory(external/GArgs)
add_subdirectory(external/GParsing)

add_subdirectory(src)
