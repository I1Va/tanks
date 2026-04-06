cmake_minimum_required(VERSION 3.10)
project(client)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_library(SDL2_GFX_LIB SDL2_gfx)
find_package(SDL2_ttf REQUIRED)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE
    ${PROJECT_SOURCE_DIR}/inc
    ${PROJECT_SOURCE_DIR}/../
)

target_link_libraries(${PROJECT_NAME} INTERFACE
    SDL2::SDL2
    SDL2_image::SDL2_image
    ${SDL2_GFX_LIB}
    SDL2_ttf::SDL2_ttf
)


