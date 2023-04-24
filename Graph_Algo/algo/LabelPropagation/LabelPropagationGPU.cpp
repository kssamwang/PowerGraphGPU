//
// Created by cave-g-f on 2019-09-22.
//

#include "LabelPropagationGPU.h"
#include "kernel_src/LabelPropagationGPU_kernel.h"

#include <iostream>
#include <algorithm>
#include <chrono>

#define NULLMSG -1

//Internal method for different GPU copy situations in BF algo
template <typename VertexValueType, typename MessageValueType>
auto LabelPropagationGPU<VertexValueType, MessageValueType>::MSGGenMerge_GPU_MVCopy(Vertex *d_vSet, const Vertex *vSet,
                                                                                    LPA_Value *d_vValues, const LPA_Value *vValues,
                                                                                    LPA_MSG *d_mTransformedMergedMSGValueSet,
                                                                                    LPA_MSG *mTransformedMergedMSGValueSet,
                                                                                    int vGCount, int eGCount)
{
    auto err = cudaSuccess;

    //vSet copy
//    err = cudaMemcpy(d_vSet, vSet, vGCount * sizeof(Vertex), cudaMemcpyHostToDevice);

    //vValueSet copy
    err = cudaMemcpy(d_vValues, vValues, vGCount * sizeof(VertexValueType), cudaMemcpyHostToDevice);

    //test
    // std::cout << "========value info========" << std::endl;
    // for(int i = 0; i < vGCount; i++)
    // {
    //     std::cout << i << " " << vValues[i << 1] << " " << vValues[(i << 1) + 1] << std::endl;
    // }
    // std::cout << "=========value end=======" << std::endl << std::endl;

    return err;
}

template <typename VertexValueType, typename MessageValueType>
auto LabelPropagationGPU<VertexValueType, MessageValueType>::MSGApply_GPU_VVCopy(LPA_Value *d_vValues, LPA_Value *vValues, int *d_offsetInValues, int vGCount, int eGCount)
{
    auto err = cudaSuccess;

    int vValueSize = vGCount;

    for(int i = 0; i < vValueSize; i++)
    {
        vValues[i].destVId = -1;
        vValues[i].label = -1;
        vValues[i].labelCnt = 0;
    }

    //vValueSet copy
    err = cudaMemcpy(d_vValues, vValues, vValueSize * sizeof(VertexValueType), cudaMemcpyHostToDevice);

    return err;
}

template <typename VertexValueType, typename MessageValueType>
LabelPropagationGPU<VertexValueType, MessageValueType>::LabelPropagationGPU()
{

}

template <typename VertexValueType, typename MessageValueType>
void LabelPropagationGPU<VertexValueType, MessageValueType>::Init(int vCount, int eCount, int numOfInitV)
{
    LabelPropagation<VertexValueType, MessageValueType>::Init(vCount, eCount, numOfInitV);

    this->vertexLimit = VERTEXSCALEINGPU;
    this->mPerMSGSet = MSGSCALEINGPU;
    this->ePerEdgeSet = EDGESCALEINGPU;
}

template <typename VertexValueType, typename MessageValueType>
void LabelPropagationGPU<VertexValueType, MessageValueType>::GraphInit(Graph<VertexValueType> &g, std::set<int> &activeVertices, const std::vector<int> &initVList)
{
    LabelPropagation<VertexValueType, MessageValueType>::GraphInit(g, activeVertices, initVList);
}

template <typename VertexValueType, typename MessageValueType>
void LabelPropagationGPU<VertexValueType, MessageValueType>::Deploy(int vCount, int eCount, int numOfInitV)
{
    LabelPropagation<VertexValueType, MessageValueType>::Deploy(vCount, eCount, numOfInitV);

    cudaError_t err = cudaSuccess;

    int vValueSize = std::max(vCount, eCount);

    err = cudaMalloc((void **)&this->d_vValueSet, vValueSize * sizeof(VertexValueType));
    err = cudaMalloc((void **)&this->d_vSet, vCount * sizeof(Vertex));
    err = cudaMalloc((void **)&this->d_eGSet, ePerEdgeSet * sizeof(Edge));

    int mSize = std::max(ePerEdgeSet, mPerMSGSet);

    this->mValueSet = new MessageValueType [mSize];
    err = cudaMalloc((void **)&this->d_mValueSet, mSize * sizeof(MessageValueType));
    err = cudaMalloc((void **)&d_mTransformedMergedMSGValueSet, mSize * sizeof(MessageValueType));
}

