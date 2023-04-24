//
// Created by Thoh Testarossa on 2019-04-06.
//

#include "../core/Graph.h"
#include "../core/GraphUtil.h"
#include "../srv/UtilClient.h"
#include "../srv/UNIX_shm.h"
#include "../srv/UNIX_msg.h"

#include <iostream>
#include <fstream>
#include <vector>

#include <future>
#include <cstring>

int optimize = 0;

template <typename VertexValueType, typename MessageValueType>
void testFut(UtilClient<VertexValueType, MessageValueType> *uc, VertexValueType *vValues, Vertex *vSet, int *avSet, int avCount)
{
    uc->connect();
    if(optimize)
        uc->update(vValues, vSet, avSet, avCount);
    else
        uc->update(vValues, vSet);
    uc->requestMSGMerge();
    uc->requestMSGApply();
    uc->disconnect();
}

int main(int argc, char *argv[])
{
    if(argc != 4 && argc != 5 && argc != 6)
    {
        std::cout << "Usage:" << std::endl << "./UtilClientTest_BellmanFord vCount eCount numOfInitV [nodeCount] [optimize]" << std::endl;
        return 1;
    }

    int vCount = atoi(argv[1]);
    int eCount = atoi(argv[2]);
    int numOfInitV = atoi(argv[3]);
    int nodeCount = atoi(argv[4]);
    optimize = atoi(argv[5]);


    //Parameter check
    if(vCount <= 0 || eCount <= 0 || numOfInitV <= 0 || nodeCount <= 0)
    {
        std::cout << "Parameter illegal" << std::endl;
        return 3;
    }

    int *initVSet = new int [numOfInitV];
    bool *filteredV = new bool [vCount];
    int *timestamp = new int [vCount];
    double *vValues = new double [vCount * numOfInitV];

    std::vector<Vertex> vSet = std::vector<Vertex>();

    for(int i = 0; i < vCount; i++) filteredV[i] = false;
    for(int i = 0; i < vCount; i++) timestamp[i] = -1;
    for(int i = 0; i < vCount; i++) vSet.emplace_back(i, true, -1);

    for(int i = 0; i < vCount * numOfInitV; i++) vValues[i] = INT32_MAX  >> 1;

    //Easy init
    initVSet[0] = 828192;
    initVSet[1] = 9808777;
    initVSet[2] = 13425140;
    initVSet[3] = 22675645;
    for(int i = 0; i < numOfInitV; i++) vValues[initVSet[i] * numOfInitV + i] = 0;

    for(int i = 0; i < numOfInitV; i++)
    {
        vSet.at(initVSet[i]).initVIndex = i;
        vSet.at(initVSet[i]).isActive = true;
    }

    std::vector<std::vector<Edge>> eSets;
    eSets.reserve(4);
    eSets.assign(4, std::vector<Edge>());

    int eCounts[4];

    //pid0
    std::cout << "../../data/wrn/pid0.txt" << std::endl;
    std::ifstream Gin("../../data/wrn/pid0.txt");
    if(!Gin.is_open())
    {
        std::cout << "Error! File testGraph.txt not found!" << std::endl;
        return 4;
    }

    int tmp;
    Gin >> tmp;
    if(vCount != tmp)
    {
        std::cout << "Graph file doesn't match up UtilClient's parameter" << std::endl;
        return 5;
    }

    Gin >> eCounts[0];

    for(int i = 0; i < eCounts[0]; i++)
    {
        int src, dst;
        double weight;
        Gin >> src >> dst >> weight;
        eSets.at(0).emplace_back(src, dst, weight);
    }
    Gin.close();

    //pid1
    std::cout << "../../data/wrn/pid1.txt" << std::endl;
    std::ifstream Gin1("../../data/wrn/pid1.txt");
    if(!Gin1.is_open())
    {
        std::cout << "Error! File testGraph.txt not found!" << std::endl;
        return 4;
    }

    Gin1 >> tmp;
    if(vCount != tmp)
    {
        std::cout << "Graph file doesn't match up UtilClient's parameter" << std::endl;
        return 5;
    }

    Gin1 >> eCounts[1];

    for(int i = 0; i < eCounts[1]; i++)
    {
        int src, dst;
        double weight;
        Gin1 >> src >> dst >> weight;
        eSets.at(1).emplace_back(src, dst, weight);
    }
    Gin1.close();

    //pid2
    std::cout << "../../data/wrn/pid2.txt" << std::endl;
    std::ifstream Gin2("../../data/wrn/pid2.txt");
    if(!Gin2.is_open())
    {
        std::cout << "Error! File testGraph.txt not found!" << std::endl;
        return 4;
    }

    Gin2 >> tmp;
    if(vCount != tmp)
    {
        std::cout << "Graph file doesn't match up UtilClient's parameter" << std::endl;
        return 5;
    }

    Gin2 >> eCounts[2];

    for(int i = 0; i < eCounts[2]; i++)
    {
        int src, dst;
        double weight;
        Gin2 >> src >> dst >> weight;
        eSets.at(2).emplace_back(src, dst, weight);
    }
    Gin2.close();

    //pid3
    std::cout << "../../data/wrn/pid3.txt" << std::endl;
    std::ifstream Gin3("../../data/wrn/pid3.txt");
    if(!Gin3.is_open())
    {
        std::cout << "Error! File testGraph.txt not found!" << std::endl;
        return 4;
    }

    Gin3 >> tmp;
    if(vCount != tmp)
    {
        std::cout << "Graph file doesn't match up UtilClient's parameter" << std::endl;
        return 5;
    }

    Gin3 >> eCounts[3];

    for(int i = 0; i < eCounts[3]; i++)
    {
        int src, dst;
        double weight;
        Gin3 >> src >> dst >> weight;
        eSets.at(3).emplace_back(src, dst, weight);
    }
    Gin3.close();


    //Client Init Data Transfer
    auto clientVec = std::vector<UtilClient<double, double>>();
    for(int i = 0; i < nodeCount; i++)
        clientVec.push_back(UtilClient<double, double>(vCount, eCounts[i], numOfInitV, i));
    int chk = 0;
    for(int i = 0; i < nodeCount && chk != -1; i++)
    {
        chk = clientVec.at(i).connect();
        if (chk == -1)
        {
            std::cout << "Cannot establish the connection with server correctly" << std::endl;
            return 2;
        }

        chk = clientVec.at(i).transfer(vValues, &vSet[0], &eSets.at(i)[0], initVSet, filteredV, timestamp);

        if(chk == -1)
        {
            std::cout << "Parameter illegal" << std::endl;
            return 3;
        }

        clientVec.at(i).disconnect();
    }

    int avCount = 0;
    std::vector<int> avSet;
    avSet.reserve(vCount);
    avSet.assign(vCount, 0);

    bool isActive = false;
    for(int i = 0; i < vCount; i++)
    {
        isActive |= vSet[i].isActive;
        if(vSet[i].isActive)
        {
            avSet.at(avCount) = i;
            avCount++;
        }
    }

    bool *ret_AVCheckSet = new bool [vCount];
    int iterCount = 0;

    //Test
    std::cout << "Init finished" << std::endl;
    //Test end

    auto start = std::chrono::system_clock::now();
    while(isActive)
    {
        //Test
        std::cout << "Processing at iter " << ++iterCount << std::endl;
        //Test end

        for(int i = 0; i < vCount; i++) ret_AVCheckSet[i] = false;

        auto futList = new std::future<void> [nodeCount];
        for(int i = 0; i < nodeCount; i++)
        {
            std::future<void> tmpFut = std::async(testFut<double, double>, &clientVec.at(i), vValues, &vSet[0], &avSet[0], avCount);
            futList[i] = std::move(tmpFut);
        }

        for(int i = 0; i < nodeCount; i++)
            futList[i].get();

        //Retrieve data
        for(int i = 0; i < nodeCount; i++)
        {
            clientVec.at(i).connect();

            //Collect data
            for(int j = 0; j < vCount * numOfInitV; j++)
            {
                if (clientVec.at(i).vValues[j] < vValues[j])
                    vValues[j] = clientVec.at(i).vValues[j];
            }

            for(int j = 0; j < vCount; j++)
                ret_AVCheckSet[j] |= clientVec.at(i).vSet[j].isActive;

            clientVec.at(i).disconnect();
        }

        for(int i = 0; i < vCount; i++) vSet[i].isActive = ret_AVCheckSet[i];

        isActive = false;
        avCount = 0;
        for(int i = 0; i < vCount; i++)
        {
            isActive |= vSet[i].isActive;
            if(vSet[i].isActive)
            {
                avSet.at(avCount) = i;
                avCount++;
            }
        }

        std::cout << "avCount :" << avCount << std::endl;
    }
    auto end = std::chrono::system_clock::now();

    std::cout << "Total Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;

    for(int i = 0; i < nodeCount; i++) clientVec.at(i).shutdown();

    //result check
//    for(int i = 0; i < vCount * numOfInitV; i++)
//    {
//        if(i % numOfInitV == 0) std::cout << i / numOfInitV << ": ";
//        std::cout << "(" << initVSet[i % numOfInitV] << " -> " << vValues[i] << ")";
//        if(i % numOfInitV == numOfInitV - 1) std::cout << std::endl;
//    }
}