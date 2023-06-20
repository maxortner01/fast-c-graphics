cmake_minimum_required(VERSION 3.8)

project(fast-cpp-graphics LANGUAGES CXX)

add_compile_definitions(FCG_OPENGL=1 FCG_VULKAN=2)
add_compile_definitions(FCG_RENDER_API_TYPE=2)
set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories( ${CMAKE_SOURCE_DIR}/build/src/fcg)

add_executable(fast-cpp-graphics 
    main.cpp 
    Graphics/Machine.cpp
    Graphics/BufferLayout.cpp
    Graphics/Buffer.cpp)

target_link_libraries(fast-cpp-graphics ctest_graphics)