template <typename VertexValueType, typename MessageValueType>
void LabelPropagationGPU<VertexValueType, MessageValueType>::Free()
{
    LabelPropagation<VertexValueType, MessageValueType>::Free();

    cudaFree(this->d_vValueSet);

    cudaFree(this->d_vSet);
    cudaFree(this->d_eGSet);

    free(this->mValueSet);
    cudaFree(this->d_mValueSet);

    cudaFree(this->d_mTransformedMergedMSGValueSet);
}

template <typename VertexValueType, typename MessageValueType>
int LabelPropagationGPU<VertexValueType, MessageValueType>::MSGApply_array(int vCount, int eCount, Vertex *vSet, int numOfInitV, const int *initVSet, VertexValueType *vValues, MessageValueType *mValues)
{
    //Availability check
    if(vCount == 0) return 0;

    //CUDA init
    cudaError_t err = cudaSuccess;

    bool needReflect = false;

    int vValueSize = std::max(vCount, eCount);
    int mValuesSize = std::max(vCount, eCount);

//    if(!needReflect)
//    {
//        err = MSGApply_GPU_VVCopy(this->d_vValueSet, vValues, this->d_offsetInValues, vCount, eCount);
//    }

    //Apply msgs to v
    int mGCount = 0;
    auto mGSet = MessageSet<MessageValueType>();

    auto r_mGSet = MessageSet<MessageValueType>();
    auto r_vSet = std::vector<Vertex>();
    auto r_vValueSet = std::vector<VertexValueType>();

    for(int i = 0; i < mValuesSize; i++)
    {
        if(mValues[i].destVId != -1 && vSet[mValues[i].destVId].isMaster)
        {
            mValues[i].mValueIndex = i;
            mGSet.insertMsg(Message<MessageValueType>(0, mValues[i].destVId, mValues[i]));
            mGCount++;
        }

        if(mGCount == this->mPerMSGSet || i == mValuesSize - 1) //A batch of msgs will be transferred into GPU. Don't forget last batch!
        {
            std::cout << "size: " << mGSet.mSet.size() << std::endl;

            auto reflectIndex = std::vector<int>();
            auto reversedIndex = std::vector<int>();

            //Reflection for message & vertex & vValues
            if(needReflect)
            {
                //todo : reflect
                // //MSG reflection
                // r_mGSet = this->reflectM(mGSet, vCount, reflectIndex, reversedIndex);

                // for(int j = 0; j < r_mGSet.mSet.size(); j++)
                // {
                //     this->mInitVIndexSet[j] = vSet[r_mGSet.mSet.at(j).src].initVIndex;
                //     this->mDstSet[j] = r_mGSet.mSet.at(j).dst;
                //     this->mValueSet[j] = r_mGSet.mSet.at(j).value;
                // }

                // //v reflection
                // r_vSet.clear();
                // for(int j = 0; j < reflectIndex.size(); j++)
                //     r_vSet.emplace_back(j, false, vSet[reflectIndex.at(j)].initVIndex);

                // //vValue reflection
                // r_vValueSet.clear();
                // r_vValueSet.reserve(mPerMSGSet * numOfInitV);
                // r_vValueSet.assign(mPerMSGSet * numOfInitV, INT32_MAX >> 1);
                // for(int j = 0; j < reflectIndex.size(); j++)
                // {
                //     for(int k = 0; k < numOfInitV; k++)
                //         r_vValueSet.at(j * numOfInitV + k) = vValues[reflectIndex[j] * numOfInitV + k];
                // }

                // //vSet & vValueSet Init
                // err = MSGApply_GPU_VVCopy(d_vSet, &r_vSet[0],
                //                     d_vValueSet, (double *)&r_vValueSet[0],
                //                     reflectIndex.size(), numOfInitV);
            }
            else
            {
                //Use original msg
                for(int j = 0; j < mGSet.mSet.size(); j++)
                {
                    this->mValueSet[j] = mGSet.mSet.at(j).value;
                }
            }

            //MSG memory copy
            err = cudaMemcpy(this->d_mValueSet, this->mValueSet, mGCount * sizeof(MessageValueType), cudaMemcpyHostToDevice);

            //Kernel Execution
            for(int j = 0; j < mGCount; j += NUMOFGPUCORE)
            {
                int msgNumUsedForExec = (mGCount - j > NUMOFGPUCORE) ? NUMOFGPUCORE : (mGCount - j);

                err = MSGApply_kernel_exec(this->d_vSet, this->d_vValueSet, msgNumUsedForExec, &this->d_mValueSet[j]);
            }

            //Deflection
            if(needReflect)
            {
                // err = cudaMemcpy(&r_vSet[0], this->d_vSet, reflectIndex.size() * sizeof(Vertex), cudaMemcpyDeviceToHost);
                // err = cudaMemcpy((double *)&r_vValueSet[0], this->d_vValueSet, reflectIndex.size() * numOfInitV * sizeof(double),
                //                  cudaMemcpyDeviceToHost);

                // for(int j = 0; j < reflectIndex.size(); j++)
                // {
                //     vSet[reflectIndex[j]] = r_vSet[j];
                //     Don't forget to deflect vertexID in Vertex obj!!
                //     vSet[reflectIndex[j]].vertexID = reflectIndex[j];
                //     for(int k = 0; k < numOfInitV; k++)
                //         vValues[reflectIndex[j] * numOfInitV + k] = r_vValueSet[j * numOfInitV + k];
                // }
            }

            mGSet.mSet.clear();
            mGCount = 0;
        }
    }

    //Re-package the data

    //Memory copy back
    if(!needReflect)
    {
        err = cudaMemcpy(vValues, this->d_vValueSet, vValueSize * sizeof(VertexValueType), cudaMemcpyDeviceToHost);
    }

    //test
//     std::cout << "========value info========" << std::endl;
//     for(int i = 0; i < vValueSize; i++)
//     {
//         if(vValues[i].destVId != -1)
//            std::cout << vValues[i].destVId << " " << vValues[i].label << " " << vValues[i].labelCnt  << std::endl;
//     }
//     std::cout << "=========value end=======" << std::endl << std::endl;

    return 0;
}

