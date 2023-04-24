cmake_minimum_required(VERSION 3.9)
project(Graph_Algo)

set(CMAKE_CXX_STANDARD 14)

find_package(CUDA)

add_library(algo_BellmanFord
        BellmanFord.h
        BellmanFord.cpp
        BellmanFord_impl.cpp)

target_link_libraries(algo_BellmanFord
        core_Graph
        core_GraphUtil
        core_MessageSet)

if(CUDA_FOUND)
    CUDA_SELECT_NVCC_ARCH_FLAGS(ARCH_FLAGS Auto)
    set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS}; ${ARCH_FLAGS}; -std=c++14;)

    cuda_add_library(algo_BellmanFordGPU_kernel STATIC
            kernel_src/BellmanFordGPU_kernel.h
            kernel_src/BellmanFordGPU_kernel.cu)

    add_library(algo_BellmanFordGPU
            BellmanFordGPU.h
            BellmanFordGPU.cpp
            BellmanFordGPU_impl.cpp)

    target_link_libraries(algo_BellmanFordGPU
            algo_BellmanFord
            algo_BellmanFordGPU_kernel)

endif(CUDA_FOUND)