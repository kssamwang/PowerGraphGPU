//
// Created by cave-g-f on 2019-09-22.
//

#include "PageRankGPU.h"
#include "kernel_src/PageRankGPU_kernel.h"

#include <iostream>
#include <algorithm>
#include <chrono>

#define NULLMSG -1

//Internal method for different GPU copy situations in BF algo
template <typename VertexValueType, typename MessageValueType>
auto PageRankGPU<VertexValueType, MessageValueType>::MSGGenMerge_GPU_MVCopy(Vertex *d_vSet, const Vertex *vSet,
                                   double *d_vValues, const double *vValues,
                                   PRA_MSG *d_mTransformedMergedMSGValueSet,
                                   PRA_MSG *mTransformedMergedMSGValueSet,
                                   int vGCount, int eGCount)
{
    auto err = cudaSuccess;

    //vSet copy
    err = cudaMemcpy(d_vSet, vSet, vGCount * sizeof(Vertex), cudaMemcpyHostToDevice);

    //vValueSet copy
    err = cudaMemcpy(d_vValues, vValues, vGCount * sizeof(VertexValueType), cudaMemcpyHostToDevice);

    //mValues copy

    err = cudaMemcpy(d_mTransformedMergedMSGValueSet, mTransformedMergedMSGValueSet, vGCount * sizeof(MessageValueType), cudaMemcpyHostToDevice);

    //test
//     std::cout << "========value info========" << std::endl;
//     for(int i = 0; i < vGCount; i++)
//     {
//         std::cout << i << " " << vValues[i << 1] << " " << vValues[(i << 1) + 1] << std::endl;
//     }
//     std::cout << "=========value end=======" << std::endl << std::endl;
    return err;
}

template <typename VertexValueType, typename MessageValueType>
auto PageRankGPU<VertexValueType, MessageValueType>::MSGApply_GPU_VVCopy(Vertex *d_vSet, Vertex *vSet,
                                double *d_vValues, double *vValues,
                                int vGCount)
{
    auto err = cudaSuccess;

    //vSet Copy
    err = cudaMemcpy(d_vSet, vSet, vGCount * sizeof(Vertex), cudaMemcpyHostToDevice);

    //vValueSet copy
    err = cudaMemcpy(d_vValues, vValues, vGCount * sizeof(VertexValueType), cudaMemcpyHostToDevice);

    return err;
}

template <typename VertexValueType, typename MessageValueType>
PageRankGPU<VertexValueType, MessageValueType>::PageRankGPU()
{

}

template <typename VertexValueType, typename MessageValueType>
void PageRankGPU<VertexValueType, MessageValueType>::Init(int vCount, int eCount, int numOfInitV)
{
    PageRank<VertexValueType, MessageValueType>::Init(vCount, eCount, numOfInitV);

    this->vertexLimit = VERTEXSCALEINGPU;
    this->mPerMSGSet = MSGSCALEINGPU;
    this->ePerEdgeSet = EDGESCALEINGPU;
}

template <typename VertexValueType, typename MessageValueType>
void PageRankGPU<VertexValueType, MessageValueType>::GraphInit(Graph<VertexValueType> &g, std::set<int> &activeVertices, const std::vector<int> &initVList)
{
    PageRank<VertexValueType, MessageValueType>::GraphInit(g, activeVertices, initVList);
}

template <typename VertexValueType, typename MessageValueType>
void PageRankGPU<VertexValueType, MessageValueType>::Deploy(int vCount, int eCount, int numOfInitV)
{
    PageRank<VertexValueType, MessageValueType>::Deploy(vCount, eCount, numOfInitV);

    cudaError_t err = cudaSuccess;

    int vertexForMalloc = vCount > this->vertexLimit ? this->vertexLimit : vCount;

    err = cudaMalloc((void **)&this->d_vValueSet, vertexForMalloc * sizeof(VertexValueType));
    err = cudaMalloc((void **)&this->d_vSet, vertexForMalloc * sizeof(Vertex));

    int mSize = std::max(this->ePerEdgeSet, this->mPerMSGSet);

    mSize = eCount > mSize ? mSize : eCount;

    err = cudaMalloc((void **)&this->d_eGSet, mSize * sizeof(Edge));

    this->mValueSet = new MessageValueType [vertexForMalloc];
    err = cudaMalloc((void **)&this->d_mValueSet, vertexForMalloc * sizeof(MessageValueType));

    this->mDstSet = new int [mSize];
    err = cudaMalloc((void **)&this->d_mDstSet, mSize * sizeof(int));

    err = cudaMalloc((void **)&this->d_mTransformedMergedMSGValueSet, vertexForMalloc * sizeof(MessageValueType));
    this->mTransformedMergedMSGValueSet = new MessageValueType[vertexForMalloc];
}

