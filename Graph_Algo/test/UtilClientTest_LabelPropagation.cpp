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

    int *initVSet = new int [numOfInitV];
    LPA_Value *vValues = new LPA_Value [eCount];
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

    Graph<LPA_Value> test = Graph<LPA_Value>(vCount);
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

    for(int i = 0; i < vCount; i++)
    {
        vValues[i] = LPA_Value(i, i, 0);
    }

    numOfInitV = 1;

    std::cout << "start transfer" << std::endl;

    //Client Init Data Transfer
    auto clientVec = std::vector<UtilClient<LPA_Value, LPA_MSG>>();
    for(int i = 0; i < nodeCount; i++)
        clientVec.push_back(UtilClient<LPA_Value, LPA_MSG>(vCount, ((i + 1) * eCount) / nodeCount - (i * eCount) / nodeCount, numOfInitV, i));
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
    int iterCount = 0;

    //Test
    std::cout << "Init finished" << std::endl;
    //Test end

    while(iterCount < 50)
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
            std::future<void> tmpFut = std::async(testFut<LPA_Value, LPA_MSG>, &clientVec.at(i), vValues, &test.vList[0]);
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
                    labelCnt[value.label] = 1;
                }
                else
                {
                    labelCnt[value.label] += 1;
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

        std::cout << "graph merge time: " <<  std::chrono::duration_cast<std::chrono::microseconds>(mergeEnd - start).count() << std::endl;


        for(int i = 0; i < vCount; i++)
        {
            if(maxLabelCnt.at(i).second != 0)
            {
                vValues[i].destVId = i;
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