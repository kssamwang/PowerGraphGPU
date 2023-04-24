#include "PageRankGPU_kernel.h"

__global__ void MSGApply_kernel(Vertex *vSet, double *vValues, int numOfMsg, int *mDstSet, PRA_MSG *mValueSet)
{
	int tid = threadIdx.x;

	if(tid < numOfMsg)
	{
        int vID = mDstSet[tid];

		vSet[vID].isActive = true;
		vValues[(vID << 1)] += mValueSet[tid].rank;
		vValues[(vID << 1) + 1] = mValueSet[tid].rank;
	}
}

cudaError_t MSGApply_kernel_exec(Vertex *vSet, double *vValues, int numOfMsg, int *mDstSet, PRA_MSG *mValueSet)
{
	cudaError_t err = cudaSuccess;

	MSGApply_kernel<<<1, NUMOFGPUCORE>>>(vSet, vValues, numOfMsg, mDstSet, mValueSet);
    err = cudaGetLastError();

	cudaDeviceSynchronize();

	return err;
}

__global__ void MSGGenMerge_kernel(PRA_MSG *mTransformdMergedMSGValueSet,
	Vertex *vSet, double *vValues, int numOfEdge, Edge *eSet, double resetProb)
{
	int tid = threadIdx.x;

	if(tid < numOfEdge)
	{
		int srcVid = eSet[tid].src;
		int mValueIndex = eSet[tid].dst;

        //test
//         printf("msg - srcVid: %d destVid: %d\n", srcVid, eSet[tid].dst);
//         printf("vValue: %f weight %f\n", vValues[(srcVid << 1) + 1], eSet[tid].weight);
//         printf("mValueIndex = %d\n", mValueIndex);

        mTransformdMergedMSGValueSet[mValueIndex].destVId = mValueIndex;
        atomicAdd(&mTransformdMergedMSGValueSet[mValueIndex].rank, vValues[(srcVid << 1) + 1] * eSet[tid].weight * (1.0 - resetProb));
	}
}

cudaError_t MSGGenMerge_kernel_exec(PRA_MSG *mTransformdMergedMSGValueSet,
	Vertex *vSet, double *vValues, int numOfEdge, Edge *eSet, double resetProb)
{
	cudaError_t err = cudaSuccess;

	MSGGenMerge_kernel<<<1, NUMOFGPUCORE>>>(mTransformdMergedMSGValueSet, vSet, vValues, numOfEdge, eSet, resetProb);
	err = cudaGetLastError();

	cudaDeviceSynchronize();

	return err;
}