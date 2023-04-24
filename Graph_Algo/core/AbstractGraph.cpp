//
// Created by Thoh Testarossa on 2019-07-19.
//

#include "../core/AbstractGraph.h"

Vertex::Vertex(int vertexID, bool activeness, int initVIndex)
{
    this->vertexID = vertexID;
    this->isActive = activeness;
    this->initVIndex = initVIndex;
    this->outDegree = 0;
    this->inDegree = 0;
    this->isMaster = false;
}

Vertex::Vertex()
{
    this->vertexID = -1;
    this->isActive = false;
    this->isMaster = false;
    this->initVIndex = INVALID_INITV_INDEX;
    this->outDegree = 0;
    this->inDegree = 0;
}

Edge::Edge(int src, int dst, double weight)
{
    this->src = src;
    this->dst = dst;
    this->weight = weight;
}

Edge::Edge()
{

}

AbstractGraph::AbstractGraph(int vCount)
{
    this->vList = std::vector<Vertex>();
    this->eList = std::vector<Edge>();

    this->vCount = vCount;
    for(int i = 0; i < vCount; i++) this->vList.emplace_back(i, false, INVALID_INITV_INDEX);
    this->eCount = 0;
}

AbstractGraph::AbstractGraph(const std::vector<Vertex> &vSet, const std::vector<Edge> &eSet)
{
    this->vCount = vSet.size();
    this->eCount = eSet.size();
    this->vList = vSet;
    this->eList = eSet;
}

AbstractGraph::AbstractGraph(int vCount, int eCount, int *eSrcSet, int *eDstSet, double *eWeightSet)
{
    this->vCount = vCount;
    this->eCount = eCount;

    this->vList = std::vector<Vertex>();
    this->eList = std::vector<Edge>();

    //v assemble
    for(int i = 0; i < this->vCount; i++)
    {
        auto v = Vertex(i, false, INVALID_INITV_INDEX);
        this->vList.emplace_back(v);
    }

    //e assemble
    for(int i = 0; i < this->eCount; i++)
    {
        this->eList.emplace_back(eSrcSet[i], eDstSet[i], eWeightSet[i]);
        this->vList.at(eSrcSet[i]).outDegree += 1;
        this->vList.at(eDstSet[i]).inDegree += 1;
    }
}

void AbstractGraph::insertEdge(int src, int dst, double weight)
{
    this->eList.emplace_back(src, dst, weight);
    this->eCount++;
    this->vList.at(src).outDegree += 1;
    this->vList.at(dst).inDegree += 1;
}


