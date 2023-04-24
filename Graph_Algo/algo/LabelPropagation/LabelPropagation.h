//
// Created by cave-g-f on 2019-05-17.
//

#pragma once

#ifndef GRAPH_ALGO_LABELPROPAGATION_H
#define GRAPH_ALGO_LABELPROPAGATION_H

#include "../../core/GraphUtil.h"

class LPA_Value
{
public:
    LPA_Value():LPA_Value(INVALID_INITV_INDEX, -1, -1)
    {

    }

    LPA_Value(int destVId, int label, int labelCnt)
    {
        this->destVId = destVId;
        this->label = label;
        this->labelCnt = labelCnt;
    }

    int destVId;
    int label;
    int labelCnt;
};

class LPA_MSG
{
public:
    LPA_MSG():LPA_MSG(-1, -1)
    {

    }

    LPA_MSG(int destVId, int label)
    {
        this->destVId = destVId;
        this->label = label;
    }

    int destVId;
    int label;
    //used for gpu apply array
    int mValueIndex;
};

template <typename VertexValueType, typename MessageValueType>
class LabelPropagation : public GraphUtil<VertexValueType, MessageValueType>
{
public:
    LabelPropagation();

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

    std::vector<Graph<VertexValueType>> DivideGraphByEdge(const Graph<VertexValueType> &g, int partitionCount);

    void ApplyStep(Graph<VertexValueType> &g, const std::vector<int> &initVSet, std::set<int> &activeVertices);
    void Apply(Graph<VertexValueType> &g, const std::vector<int> &initVList);

    void ApplyD(Graph<VertexValueType> &g, const std::vector<int> &initVList, int partitionCount);
};

#endif
