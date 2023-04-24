//
// Created by cave-g-f on 2019-10-22
//

#pragma once

#ifndef GRAPH_ALGO_LABELPROPAGATIONGPU_H
#define GRAPH_ALGO_LABELPROPAGATIONGPU_H

#include "LabelPropagation.h"
#include "../../include/GPUconfig.h"
#include "kernel_src/LabelPropagationGPU_kernel.h"

template <typename VertexValueType, typename MessageValueType>
class LabelPropagationGPU : public LabelPropagation<VertexValueType, MessageValueType>
{
public:
    LabelPropagationGPU();

    void Init(int vCount, int eCount, int numOfInitV) override;
    void GraphInit(Graph<VertexValueType> &g, std::set<int> &activeVertices, const std::vector<int> &initVList) override;
    void Deploy(int vCount, int eCount, int numOfInitV) override;
    void Free() override;

    int MSGApply_array(int vCount, int eCount, Vertex *vSet, int numOfInitV, const int *initVSet, VertexValueType *vValues, MessageValueType *mValues) override;
    int MSGGenMerge_array(int vCount, int eCount, const Vertex *vSet, const Edge *eSet, int numOfInitV, const int *initVSet, const VertexValueType *vValues, MessageValueType *mValues) override;

protected:
    int vertexLimit;
    int mPerMSGSet;
    int ePerEdgeSet;

    LPA_Value *d_vValueSet;        //limit size = vertexLimit

    LPA_MSG *mValueSet;
    LPA_MSG *d_mValueSet; //limist size = max(mPerMSGSet, ePerEdgeSet)

    Vertex *d_vSet; //limit size = vertexLimit
    Edge *d_eGSet;  //limit size = ePerEdgeSet

    LPA_MSG *d_mTransformedMergedMSGValueSet; //limist size = max(mPerMSGSet, ePerEdgeSet)


private:
    auto MSGGenMerge_GPU_MVCopy(Vertex *d_vSet, const Vertex *vSet,
                                LPA_Value *d_vValues, const LPA_Value *vValues,
                                LPA_MSG *d_mTransformedMergedMSGValueSet,
                                LPA_MSG *mTransformedMergedMSGValueSet,
                                int vGCount, int eGCount);

    auto MSGApply_GPU_VVCopy(LPA_Value *d_vValues, LPA_Value *vValues, int *d_offsetInValues, int vGCount, int eGCount);
};

#endif //GRAPH_ALGO_LabelPropagationGPU_H
