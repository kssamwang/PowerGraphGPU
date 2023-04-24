//
// Created by Thoh Testarossa on 2019-04-05.
//

#include "../srv/UtilServer.h"
#include "../util/TIsExtended.hpp"
#include <string>
#include <iostream>
#include <chrono>

template <typename GraphUtilType, typename VertexValueType, typename MessageValueType>
UtilServer<GraphUtilType, VertexValueType, MessageValueType>::UtilServer(int vCount, int eCount, int numOfInitV, int nodeNo)
{
    //Test
    std::cout << "Server init" << std::endl;
    //Test end

    this->nodeNo = nodeNo;

    this->vCount = vCount;
    this->eCount = eCount;
    this->numOfInitV = numOfInitV;

    this->isLegal = TIsExtended<GraphUtilType, GraphUtil<VertexValueType, MessageValueType>>::Result &&
                    vCount > 0 &&
                    eCount > 0 &&
                    numOfInitV > 0 &&
                    nodeNo >= 0;

    this->vValues = nullptr;
    this->mValues = nullptr;
    this->vSet = nullptr;
    this->eSet = nullptr;
    this->initVSet = nullptr;
    this->filteredV = nullptr;
    this->timestamp = nullptr;

    this->avESet = new Edge[this->eCount];

    if(this->isLegal)
    {
        int chk = 0;

        this->executor = GraphUtilType();
        this->executor.Init(vCount, eCount, numOfInitV);
        this->executor.partitionId = nodeNo;
        this->vValues_shm = UNIX_shm();
        this->vSet_shm = UNIX_shm();
        this->eSet_shm = UNIX_shm();
        this->initVSet_shm = UNIX_shm();
        this->filteredV_shm = UNIX_shm();
        this->timestamp_shm = UNIX_shm();
        this->avCount_shm = UNIX_shm();
        this->avSet_shm = UNIX_shm();

        this->server_msq = UNIX_msg();
        this->init_msq = UNIX_msg();
        this->client_msq = UNIX_msg();

        if(chk != -1)
            chk = this->vValues_shm.create(((this->nodeNo << NODE_NUM_OFFSET) | (VVALUES_SHM << SHM_OFFSET)),
                this->executor.totalVValuesCount * sizeof(VertexValueType),
                0666);
        if(chk != -1)
            chk = this->mValues_shm.create(((this->nodeNo << NODE_NUM_OFFSET) | (MVALUES_SHM << SHM_OFFSET)),
                this->executor.totalMValuesCount * sizeof(MessageValueType),
                0666);
        if(chk != -1)
            chk = this->vSet_shm.create(((this->nodeNo << NODE_NUM_OFFSET) | (VSET_SHM << SHM_OFFSET)),
                this->vCount * sizeof(Vertex),
                0666);
        if(chk != -1)
            chk = this->eSet_shm.create(((this->nodeNo << NODE_NUM_OFFSET) | (ESET_SHM << SHM_OFFSET)),
                this->eCount * sizeof(Edge),
                0666);
        if(chk != -1)
            chk = this->initVSet_shm.create(((this->nodeNo << NODE_NUM_OFFSET) | (INITVSET_SHM << SHM_OFFSET)),
                this->numOfInitV * sizeof(int),
                0666);
        if(chk != -1)
            chk = this->filteredV_shm.create(((this->nodeNo << NODE_NUM_OFFSET) | (FILTEREDV_SHM << SHM_OFFSET)),
                this->vCount * sizeof(bool),
                0666);
        if(chk != -1)
            chk = this->timestamp_shm.create(((this->nodeNo << NODE_NUM_OFFSET) | (TIMESTAMP_SHM << SHM_OFFSET)),
                this->vCount * sizeof(int),
                0666);
        if(chk != -1)
            chk = this->avSet_shm.create(((this->nodeNo << NODE_NUM_OFFSET) | (AVSET_SHM << SHM_OFFSET)),
                this->vCount * sizeof(int),
                0666);
        if(chk != -1)
            chk = this->avCount_shm.create(((this->nodeNo << NODE_NUM_OFFSET) | (AVCOUNT_SHM << SHM_OFFSET)),
                sizeof(int),
                0666);


        if(chk != -1)
            chk = this->server_msq.create(((this->nodeNo << NODE_NUM_OFFSET) | (SRV_MSG_TYPE << MSG_TYPE_OFFSET)),
                0666);
        if(chk != -1)
            chk = this->init_msq.create(((this->nodeNo << NODE_NUM_OFFSET) | (INIT_MSG_TYPE << MSG_TYPE_OFFSET)),
                0666);
        if(chk != -1)
            chk = this->client_msq.create(((this->nodeNo << NODE_NUM_OFFSET) | (CLI_MSG_TYPE << MSG_TYPE_OFFSET)),
                0666);

        if(chk != -1)
        {
            this->vValues_shm.attach(0666);
            this->mValues_shm.attach(0666);
            this->vSet_shm.attach(0666);
            this->eSet_shm.attach(0666);
            this->initVSet_shm.attach(0666);
            this->filteredV_shm.attach(0666);
            this->timestamp_shm.attach(0666);
            this->avSet_shm.attach(0666);
            this->avCount_shm.attach(0666);

            this->vValues = (VertexValueType *) this->vValues_shm.shmaddr;
            this->mValues = (MessageValueType *) this->mValues_shm.shmaddr;
            this->vSet = (Vertex *) this->vSet_shm.shmaddr;
            this->eSet = (Edge *) this->eSet_shm.shmaddr;
            this->initVSet = (int *) this->initVSet_shm.shmaddr;
            this->filteredV = (bool *) this->filteredV_shm.shmaddr;
            this->timestamp = (int *) this->timestamp_shm.shmaddr;
            this->avSet = (int *) this->avSet_shm.shmaddr;
            this->avCount = (int *) this->avCount_shm.shmaddr;

            this->init_msq.send("initiated", (INIT_MSG_TYPE << MSG_TYPE_OFFSET), 256);

            //Test
            std::cout << "Init succeeded." << std::endl;
            //Test end

            this->executor.Deploy(vCount, eCount, numOfInitV);
        }
        else
        {
            this->isLegal = false;

            //Test
            std::cout << "Init failed with errno " << errno << std::endl;
            //Test end
        }
    }
}

