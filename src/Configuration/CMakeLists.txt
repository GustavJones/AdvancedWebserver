cmake_minimum_required(VERSION 3.5)
project(AdvancedWebserver-Configuration)

file(GLOB SOURCES "*.cpp")
add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ../../include/)
