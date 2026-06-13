cmake_minimum_required(VERSION 4.0)
cmake_policy(SET CMP0048 NEW)

project(TCScheduler VERSION 0.0.1)
set(PLATFORM WIN32) # Comment this line to show the console window

if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

message(STATUS "Preparing solution with CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(
    THIRD_PARTY_SRC_FILES
    ${IMGUI_SRC_FILES}
    src/thirdparty/json11.cpp 
    src/thirdparty/PlatformExtensions.cpp 
    src/thirdparty/RendererExtensions.cpp 
    src/thirdparty/RmlUi_Backend_GLFW_GL3.cpp 
    src/thirdparty/RmlUi_Platform_GLFW.cpp 
    src/thirdparty/RmlUi_Renderer_GL3.cpp 
    src/thirdparty/Shell.cpp 
    src/thirdparty/ShellFileInterface.cpp
)

set( 
    LIBRARIES
    ${CMAKE_SOURCE_DIR}/lib/glfw3dll.lib
    ${CMAKE_SOURCE_DIR}/lib/rmlui_debugger.lib
    ${CMAKE_SOURCE_DIR}/lib/rmlui.lib
    ${CMAKE_SOURCE_DIR}/lib/freetyped.lib
    shlwapi
)

set(APP TCScheduler)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

file(
    GLOB_RECURSE
    SRC_FILES
    ${SRC_DIR}/*
)

add_executable(
    ${APP}
    ${PLATFORM}
    ${SRC_FILES}
    ${THIRD_PARTY_SRC_FILES}
)

target_include_directories(
    ${APP}
    PUBLIC
    include/RmlUi
    include/GLFW
    include
)

set_property(TARGET ${APP} PROPERTY CXX_STANDARD 20)

target_link_libraries(
    ${APP}
    ${LIBRARIES}
)

install(
    TARGETS ${APP}
    DESTINATION .
)

install(
    DIRECTORY ${SRC_DIR}/dlls/
    DESTINATION .
    FILES_MATCHING PATTERN "*.dll"
)