template <typename VertexValueType, typename MessageValueType>
void PageRankGPU<VertexValueType, MessageValueType>::Free()
{
    PageRank<VertexValueType, MessageValueType>::Free();

    cudaFree(this->d_vValueSet);

    cudaFree(this->d_vSet);
    cudaFree(this->d_eGSet);

    free(this->mValueSet);
    cudaFree(this->d_mValueSet);

    free(this->mTransformedMergedMSGValueSet);
    cudaFree(this->d_mTransformedMergedMSGValueSet);

    free(this->mDstSet);
    cudaFree(this->d_mDstSet);
}

template <typename VertexValueType, typename MessageValueType>
int PageRankGPU<VertexValueType, MessageValueType>::MSGApply_array(int vCount, int eCount, Vertex *vSet, int numOfInitV, const int *initVSet, VertexValueType *vValues, MessageValueType *mValues)
{
    //Availability check
    if(vCount == 0) return 0;

    //CUDA init
    cudaError_t err = cudaSuccess;

    bool needReflect = vCount > this->vertexLimit;

    for(int i = 0; i < vCount; i++)
    {
        vSet[i].isActive = false;
    }

    if(!needReflect)
    {
        err = MSGApply_GPU_VVCopy(this->d_vSet, vSet,
                            this->d_vValueSet, (double *)vValues,
                            vCount);
    }

    //Apply msgs to v
    int mGCount = 0;
    auto mGSet = MessageSet<MessageValueType>();

    auto r_mGSet = MessageSet<MessageValueType>();
    auto r_vSet = std::vector<Vertex>();
    auto r_vValueSet = std::vector<VertexValueType>();

    for(int i = 0; i < vCount; i++)
    {
        if(mValues[i].destVId != -1 && vSet[i].isMaster) //Adding msgs to batchs
        {
            mGSet.insertMsg(Message<MessageValueType>(0, mValues[i].destVId, mValues[i]));
            mGCount++;
        }

        if(mGCount == this->mPerMSGSet || i == vCount - 1) //A batch of msgs will be transferred into GPU. Don't forget last batch!
        {

            std::cout << mGCount << std::endl;

            //-----reflect-----
            auto reflectIndex = std::vector<int>();
            auto reversedIndex = std::vector<int>();

            //Reflection for message & vertex & vValues
            if(needReflect)
            {
                //MSG reflection
                 r_mGSet = this->reflectM(mGSet, vCount, reflectIndex, reversedIndex);

                 for(int j = 0; j < r_mGSet.mSet.size(); j++)
                 {
                     this->mValueSet[j] = r_mGSet.mSet.at(j).value;
                     this->mDstSet[j] = r_mGSet.mSet.at(j).dst;
                 }

                 //v reflection
                 r_vSet.clear();
                 for(int j = 0; j < reflectIndex.size(); j++)
                     r_vSet.emplace_back(j, false, vSet[reflectIndex.at(j)].initVIndex);

                 //vValue reflection
                 r_vValueSet.clear();
                 r_vValueSet.reserve(this->vertexLimit);
                 r_vValueSet.assign(this->vertexLimit, VertexValueType(0, 0));

                 for(int j = 0; j < reflectIndex.size(); j++)
                 {
                     r_vValueSet.at(j) = vValues[reflectIndex.at(j)];
                 }

                 //vSet & vValueSet Init
                 err = MSGApply_GPU_VVCopy(d_vSet, &r_vSet[0],
                                     d_vValueSet, (double *)&r_vValueSet[0],
                                     reflectIndex.size());
            }
            else
            {
                //Use original msg
                for(int j = 0; j < mGSet.mSet.size(); j++)
                {
                    this->mValueSet[j] = mGSet.mSet.at(j).value;
                    this->mDstSet[j] = mGSet.mSet.at(j).dst;
                }
            }

            //MSG memory copy
            err = cudaMemcpy(this->d_mValueSet, this->mValueSet, mGCount * sizeof(MessageValueType), cudaMemcpyHostToDevice);
            err = cudaMemcpy(this->d_mDstSet, this->mDstSet, mGCount * sizeof(int), cudaMemcpyHostToDevice);

            //Kernel Execution
            for(int j = 0; j < mGCount; j += NUMOFGPUCORE)
            {
                int msgNumUsedForExec = (mGCount - j > NUMOFGPUCORE) ? NUMOFGPUCORE : (mGCount - j);
                err = MSGApply_kernel_exec(this->d_vSet, this->d_vValueSet, msgNumUsedForExec, &this->d_mDstSet[j], &this->d_mValueSet[j]);
            }

            //Deflection
            if(needReflect)
            {
                 err = cudaMemcpy(&r_vSet[0], this->d_vSet, reflectIndex.size() * sizeof(Vertex), cudaMemcpyDeviceToHost);
                 err = cudaMemcpy((double *)&r_vValueSet[0], this->d_vValueSet, reflectIndex.size() * sizeof(VertexValueType),
                                  cudaMemcpyDeviceToHost);

                 for(int j = 0; j < reflectIndex.size(); j++)
                 {
                     vSet[reflectIndex[j]] = r_vSet[j];
                     vSet[reflectIndex[j]].vertexID = reflectIndex[j];
                     vValues[reflectIndex[j]] = r_vValueSet[j];
                 }
            }

            mGSet.mSet.clear();
            mGCount = 0;
        }
    }

    //Re-package the data

    //Memory copy back
    if(!needReflect)
    {
        err = cudaMemcpy((double *)vValues, this->d_vValueSet, vCount * sizeof(VertexValueType),
                         cudaMemcpyDeviceToHost);

        err = cudaMemcpy(vSet, this->d_vSet, vCount * sizeof(Vertex), cudaMemcpyDeviceToHost);
    }

//    //test
//     std::cout << "========value info========" << std::endl;
//     for(int i = 0; i < vCount; i++)
//     {
//         std::cout << i << " " << vValues[i].first << " " << vValues[i].second << std::endl;
//     }
//     std::cout << "=========value end=======" << std::endl << std::endl;

    //avCount calculation
    int avCount = 0;

    //test
//    for(int i = 0 ; i < vCount; i++)
//    {
//        if(vSet[i].isActive) avCount++;
//    }
//    std::cout << "gpu avCount " << avCount << std::endl;

    return avCount;
}

