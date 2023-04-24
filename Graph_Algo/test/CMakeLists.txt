cmake_minimum_required(VERSION 3.9)
project(Graph_Algo)

set(CMAKE_CXX_STANDARD 14)

find_package(CUDA)

add_executable(core_GraphUtilTest
        GraphUtilTest.cpp)

target_link_libraries(core_GraphUtilTest
        core_Graph
        core_GraphUtil
        algo_BellmanFord)

add_executable(algo_BellmanFordTest
        BellmanFordTest.cpp)

target_link_libraries(algo_BellmanFordTest
        algo_BellmanFord
        core_Graph
        core_MessageSet
        core_GraphUtil)
		
add_executable(algo_LabelPropagationTest
		LabelPropagationTest.cpp)

target_link_libraries(algo_LabelPropagationTest
        algo_LabelPropagation
        core_Graph
        core_MessageSet
        core_GraphUtil)

add_executable(algo_ConnectedComponentTest
        ConnectedComponentTest.cpp)

target_link_libraries(algo_ConnectedComponentTest
        algo_ConnectedComponent
        core_Graph
        core_MessageSet
        core_GraphUtil)

add_executable(algo_DDFSTest
        DDFSTest.cpp)

target_link_libraries(algo_DDFSTest
        algo_DDFS
        core_Graph
        core_MessageSet
        core_GraphUtil)

add_executable(core_GraphAccessTest
        GraphAccessTest.cpp)

target_link_libraries(core_GraphAccessTest
        core_Graph
        core_MessageSet
        core_GraphUtil)

add_executable(algo_PageRankTest
        PageRankTest.cpp)

target_link_libraries(algo_PageRankTest
        algo_PageRank
        core_Graph
        core_MessageSet
        core_GraphUtil)

if(CUDA_FOUND)
    include_directories("${CUDA_INCLUDE_DIRS}")

    add_executable(algo_BellmanFordGPUTest
            BellmanFordGPUTest.cpp)

    target_link_libraries(algo_BellmanFordGPUTest
            algo_BellmanFordGPU
            core_Graph
            core_MessageSet
            core_GraphUtil)

    add_executable(algo_PageRankGPUTest
            PageRankGPUTest.cpp)

    target_link_libraries(algo_PageRankGPUTest
            algo_PageRankGPU
            core_Graph
            core_MessageSet
            core_GraphUtil)
        
    add_executable(algo_LabelPropagationGPUTest
            LabelPropagationGPUTest.cpp)

    target_link_libraries(algo_LabelPropagationGPUTest
            algo_LabelPropagationGPU
            core_Graph
            core_MessageSet
            core_GraphUtil)

    add_executable(algo_ConnectedComponentGPUTest
            ConnectedComponentGPUTest.cpp)

    target_link_libraries(algo_ConnectedComponentGPUTest
            algo_ConnectedComponentGPU
            core_Graph
            core_MessageSet
            core_GraphUtil)
endif(CUDA_FOUND)

