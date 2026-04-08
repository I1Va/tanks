cmake_minimum_required(VERSION 3.10)
project(npc)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(${PROJECT_NAME} STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/script1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/script2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/script3.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    server
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${PROJECT_SOURCE_DIR}/inc
    ${PROJECT_SOURCE_DIR}/../
)

