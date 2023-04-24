//
// Created by Thoh Testarossa on 2019-03-12.
//

#include "BellmanFordGPU.h"
#include "kernel_src/BellmanFordGPU_kernel.h"

#include <iostream>
#include <algorithm>
#include <chrono>

#define NULLMSG -1

//Transformation function
//This two function is the parts of MSGMerge for adapting CUDA's atomic ops
//double -> long long int
unsigned long long int doubleAsLongLongInt(double a)
{
    unsigned long long int *ptr = (unsigned long long int *)&a;
    return *ptr;
}
//long long int -> double
double longLongIntAsDouble(unsigned long long int a)
{
    double *ptr = (double *)&a;
    return *ptr;
}
//Transformation functions end

//Internal method for different GPU copy situations in BF algo
template <typename VertexValueType, typename MessageValueType>
auto BellmanFordGPU<VertexValueType, MessageValueType>::MSGGenMerge_GPU_MVCopy(Vertex *d_vSet, const Vertex *vSet,
                                   double *d_vValues, const double *vValues,
                                   int eGCount, const Edge *eSet,
                                   unsigned long long int *d_mTransformedMergedMSGValueSet,
                                   unsigned long long int *mTransformedMergedMSGValueSet,
                                   int vGCount, int numOfInitV)
{
    auto start = std::chrono::system_clock::now();

    auto err = cudaSuccess;

    //vSet copy
    if(this->optimize)
    {
        if(!this->isInited)
        {
            err = cudaMemcpy(d_vSet, vSet, vGCount * sizeof(Vertex), cudaMemcpyHostToDevice);
        }
    }
    else
    {
        err = cudaMemcpy(d_vSet, vSet, vGCount * sizeof(Vertex), cudaMemcpyHostToDevice);
    }

    //vValueSet copy
    if(this->optimize)
    {
        if(!this->isInited)
        {
            err = cudaMemcpy(d_vValues, vValues, vGCount * numOfInitV * sizeof(double), cudaMemcpyHostToDevice);
        }
        else
        {
            for(int i = 0; i < eGCount; i++)
            {
                auto srcVID = eSet[i].src;
                auto dstVID = eSet[i].dst;
                err = cudaMemcpy(&d_vValues[srcVID * numOfInitV], &vValues[srcVID * numOfInitV], numOfInitV * sizeof(double), cudaMemcpyHostToDevice);
                err = cudaMemcpy(&d_vValues[dstVID * numOfInitV], &vValues[dstVID * numOfInitV], numOfInitV * sizeof(double), cudaMemcpyHostToDevice);
            }
        }
    }
    else
    {
        err = cudaMemcpy(d_vValues, vValues, vGCount * numOfInitV * sizeof(double), cudaMemcpyHostToDevice);
    }

    //Transform to the long long int form which CUDA can do atomic ops
    //unsigned long long int *mTransformedMergedMSGValueSet = new unsigned long long int [g.vCount * numOfInitV];
    if(this->optimize)
    {
        for(int i = 0; i < eGCount; i++)
        {
            for(int j = 0; j < numOfInitV; j++)
            {
                auto vID = eSet[i].dst;
                mTransformedMergedMSGValueSet[vID * numOfInitV + j] = doubleAsLongLongInt((double) INVALID_MASSAGE);

                err = cudaMemcpy(&d_mTransformedMergedMSGValueSet[vID * numOfInitV + j],
                                 &mTransformedMergedMSGValueSet[vID * numOfInitV + j],
                                 sizeof(unsigned long long int), cudaMemcpyHostToDevice);
            }
        }

    }
    else
    {
        for (int i = 0; i < vGCount * numOfInitV; i++)
            mTransformedMergedMSGValueSet[i] = doubleAsLongLongInt((double) INVALID_MASSAGE);

        err = cudaMemcpy(d_mTransformedMergedMSGValueSet, mTransformedMergedMSGValueSet,
                         vGCount * numOfInitV * sizeof(unsigned long long int), cudaMemcpyHostToDevice);
    }

    auto end = std::chrono::system_clock::now();
    std::cout << "MSGGenMerge_GPU_MVCopy time : " <<  std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;

    return err;
}

