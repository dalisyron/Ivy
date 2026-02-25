cmake_minimum_required(VERSION 3.24)

project(IvyExternalConsumer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Ivy REQUIRED CONFIG)

add_executable(ivy_external_consumer main.cpp)
target_link_libraries(ivy_external_consumer PRIVATE Ivy::ivy)
