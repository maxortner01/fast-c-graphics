cmake_minimum_required(VERSION 3.8)

project(ctest LANGUAGES C CXX)
set (CMAKE_CXX_STANDARD 14) # For VMA

if (NOT DEFINED FCG_RENDER_API)
    message("Render API not specified, defaulting to OpenGL.")
    set(FCG_RENDER_API "Vulkan")
endif()

if (NOT DEFINED CMAKE_BUILD_TYPE)
    message("Build type not specified, defaulting to Release.")
    set(CMAKE_BUILD_TYPE "Release")
endif()

# Go ahead and compile SDL3
set(SDL_STATIC ON CACHE BOOL "enable sdl static" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/extern/SDL build EXCLUDE_FROM_ALL)

# Render API type enums
add_compile_definitions(FCG_OPENGL=1 FCG_VULKAN=2 FCG_BUILD)
set(FCG_LIBRARIES SDL3::SDL3-static)
set(
    FCG_INCLUDE 
    ${CMAKE_SOURCE_DIR}/include 
    ${CMAKE_SOURCE_DIR}/extern/SDL/include
)

file(GLOB RENDER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/graphics/${FCG_RENDER_API}/*.c")
file(GLOB RENDER_CPP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/graphics/${FCG_RENDER_API}/*.cpp")

set(
    FCG_SOURCE 
    ${RENDER_SOURCES}
    ${RENDER_CPP_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/graphics/gdi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/graphics/surface.c
    ${CMAKE_CURRENT_SOURCE_DIR}/graphics/device.c
    ${CMAKE_CURRENT_SOURCE_DIR}/graphics/pipeline.c
    ${CMAKE_CURRENT_SOURCE_DIR}/memory/stack.c
    ${CMAKE_CURRENT_SOURCE_DIR}/memory/queue.c
)

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    message("In debug mode.")
endif()
add_compile_definitions(FCG_DEBUG)

# If OpenGL is selected, search for it and add it to the list
if (${FCG_RENDER_API} MATCHES "OpenGL")
    message("OpenGL chosen as render API.")

    find_package(OpenGL REQUIRED)
    list(APPEND FCG_LIBRARIES ${OPENGL_LIBRARIES})
    list(APPEND FCG_INCLUDE ${CMAKE_SOURCE_DIR}/extern/glew-2.1.0/include)
    list(APPEND FCG_SOURCE  ${CMAKE_SOURCE_DIR}/extern/glew-2.1.0/src/glew.c)

    message("OpenGl directory: ${OPENGL_LIBRARIES}")
    add_compile_definitions(FCG_RENDER_API_TYPE=1)

# Otherwise, do the same for vulkan
elseif(${FCG_RENDER_API} MATCHES "Vulkan")

    # Since the implementation of VMA is c++, we need to precompile it
    # Here we would build it

    message("Vulkan chosen as render API.")

    find_package(Vulkan REQUIRED)
    message(${Vulkan_LIBRARIES})
    list(APPEND FCG_LIBRARIES  
        ${Vulkan_LIBRARIES})
    
    list(APPEND FCG_INCLUDE 
        ${Vulkan_INCLUDE_DIR} 
        ${CMAKE_SOURCE_DIR}/extern/VMA/include)

    message("Including: ${Vulkan_INCLUDE_DIR}")

    # Grab the right type of shaderc depending upon the version
    if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
        set(SHADERC_LIB shaderc_combinedd)
    elseif (${CMAKE_BUILD_TYPE} MATCHES "Release")
        set(SHADERC_LIB shaderc_combined)
    endif()

    # If there is no debug compiled .a, then just use the release (might yield errors)
    find_library(SHADERC ${SHADERC_LIB} HINTS ${Vulkan_LIBRARY_DIR})
    if (${SHADERC} MATCHES "SHADERC-NOTFOUND")
        find_library(SHADERC shaderc_combined HINTS ${Vulkan_LIBRARY_DIR})
    endif()

    if (NOT ${SHADERC} MATCHES "SHADERC-NOTFOUND")
        message("Using shaderc")
        list(APPEND FCG_LIBRARIES ${SHADERC})
        add_compile_definitions(USE_SHADERC)
    endif()

    message("Vulkan directory: ${Vulkan_LIBRARIES}")
    add_compile_definitions(FCG_RENDER_API_TYPE=2)
endif()

include_directories(
    ${FCG_INCLUDE}
)

add_executable(
    ctest
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
    ${FCG_SOURCE}
)

add_library(
    ctest_graphics
    SHARED
    ${FCG_SOURCE}
)

add_library(
    ctest_graphics_static
    STATIC
    ${FCG_SOURCE}
)

target_link_libraries(ctest                 PRIVATE ${FCG_LIBRARIES})
target_link_libraries(ctest_graphics        PRIVATE ${FCG_LIBRARIES})
target_link_libraries(ctest_graphics_static PRIVATE ${FCG_LIBRARIES})