template <typename VertexValueType, typename MessageValueType>
auto BellmanFordGPU<VertexValueType, MessageValueType>::MSGApply_GPU_VVCopy(Vertex *d_vSet, const Vertex *vSet,
                                double *d_vValues, const double *vValues,
                                int vGCount, int numOfInitV)
{

    auto start = std::chrono::system_clock::now();

    auto err = cudaSuccess;

    //vSet copy
    if(this->optimize)
    {
        if(!this->isInited)
        {
            err = cudaMemcpy(d_vSet, vSet, vGCount * sizeof(Vertex), cudaMemcpyHostToDevice);
        }
    }
    else
    {
        err = cudaMemcpy(d_vSet, vSet, vGCount * sizeof(Vertex), cudaMemcpyHostToDevice);
        err = cudaMemcpy(d_vValues, vValues, vGCount * numOfInitV * sizeof(double), cudaMemcpyHostToDevice);
    }

    auto end = std::chrono::system_clock::now();
    std::cout << "MSGApply_GPU_VVCopy time : " <<  std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;

    return err;
}

template <typename VertexValueType, typename MessageValueType>
BellmanFordGPU<VertexValueType, MessageValueType>::BellmanFordGPU()
{
    this->isInited = false;
}

template <typename VertexValueType, typename MessageValueType>
void BellmanFordGPU<VertexValueType, MessageValueType>::Init(int vCount, int eCount, int numOfInitV)
{
    BellmanFord<VertexValueType, MessageValueType>::Init(vCount, eCount, numOfInitV);

    this->vertexLimit = VERTEXSCALEINGPU;
    this->mPerMSGSet = MSGSCALEINGPU;
    this->ePerEdgeSet = EDGESCALEINGPU;
}

template <typename VertexValueType, typename MessageValueType>
void BellmanFordGPU<VertexValueType, MessageValueType>::GraphInit(Graph<VertexValueType> &g, std::set<int> &activeVertices, const std::vector<int> &initVList)
{
    BellmanFord<VertexValueType, MessageValueType>::GraphInit(g, activeVertices, initVList);
}

template <typename VertexValueType, typename MessageValueType>
void BellmanFordGPU<VertexValueType, MessageValueType>::Deploy(int vCount, int eCount, int numOfInitV)
{
    BellmanFord<VertexValueType, MessageValueType>::Deploy(vCount, eCount, numOfInitV);

    cudaError_t err = cudaSuccess;

    this->initVSet = new int [numOfInitV];
    err = cudaMalloc((void **)&this->d_initVSet, this->numOfInitV * sizeof(int));

    this->vValueSet = new VertexValueType [vCount * this->numOfInitV];
    err = cudaMalloc((void **)&this->d_vValueSet, vertexLimit * this->numOfInitV * sizeof(double));

    err = cudaMalloc((void **)&this->d_vSet, vertexLimit * sizeof(Vertex));
    err = cudaMalloc((void **)&this->d_eGSet, ePerEdgeSet * sizeof(Edge));

    int mSize = std::max(this->numOfInitV * ePerEdgeSet, mPerMSGSet);

    this->mInitVIndexSet = new int [vertexLimit * this->numOfInitV];
    err = cudaMalloc((void **)&this->d_mInitVIndexSet, mSize * sizeof(int));
    this->mDstSet = new int [vertexLimit * this->numOfInitV];
    err = cudaMalloc((void **)&this->d_mDstSet, mSize * sizeof(int));
    this->mValueSet = new MessageValueType [mSize];
    err = cudaMalloc((void **)&this->d_mValueSet, mSize * sizeof(double));

    this->avSet = new int [vertexLimit];
    this->isDst = new bool[vertexLimit];

    this->mValueSetOpt = new MessageValueType [vertexLimit * numOfInitV];

    this->mTransformedMergedMSGValueSet = new unsigned long long int [vertexLimit * numOfInitV];
    err = cudaMalloc((void **)&d_mTransformedMergedMSGValueSet, numOfInitV * vertexLimit * sizeof(unsigned long long int));
}