if(UNIX)
    add_executable(srv_Test_srv
            SRVTest_srv.cpp)

    target_link_libraries(srv_Test_srv
            srv_UNIX_shm
            srv_UNIX_msg
            srv_UtilServer
            srv_UtilClient)

    add_executable(srv_Test_cli
            SRVTest_cli.cpp)

    target_link_libraries(srv_Test_cli
            srv_UNIX_shm
            srv_UNIX_msg
            srv_UtilServer
            srv_UtilClient)

    IF(CUDA_FOUND)
        add_executable(srv_UtilServerTest_BellmanFordGPU
                UtilServerTest_BellmanFordGPU.cpp)

        target_link_libraries(srv_UtilServerTest_BellmanFordGPU
                srv_UNIX_shm
                srv_UNIX_msg
                srv_UtilServer
                algo_BellmanFord
                algo_BellmanFordGPU)

        add_executable(srv_UtilServerTest_PageRankGPU
                UtilServerTest_PageRankGPU.cpp)

        target_link_libraries(srv_UtilServerTest_PageRankGPU
                srv_UNIX_shm
                srv_UNIX_msg
                srv_UtilServer
                algo_PageRank
                algo_PageRankGPU)

        add_executable(srv_UtilServerTest_LabelPropagationGPU
                UtilServerTest_LabelPropagationGPU.cpp)

        target_link_libraries(srv_UtilServerTest_LabelPropagationGPU
                srv_UNIX_shm
                srv_UNIX_msg
                srv_UtilServer
                algo_LabelPropagation
                algo_LabelPropagationGPU)

        add_executable(srv_UtilServerTest_ConnectedComponentGPU
                UtilServerTest_ConnectedComponentGPU.cpp)

        target_link_libraries(srv_UtilServerTest_ConnectedComponentGPU
                srv_UNIX_shm
                srv_UNIX_msg
                srv_UtilServer
                algo_ConnectedComponent
                algo_ConnectedComponentGPU)
    ENDIF(CUDA_FOUND)

    add_executable(srv_UtilServerTest_BellmanFord
            UtilServerTest_BellmanFord.cpp)

    target_link_libraries(srv_UtilServerTest_BellmanFord
            srv_UNIX_shm
            srv_UNIX_msg
            srv_UtilServer
            algo_BellmanFord)

    add_executable(srv_UtilServerTest_LabelPropagation
            UtilServerTest_LabelPropagation.cpp)

    target_link_libraries(srv_UtilServerTest_LabelPropagation
            srv_UNIX_shm
            srv_UNIX_msg
            srv_UtilServer
            algo_LabelPropagation)

    add_executable(srv_UtilServerTest_PageRank
            UtilServerTest_PageRank.cpp)

    target_link_libraries(srv_UtilServerTest_PageRank
            srv_UNIX_shm
            srv_UNIX_msg
            srv_UtilServer
            algo_PageRank)

    add_executable(srv_UtilServerTest_ConnectedComponent
            UtilServerTest_ConnectedComponent.cpp)

    target_link_libraries(srv_UtilServerTest_ConnectedComponent
            srv_UNIX_shm
            srv_UNIX_msg
            srv_UtilServer
            algo_ConnectedComponent)

    add_executable(srv_UtilServerTest_JumpIteration
            UtilServerTest_JumpIteration.cpp)

    target_link_libraries(srv_UtilServerTest_JumpIteration
            srv_UNIX_shm
            srv_UNIX_msg
            srv_UtilServer
            algo_JumpIteration)

    add_executable(srv_UtilClientTest
            UtilClientTest.cpp)

    target_link_libraries(srv_UtilClientTest
            pthread
            srv_UNIX_shm
            srv_UNIX_msg
            srv_UtilClient
            algo_BellmanFord)

    add_executable(srv_UtilClientTest_BFOptimize
            UtilClientTest_BFOptimize.cpp)

    target_link_libraries(srv_UtilClientTest_BFOptimize
            pthread
            srv_UNIX_shm
            srv_UNIX_msg
            srv_UtilClient
            algo_BellmanFord)

    add_executable(srv_UtilClientTest_BFOptimizePartition
            UtilClientTest_BFOptimizePartition.cpp)

    target_link_libraries(srv_UtilClientTest_BFOptimizePartition
            pthread
            srv_UNIX_shm
            srv_UNIX_msg
            srv_UtilClient
            algo_BellmanFord)

    add_executable(srv_UtilClientTest_BFJ
            UtilClientTest_BFJump.cpp)

    target_link_libraries(srv_UtilClientTest_BFJ
            pthread
            srv_UNIX_shm
            srv_UNIX_msg
            srv_UtilClient
            algo_BellmanFord)

    add_executable(srv_UtilClientTest_LabelPropagation
            UtilClientTest_LabelPropagation.cpp)

    target_link_libraries(srv_UtilClientTest_LabelPropagation
            pthread
            srv_UNIX_shm
            srv_UNIX_msg
            srv_UtilClient
            algo_LabelPropagation)

    add_executable(srv_UtilClientTest_LabelPropagationPartition
            UtilClientTest_LabelPropagationPartition.cpp)

    target_link_libraries(srv_UtilClientTest_LabelPropagationPartition
            pthread
            srv_UNIX_shm
            srv_UNIX_msg
            srv_UtilClient
            algo_LabelPropagation)

    add_executable(srv_UtilClientTest_PageRank
            UtilClientTest_PageRank.cpp)

    target_link_libraries(srv_UtilClientTest_PageRank
            pthread
            srv_UNIX_shm
            srv_UNIX_msg
            srv_UtilClient
            algo_PageRank)

    add_executable(srv_UtilClientTest_JumpIteration
            UtilClientTest_JumpIteration.cpp)

    target_link_libraries(srv_UtilClientTest_JumpIteration
            pthread
            srv_UNIX_shm
            srv_UNIX_msg
            srv_UtilClient
            algo_JumpIteration)

ENDIF(UNIX)
