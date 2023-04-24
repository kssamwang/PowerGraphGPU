//
// Created by cave-g-f on 10/15/19.
//

#include "JumpIteration.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

template<typename VertexValueType, typename MessageValueType>
int
JumpIteration<VertexValueType, MessageValueType>::MSGApply(Graph<VertexValueType> &g, const std::vector<int> &initVSet,
                                                           std::set<int> &activeVertice,
                                                           const MessageSet<MessageValueType> &mSet) {

}

template<typename VertexValueType, typename MessageValueType>
int JumpIteration<VertexValueType, MessageValueType>::MSGGenMerge(const Graph<VertexValueType> &g,
                                                                  const std::vector<int> &initVSet,
                                                                  const std::set<int> &activeVertice,
                                                                  MessageSet<MessageValueType> &mSet) {

}

template<typename VertexValueType, typename MessageValueType>
JumpIteration<VertexValueType, MessageValueType>::JumpIteration()
{

}

template<typename VertexValueType, typename MessageValueType>
int JumpIteration<VertexValueType, MessageValueType>::loadIterationInfoFile(int vCount)
{
    std::stringstream filePath;
    filePath << "../../data/iterationJump" << vCount << "/iterationJump" << vCount << ".txt";

    std::ifstream Gin(filePath.str());

    std::cout << "start read file" << std::endl;

    if(!Gin.is_open()) {std::cout << "open" << filePath.str() << "error!" << std::endl;}

    while(!Gin.eof())
    {
        int num;
        Gin>>num;
        this->jumpIteration.push(num);
    }

    std::cout << "read file complete" << std::endl;

    Gin.close();

    return 1;
}

template<typename VertexValueType, typename MessageValueType>
int JumpIteration<VertexValueType, MessageValueType>::MSGApply_array(int vCount, int eCount, Vertex *vSet, int numOfInitV, const int *initVSet, VertexValueType *vValues, MessageValueType *mValues)
{
    for(int i = 0; i < vCount; i++)
        vSet[i].isActive = true;

    this->iterationCount++;

    if(!this->jumpIteration.empty())
    {
        this->iterationCount = this->jumpIteration.front();
        this->jumpIteration.pop();
    }

    std::stringstream filePath;
    std::string s;

    filePath << "../../data/iterationJump" << vCount << "/graph" << vCount << "Pid" << this->partitionId << "iter" << this->iterationCount << ".txt";

    std::ifstream Gin(filePath.str());

    if(!Gin.is_open()) {std::cout << "open " << filePath.str() << " error " << std::endl;}

    getline(Gin, s);
    std::cout << "return iteration " << this->iterationCount << std::endl;

    for(int i = 0; i < vCount; i++)
    {
        Gin >> vSet[i].isActive;
    }

    for(int i = 0; i < vCount * numOfInitV; i++)
    {
        Gin >> vValues[i];
    }

    Gin.close();


    return 0;
}

template<typename VertexValueType, typename MessageValueType>
int JumpIteration<VertexValueType, MessageValueType>::MSGGenMerge_array(int vCount, int eCount, const Vertex *vSet,
                                                                        const Edge *eSet, int numOfInitV,
                                                                        const int *initVSet,
                                                                        const VertexValueType *vValues,
                                                                        MessageValueType *mValues) {
    return eCount;
}

template<typename VertexValueType, typename MessageValueType>
void JumpIteration<VertexValueType, MessageValueType>::MergeGraph(Graph<VertexValueType> &g,
                                                                  const std::vector<Graph<VertexValueType>> &subGSet,
                                                                  std::set<int> &activeVertices,
                                                                  const std::vector<std::set<int>> &activeVerticeSet,
                                                                  const std::vector<int> &initVList){

}

template<typename VertexValueType, typename MessageValueType>
void JumpIteration<VertexValueType, MessageValueType>::Init(int vCount, int eCount, int numOfInitV)
{
    this->numOfInitV = numOfInitV;

    //Memory parameter init
    this->totalVValuesCount = vCount * numOfInitV;
    this->totalMValuesCount = vCount * numOfInitV;

    this->iterationCount = 0;
    this->jumpIteration = std::queue<int>();
    this->loadIterationInfoFile(vCount);
}

template<typename VertexValueType, typename MessageValueType>
void JumpIteration<VertexValueType, MessageValueType>::GraphInit(Graph<VertexValueType> &g, std::set<int> &activeVertices, const std::vector<int> &initVList)
{

}

template<typename VertexValueType, typename MessageValueType>
void JumpIteration<VertexValueType, MessageValueType>::Deploy(int vCount, int eCount, int numOfInitV)
{

}

template<typename VertexValueType, typename MessageValueType>
void JumpIteration<VertexValueType, MessageValueType>::Free()
{

}