template <typename VertexValueType, typename MessageValueType>
void BellmanFordGPU<VertexValueType, MessageValueType>::Free()
{
    BellmanFord<VertexValueType, MessageValueType>::Free();

    free(this->initVSet);
    cudaFree(this->d_initVSet);

    free(this->vValueSet);
    cudaFree(this->d_vValueSet);

    cudaFree(this->d_vSet);
    cudaFree(this->d_eGSet);

    free(this->mInitVIndexSet);
    cudaFree(this->d_mInitVIndexSet);
    free(this->mDstSet);
    cudaFree(this->d_mDstSet);
    free(this->mValueSet);
    cudaFree(this->d_mValueSet);

    free(this->mTransformedMergedMSGValueSet);
    cudaFree(this->d_mTransformedMergedMSGValueSet);

    free(this->isDst);
    free(this->avSet);
    free(this->mValueSetOpt);
}

template <typename VertexValueType, typename MessageValueType>
int BellmanFordGPU<VertexValueType, MessageValueType>::MSGApply_array(int vCount, int eCount, Vertex *vSet, int numOfInitV, const int *initVSet, VertexValueType *vValues, MessageValueType *mValues)
{
    std::cout << "=============MSGApply_array time=============" << std::endl;

    //Availability check
    if(vCount == 0) return 0;

    auto start = std::chrono::system_clock::now();

    //CUDA init
    cudaError_t err = cudaSuccess;

    //initVSet Init
    if(this->optimize)
    {
        if(!this->isInited)
            err = cudaMemcpy(this->d_initVSet, initVSet, numOfInitV * sizeof(int), cudaMemcpyHostToDevice);
    }
    else
    {
        err = cudaMemcpy(this->d_initVSet, initVSet, numOfInitV * sizeof(int), cudaMemcpyHostToDevice);
    }

    bool needReflect = vCount > this->vertexLimit;

    //AVCheck
    if(this->optimize)
    {
        if(!this->isInited)
        {
            for (int i = 0; i < vCount; i++) vSet[i].isActive = false;
        }
        else
        {
            for(int i = 0; i < this->avCount; i++)
            {
                auto vID = this->avSet[i];
                vSet[vID].isActive = false;
                err = cudaMemcpy(&this->d_vSet[vID], &vSet[vID], sizeof(Vertex), cudaMemcpyHostToDevice);
            }
        }
    }
    else
    {
        for (int i = 0; i < vCount; i++) vSet[i].isActive = false;
    }

    auto end = std::chrono::system_clock::now();
    std::cout << "MSGApply_array init time: " <<  std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;

    if(!needReflect)
    {
        err = MSGApply_GPU_VVCopy(this->d_vSet, vSet,
                            this->d_vValueSet, (double *)vValues,
                            vCount, numOfInitV);
    }

    //Apply msgs to v
    int mGCount = 0;
    auto mGSet = MessageSet<MessageValueType>();

    auto r_mGSet = MessageSet<MessageValueType>();
    auto r_vSet = std::vector<Vertex>();
    auto r_vValueSet = std::vector<VertexValueType>();

    start = std::chrono::system_clock::now();

    if(!this->optimize)
    {
        eCount = numOfInitV * vCount;
    }

    for(int i = 0; i < eCount; i++)
    {
        if(this->optimize)
        {
            mGSet.insertMsg(Message<MessageValueType>(initVSet[this->mInitVIndexSet[i]], this->mDstSet[i], this->mValueSetOpt[i]));
            mGCount++;
        }
        else
        {
            if(mValues[i] != (MessageValueType)INVALID_MASSAGE && vSet[i / numOfInitV].isMaster)
            {
                mGSet.insertMsg(Message<MessageValueType>(initVSet[i % numOfInitV], i / numOfInitV, mValues[i]));
                mGCount++;
            }
        }
        if(mGCount == this->mPerMSGSet || i == eCount - 1)
        {
            end = std::chrono::system_clock::now();
            std::cout << "Adding msgs to batches time: " <<  std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;
            start = std::chrono::system_clock::now();

            auto reflectIndex = std::vector<int>();
            auto reversedIndex = std::vector<int>();

            //Reflection for message & vertex & vValues
            if(needReflect)
            {
                //MSG reflection
                r_mGSet = this->reflectM(mGSet, vCount, reflectIndex, reversedIndex);

                for(int j = 0; j < r_mGSet.mSet.size(); j++)
                {
                    this->mInitVIndexSet[j] = vSet[r_mGSet.mSet.at(j).src].initVIndex;
                    this->mDstSet[j] = r_mGSet.mSet.at(j).dst;
                    this->mValueSet[j] = r_mGSet.mSet.at(j).value;
                }

                //v reflection
                r_vSet.clear();
                for(int j = 0; j < reflectIndex.size(); j++)
                    r_vSet.emplace_back(j, false, vSet[reflectIndex.at(j)].initVIndex);

                //vValue reflection
                r_vValueSet.clear();
                r_vValueSet.reserve(mPerMSGSet * numOfInitV);
                r_vValueSet.assign(mPerMSGSet * numOfInitV, INT32_MAX >> 1);
                for(int j = 0; j < reflectIndex.size(); j++)
                {
                    for(int k = 0; k < numOfInitV; k++)
                        r_vValueSet.at(j * numOfInitV + k) = vValues[reflectIndex[j] * numOfInitV + k];
                }

                //vSet & vValueSet Init
                err = MSGApply_GPU_VVCopy(d_vSet, &r_vSet[0],
                                    d_vValueSet, (double *)&r_vValueSet[0],
                                    reflectIndex.size(), numOfInitV);
            }
            else
            {
                //Use original msg
                for(int j = 0; j < mGSet.mSet.size(); j++)
                {
                    this->mInitVIndexSet[j] = vSet[mGSet.mSet.at(j).src].initVIndex;
                    this->mDstSet[j] = mGSet.mSet.at(j).dst;
                    this->mValueSet[j] = mGSet.mSet.at(j).value;
                }
            }

            //MSG memory copy
            err = cudaMemcpy(this->d_mInitVIndexSet, this->mInitVIndexSet, mGCount * sizeof(int), cudaMemcpyHostToDevice);
            err = cudaMemcpy(this->d_mDstSet, this->mDstSet, mGCount * sizeof(int), cudaMemcpyHostToDevice);
            err = cudaMemcpy(this->d_mValueSet, (double *)this->mValueSet, mGCount * sizeof(double), cudaMemcpyHostToDevice);

            end = std::chrono::system_clock::now();
            std::cout << "batch pre-processing time: " <<  std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;

            start = std::chrono::system_clock::now();
            //Kernel Execution
            for(int j = 0; j < mGCount; j += NUMOFGPUCORE)
            {
                int msgNumUsedForExec = (mGCount - j > NUMOFGPUCORE) ? NUMOFGPUCORE : (mGCount - j);

                err = MSGApply_kernel_exec(this->d_vSet, numOfInitV, this->d_initVSet, this->d_vValueSet, msgNumUsedForExec,
                                           &this->d_mDstSet[j], &this->d_mInitVIndexSet[j], &this->d_mValueSet[j]);
            }
            end = std::chrono::system_clock::now();
            std::cout << "kernel Execution time: " <<  std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;

            //Deflection
            if(needReflect)
            {
                err = cudaMemcpy(&r_vSet[0], this->d_vSet, reflectIndex.size() * sizeof(Vertex), cudaMemcpyDeviceToHost);
                err = cudaMemcpy((double *)&r_vValueSet[0], this->d_vValueSet, reflectIndex.size() * numOfInitV * sizeof(double),
                                 cudaMemcpyDeviceToHost);

                for(int j = 0; j < reflectIndex.size(); j++)
                {
                    vSet[reflectIndex[j]] = r_vSet[j];
                    //Don't forget to deflect vertexID in Vertex obj!!
                    vSet[reflectIndex[j]].vertexID = reflectIndex[j];
                    for(int k = 0; k < numOfInitV; k++)
                        vValues[reflectIndex[j] * numOfInitV + k] = r_vValueSet[j * numOfInitV + k];
                }
            }

            if(!needReflect)
            {
                if(this->optimize)
                {
                    for(int j = 0; j < mGCount; j++)
                    {
                        auto vID = this->mDstSet[j];

                        err = cudaMemcpy(&vSet[vID], &this->d_vSet[vID], sizeof(Vertex), cudaMemcpyDeviceToHost);
                        err = cudaMemcpy(&vValues[vID * numOfInitV], &this->d_vValueSet[vID * numOfInitV],
                                         numOfInitV * sizeof(double), cudaMemcpyDeviceToHost);
                    }
                }
            }

            mGSet.mSet.clear();
            mGCount = 0;
            start = std::chrono::system_clock::now();
        }
    }

    //Re-package the data


    start = std::chrono::system_clock::now();

    this->isInited = true;

    if(this->optimize)
    {
        end = std::chrono::system_clock::now();
        std::cout << "copyback time: " <<  std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;
    }
    else
    {
        //Memory copy back
        if(!needReflect)
        {
            err = cudaMemcpy(vSet, this->d_vSet, vCount * sizeof(Vertex), cudaMemcpyDeviceToHost);
            err = cudaMemcpy((double *)vValues, this->d_vValueSet, vCount * numOfInitV * sizeof(double),
                             cudaMemcpyDeviceToHost);
        }
        end = std::chrono::system_clock::now();
        std::cout << "copyback time: " <<  std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;

        start = std::chrono::system_clock::now();

        //avCount calculation
        this->avCount = 0;
        for(int i = 0; i < vCount; i++)
        {
            if(vSet[i].isActive)
                this->avCount++;
        }
        std::cout << "gpu avcount " << this->avCount << std::endl;

        end = std::chrono::system_clock::now();
        std::cout << "avCount calculation time: " <<  std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;
    }


    return this->avCount;
}

