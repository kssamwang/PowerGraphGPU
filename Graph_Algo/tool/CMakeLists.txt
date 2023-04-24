cmake_minimum_required(VERSION 3.9)
project(Graph_Algo)

set(CMAKE_CXX_STANDARD 14)

add_executable(tool_RandGraphGen
        RandGraphGen.cpp)

add_executable(tool_WRNGraphProcessing
        WRNGraphProcessing.cpp)

add_executable(tool_TwitterGraphProcessing
        TwitterGraphProcessing.cpp)

add_executable(tool_UK2007GraphProcessing
        UK2007GraphProcessing.cpp)
