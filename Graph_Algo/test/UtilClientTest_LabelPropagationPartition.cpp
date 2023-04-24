//
// Created by Thoh Testarossa on 2019-04-06.
//

#include "../core/Graph.h"
#include "../core/GraphUtil.h"
#include "../srv/UtilClient.h"
#include "../srv/UNIX_shm.h"
#include "../srv/UNIX_msg.h"
#include "../algo/LabelPropagation/LabelPropagation.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include <future>
#include <cstring>

template <typename VertexValueType, typename MessageValueType>
void testFut(UtilClient<VertexValueType, MessageValueType> *uc, VertexValueType *vValues, Vertex *vSet)
{
    uc->connect();
    uc->update(vValues);
    uc->requestMSGMerge();
    uc->requestMSGApply();
    uc->disconnect();
}

bool edgeCmp(Edge &e1, Edge &e2)
{
    return e1.dst < e2.dst;
}

int main(int argc, char *argv[])
{
    if(argc != 4 && argc != 5)
    {
        std::cout << "Usage:" << std::endl << "./UtilClientTest_Propagation vCount eCount numOfInitV [nodeCount]" << std::endl;
        return 1;
    }

    int vCount = atoi(argv[1]);
    int eCount = atoi(argv[2]);
    int numOfInitV = atoi(argv[3]);
    int nodeCount = atoi(argv[4]);

    //Parameter check
    if(vCount <= 0 || eCount <= 0 || numOfInitV <= 0 || nodeCount <= 0)
    {
        std::cout << "Parameter illegal" << std::endl;
        return 3;
    }

    int *initVSet = new int [numOfInitV];
    bool *filteredV = new bool [vCount];
    int *timestamp = new int [vCount];

    std::vector<Vertex> vSet = std::vector<Vertex>();

    for(int i = 0; i < vCount; i++) filteredV[i] = false;
    for(int i = 0; i < vCount; i++) timestamp[i] = -1;
    for(int i = 0; i < vCount; i++) vSet.emplace_back(i, true, -1);

    LPA_Value *vValues = new LPA_Value [eCount];

    for(int i = 0; i < vCount; i++)
    {
        vValues[i] = LPA_Value(i, i, 0);
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

    auto clientVec = std::vector<UtilClient<LPA_Value, LPA_MSG>>();
    for(int i = 0; i < nodeCount; i++)
        clientVec.push_back(UtilClient<LPA_Value, LPA_MSG>(vCount, eCounts[i], numOfInitV, i));
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

        if(chk == -1)
        {
            std::cout << "copy Back error" << std::endl;
            return 3;
        }

        clientVec.at(i).disconnect();
    }
    int iterCount = 0;

    //Test
    std::cout << "Init finished" << std::endl;
    //Test end

    while(iterCount < 10)
    {
        //Test
        std::cout << "Processing at iter " << ++iterCount << std::endl;
        //Test end

        auto labelCntPerVertice = std::vector<std::map<int, int>>();
        auto maxLabelCnt = std::vector<std::pair<int, int>>();

        labelCntPerVertice.reserve(vCount);
        labelCntPerVertice.assign(vCount, std::map<int, int>());
        maxLabelCnt.reserve(vCount);
        maxLabelCnt.assign(vCount, std::pair<int, int>(0, 0));

        auto futList = new std::future<void> [nodeCount];
        for(int i = 0; i < nodeCount; i++)
        {
            std::future<void> tmpFut = std::async(testFut<LPA_Value, LPA_MSG>, &clientVec.at(i), vValues, &vSet[0]);
            futList[i] = std::move(tmpFut);
        }

        for(int i = 0; i < nodeCount; i++)
            futList[i].get();

        std::cout << "merge the data" << std::endl;

        auto start = std::chrono::system_clock::now();

        //Retrieve data
        for(int i = 0; i < nodeCount; i++)
        {
            clientVec.at(i).connect();

            //Collect data
            for(int j = 0; j < clientVec.at(i).eCount; j++)
            {
                auto value = clientVec.at(i).vValues[j];

                if(value.destVId == INVALID_INITV_INDEX)
                {
                    continue;
                }

                auto &labelCnt = labelCntPerVertice.at(value.destVId);
                auto &maxLabel = maxLabelCnt.at(value.destVId);

                if(labelCnt.find(value.label) == labelCnt.end())
                {
                    labelCnt[value.label] = value.labelCnt;
                }
                else
                {
                    labelCnt[value.label] += value.labelCnt;
                }

                if(maxLabel.second <= labelCnt[value.label])
                {
                    maxLabel.first = value.label;
                    maxLabel.second = labelCnt[value.label];
                }
            }

            clientVec.at(i).disconnect();
        }

        auto mergeEnd = std::chrono::system_clock::now();

        int msgCnt = 0;

        for(auto &labelCnt : labelCntPerVertice)
        {
            if(labelCnt.empty())
                continue;

            msgCnt += 1;
        }

        std::cout << "msgCnt" << msgCnt << std::endl;

        std::cout << "graph merge time: " <<  std::chrono::duration_cast<std::chrono::microseconds>(mergeEnd - start).count() << std::endl;


        for(int i = 0; i < vCount; i++)
        {
            if(maxLabelCnt.at(i).second != 0)
            {
                vValues[i].destVId = i;
                if(vValues[i].label == maxLabelCnt.at(i).first)
                {
                    vSet[i].isActive = false;
                }
                vValues[i].label = maxLabelCnt.at(i).first;
                vValues[i].labelCnt = maxLabelCnt.at(i).second;
            }
            else
            {
                vValues[i].destVId = i;
                vValues[i].label = i;
                vValues[i].labelCnt = 0;
            }
        }
    }

    std::cout << "=========result=======" << std::endl;
    //result check
    for(int i = 0; i < vCount; i++)
    {
        std::cout << i << ":" << vValues[i].label << " " << vValues[i].labelCnt << std::endl ;
    }

    for(int i = 0; i < nodeCount; i++) clientVec.at(i).shutdown();
}