template <typename VertexValueType, typename MessageValueType>
int BellmanFordGPU<VertexValueType, MessageValueType>::MSGGenMerge_array(int vCount, int eCount, const Vertex *vSet, const Edge *eSet, int numOfInitV, const int *initVSet, const VertexValueType *vValues, MessageValueType *mValues)
{
    std::cout << "=============MSGGenMerge_array time=============" << std::endl;
    //Generate merged msgs directly

    //Availability check
    if(vCount == 0) return 0;

    auto start = std::chrono::system_clock::now();

    //Invalid message init
    if(this->optimize)
    {
        for(int i = 0; i < eCount; i++)
        {
            for(int j = 0; j < numOfInitV; j++)
            {
                auto vID = eSet[i].dst;
                mValues[vID * numOfInitV + j] = MessageValueType(INVALID_MASSAGE);
            }
        }
    }
    else
    {
        for(int i = 0; i < vCount * numOfInitV; i++) mValues[i] = (MessageValueType)INVALID_MASSAGE;
    }

    //Memory allocation
    cudaError_t err = cudaSuccess;

    //initVSet Init
    err = cudaMemcpy(this->d_initVSet, initVSet, numOfInitV * sizeof(int), cudaMemcpyHostToDevice);

    //Graph scale check
    bool needReflect = vCount > this->vertexLimit;

    auto end = std::chrono::system_clock::now();
    std::cout << "init time: " <<  std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;

    if(!needReflect)
        err = MSGGenMerge_GPU_MVCopy(this->d_vSet, vSet,
                               this->d_vValueSet, (double *)vValues,
                               eCount, eSet,
                               this->d_mTransformedMergedMSGValueSet,
                               this->mTransformedMergedMSGValueSet,
                               vCount, numOfInitV);

    //Init for possible reflection
    //Maybe can use lambda style?
    bool *tmp_AVCheckList = new bool [vCount];
    auto tmp_o_g = Graph<VertexValueType>(0);
    if(needReflect)
    {
        for(int i = 0; i < vCount; i++) tmp_AVCheckList[i] = vSet[i].isActive;
        tmp_o_g = Graph<VertexValueType>(vCount, 0, numOfInitV, initVSet, nullptr, nullptr, nullptr, tmp_AVCheckList);
        tmp_o_g.verticesValue.reserve(vCount * numOfInitV);
        tmp_o_g.verticesValue.insert(tmp_o_g.verticesValue.begin(), vValues, vValues + (numOfInitV * vCount));
    }

    //This checkpoint is to used to prevent from mistaking mValues gathering in deflection
    if(this->optimize)
    {
        if(!this->isInited)
        {
            for(int i = 0; i < vCount; i++) this->isDst[i] = false;
        }
    }
    else
    {
        for(int i = 0; i < vCount; i++) this->isDst[i] = false;
    }

    //e batch processing
    int eGCount = 0;

    std::vector<Edge> eGSet = std::vector<Edge>();
    eGSet.reserve(this->ePerEdgeSet);

    start = std::chrono::system_clock::now();

    for(int i = 0; i < eCount; i++)
    {
        if(vSet[eSet[i].src].isActive) //Add es to batches
        {
            for(int j = 0; j < numOfInitV; j++)
            {
                if(vValues[eSet[i].src * numOfInitV + j] + (VertexValueType)eSet[i].weight < vValues[eSet[i].dst * numOfInitV + j])
                {
                    eGSet.emplace_back(eSet[i]);
                    eGCount++;
                    //Only dst receives message
                    this->isDst[eSet[i].dst] = true;

                    break;
                }
            }
        }
        if(eGCount == this->ePerEdgeSet || i == eCount - 1) //A batch of es will be transferred into GPU. Don't forget last batch!
        {
            end = std::chrono::system_clock::now();
            std::cout << "add eSet to batches time : " <<  std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;
            auto reflectIndex = std::vector<int>();
            auto reversedIndex = std::vector<int>();

            auto r_g = Graph<VertexValueType>(0);

            start = std::chrono::system_clock::now();

            //Reflection
            if(needReflect)
            {
                r_g = this->reflectG(tmp_o_g, eGSet, reflectIndex, reversedIndex);

                err = MSGGenMerge_GPU_MVCopy(this->d_vSet, &r_g.vList[0],
                                             this->d_vValueSet, (double *)&r_g.verticesValue[0],
                                             eGCount, this->d_eGSet,
                                             this->d_mTransformedMergedMSGValueSet,
                                             this->mTransformedMergedMSGValueSet,
                                             r_g.vCount, numOfInitV);

                err = cudaMemcpy(this->d_eGSet, &r_g.eList[0], eGCount * sizeof(Edge), cudaMemcpyHostToDevice);
            }
            else
                err = cudaMemcpy(this->d_eGSet, &eGSet[0], eGCount * sizeof(Edge), cudaMemcpyHostToDevice);

            end = std::chrono::system_clock::now();
            std::cout << "batch pre-processing time : " <<  std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;

            start = std::chrono::system_clock::now();
            //Kernel Execution (no matter whether g is reflected or not)
            for(int j = 0; j < eGCount; j += NUMOFGPUCORE)
            {
                int edgeNumUsedForExec = (eGCount - j > NUMOFGPUCORE) ? NUMOFGPUCORE : (eGCount - j);

                err = MSGGenMerge_kernel_exec(this->d_mTransformedMergedMSGValueSet, this->d_vSet, numOfInitV,
                                              this->d_initVSet, this->d_vValueSet, edgeNumUsedForExec, &this->d_eGSet[j]);
            }
            end = std::chrono::system_clock::now();
            std::cout << "kernel exec time : " <<  std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;

            //Deflection
            if(needReflect)
            {
                //Re-package the data
                //Memory copy back
                err = cudaMemcpy(this->mTransformedMergedMSGValueSet, this->d_mTransformedMergedMSGValueSet,
                                 r_g.vCount * numOfInitV * sizeof(unsigned long long int), cudaMemcpyDeviceToHost);

                //Valid message transformed back to original double form (deflection)
                for (int j = 0; j < r_g.vCount * numOfInitV; j++)
                {
                    int o_dst = reflectIndex[j / numOfInitV];
                    //If the v the current msg point to is not a dst, it should not be copied back because the current msg value is not correct)
                    if(this->isDst[o_dst])
                    {
                        if(mValues[o_dst * numOfInitV + j % numOfInitV] > (MessageValueType) (longLongIntAsDouble(this->mTransformedMergedMSGValueSet[j])))
                            mValues[o_dst * numOfInitV + j % numOfInitV] = (MessageValueType) (longLongIntAsDouble(
                                this->mTransformedMergedMSGValueSet[j]));
                    }
                }
            }
            else;

            start = std::chrono::system_clock::now();
            //Checkpoint reset

            if(this->optimize)
            {
                for(int j = 0; j < eGCount; j++)
                {
                    this->isDst[eGSet.at(j).dst] = false;
                }
            }
            else
            {
                for(int j = 0; j < vCount; j++) this->isDst[j] = false;
            }

            eGCount = 0;
            eGSet.clear();

            end = std::chrono::system_clock::now();
            std::cout << "batch post-processing time : " <<  std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;

            start = std::chrono::system_clock::now();
        }
    }

    start = std::chrono::system_clock::now();
    int msgCnt = 0;
    if(!needReflect)
    {
        //Re-package the data
        //Memory copy back

        if(this->optimize)
        {
            for(int i = 0; i < eCount; i++)
            {
                for(int j = 0; j < numOfInitV; j++)
                {
                    auto vID = eSet[i].dst;

                    err = cudaMemcpy(&this->mTransformedMergedMSGValueSet[vID * numOfInitV + j],
                                     &this->d_mTransformedMergedMSGValueSet[vID * numOfInitV + j],
                                     sizeof(unsigned long long int), cudaMemcpyDeviceToHost);

                    if(this->mTransformedMergedMSGValueSet[vID * numOfInitV + j] != INVALID_MASSAGE)
                    {
                        this->mValueSetOpt[msgCnt] = (MessageValueType)(longLongIntAsDouble(this->mTransformedMergedMSGValueSet[vID * numOfInitV + j]));
                        mValues[vID * numOfInitV + j] = (MessageValueType)(longLongIntAsDouble(this->mTransformedMergedMSGValueSet[vID * numOfInitV + j]));
                        this->mInitVIndexSet[msgCnt] = j;
                        this->mDstSet[msgCnt] = vID;
                        msgCnt++;
                    }
                }
            }

            this->avCount = 0;
            for(int i = 0; i < vCount; i++)
            {
                if(vSet[i].isActive)
                {
                    this->avSet[this->avCount] = i;
                    this->avCount++;
                }
            }
        }
        else
        {
            err = cudaMemcpy(mValues, this->d_mTransformedMergedMSGValueSet,
                             vCount * numOfInitV * sizeof(unsigned long long int), cudaMemcpyDeviceToHost);
            msgCnt = numOfInitV * vCount;
        }

    }
    end = std::chrono::system_clock::now();
    std::cout << "copy-back time : " <<  std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;

    std::cout << std::endl;

    return msgCnt;
}