template <typename GraphUtilType, typename VertexValueType, typename MessageValueType>
UtilServer<GraphUtilType, VertexValueType, MessageValueType>::~UtilServer()
{
    this->executor.Free();

    delete this->avESet;

    this->vValues = nullptr;
    this->mValues = nullptr;
    this->vSet = nullptr;
    this->eSet = nullptr;
    this->initVSet = nullptr;
    this->filteredV = nullptr;
    this->timestamp = nullptr;

    this->vValues_shm.control(IPC_RMID);
    this->mValues_shm.control(IPC_RMID);
    this->vSet_shm.control(IPC_RMID);
    this->eSet_shm.control(IPC_RMID);
    this->initVSet_shm.control(IPC_RMID);
    this->filteredV_shm.control(IPC_RMID);
    this->timestamp_shm.control(IPC_RMID);
    this->avSet_shm.control(IPC_RMID);
    this->avCount_shm.control(IPC_RMID);

    this->server_msq.control(IPC_RMID);
    this->init_msq.control(IPC_RMID);
    this->client_msq.control(IPC_RMID);
}

template <typename GraphUtilType, typename VertexValueType, typename MessageValueType>
void UtilServer<GraphUtilType, VertexValueType, MessageValueType>::run()
{
    if(!this->isLegal) return;

    //VertexValueType *mValues = new VertexValueType [this->vCount * this->numOfInitV];
    char msgp[256];
    std::string cmd = std::string("");

    int iterCount = 0;

    int msgCount = 0;
    int avCount = 0;

    //test
    int total = 0;

    while(this->client_msq.recv(msgp, (CLI_MSG_TYPE << MSG_TYPE_OFFSET), 256) != -1)
    {
        //Test
        std::cout << "Processing at iter " << ++iterCount << std::endl;
        //Test end

        cmd = msgp;
        if(std::string("init") == cmd)
        {
            auto start = std::chrono::system_clock::now();

            if(this->executor.optimize)
                this->graphInit();
            this->server_msq.send("finished", (SRV_MSG_TYPE << MSG_TYPE_OFFSET), 256);

            auto end = std::chrono::system_clock::now();

            std::cout << "server init time : " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;

            total += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        }
        else if(std::string("execute_msg_apply") == cmd)
        {
            std::cout << "apply message" << std::endl;

            auto start = std::chrono::system_clock::now();

            avCount = this->executor.MSGApply_array(this->vCount, this->eCount, this->vSet, this->numOfInitV, this->initVSet, this->vValues, this->mValues);

            auto applyEnd = std::chrono::system_clock::now();

            std::cout << "apply time: " <<  std::chrono::duration_cast<std::chrono::microseconds>(applyEnd - start).count() << std::endl;

            this->server_msq.send("finished msg apply", (SRV_MSG_TYPE << MSG_TYPE_OFFSET), 256);
        }
        else if(std::string("execute_msg_merge") == cmd)
        {
            std::cout << "merge message" << std::endl;

            this->executor.optimize = this->getEdgesFromAvSet();

            auto start = std::chrono::system_clock::now();

            if(this->executor.optimize)
                msgCount = this->executor.MSGGenMerge_array(this->vCount, this->avECount, this->vSet, this->avESet, this->numOfInitV, this->initVSet, this->vValues, this->mValues);
            else
                msgCount = this->executor.MSGGenMerge_array(this->vCount, this->eCount, this->vSet, this->eSet, this->numOfInitV, this->initVSet, this->vValues, this->mValues);

            auto mergeEnd = std::chrono::system_clock::now();
            std::cout << "msg gen time: " <<  std::chrono::duration_cast<std::chrono::microseconds>(mergeEnd - start).count() << std::endl;

            this->server_msq.send("finished msg merge", (SRV_MSG_TYPE << MSG_TYPE_OFFSET), 256);
        }
        else if(std::string("exit") == cmd)
            break;
    }

    //Test
    std::cout << "total time: " << total << std::endl;
    std::cout << "Shutdown properly" << std::endl;
    //Test end
}

