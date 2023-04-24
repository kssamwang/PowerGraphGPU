//
// Created by Thoh Testarossa on 2019-08-28.
//

#include "../algo/DDFS/DDFS.h"

#include <iostream>
#include <fstream>

int main()
{
    //Read the Graph
    std::ifstream Gin("testGraph.txt");
    if(!Gin.is_open()) {std::cout << "Error! File testGraph.txt not found!" << std::endl; return 1; }

    int vCount, eCount;
    Gin >> vCount >> eCount;

    Graph<DFSValue> test = Graph<DFSValue>(vCount);
    for(int i = 0; i < eCount; i++)
    {
        int src, dst;
        double weight;

        Gin >> src >> dst >> weight;
        test.insertEdge(src, dst, weight);
    }

    Gin.close();

    std::vector<int> initVList = std::vector<int>();
    initVList.push_back(0);

    auto executor = DDFS<DFSValue, DFSMSG>();
    executor.ApplyD(test, initVList, 4);

    for(int i = 0; i < test.vCount; i++)
    {
        std::cout << i << ": " << std::endl;

        std::cout << "Parent: ";
        for(const auto &vV : test.verticesValue.at(i).vStateList)
        {
            if(vV.second.second == MARK_PARENT)
                std::cout << vV.second.first << " ";
        }
        std::cout << std::endl;

        std::cout << "Sons: ";
        for(const auto &vV : test.verticesValue.at(i).vStateList)
        {
            if(vV.second.second == MARK_SON)
                std::cout << vV.second.first << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}