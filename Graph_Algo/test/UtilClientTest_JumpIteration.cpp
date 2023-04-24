//
// Created by cave-g-f on 10/16/19.
//


#include "../core/Graph.h"
#include "../core/GraphUtil.h"
#include "../srv/UtilClient.h"
#include "../srv/UNIX_shm.h"
#include "../srv/UNIX_msg.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <queue>
#include <future>
#include <cstring>

template <typename VertexValueType, typename MessageValueType>
void testFut(UtilClient<VertexValueType, MessageValueType> *uc, double *vValues, Vertex *vSet)
{
    uc->connect();
    uc->update(vValues, vSet);
    uc->requestMSGMerge();
    uc->requestMSGApply();
    uc->disconnect();
}

int main(int argc, char *argv[])
{
    if(argc != 4 && argc != 5)
    {
        std::cout << "Usage:" << std::endl << "./UtilClientTest_BellmanFord vCount eCount numOfInitV [nodeCount]" << std::endl;
        return 1;
    }

    int vCount = atoi(argv[1]);
    int eCount = atoi(argv[2]);
    int numOfInitV = atoi(argv[3]);
    int nodeCount = (argc == 4) ? 1 : atoi(argv[4]);

    //Parameter check
    if(vCount <= 0 || eCount <= 0 || numOfInitV <= 0 || nodeCount <= 0)
    {
        std::cout << "Parameter illegal" << std::endl;
        return 3;
    }

    //Init the Graph
    int *initVSet = new int [numOfInitV];
    double *vValues = new double [vCount * numOfInitV];
    bool *filteredV = new bool [vCount];
    int *timestamp = new int [vCount];

    std::vector<Vertex> vSet = std::vector<Vertex>();
    std::vector<Edge> eSet = std::vector<Edge>();

    for(int i = 0; i < vCount * numOfInitV; i++) vValues[i] = INT32_MAX  >> 1;

    //Easy init
//    initVSet[0] = 1;
//    initVSet[1] = 200004;
//    initVSet[2] = 300007;
//    initVSet[3] = 100002;

    initVSet[0] = 1;
    initVSet[1] = 400002;
    initVSet[2] = 800004;
    initVSet[3] = 1200007;
    for(int i = 0; i < numOfInitV; i++) vValues[initVSet[i] * numOfInitV + i] = 0;

    for(int i = 0; i < vCount; i++) filteredV[i] = false;

    for(int i = 0; i < vCount; i++) timestamp[i] = -1;

    for(int i = 0; i < vCount; i++) vSet.emplace_back(i, false, -1);
    for(int i = 0; i < numOfInitV; i++)
    {
        vSet.at(initVSet[i]).initVIndex = i;
        vSet.at(initVSet[i]).isActive = true;
    }

    int partitionV[nodeCount];
    int partitionE[nodeCount];

    //read test file
    std::string filePrefix = "../../data/testGraphPid";
    std::string filePostfix = ".txt";

    for(int i = 0; i < nodeCount; i++)
    {
        std::stringstream fileName;
        fileName << filePrefix << i << filePostfix;

        std::ifstream Gin(fileName.str());

        if(!Gin.is_open()) {std::cout << "open file " << fileName.str() << "error!" << std::endl;}

        Gin >> partitionV[i] >> partitionE[i];

        for(int j = 0; j < partitionE[i]; j++)
        {
            int src, dst;
            double weight;
            Gin >> src >> dst >> weight;
            eSet.emplace_back(src, dst, weight);
        }

        Gin.close();
    }

    //read iteration jump file
    std::ifstream Gin("../../data/iterationJump400000/iterationJump400000.txt");
    std::queue<int> iterationJump = std::queue<int>();

    if(!Gin.is_open()) {std::cout << "open jump 400000 error!" << std::endl;}

    while(!Gin.eof())
    {
        int iterationNum = 0;
        Gin >> iterationNum;
        iterationJump.push(iterationNum);
    }

    Gin.close();

    //Client Init Data Transfer
    auto clientVec = std::vector<UtilClient<double, double>>();
    for(int i = 0; i < nodeCount; i++)
        clientVec.push_back(UtilClient<double, double>(vCount, partitionE[i], numOfInitV, i));
    int chk = 0;
    for(int i = 0; i < nodeCount && chk != -1; i++)
    {
        chk = clientVec.at(i).connect();
        if (chk == -1)
        {
            std::cout << "Cannot establish the connection with server correctly" << std::endl;
            return 2;
        }

        int eSetOffset = 0;

        for(int j = 0; j < i; j++)
        {
            eSetOffset += partitionE[j];
        }

        chk = clientVec.at(i).transfer(vValues, &vSet[0], &eSet[eSetOffset], initVSet, filteredV, timestamp);

        if(chk == -1)
        {
            std::cout << "Parameter illegal" << std::endl;
            return 3;
        }

        clientVec.at(i).disconnect();
    }

    bool isActive = false;
    for(int i = 0; i < vCount; i++) isActive |= vSet[i].isActive;

    bool *ret_AVCheckSet = new bool [vCount];
    int iterCount = 0;

    //Test
    std::cout << "Init finished" << std::endl;
    //Test end

    while(isActive)
    {
        std::cout << "Processing at iter " << ++iterCount << std::endl;

        for(int i = 0; i < vCount; i++) ret_AVCheckSet[i] = false;

        auto futList = new std::future<void> [nodeCount];
        for(int i = 0; i < nodeCount; i++)
        {
            std::future<void> tmpFut = std::async(testFut<double, double>, &clientVec.at(i), vValues, &vSet[0]);
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
        for(int i = 0; i < vCount; i++) isActive |= vSet[i].isActive;
    }

    for(int i = 0; i < nodeCount; i++) clientVec.at(i).shutdown();
}