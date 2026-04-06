cmake_minimum_required(VERSION 3.10)

project(tanks)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/server ${CMAKE_CURRENT_BINARY_DIR}/server)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/client ${CMAKE_CURRENT_BINARY_DIR}/client)

add_executable(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    server
    client
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/inc
)

