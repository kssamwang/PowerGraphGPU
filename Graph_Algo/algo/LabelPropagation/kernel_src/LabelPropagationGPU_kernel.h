//
// Created by cave-g-f on 2019-10-11.
//

#pragma once

#ifndef GRAPH_ALGO_LABELPROPAGATIONGPU_KERNEL_H
#define GRAPH_ALGO_LABELPROPAGATIONGPU_KERNEL_H

#include "../../../include/GPUconfig.h"
#include "../LabelPropagationGPU.h"

#include <cuda_runtime.h>

__global__ void MSGApply_kernel(Vertex *vSet, LPA_Value *vValues, int numOfMsg, LPA_MSG *mValueSet);

cudaError_t MSGApply_kernel_exec(Vertex *vSet, LPA_Value *vValues, int numOfMsg, LPA_MSG *mValueSet);

__global__ void MSGGenMerge_kernel(LPA_MSG *mTransformdMergedMSGValueSet, Vertex *vSet, LPA_Value *vValues, int numOfEdge, Edge *eSet, int batchCnt);

cudaError_t MSGGenMerge_kernel_exec(LPA_MSG *mTransformdMergedMSGValueSet, Vertex *vSet, LPA_Value *vValues, int numOfEdge, Edge *eSet, int batchCnt);

#endif //GRAPH_ALGO_LABELPROPAGATIONGPU_KERNEL_H
