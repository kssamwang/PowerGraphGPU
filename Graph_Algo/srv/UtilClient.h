//
// Created by Thoh Testarossa on 2019-04-05.
//

#pragma once

#ifndef GRAPH_ALGO_UTILCLIENT_H
#define GRAPH_ALGO_UTILCLIENT_H

#include "../core/GraphUtil.h"
#include "../srv/UNIX_shm.h"
#include "../srv/UNIX_msg.h"
#include "../include/UNIX_macro.h"

template <typename VertexValueType, typename MessageValueType>
class UtilClient
{
public:
    UtilClient(int vCount, int eCount, int numOfInitV, int nodeNo = 0);
    ~UtilClient() = default;

    int connect();
    int transfer(VertexValueType *vValues, Vertex *vSet, Edge *eSet, int *initVSet, bool *filteredV, int *timestamp);
    int update(VertexValueType *vValues, Vertex *vSet, int *avSet = nullptr, int avCount = -1);
    int update(VertexValueType *vValues, int *avSet = nullptr, int avCount = -1);
    void requestMSGApply();
    void requestMSGMerge();
    void disconnect();
    void shutdown();
    void graphInit();

    int nodeNo;

    int vCount;
    int eCount;
    int numOfInitV;

    int *initVSet;
    bool *filteredV;
    int *timestamp;
    VertexValueType *vValues;
    MessageValueType *mValues;
    Vertex *vSet;
    Edge *eSet;

    int *avSet;
    int *avCount;

private:
    UNIX_shm initVSet_shm;
    UNIX_shm filteredV_shm;
    UNIX_shm timestamp_shm;
    UNIX_shm vValues_shm;
    UNIX_shm mValues_shm;
    UNIX_shm vSet_shm;
    UNIX_shm eSet_shm;
    UNIX_shm avSet_shm;
    UNIX_shm avCount_shm;

    UNIX_msg server_msq;
    UNIX_msg client_msq;
};

#endif //GRAPH_ALGO_UTILCLIENT_H