template <typename VertexValueType, typename MessageValueType>
int LabelPropagationGPU<VertexValueType, MessageValueType>::MSGGenMerge_array(int vCount, int eCount, const Vertex *vSet, const Edge *eSet, int numOfInitV, const int *initVSet, const VertexValueType *vValues, MessageValueType *mValues)
{
    //Generate merged msgs directly

    //Availability check
    if(vCount == 0) return 0;

    //Memory allocation
    cudaError_t err = cudaSuccess;

    //Graph scale check
    bool needReflect = false;

    if(!needReflect)
        err = MSGGenMerge_GPU_MVCopy(this->d_vSet, vSet,
                                     this->d_vValueSet, vValues,
                                     this->d_mTransformedMergedMSGValueSet,
                                     mValues, vCount, eCount);

    //Init for possible reflection
    //Maybe can use lambda style?
    // bool *tmp_AVCheckList = new bool [vCount];
    // auto tmp_o_g = Graph<VertexValueType>(0);
    if(needReflect)
    {
        // for(int i = 0; i < vCount; i++) tmp_AVCheckList[i] = vSet[i].needMerge;
        // tmp_o_g = Graph<VertexValueType>(vCount, 0, numOfInitV, initVSet, nullptr, nullptr, nullptr, tmp_AVCheckList);
        // tmp_o_g.verticesValue.reserve(vCount * numOfInitV);
        // tmp_o_g.verticesValue.insert(tmp_o_g.verticesValue.begin(), vValues, vValues + (numOfInitV * vCount));
    }
    //This checkpoint is to used to prevent from mistaking mValues gathering in deflection
    // bool *isDst = new bool [vCount];
    // for(int i = 0; i < vCount; i++) isDst[i] = false;

    //e batch processing
    int eGCount = 0;

    std::vector<Edge> eGSet = std::vector<Edge>();
    eGSet.reserve(this->ePerEdgeSet);

    int batchCnt = 0;
    for(int i = 0; i < eCount; i++)
    {
        eGSet.emplace_back(eSet[i]);
        eGCount++;

        //Only dst receives message
        //isDst[eSet[i].dst] = true;

        if(eGCount == this->ePerEdgeSet || i == eCount - 1) //A batch of es will be transferred into GPU. Don't forget last batch!
        {
            auto reflectIndex = std::vector<int>();
            auto reversedIndex = std::vector<int>();

            std::cout << "size: " << eGSet.size() << std::endl;
            std::cout << "eGCount: " << eGCount << std::endl;

            auto r_g = Graph<VertexValueType>(0);

            //Reflection
            if(needReflect)
            {
                // r_g = this->reflectG(tmp_o_g, eGSet, reflectIndex, reversedIndex);

                // err = MSGGenMerge_GPU_MVCopy(this->d_vSet, &r_g.vList[0],
                //                              this->d_vValueSet, (double *)&r_g.verticesValue[0],
                //                              this->d_mTransformedMergedMSGValueSet,
                //                              this->mTransformedMergedMSGValueSet,
                //                              r_g.vCount, numOfInitV);

                // err = cudaMemcpy(this->d_eGSet, &r_g.eList[0], eGCount * sizeof(Edge), cudaMemcpyHostToDevice);
            }
            else
                err = cudaMemcpy(this->d_eGSet, &eGSet[0], eGCount * sizeof(Edge), cudaMemcpyHostToDevice);

            //Kernel Execution (no matter whether g is reflected or not)
            int kernelBatchCnt = 0;
            for(int j = 0; j < eGCount; j += NUMOFGPUCORE)
            {
                int edgeNumUsedForExec = (eGCount - j > NUMOFGPUCORE) ? NUMOFGPUCORE : (eGCount - j);

                err = MSGGenMerge_kernel_exec(this->d_mTransformedMergedMSGValueSet, this->d_vSet,
                                              this->d_vValueSet, edgeNumUsedForExec, &this->d_eGSet[j],  kernelBatchCnt);
                kernelBatchCnt++;
            }

            //Deflection
            if(needReflect)
            {
                // //Re-package the data
                // //Memory copy back
                // err = cudaMemcpy(this->mTransformedMergedMSGValueSet, this->d_mTransformedMergedMSGValueSet,
                //                  r_g.vCount * numOfInitV * sizeof(unsigned long long int), cudaMemcpyDeviceToHost);

                // //Valid message transformed back to original double form (deflection)
                // for (int j = 0; j < r_g.vCount * numOfInitV; j++)
                // {
                //     int o_dst = reflectIndex[j / numOfInitV];
                //     //If the v the current msg point to is not a dst, it should not be copied back because the current msg value is not correct)
                //     if(isDst[o_dst])
                //     {
                //         if(mValues[o_dst * numOfInitV + j % numOfInitV] > (MessageValueType) (longLongIntAsDouble(this->mTransformedMergedMSGValueSet[j])))
                //             mValues[o_dst * numOfInitV + j % numOfInitV] = (MessageValueType) (longLongIntAsDouble(
                //                 this->mTransformedMergedMSGValueSet[j]));
                //     }
                // }
            }
            else;

            //copy back
            err = cudaMemcpy(&mValues[batchCnt * this->ePerEdgeSet], this->d_mTransformedMergedMSGValueSet, eGCount * sizeof(MessageValueType), cudaMemcpyDeviceToHost);

            batchCnt++;

            //Checkpoint reset
            eGCount = 0;
            eGSet.clear();
            //for(int j = 0; j < vCount; j++) isDst[j] = false;
        }
    }

    //maybe eCount < vCount
    for(int i = eCount; i < vCount; i++)
    {
        mValues[i].destVId = -1;
    }

    return std::max(eCount, vCount);
}
