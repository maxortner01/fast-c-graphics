cmake_minimum_required(VERSION 3.8)

project(ctest C)

if (NOT DEFINED FCG_RENDER_API)
    message("Render API not specified, defaulting to OpenGL.")
    set(FCG_RENDER_API "Vulkan")
endif()

if (NOT DEFINED CMAKE_BUILD_TYP)
    message("Build type not specified, defaulting to Release.")
    set(CMAKE_BUILD_TYPE "Release")
endif()

# Go ahead and compile SDL3
add_subdirectory(${CMAKE_SOURCE_DIR}/extern/SDL EXCLUDE_FROM_ALL)

# Render API type enums
add_compile_definitions(FCG_OPENGL=1 FCG_VULKAN=2)
set(FCG_LIBRARIES SDL3::SDL3)
set(
    FCG_INCLUDE 
    ${CMAKE_SOURCE_DIR}/include 
    ${CMAKE_SOURCE_DIR}/extern/SDL/include
)

set(
    FCG_SOURCE 
    ${CMAKE_SOURCE_DIR}/src/main.c
    ${CMAKE_SOURCE_DIR}/src/graphics/${FCG_RENDER_API}/gdi.c
    ${CMAKE_SOURCE_DIR}/src/graphics/${FCG_RENDER_API}/device.c
    ${CMAKE_SOURCE_DIR}/src/graphics/${FCG_RENDER_API}/surface.c
    ${CMAKE_SOURCE_DIR}/src/graphics/gdi.c
    ${CMAKE_SOURCE_DIR}/src/graphics/surface.c
    ${CMAKE_SOURCE_DIR}/src/memory/stack.c
)

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
    message("Vulkan chosen as render API.")

    find_package(Vulkan REQUIRED)
    list(APPEND FCG_LIBRARIES ${Vulkan_LIBRARIES})
    list(APPEND FCG_INCLUDE ${Vulkan_INCLUDE_DIR})

    message("Including: ${Vulkan_INCLUDE_DIR}")

    message("Vulkan directory: ${Vulkan_LIBRARIES}")
    add_compile_definitions(FCG_RENDER_API_TYPE=2)
endif()

include_directories(
    ${FCG_INCLUDE}
)

add_executable(
    ctest
    ${FCG_SOURCE}
)

target_link_libraries(ctest PRIVATE ${FCG_LIBRARIES})