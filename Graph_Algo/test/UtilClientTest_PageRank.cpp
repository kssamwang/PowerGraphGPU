//
// Created by Thoh Testarossa on 2019-04-06.
//

#include "../core/Graph.h"
#include "../srv/UtilClient.h"
#include "../algo/PageRank/PageRank.h"

#include <iostream>
#include <fstream>
#include <vector>

#include <future>

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
        std::cout << "Usage:" << std::endl << "./UtilClientTest_PageRank graph vCount eCount numOfInitV nodecount" << std::endl;
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
        return -1;
    }

    int *initVSet = new int [numOfInitV];
    std::pair<double, double> *vValues = new std::pair<double, double> [vCount];
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

    initVSet[0] = -1;

    std::cout << "init vSet ..." << std::endl;

    Graph<std::pair<double, double>> test = Graph<std::pair<double, double>>(vCount);
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

    if(initVSet[0] == -1)
    {
        for(int i = 0; i < vCount; i++)
        {
            auto newRank = 0.15;
            vValues[i] = std::pair<double, double>(newRank, newRank);
            test.vList.at(i).isActive = true;
        }
    }
    else
    {
        for(int i = 0; i < vCount; i++)
        {
            if(test.vList.at(i).initVIndex == INVALID_INITV_INDEX)
            {
                vValues[i] = std::pair<double, double>(0.0, 0.0);
            }
            else
            {
                vValues[i] = std::pair<double, double>(1.0, 1.0);
                test.vList.at(i).isActive = true;
            }
        }

    }

    std::cout << "init edge ..." << std::endl;

    for(int i = 0; i < eCount; i++)
    {
        test.eList.at(i).weight = 1.0 / test.vList.at(test.eList.at(i).src).outDegree;
    }

    numOfInitV = 1;

    std::cout << "start transfer" << std::endl;

    //Client Init Data Transfer
    auto clientVec = std::vector<UtilClient<std::pair<double, double>, PRA_MSG>>();
    for(int i = 0; i < nodeCount; i++)
        clientVec.push_back(UtilClient<std::pair<double, double>, PRA_MSG>(vCount, ((i + 1) * eCount) / nodeCount - (i * eCount) / nodeCount, numOfInitV, i));
    int chk = 0;

    for(int i = 0; i < nodeCount && chk != -1; i++)
    {
        std::cout << "connect" << std::endl;
        chk = clientVec.at(i).connect();
        std::cout << "connect end" << std::endl;
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
    int iterCount = 0;

    //Test
    std::cout << "Init finished" << std::endl;
    //Test end

    bool isActive = true;

    int avCount = 0;
    std::vector<int> avSet;
    avSet.reserve(vCount);
    avSet.assign(vCount, 0);

    while(isActive)
    {
        //Test
        std::cout << "Processing at iter " << ++iterCount << std::endl;
        //Test end

        isActive = false;

        auto futList = new std::future<void> [nodeCount];
        for(int i = 0; i < nodeCount; i++)
        {
            std::future<void> tmpFut = std::async(testFut<std::pair<double, double>, PRA_MSG>, &clientVec.at(i), vValues, &test.vList[0], &avSet[0], avCount);
            futList[i] = std::move(tmpFut);
        }

        for(int i = 0; i < nodeCount; i++)
            futList[i].get();

        auto start = std::chrono::system_clock::now();

        //clear active info
        for(int i = 0; i < vCount; i++)
        {
            test.vList[i].isActive = false;
        }

        //Retrieve data
        for(int i = 0; i < nodeCount; i++)
        {
            clientVec.at(i).connect();


            //Collect data
            for(int j = 0; j < vCount; j++)
            {
                auto &value = clientVec.at(i).vValues[j];
                if(clientVec.at(i).vSet[j].isActive)
                {
                    if(!test.vList.at(j).isActive)
                    {
                        test.vList.at(j).isActive = clientVec.at(i).vSet[j].isActive;
                        vValues[j].first = value.first;
                        vValues[j].second = value.second;
                    }
                    else
                    {
                        vValues[j].first += value.second;
                        vValues[j].second += value.second;
                    }
                }
                else if(!test.vList.at(j).isActive)
                {
                    vValues[j] = value;
                }
            }

            clientVec.at(i).disconnect();
        }

        auto mergeEnd = std::chrono::system_clock::now();

        std::cout << "graph merge time: " <<  std::chrono::duration_cast<std::chrono::microseconds>(mergeEnd - start).count() << std::endl;

        avCount = 0;
        for(int i = 0; i < vCount; i++)
        {
            if(test.vList[i].isActive)
            {
                avSet.at(avCount) = i;
                avCount++;
                isActive = true;
            }
        }

        std::cout << "avCount : " << avCount << std::endl;

        //test
//        for(int i = 0; i < vCount; i++)
//        {
//            std::cout << i << ":" << vValues[i].first << " " << vValues[i].second << std::endl ;
//        }
    }

    std::cout << "===========result===========" << std::endl;
    //result check
    for(int i = 0; i < vCount; i++)
    {
        std::cout << i << ":" << vValues[i].first << " " << vValues[i].second << std::endl ;
    }

    for(int i = 0; i < nodeCount; i++) clientVec.at(i).shutdown();
}