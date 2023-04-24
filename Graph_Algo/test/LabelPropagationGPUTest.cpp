//
// Created by cave-g-f on 2019-05-24.
//

#include "../algo/LabelPropagation/LabelPropagationGPU.h"

#include <iostream>
#include <fstream>
#include <algorithm>
int main(int argc, char *argv[])
{
    if(argc != 4)
    {
        std::cout << "Usage:" << std::endl << "./algo_LabelPropagationTest graph_path vcount ecount" << std::endl;
        return 1;
    }

    //Read the Graph
    std::ifstream Gin(argv[1]);
    if(!Gin.is_open()) {std::cout << "Error! File testGraph.txt not found!" << std::endl; return 1; }

    int vCount = atoi(argv[2]);
    int eCount = atoi(argv[3]);

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

    std::vector<int> initVList = std::vector<int>();
    initVList.push_back(-1);

    auto executor = LabelPropagationGPU<LPA_Value, LPA_MSG>();
    executor.ApplyD(test, initVList, 4);
}

