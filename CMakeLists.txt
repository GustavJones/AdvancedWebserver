cmake_minimum_required(VERSION 3.5)
project(AdvancedWebserver)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

add_subdirectory(external/GNetworking)
add_subdirectory(external/GArgs)
add_subdirectory(external/GParsing)

add_subdirectory(src)
