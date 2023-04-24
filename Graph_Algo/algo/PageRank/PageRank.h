//
// Created by cave-g-f on 2019-9-21
//

#ifndef GRAPH_ALGO_PAGERANK_H
#define GRAPH_ALGO_PAGERANK_H

#include "../../core/GraphUtil.h"

class PRA_MSG
{
public:
    PRA_MSG():PRA_MSG(-1, -1)
    {

    }

    PRA_MSG(int destVId, double rank)
    {
        this->destVId = destVId;
        this->rank = rank;
    }

    int destVId;
    double rank;
};

template <typename VertexValueType, typename MessageValueType>
class PageRank : public GraphUtil<VertexValueType, MessageValueType>
{
public:
    PageRank();

    int MSGApply(Graph<VertexValueType> &g, const std::vector<int> &initVSet, std::set<int> &activeVertice, const MessageSet<MessageValueType> &mSet) override;
    int MSGGenMerge(const Graph<VertexValueType> &g, const std::vector<int> &initVSet, const std::set<int> &activeVertice, MessageSet<MessageValueType> &mSet) override;

    //Unified interface but actually algo_BellmanFord didn't use this form
    int MSGApply_array(int vCount, int eCount, Vertex *vSet, int numOfInitV, const int *initVSet, VertexValueType *vValues, MessageValueType *mValues) override;
    int MSGGenMerge_array(int vCount, int eCount, const Vertex *vSet, const Edge *eSet, int numOfInitV, const int *initVSet, const VertexValueType *vValues, MessageValueType *mValues) override;

    void MergeGraph(Graph<VertexValueType> &g, const std::vector<Graph<VertexValueType>> &subGSet,
                    std::set<int> &activeVertices, const std::vector<std::set<int>> &activeVerticeSet,
                    const std::vector<int> &initVList) override;

    std::vector<Graph<VertexValueType>> DivideGraphByEdge(const Graph<VertexValueType> &g, int partitionCount);

    void Init(int vCount, int eCount, int numOfInitV) override;
    void GraphInit(Graph<VertexValueType> &g, std::set<int> &activeVertices, const std::vector<int> &initVList) override;
    void Deploy(int vCount, int eCount, int numOfInitV) override;
    void Free() override;

    void ApplyStep(Graph<VertexValueType> &g, const std::vector<int> &initVSet, std::set<int> &activeVertices);
    void Apply(Graph<VertexValueType> &g, const std::vector<int> &initVList);

    void ApplyD(Graph<VertexValueType> &g, const std::vector<int> &initVList, int partitionCount);

protected:
    int numOfInitV;
    double resetProb;
    double deltaThreshold;
};

#endif //GRAPH_ALGO_PAGERANK_H
