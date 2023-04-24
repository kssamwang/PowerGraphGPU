//
// Created by Thoh Testarossa on 2019-04-05.
//

#pragma once

#ifndef GRAPH_ALGO_UTILSERVER_H
#define GRAPH_ALGO_UTILSERVER_H

#include "../core/GraphUtil.h"
#include "../srv/UNIX_shm.h"
#include "../srv/UNIX_msg.h"
#include "../include/UNIX_macro.h"

template <typename GraphUtilType, typename VertexValueType, typename MessageValueType>
class UtilServer
{
public:
    UtilServer(int vCount, int eCount, int numOfInitV, int nodeNo = 0);
    ~UtilServer();

    void run();

    void graphInit();
    bool getEdgesFromAvSet();

    int nodeNo;
    GraphUtilType executor;
    bool isLegal;

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

    std::vector<std::vector<Edge>> adjacencyTable;
    int *avSet;
    int *avCount;

    Edge *avESet;
    int avECount;

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
    UNIX_msg init_msq;
    UNIX_msg client_msq;
};

#endif //GRAPH_ALGO_UTILSERVER_H
