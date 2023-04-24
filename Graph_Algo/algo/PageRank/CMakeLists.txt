cmake_minimum_required(VERSION 3.9)
project(Graph_Algo)

set(CMAKE_CXX_STANDARD 14)

find_package(CUDA)

add_library(algo_PageRank
        PageRank.h
        PageRank.cpp
        PageRank_impl.cpp)

target_link_libraries(algo_PageRank
        core_Graph
        core_GraphUtil
        core_MessageSet)

if(CUDA_FOUND)
        CUDA_SELECT_NVCC_ARCH_FLAGS(ARCH_FLAGS Auto)
        set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS}; ${ARCH_FLAGS}; -std=c++14;)
    
        cuda_add_library(algo_PageRankGPU_kernel STATIC
                kernel_src/PageRankGPU_kernel.h
                kernel_src/PageRankGPU_kernel.cu)
    
        add_library(algo_PageRankGPU
                PageRankGPU.h
                PageRankGPU.cpp
                PageRankGPU_impl.cpp)
    
        target_link_libraries(algo_PageRankGPU
                algo_PageRank
                algo_PageRankGPU_kernel)
    
 endif(CUDA_FOUND)