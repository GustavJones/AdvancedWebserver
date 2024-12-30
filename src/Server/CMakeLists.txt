cmake_minimum_required(VERSION 3.15)
project(AdvancedWebserver-Server)

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

file(GLOB SOURCES "*.cpp")
add_executable(AdvancedWebserver ${SOURCES})
set_target_properties(AdvancedWebserver PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
target_include_directories(AdvancedWebserver PRIVATE ../../include/ ../../external/GArgs/include/ ../../external/GNetworking/include/ ../../external/GParsing/include/)
target_link_libraries(AdvancedWebserver PRIVATE GArgs GNetworking GParsing-HTTP OpenSSL::SSL AdvancedWebserver-Configuration)

install(TARGETS AdvancedWebserver)
