cmake_minimum_required(VERSION 3.5)
project(AdvancedWebserver-Server)

file(GLOB SOURCES "*.cpp")
add_executable(AdvancedWebserver ${SOURCES})
target_include_directories(AdvancedWebserver PRIVATE ../../include/ ../../external/GArgs/include/ ../../external/GNetworking/include/)
target_link_libraries(AdvancedWebserver PRIVATE GArgs GNetworking GParsing-HTTP)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
  target_compile_options(AdvancedWebserver PRIVATE -fsanitize=address -fno-omit-frame-pointer)
  target_link_options(AdvancedWebserver PRIVATE -fsanitize=address -fno-omit-frame-pointer)
endif()
