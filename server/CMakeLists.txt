cmake_minimum_required(VERSION 3.10)
project(server)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# add_executable(${PROJECT_NAME}
#     ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
# )

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE
    ${PROJECT_SOURCE_DIR}/inc
)

