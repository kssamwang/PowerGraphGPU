cmake_minimum_required(VERSION 3.9)
project(Graph_Algo)

set(CMAKE_CXX_STANDARD 14)

add_library(algo_JumpIteration
        JumpIteration.h
        JumpIteration.cpp
        JumpIteration_impl.cpp)

target_link_libraries(algo_JumpIteration
        core_Graph
        core_GraphUtil
        core_MessageSet)