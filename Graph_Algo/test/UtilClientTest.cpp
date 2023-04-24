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
    if(argc != 4 && argc != 5 && argc != 6)
    {
        std::cout << "Usage:" << std::endl << "./UtilClientTest_LabelPropagation graph vCount eCount numOfInitV nodecount" << std::endl;
        return 1;
    }

    int vCount = atoi(argv[2]);
    int eCount = atoi(argv[3]);
    int numOfInitV = atoi(argv[4]);
    int nodeCount = atoi(argv[5]);

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

    std::ifstream Gin(argv[1]);
    if(!Gin.is_open())
    {
        std::cout << "Error! File testGraph.txt not found!" << std::endl;
        return 4;
    }

    //init v index
    std::cout << "init initVSet ..." << std::endl;

    initVSet[0] = 1;

    std::cout << "init vSet ..." << std::endl;

    Graph<double> test = Graph<double>(vCount);
    for(int i = 0; i < eCount; i++)
    {
        int src, dst;
        double weight;

        Gin >> src >> dst >> weight;
        test.insertEdge(src, dst, weight);

        //for edge-cut partition
        test.vList.at(src).isMaster = true;
        test.vList.at(dst).isMaster = true;
    }
    Gin.close();

    std::cout << "init vValues ..." << std::endl;

    for(int i = 0; i < vCount * numOfInitV; i++) vValues[i] = INT32_MAX  >> 1;

    for(int i = 0; i < numOfInitV; i++) vValues[initVSet[i] * numOfInitV + i] = 0;

    for(int i = 0; i < vCount; i++) filteredV[i] = false;

    for(int i = 0; i < vCount; i++) timestamp[i] = -1;

    for(int i = 0; i < numOfInitV; i++)
    {
        test.vList.at(initVSet[i]).initVIndex = i;
        test.vList.at(initVSet[i]).isActive = true;
    }

    //Client Init Data Transfer
    auto clientVec = std::vector<UtilClient<double, double>>();
    for(int i = 0; i < nodeCount; i++)
        clientVec.push_back(UtilClient<double, double>(vCount, ((i + 1) * eCount) / nodeCount - (i * eCount) / nodeCount, numOfInitV, i));
    int chk = 0;
    for(int i = 0; i < nodeCount && chk != -1; i++)
    {
        chk = clientVec.at(i).connect();
        if (chk == -1)
        {
            std::cout << "Cannot establish the connection with server correctly" << std::endl;
            return 2;
        }

        chk = clientVec.at(i).transfer(vValues, &test.vList[0], &test.eList[(i * eCount) / nodeCount], initVSet, filteredV, timestamp);

        if(chk == -1)
        {
            std::cout << "Parameter illegal" << std::endl;
            return 3;
        }

        clientVec.at(i).disconnect();
    }

    bool isActive = false;
    for(int i = 0; i < vCount; i++) isActive |= test.vList[i].isActive;

    bool *ret_AVCheckSet = new bool [vCount];
    int iterCount = 0;

    //Test
    std::cout << "Init finished" << std::endl;
    //Test end

    while(isActive)
    {
        //Test
        std::cout << "Processing at iter " << ++iterCount << std::endl;
        //Test end

        for(int i = 0; i < vCount; i++) ret_AVCheckSet[i] = false;

        auto futList = new std::future<void> [nodeCount];
        for(int i = 0; i < nodeCount; i++)
        {
            std::future<void> tmpFut = std::async(testFut<double, double>, &clientVec.at(i), vValues, &test.vList[0]);
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

        for(int i = 0; i < vCount; i++) test.vList[i].isActive = ret_AVCheckSet[i];

        isActive = false;
        for(int i = 0; i < vCount; i++) isActive |= test.vList[i].isActive;
    }

    for(int i = 0; i < nodeCount; i++) clientVec.at(i).shutdown();

    //result check
    for(int i = 0; i < vCount * numOfInitV; i++)
    {
        if(i % numOfInitV == 0) std::cout << i / numOfInitV << ": ";
        std::cout << "(" << initVSet[i % numOfInitV] << " -> " << vValues[i] << ")";
        if(i % numOfInitV == numOfInitV - 1) std::cout << std::endl;
    }
}