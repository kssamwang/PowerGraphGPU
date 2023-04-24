//
// Created by cave-g-f on 10/15/19.
//

#pragma once

#ifndef GRAPH_ALGO_JUMPITERATION_H
#define GRAPH_ALGO_JUMPITERATION_H

#include <queue>
#include "../core/Graph.h"
#include "../core/GraphUtil.h"

template <typename VertexValueType, typename MessageValueType>
class JumpIteration : public GraphUtil<VertexValueType, MessageValueType>
{
public:

    JumpIteration();

    int MSGApply(Graph<VertexValueType> &g, const std::vector<int> &initVSet, std::set<int> &activeVertice, const MessageSet<MessageValueType> &mSet) override;
    int MSGGenMerge(const Graph<VertexValueType> &g, const std::vector<int> &initVSet, const std::set<int> &activeVertice, MessageSet<MessageValueType> &mSet) override;

    //Unified interface but actually algo_BellmanFord didn't use this form
    int MSGApply_array(int vCount, int eCount, Vertex *vSet, int numOfInitV, const int *initVSet, VertexValueType *vValues, MessageValueType *mValues) override;
    int MSGGenMerge_array(int vCount, int eCount, const Vertex *vSet, const Edge *eSet, int numOfInitV, const int *initVSet, const VertexValueType *vValues, MessageValueType *mValues) override;

    void MergeGraph(Graph<VertexValueType> &g, const std::vector<Graph<VertexValueType>> &subGSet,
                    std::set<int> &activeVertices, const std::vector<std::set<int>> &activeVerticeSet,
                    const std::vector<int> &initVList) override;

    void Init(int vCount, int eCount, int numOfInitV) override;
    void GraphInit(Graph<VertexValueType> &g, std::set<int> &activeVertices, const std::vector<int> &initVList) override;
    void Deploy(int vCount, int eCount, int numOfInitV) override;
    void Free() override;

    int loadIterationInfoFile(int vCount);

    int iterationCount;
    std::queue<int> jumpIteration;

protected:
    int numOfInitV;
};

#endif //GRAPH_ALGO_JUMPITERATION_H