template <typename VertexValueType, typename MessageValueType>
int PageRankGPU<VertexValueType, MessageValueType>::MSGGenMerge_array(int vCount, int eCount, const Vertex *vSet, const Edge *eSet, int numOfInitV, const int *initVSet, const VertexValueType *vValues, MessageValueType *mValues)
{
    //Generate merged msgs directly

    //Availability check
    if(vCount == 0) return 0;

    //Memory allocation
    cudaError_t err = cudaSuccess;

    //Graph scale check
    bool needReflect = vCount > this->vertexLimit;

    for(int i = 0; i < vCount; i++)
    {
        mValues[i].destVId = -1;
        mValues[i].rank = 0;
    }


    if(!needReflect)
        err = MSGGenMerge_GPU_MVCopy(this->d_vSet, vSet,
                               this->d_vValueSet, (double *)vValues,
                               this->d_mTransformedMergedMSGValueSet,
                               mValues, vCount, eCount);

    //Init for possible reflection
    //-----reflection-----
     bool *tmp_AVCheckList = new bool [vCount];
     auto tmp_o_g = Graph<VertexValueType>(0);

    if(needReflect)
    {
         for(int i = 0; i < vCount; i++) tmp_AVCheckList[i] = vSet[i].isActive;
         tmp_o_g = Graph<VertexValueType>(vCount, 0, numOfInitV, initVSet, nullptr, nullptr, nullptr, tmp_AVCheckList);
         tmp_o_g.verticesValue.reserve(vCount);
         tmp_o_g.verticesValue.insert(tmp_o_g.verticesValue.begin(), vValues, vValues + vCount);
    }

    //This checkpoint is to used to prevent from mistaking mValues gathering in deflection
    bool *isDst = new bool [vCount];
    for(int i = 0; i < vCount; i++) isDst[i] = false;
    //-----reflection-----

    //e batch processing
    int eGCount = 0;

    std::vector<Edge> eGSet = std::vector<Edge>();
    eGSet.reserve(this->ePerEdgeSet);

    for(int i = 0; i < eCount; i++)
    {
        if(vSet[eSet[i].src].isActive && vValues[eSet[i].src].second > this->deltaThreshold)
        {
            eGSet.emplace_back(eSet[i]);
            eGCount++;

            if(needReflect)
            {
                isDst[eSet[i].dst] = true;
            }
        }

        if(eGCount == this->ePerEdgeSet || i == eCount - 1) //A batch of es will be transferred into GPU. Don't forget last batch!
        {

            //-----reflection-----
            auto reflectIndex = std::vector<int>();
            auto reversedIndex = std::vector<int>();
            auto r_g = Graph<VertexValueType>(0);

            if(needReflect)
            {
                r_g = this->reflectG(tmp_o_g, eGSet, reflectIndex, reversedIndex);

                err = MSGGenMerge_GPU_MVCopy(this->d_vSet, &r_g.vList[0],
                        this->d_vValueSet, (double *)&r_g.verticesValue[0],
                        this->d_mTransformedMergedMSGValueSet,
                        this->mTransformedMergedMSGValueSet,
                        r_g.vCount, r_g.eCount);

                err = cudaMemcpy(this->d_eGSet, &r_g.eList[0], eGCount * sizeof(Edge), cudaMemcpyHostToDevice);

            }
            //-----reflection-----
            else
                err = cudaMemcpy(this->d_eGSet, &eGSet[0], eGCount * sizeof(Edge), cudaMemcpyHostToDevice);


            for(int j = 0; j < eGCount; j += NUMOFGPUCORE)
            {
                int edgeNumUsedForExec = (eGCount - j > NUMOFGPUCORE) ? NUMOFGPUCORE : (eGCount - j);

                err = MSGGenMerge_kernel_exec(this->d_mTransformedMergedMSGValueSet, this->d_vSet,
                                                this->d_vValueSet, edgeNumUsedForExec, &this->d_eGSet[j], this->resetProb);
            }

            //Deflection
            if(needReflect)
            {
                 //Re-package the data
                 //Memory copy back
                 err = cudaMemcpy(this->mTransformedMergedMSGValueSet, this->d_mTransformedMergedMSGValueSet,
                                  r_g.vCount * sizeof(MessageValueType), cudaMemcpyDeviceToHost);

                 //Valid message transformed back to original double form (deflection)
                 for (int j = 0; j < r_g.vCount; j++)
                 {
                     int o_dst = reflectIndex[j];
                     //If the v the current msg point to is not a dst, it should not be copied back because the current msg value is not correct)
                     if(isDst[o_dst])
                     {
                         //test
//                         std::cout << "o_dst: " << o_dst << std::endl;
//                         std::cout << j << std::endl;
//                         std::cout << "rank = " << (this->mTransformedMergedMSGValueSet)[j].rank << std::endl;

                         mValues[o_dst].destVId = o_dst;
                         mValues[o_dst].rank += (this->mTransformedMergedMSGValueSet)[j].rank;
                     }
                 }
            }

            //Checkpoint reset
            eGCount = 0;
            eGSet.clear();
            for(int j = 0; j < vCount; j++) isDst[j] = false;
        }
    }

    if(!needReflect)
    {
        //copy back
        err = cudaMemcpy(mValues, this->d_mTransformedMergedMSGValueSet, vCount * sizeof(MessageValueType), cudaMemcpyDeviceToHost);
    }

    return vCount;
}

