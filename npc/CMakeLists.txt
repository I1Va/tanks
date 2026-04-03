# cmake_minimum_required(VERSION 3.10)
# project(lldoter)
# set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/graph_builder ${CMAKE_CURRENT_BINARY_DIR}/graph_builder)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/serializer    ${CMAKE_CURRENT_BINARY_DIR}/serializer)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/dynamic_info  ${CMAKE_CURRENT_BINARY_DIR}/dynamic_info)

# add_executable(${PROJECT_NAME}
#     ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/cluster_dump.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/node_dump.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/edge_dump.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/dot_graph.cpp
# )

# target_link_libraries(${PROJECT_NAME} PRIVATE
#     graph_builder
#     serializer
# )

# target_include_directories(${PROJECT_NAME}
#     PUBLIC
#     ${LLVM_INCLUDE_DIR}
#     ${PROJECT_SOURCE_DIR}/inc
# )

