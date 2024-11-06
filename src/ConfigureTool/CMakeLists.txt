cmake_minimum_required(VERSION 3.5)
project(AdvancedWebserver-Configure-Tool)

file(GLOB SOURCES "*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ../../include/ ../../external/GArgs/include/)
target_link_libraries(${PROJECT_NAME} PRIVATE GArgs AdvancedWebserver-Configuration)

install(TARGETS ${PROJECT_NAME})
