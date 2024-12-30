cmake_minimum_required(VERSION 3.15)
project(AdvancedWebserver-Configuration)

file(GLOB SOURCES "*.cpp")
file(REAL_PATH "../../include/Configuration/" HEADERS_DIR)
file(GLOB HEADERS "${HEADERS_DIR}/*.h")
add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_compile_definitions(${PROJECT_NAME} PRIVATE -DBUILDING_ADVANCEDWEBSERVER_CONFIGURATION)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${HEADERS}")

target_include_directories(${PROJECT_NAME} PRIVATE ../../include/)
install(TARGETS ${PROJECT_NAME} PUBLIC_HEADER DESTINATION include/Configuration)
