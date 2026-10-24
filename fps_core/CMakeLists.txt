cmake_minimum_required(VERSION 3.20)
project(CompetitiveFPSCore VERSION 0.6.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /O2 /fp:fast /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O3 -march=native)
endif()

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(fps_core
    main.cpp
    Engine.cpp
    Movement.cpp
)

target_compile_definitions(fps_core PRIVATE
    SDL_MAIN_HANDLED
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    _CRT_SECURE_NO_WARNINGS
)

if(TARGET SDL2::SDL2)
    target_link_libraries(fps_core PRIVATE SDL2::SDL2 OpenGL::GL)
    get_target_property(SDL2_INC SDL2::SDL2 INTERFACE_INCLUDE_DIRECTORIES)
    if(SDL2_INC)
        target_include_directories(fps_core PRIVATE ${SDL2_INC} "${SDL2_INC}/..")
    endif()
else()
    target_include_directories(fps_core PRIVATE ${SDL2_INCLUDE_DIRS} "${SDL2_INCLUDE_DIRS}/..")
    target_link_libraries(fps_core PRIVATE ${SDL2_LIBRARIES} OpenGL::GL)
endif()

if(WIN32)
    target_link_libraries(fps_core PRIVATE ws2_32)
endif()