template<typename VertexValueType, typename MessageValueType>
int PageRankGPU<VertexValueType, MessageValueType>::reflect(const std::vector<int> &originalIntList, int originalIntRange,
                                                        std::vector<int> &reflectIndex, std::vector<int> &reversedIndex)
{
    //Reflection
    int reflectCount = 0;

    for(auto o_i : originalIntList)
    {
        if(reversedIndex.at(o_i) == NO_REFLECTION)
        {
            reflectIndex.emplace_back(o_i);
            reversedIndex.at(o_i) = reflectCount++;
        }
    }

    return reflectCount;
}

template<typename VertexValueType, typename MessageValueType>
Graph<VertexValueType> PageRankGPU<VertexValueType, MessageValueType>::reflectG(const Graph<VertexValueType> &o_g,
                                                                                const std::vector<Edge> &eSet,
                                                                                std::vector<int> &reflectIndex,
                                                                                std::vector<int> &reversedIndex)
{
    //Init
    int vCount = o_g.vCount;
    int eCount = eSet.size();

    reflectIndex.clear();
    reversedIndex.clear();
    reflectIndex.reserve(2 * eCount);
    reversedIndex.reserve(vCount);
    reversedIndex.assign(vCount, NO_REFLECTION);

    //Calculate reflection using eSet and generate reflected eSet
    auto r_eSet = std::vector<Edge>();
    r_eSet.reserve(2 * eCount);

    auto originalIntList = std::vector<int>();
    originalIntList.reserve(2 * eCount);

    for(const auto &e : eSet)
    {
        originalIntList.emplace_back(e.src);
        originalIntList.emplace_back(e.dst);
    }

    int reflectCount = this->reflect(originalIntList, vCount, reflectIndex, reversedIndex);

    //Generate reflected eSet
    for(const auto &e : eSet)
        r_eSet.emplace_back(reversedIndex.at(e.src), reversedIndex.at(e.dst), e.weight);

    //Generate reflected vSet & vValueSet
    auto r_vSet = std::vector<Vertex>();
    r_vSet.reserve(reflectCount * sizeof(Vertex));

    auto r_vValueSet = std::vector<VertexValueType>();
    r_vValueSet.reserve(reflectCount * sizeof(VertexValueType));

    for(int i = 0; i < reflectCount; i++)
    {
        r_vSet.emplace_back(o_g.vList.at(reflectIndex.at(i)));
        r_vValueSet.emplace_back(o_g.verticesValue.at(reflectIndex.at(i)));
        r_vSet.at(i).vertexID = i;
    }

    //Generate reflected graph and return
    return Graph<VertexValueType>(r_vSet, r_eSet, r_vValueSet);
}

template<typename VertexValueType, typename MessageValueType>
MessageSet<MessageValueType> PageRankGPU<VertexValueType, MessageValueType>::reflectM(const MessageSet<MessageValueType> &o_mSet, int vCount,
                                                        std::vector<int> &reflectIndex,std::vector<int> &reversedIndex)
{
    auto r_mSet = MessageSet<MessageValueType>();

    reflectIndex.reserve(o_mSet.mSet.size());
    reversedIndex.reserve(vCount);
    reversedIndex.assign(vCount, NO_REFLECTION);

    auto originalIntList = std::vector<int>();
    originalIntList.reserve(o_mSet.mSet.size());

    for(const auto &m : o_mSet.mSet) originalIntList.emplace_back(m.dst);

    int reflectCount = this->reflect(originalIntList, vCount, reflectIndex, reversedIndex);

    for(auto &m : o_mSet.mSet)
    {
        r_mSet.insertMsg(Message<MessageValueType>(m.src, reversedIndex.at(m.dst), m.value));
    }

    return r_mSet;
}