template<typename GraphUtilType, typename VertexValueType, typename MessageValueType>
void UtilServer<GraphUtilType, VertexValueType, MessageValueType>::graphInit()
{
    this->adjacencyTable.reserve(this->vCount);
    this->adjacencyTable.assign(this->vCount, std::vector<Edge>());

    for(int i = 0; i < this->vCount; i++)
    {
        this->adjacencyTable.at(i).reserve(this->vSet[i].outDegree);
    }

    for(int i = 0; i < this->eCount; i++)
    {
        auto edge = this->eSet[i];
        this->adjacencyTable.at(edge.src).emplace_back(edge.src, edge.dst, edge.weight);
    }
}

template<typename GraphUtilType, typename VertexValueType, typename MessageValueType>
bool UtilServer<GraphUtilType, VertexValueType, MessageValueType>::getEdgesFromAvSet()
{
    if(this->adjacencyTable.empty() || *(this->avCount) <= 0 || *(this->avCount) > (this->vCount >> 1))
    {
        return false;
    }

    this->avECount = 0;

    for(int i = 0; i < *(this->avCount); i++)
    {
        int avID = this->avSet[i];

        for(auto edge : this->adjacencyTable.at(avID))
        {
            this->avESet[this->avECount] = edge;
            this->avECount++;
        }
    }

    return true;
}

