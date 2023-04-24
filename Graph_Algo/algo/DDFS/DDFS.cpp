//
// Created by Thoh Testarossa on 2019-08-22.
//

#include "DDFS.h"

#include <iostream>
#include <ctime>
#include <queue>

template <typename VertexValueType, typename MessageValueType>
DDFS<VertexValueType, MessageValueType>::DDFS()
{

}

template <typename VertexValueType, typename MessageValueType>
int DDFS<VertexValueType, MessageValueType>::MSGApply(Graph<VertexValueType> &g, const std::vector<int> &initVSet,
                                     std::set<int> &activeVertice, const MessageSet<MessageValueType> &mSet)
{
    //Activity reset
    activeVertice.clear();

    //Availability check
    if(g.vCount <= 0) return 0;

    //Organize MessageValueType vector
    auto tmpMSGVector = std::vector<MessageValueType>();
    tmpMSGVector.reserve(mSet.mSet.size());
    for(const auto &m : mSet.mSet) tmpMSGVector.emplace_back(m.value);

    //array form computation
    this->MSGApply_array(g.vCount, mSet.mSet.size(), &g.vList[0], this->numOfInitV, &initVSet[0], &g.verticesValue[0], &tmpMSGVector[0]);

    //Active vertices set assembly
    for(int i = 0; i < g.vCount; i++)
    {
        if(g.vList.at(i).isActive)
            activeVertice.insert(i);
    }

    return activeVertice.size();
}

template <typename VertexValueType, typename MessageValueType>
int DDFS<VertexValueType, MessageValueType>::MSGGenMerge(const Graph<VertexValueType> &g, const std::vector<int> &initVSet,
                                        const std::set<int> &activeVertice, MessageSet<MessageValueType> &mSet)
{
    //Availability check
    if(g.vCount <= 0) return 0;

    //Reset mSet
    mSet.mSet.clear();
    mSet.mSet.reserve(2 * g.eCount);

    auto tmpMSGSet = std::vector<MessageValueType>(2 * g.eCount, MessageValueType());

    //array form computation
    this->MSGGenMerge_array(g.vCount, g.eCount, &g.vList[0], &g.eList[0], this->numOfInitV, &initVSet[0], &g.verticesValue[0], &tmpMSGSet[0]);

    //Package msgs
    for(const auto &m : tmpMSGSet)
    {
        if(m.src != -1) mSet.insertMsg(m);
        else break;
    }

    return mSet.mSet.size();
}

template <typename VertexValueType, typename MessageValueType>
int DDFS<VertexValueType, MessageValueType>::MSGApply_array(int vCount, int eCount, Vertex *vSet, int numOfInitV, const int *initVSet,
                                           VertexValueType *vValues, MessageValueType *mValues)
{
    int avCount = 0;

    //Reset vertex activity
    for(int i = 0; i < vCount; i++)
        vSet[i].isActive = false;

    //Reset opbit & vNextMSGTo
    for(int i = 0; i < vCount; i++)
    {
        vValues[i].opbit = 0;
        vValues[i].vNextTokenMSGTo = -1;
    }

    //Test
    //std::cout << eCount << std::endl;

    //Check each msgs
    //msgs are sent from edges
    //eCount here is the account of edges which contains messages rather than the account of g's edges
    //(eCount = mValues.size)
    for(int i = 0; i < eCount; i++)
    {
        //Test
        std::cout << mValues[i].src << " " << mValues[i].dst << std::endl;

        //msg token check
        if(mValues[i].msgbit == MSG_TOKEN)
        {
            //Test
            std::cout << "TOKEN" << std::endl;

            if(vValues[mValues[i].dst].state == STATE_IDLE)
            {
                //Mark j as i's father
                vValues[mValues[i].dst].parent = mValues[i].src;
                //There should be some approach more efficient
                for(auto &vState : vValues[mValues[i].dst].vStateList)
                {
                    if(vState.second.first == mValues[i].src)
                    {
                        vState.second.second = MARK_PARENT;
                        break;
                    }
                }

                vValues[mValues[i].dst].state = STATE_DISCOVERED;
                vValues[mValues[i].dst].vNextTokenMSGTo = this->search(mValues[i].dst, numOfInitV, initVSet, vSet, vValues, avCount);

                //prepare to broadcast msg "visited" to other vertices
                vValues[mValues[i].dst].opbit |= OP_BROADCAST;

                //Vertex which will send msg will be activated
                if(!vSet[mValues[i].dst].isActive)
                {
                    vSet[mValues[i].dst].isActive = true;
                    avCount++;
                }
            }
            else
            {
                for(auto &vState : vValues[mValues[i].dst].vStateList)
                {
                    if(vState.second.first == mValues[i].src)
                    {
                        if(vState.second.second == MARK_UNVISITED)
                            vState.second.second = MARK_VISITED;
                        else if(vState.second.second == MARK_SON)
                        {
                            vValues[mValues[i].dst].vNextTokenMSGTo = this->search(mValues[i].dst, numOfInitV, initVSet, vSet, vValues, avCount);
                            if(vValues[mValues[i].dst].vNextTokenMSGTo != -1)
                            {
                                //Test
                                std::cout << 1145141919810 << std::endl;

                                vSet[mValues[i].dst].isActive = true;
                                avCount++;
                            }
                        }
                    }
                }
            }
        }
        //msg visited check
        else if(mValues[i].msgbit == MSG_VISITED)
        {
            //Test
            std::cout << "VISITED" << std::endl;

            //There should be some approach more efficient
            for(auto &vState : vValues[mValues[i].dst].vStateList)
            {
                if(vState.second.first == mValues[i].src)
                {
                    //Test
                    //std::cout << "V!: " << mValues[i].dst << " " << vState.second.first << std::endl;

                    if(vState.second.second == MARK_UNVISITED)
                    {
                        //Test
                        //std::cout << "V!: " << mValues[i].dst << " " << vState.second.first << std::endl;

                        vState.second.second = MARK_VISITED;
                        vValues[mValues[i].dst].vNextTokenMSGTo = -1;
                    }
                    else if(vState.second.second == MARK_SON)
                    {
                        vState.second.second = MARK_VISITED;
                        vValues[mValues[i].dst].vNextTokenMSGTo = this->search(mValues[i].dst, numOfInitV, initVSet, vSet, vValues, avCount);
                    }
                }
            }
        }
        else;
    }

    //Test
    /*
    for(int i = 0; i < vCount; i++)
    {
        for(int j = 0; j < vValues[i].relatedVCount; j++)
        {
            std::cout << i << " ";
            if(vValues[i].vStateList.at(j).first) std::cout << "->";
            else std::cout << "<-";
            std::cout << " " << vValues[i].vStateList.at(j).second.first << ": ";
            if(vValues[i].vStateList.at(j).second.second == MARK_UNVISITED)
                std::cout << "UNVISITED ";
            if(vValues[i].vStateList.at(j).second.second == MARK_VISITED)
                std::cout << "VISITED ";
            if(vValues[i].vStateList.at(j).second.second == MARK_PARENT)
                std::cout << "PARENT ";
            if(vValues[i].vStateList.at(j).second.second == MARK_SON)
                std::cout << "SON ";
            std::cout << std::endl;
        }
    }
     */

    return avCount;
}

/*
 * There are 2 problems which exist in MSGGenMerge
 * 1. VNextMSGTo may not exist in specific subgraph since there may no any edges(vState) connected to this vertex in this subgraph
 * 2. vState can not tell the relationship between 2 vertices which corresponding edge can tell
 */

template <typename VertexValueType, typename MessageValueType>
int
DDFS<VertexValueType, MessageValueType>::MSGGenMerge_array(int vCount, int eCount, const Vertex *vSet, const Edge *eSet, int numOfInitV,
                                         const int *initVSet, const VertexValueType *vValues, MessageValueType *mValues)
{
    int msgCount = 0;

    for(int i = 0; i < vCount; i++)
    {
        if(vSet[i].isActive)
        {
            for(const auto &vState : vValues[i].vStateList)
            {
                //if (vState.first)
                //{
                    //Check if needed to generate broadcast msg
                    if (vValues[i].opbit & OP_BROADCAST)
                    {
                        if (vState.second.second == MARK_UNVISITED || vState.second.second == MARK_VISITED)
                        {
                            mValues[msgCount].src = i;
                            mValues[msgCount].dst = vState.second.first;
                            mValues[msgCount].msgbit = MSG_VISITED;
                            //Not implemented yet
                            mValues[msgCount].timestamp = 0;

                            //Test
                            std::cout << i << " " << vState.second.first << ": " << "visited" << std::endl;

                            msgCount++;
                        }
                    }

                    //Check if needed to generate search msg
                    if (vValues[i].opbit & OP_MSG_FROM_SEARCH)
                    {
                        if (vState.second.first == vValues[i].vNextTokenMSGTo)
                        {
                            mValues[msgCount].src = i;
                            mValues[msgCount].dst = vValues[i].vNextTokenMSGTo;
                            mValues[msgCount].msgbit = MSG_TOKEN;
                            //Not implemented yet
                            mValues[msgCount].timestamp = 0;

                            //Test
                            std::cout << i << " " << vState.second.first << ": " << "token" << std::endl;

                            msgCount++;
                        }
                    }
                //}
            }
        }
    }

    return msgCount;
}

template <typename VertexValueType, typename MessageValueType>
void DDFS<VertexValueType, MessageValueType>::MergeGraph(Graph<VertexValueType> &g, const std::vector<Graph<VertexValueType>> &subGSet,
                                       std::set<int> &activeVertices,
                                       const std::vector<std::set<int>> &activeVerticeSet,
                                       const std::vector<int> &initVList)
{
    //Test

    for(int i = 0; i < subGSet.size(); i++)
    {
        for(int j = 0; j < subGSet.at(i).vCount; j++)
        {
            std::cout << j << ": parent: " << subGSet.at(i).verticesValue.at(j).parent << " ";
            std::cout << subGSet.at(i).vList.at(j).isActive << " ";
            if(subGSet.at(i).verticesValue.at(j).opbit & OP_BROADCAST)
                std::cout << "BROADCAST ";
            if(subGSet.at(i).verticesValue.at(j).opbit & OP_MSG_FROM_SEARCH)
                std::cout << "SEARCH MSG " << subGSet.at(i).verticesValue.at(j).vNextTokenMSGTo << " ";
            if(subGSet.at(i).verticesValue.at(j).opbit & OP_MSG_DOWNWARD)
                std::cout << "DOWNWARD ";
            std::cout << std::endl;
            for(int k = 0; k < subGSet.at(i).verticesValue.at(j).relatedVCount; k++)
            {
                std::cout << j << " ";
                if(subGSet.at(i).verticesValue.at(j).vStateList.at(k).first) std::cout << "->";
                else std::cout << "<-";
                std::cout << " " << subGSet.at(i).verticesValue.at(j).vStateList.at(k).second.first;
                std::cout << ": ";
                if(subGSet.at(i).verticesValue.at(j).vStateList.at(k).second.second == MARK_UNVISITED)
                    std::cout << "UNVISITED ";
                if(subGSet.at(i).verticesValue.at(j).vStateList.at(k).second.second == MARK_VISITED)
                    std::cout << "VISITED ";
                if(subGSet.at(i).verticesValue.at(j).vStateList.at(k).second.second == MARK_PARENT)
                    std::cout << "PARENT ";
                if(subGSet.at(i).verticesValue.at(j).vStateList.at(k).second.second == MARK_SON)
                    std::cout << "SON ";
                std::cout << std::endl;
            }
        }
        std::cout << std::endl;
    }

    //Some backups for vState fix phase
    auto backupCheckSet = std::set<int>();
    auto backupVVSet = std::vector<std::pair<int, VertexValueType>>();
    for(const auto &subG : subGSet)
    {
        for(int i = 0; i < subG.vCount; i++)
        {
            if(subG.verticesValue.at(i).vNextTokenMSGTo != -1)
            {
                if(backupCheckSet.find(i) == backupCheckSet.end())
                    backupVVSet.emplace_back(std::pair<int, VertexValueType>(i, g.verticesValue.at(i)));
            }
        }
    }

    //Reset v activity
    for(auto &v : g.vList) v.isActive = false;
    activeVertices.clear();

    //Reset global vValues
    for(auto &vV : g.verticesValue)
    {
        //state reset
        vV.state = STATE_IDLE;
        //parent reset
        vV.parent = -1;
        //vNextMSGTo reset
        vV.vNextTokenMSGTo = -1;
        //opbit reset
        vV.opbit = (char)0;
        //searchDownwardVCount reset
        vV.searchDownwardVCount = 0;

        //Didn't be implemented yet
        //startTime reset
        //endTime reset
    }

    //Merge subGs parameters
    for(const auto &subG : subGSet)
    {
        for(int i = 0; i < g.vCount; i++)
        {
            const auto &vVSub = subG.verticesValue.at(i);
            auto &vV = g.verticesValue.at(i);
            //state merge
            vV.state |= vVSub.state;

            /*
            //vNextMSGTo merge
            if((!(vV.opbit & OP_MSG_FROM_SEARCH)) && (vVSub.opbit & OP_MSG_FROM_SEARCH)) vV.vNextMSGTo = vVSub.vNextMSGTo;
            else if((!(vV.opbit & OP_MSG_DOWNWARD)) && (vVSub.opbit & OP_MSG_DOWNWARD)) vV.vNextMSGTo = vVSub.vNextMSGTo;
            else;
             */

            //opbit merge
            vV.opbit |= vVSub.opbit;
            //parent merge
            if(vVSub.parent != -1) vV.parent = vVSub.parent;

            //vV.searchDownwardVCount += vVSub.searchDownwardVCount;

            //Didn't be implemented yet
            //startTime merge
            //endTime merge
        }
    }

    //Test
    /*
    for(int i = 0; i < g.vCount; i++)
        std::cout << g.verticesValue.at(i).searchDownwardVCount << " ";
    std::cout << std::endl;
     */

    //Merge subG vStateList
    int subGCount = subGSet.size();
    int *subGIndex = new int [subGCount];

    for(int i = 0; i < g.vCount; i++)
    {
        for(int j = 0; j < subGCount; j++) subGIndex[j] = 0;

        auto &vV = g.verticesValue.at(i);
        for(int j = 0; j < vV.relatedVCount; j++)
        {
            //Test
            /*
            std::cout << i << " ";
            if(vV.vStateList.at(j).first) std::cout << "->";
            else std::cout << "<-";
            std::cout << " " << vV.vStateList.at(j).second.first;
             */

            for(int k = 0; k < subGCount; k++)
            {
                if(subGIndex[k] < subGSet.at(k).verticesValue.at(i).vStateList.size())
                {
                    const auto &vStateSub = subGSet.at(k).verticesValue.at(i).vStateList.at(subGIndex[k]);
                    if (vV.vStateList.at(j).first == vStateSub.first)
                    {
                        if(vV.vStateList.at(j).second.first == vStateSub.second.first)
                        {
                            //Test
                            /*
                            std::cout << ": " << "match at subG[" << k << "][" << subGIndex[k] << "]: ";
                            std::cout << i << " ";
                            if(vStateSub.first) std::cout << "->";
                            else std::cout << "<-";
                            std::cout << " " << vStateSub.second.first << ": ";
                            if(vStateSub.second.second == MARK_UNVISITED)
                                std::cout << "UNVISITED ";
                            if(vStateSub.second.second == MARK_VISITED)
                                std::cout << "VISITED ";
                            if(vStateSub.second.second == MARK_PARENT)
                                std::cout << "PARENT ";
                            if(vStateSub.second.second == MARK_SON)
                                std::cout << "SON ";
                            */

                            if(vV.vStateList.at(j).second.second == MARK_UNVISITED || vV.vStateList.at(j).second.second == MARK_VISITED)
                                vV.vStateList.at(j).second.second = vStateSub.second.second;
                            else if(vV.vStateList.at(j).second.second != vStateSub.second.second)
                            {
                                std::cout
                                        << "Conflicted!***************************************************************" << i << " " << vV.vStateList.at(j).second.first << " " << (int)vV.vStateList.at(j).second.second << " " << (int)vStateSub.second.second
                                        << std::endl;
                            }
                            subGIndex[k]++;
                        }
                    }
                }
            }

            //Test
            //std::cout << std::endl;
        }
    }

    //Calculate searchDownwardVCount
    for(int i = 0; i < g.vCount; i++)
    {
        for(const auto &vState : g.verticesValue.at(i).vStateList)
        {
            if(vState.first && vState.second.second == MARK_UNVISITED)
                g.verticesValue.at(i).searchDownwardVCount++;
        }
    }

    //Test
    for(int i = 0; i < g.vCount; i++)
        std::cout << g.verticesValue.at(i).searchDownwardVCount << " ";
    std::cout << std::endl;

    //Determine where token need to go if needed
    for(int i = 0; i < g.vCount; i++)
    {
        if(g.verticesValue.at(i).opbit & OP_MSG_FROM_SEARCH)
        {
            /*
             * At first, there are AT MOST 1 token can be sent in each iter -> each vertex can send at most once in each iter
             * Also, for a specific vertex named v, search for v can be executed at most in 1 partition in each iter since search always generates a token
             * Assume that result from 1 partition states that token will be sent from vertex which vV represents
             * Assume the name of this partition is pA
             */

            /*
             * If at this time, vV.searchDownwardVCount == 0
             * It means that no any other edges remained to make token go downward -> Necessary to find direction of token in results partitions returned
             * If token can go downward when examining results partitions returned, token will go downward rather than go upward
             */
            if(g.verticesValue.at(i).searchDownwardVCount == 0)
            {
                for(const auto &subG : subGSet)
                {
                    //a vertex cannot pass a token to itself
                    if(subG.verticesValue.at(i).vNextTokenMSGTo != -1 && i != subG.verticesValue.at(i).vNextTokenMSGTo)
                    {
                        g.verticesValue.at(i).vNextTokenMSGTo = subG.verticesValue.at(i).vNextTokenMSGTo;
                        if(subG.verticesValue.at(i).opbit & OP_MSG_DOWNWARD)
                            break;
                    }
                }
            }

            /*
             * If at this time, vV.searchDownwardVCount != 0, and result from pA states that token should be passed downward, that's okay.
             * If at this time, vV.searchDownwardVCount != 0, but result from pA states that token should be returned to v's parent
             * This is an error caused by incomplete graph info in each subG. Fixes needed
             */
            if(g.verticesValue.at(i).searchDownwardVCount != 0)
            {
                bool chk = false;
                int tempNextTo = -1;

                for(const auto &subG : subGSet)
                {
                    if(subG.verticesValue.at(i).vNextTokenMSGTo != -1)
                    {
                        for(const auto &vState : subG.verticesValue.at(i).vStateList)
                        {
                            //a vertex cannot pass a token to itself
                            if(vState.second.first == subG.verticesValue.at(i).vNextTokenMSGTo && i != vState.second.first)
                            {
                                if(!chk && vState.second.second == MARK_SON)
                                    tempNextTo = vState.second.first;
                                chk |= vState.second.second == MARK_SON;
                            }
                        }
                    }
                }

                if(chk) g.verticesValue.at(i).vNextTokenMSGTo = tempNextTo;
                else
                {
                    for(const auto &subG : subGSet)
                    {
                        if(subG.verticesValue.at(i).searchDownwardVCount > 0)
                        {
                            for(const auto &vState : subG.verticesValue.at(i).vStateList)
                            {
                                //Test
                                std::cout << vState.first << " " << i << " " << vState.second.first << " " << (int)vState.second.second << std::endl;

                                //a vertex cannot pass a token to itself
                                if(vState.first == true && vState.second.second == MARK_UNVISITED && i != vState.second.first)
                                {
                                    g.verticesValue.at(i).vNextTokenMSGTo = vState.second.first;
                                    break;
                                }
                            }
                            if(g.verticesValue.at(i).vNextTokenMSGTo != -1)
                            {
                                g.verticesValue.at(i).opbit |= OP_MSG_DOWNWARD;
                                break;
                            }
                        }
                    }
                }
            }
        }
        else g.verticesValue.at(i).vNextTokenMSGTo = -1;
    }

    /*
    //Merge subG vStateList
    int subGCount = subGSet.size();
    int *subGIndex = new int [subGCount];

    for(int i = 0; i < g.vCount; i++)
    {
        for(int j = 0; j < subGCount; j++) subGIndex[j] = 0;

        auto &vV = g.verticesValue.at(i);
        for(int j = 0; j < vV.relatedVCount; j++)
        {
            //Test

            std::cout << i << " ";
            if(vV.vStateList.at(j).first) std::cout << "->";
            else std::cout << "<-";
            std::cout << " " << vV.vStateList.at(j).second.first;


            for(int k = 0; k < subGCount; k++)
            {
                if(subGIndex[k] < subGSet.at(k).verticesValue.at(i).vStateList.size())
                {
                    const auto &vStateSub = subGSet.at(k).verticesValue.at(i).vStateList.at(subGIndex[k]);
                    if (vV.vStateList.at(j).first == vStateSub.first)
                    {
                        if(vV.vStateList.at(j).second.first == vStateSub.second.first)
                        {
                            //Test

                            std::cout << ": " << "match at subG[" << k << "][" << subGIndex[k] << "]: ";
                            std::cout << i << " ";
                            if(vStateSub.first) std::cout << "->";
                            else std::cout << "<-";
                            std::cout << " " << vStateSub.second.first << ": ";
                            if(vStateSub.second.second == MARK_UNVISITED)
                                std::cout << "UNVISITED ";
                            if(vStateSub.second.second == MARK_VISITED)
                                std::cout << "VISITED ";
                            if(vStateSub.second.second == MARK_PARENT)
                                std::cout << "PARENT ";
                            if(vStateSub.second.second == MARK_SON)
                                std::cout << "SON ";


                            if(vV.vStateList.at(j).second.second == MARK_UNVISITED && vStateSub.second.second != MARK_UNVISITED)
                                vV.vStateList.at(j).second.second = vStateSub.second.second;
                            else if(vV.vStateList.at(j).second.second != vStateSub.second.second)
                                std::cout << "Conflicted!" << std::endl;
                            subGIndex[k]++;
                            break;
                        }
                    }
                }
            }

            //Test
            //std::cout << std::endl;
        }
    }
    */

    //Test
    /*
    for(int j = 0; j < g.vCount; j++)
    {
        std::cout << j << ": ";
        std::cout << g.vList.at(j).isActive << " " << g.verticesValue.at(j).searchDownwardVCount << " ";
        if(g.verticesValue.at(j).opbit & OP_BROADCAST)
            std::cout << "BROADCAST ";
        if(g.verticesValue.at(j).opbit & OP_MSG_FROM_SEARCH)
            std::cout << "SEARCH MSG " << g.verticesValue.at(j).vNextTokenMSGTo << " ";
        if(g.verticesValue.at(j).opbit & OP_MSG_DOWNWARD)
            std::cout << "DOWNWARD ";
        std::cout << std::endl;
        for(int k = 0; k < g.verticesValue.at(j).relatedVCount; k++)
        {
            std::cout << j << " ";
            if(g.verticesValue.at(j).vStateList.at(k).first) std::cout << "->";
            else std::cout << "<-";
            std::cout << " " << g.verticesValue.at(j).vStateList.at(k).second.first;
            std::cout << ": ";
            if(g.verticesValue.at(j).vStateList.at(k).second.second == MARK_UNVISITED)
                std::cout << "UNVISITED ";
            if(g.verticesValue.at(j).vStateList.at(k).second.second == MARK_VISITED)
                std::cout << "VISITED ";
            if(g.verticesValue.at(j).vStateList.at(k).second.second == MARK_PARENT)
                std::cout << "PARENT ";
            if(g.verticesValue.at(j).vStateList.at(k).second.second == MARK_SON)
                std::cout << "SON ";
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
     */

    /*
     * Fixes for vStates
     * 1. Assume a token will be sent from a to b, a.vState[b] should be either parent or son
     * If a.vState[b] == unvisited, it means that a will mark b as its son
     */
    for(int i = 0; i < g.vCount; i++)
    {
        if(g.verticesValue.at(i).vNextTokenMSGTo != -1)
        {
            for(auto &vState : g.verticesValue.at(i).vStateList)
            {
                if(vState.second.first == g.verticesValue.at(i).vNextTokenMSGTo)
                {
                    if(vState.first && vState.second.second == MARK_UNVISITED)
                    {
                        g.verticesValue.at(i).searchDownwardVCount--;
                        vState.second.second = MARK_SON;
                    }
                }
            }
        }
    }

    /*
     * 2. If multiple partitions mark sons for vertex v <=> multiple partitions enter search() when processing vertex v
     * However, at most 1 vertex (which will be marked as vNextTokenMSGTo in g.verticesValue.at(v)) can be marked as the son of vertex v in each iter
     * => Marks for other vertices will be returned to the state before iter executed
     */
    for(int i = 0; i < g.vCount; i++)
    {
        if(g.verticesValue.at(i).vNextTokenMSGTo != -1)
        {
            for(auto &vState : g.verticesValue.at(i).vStateList)
            {
                if(vState.second.second == MARK_SON && vState.second.first != g.verticesValue.at(i).vNextTokenMSGTo)
                {
                    for(const auto &vVPair : backupVVSet)
                    {
                        if(vVPair.first == i)
                        {
                            for(const auto &vBState : vVPair.second.vStateList)
                            {
                                if(vBState.second.first == vState.second.first)
                                {
                                    vState.second.second = vBState.second.second;
                                    if(vBState.second.second == MARK_UNVISITED)
                                        g.verticesValue.at(i).searchDownwardVCount++;
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    //vState.second.second == MARK_UNVISITED;
                    //g.verticesValue.at(i).searchDownwardVCount++;
                }
            }
        }
    }

    //Merge activeVertices & mark active v
    for(const auto &avs : activeVerticeSet)
    {
        for(const auto &av : avs)
            activeVertices.insert(av);
    }

    for(const auto &av : activeVertices)
        g.vList.at(av).isActive = true;

    //Test

    for(int j = 0; j < g.vCount; j++)
    {
        std::cout << j << ": parent: " << g.verticesValue.at(j).parent << " ";
        std::cout << g.vList.at(j).isActive << " " << g.verticesValue.at(j).searchDownwardVCount << " ";
        if(g.verticesValue.at(j).opbit & OP_BROADCAST)
            std::cout << "BROADCAST ";
        if(g.verticesValue.at(j).opbit & OP_MSG_FROM_SEARCH)
            std::cout << "SEARCH MSG " << g.verticesValue.at(j).vNextTokenMSGTo << " ";
        if(g.verticesValue.at(j).opbit & OP_MSG_DOWNWARD)
            std::cout << "DOWNWARD ";
        std::cout << std::endl;
        for(int k = 0; k < g.verticesValue.at(j).relatedVCount; k++)
        {
            std::cout << j << " ";
            if(g.verticesValue.at(j).vStateList.at(k).first) std::cout << "->";
            else std::cout << "<-";
            std::cout << " " << g.verticesValue.at(j).vStateList.at(k).second.first;
            std::cout << ": ";
            if(g.verticesValue.at(j).vStateList.at(k).second.second == MARK_UNVISITED)
                std::cout << "UNVISITED ";
            if(g.verticesValue.at(j).vStateList.at(k).second.second == MARK_VISITED)
                std::cout << "VISITED ";
            if(g.verticesValue.at(j).vStateList.at(k).second.second == MARK_PARENT)
                std::cout << "PARENT ";
            if(g.verticesValue.at(j).vStateList.at(k).second.second == MARK_SON)
                std::cout << "SON ";
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;

}

template <typename VertexValueType, typename MessageValueType>
void DDFS<VertexValueType, MessageValueType>::Init(int vCount, int eCount, int numOfInitV)
{
    this->numOfInitV = numOfInitV;

    //Memory parameter init
    this->totalVValuesCount = vCount;
    this->totalMValuesCount = eCount;
}

template <typename VertexValueType, typename MessageValueType>
void DDFS<VertexValueType, MessageValueType>::GraphInit(Graph<VertexValueType> &g, std::set<int> &activeVertices,
                                      const std::vector<int> &initVList)
{
    int avCount = 0;

    //Global init
    //Init graph parameters
    for(int i = 0; i < g.vCount; i++) g.verticesValue.emplace_back(VertexValueType());
    //Scan edges in graph and collect info
    /*
     * for edge (a, b):
     *     add pair (b, MARK_UNVISITED) as (vid, mark) into a's vState priority queue ordered by vid
     *     add pair (a, MARK_UNVISITED) as (vid, mark) into b's vState priority queue ordered by vid
    */
    auto pqVector = std::vector<std::priority_queue<std::pair<bool, std::pair<int, char>>, std::vector<std::pair<bool, std::pair<int, char>>>, cmp>>(g.vCount, std::priority_queue<std::pair<bool, std::pair<int, char>>, std::vector<std::pair<bool, std::pair<int, char>>>, cmp>());
    for(const auto &e : g.eList)
    {
        auto srcVV = std::pair<bool, std::pair<int, char>>(true, std::pair<int, char>(e.dst, MARK_UNVISITED));
        pqVector.at(e.src).push(srcVV);
        auto dstVV = std::pair<bool, std::pair<int, char>>(false, std::pair<int, char>(e.src, MARK_UNVISITED));
        pqVector.at(e.dst).push(dstVV);
    }

    //For every vertex (for example i), pull sorted vState pairs from pq and push them into g.verticesValue.at(i).vStateList
    //The order of verticesValue.vStateList in graph can be ensured
    for(int i = 0; i < g.vCount; i++)
    {
        int pqPreV = -1;

        while(!pqVector.at(i).empty())
        {
            //To ensure that for there is only one mark for vertex b in vertex a
            //Also, the order of pq ensures that for marks generated by edges between a and b, a->b must be in front of a<-b in pq if both of them exists.
            if(pqPreV != pqVector.at(i).top().second.first)
            {
                pqPreV = pqVector.at(i).top().second.first;
                g.verticesValue.at(i).vStateList.emplace_back(pqVector.at(i).top());
                g.verticesValue.at(i).relatedVCount++;
            }
            pqVector.at(i).pop();
        }
    }

    for(int j = 0; j < g.vCount; j++)
    {
        for(auto &vState : g.verticesValue.at(j).vStateList)
        {
            if(vState.first && vState.second.second == MARK_UNVISITED)
                g.verticesValue.at(j).searchDownwardVCount++;
        }
    }

    //initV init
    int initV = initVList.at(0);

    g.vList.at(initV).isActive = true;

    auto &vV = g.verticesValue.at(initV);

    vV.state = STATE_DISCOVERED;
    vV.vNextTokenMSGTo = this->search(initV, this->numOfInitV, &initVList[0], &g.vList[0], &g.verticesValue[0], avCount);
    vV.opbit |= OP_BROADCAST;

    activeVertices.insert(initV);
}

template <typename VertexValueType, typename MessageValueType>
void DDFS<VertexValueType, MessageValueType>::Deploy(int vCount, int eCount, int numOfInitV)
{

}

template <typename VertexValueType, typename MessageValueType>
void DDFS<VertexValueType, MessageValueType>::Free()
{

}

template<typename VertexValueType, typename MessageValueType>
std::vector<Graph<VertexValueType>>
DDFS<VertexValueType, MessageValueType>::DivideGraphByEdge(const Graph<VertexValueType> &g, int partitionCount)
{
    auto res = std::vector<Graph<VertexValueType>>();

    //Divide edges into multiple subgraphs
    auto eG = std::vector<std::vector<Edge>>();
    for(int i = 0; i < partitionCount; i++) eG.emplace_back(std::vector<Edge>());
    for(int i = 0; i < partitionCount; i++)
    {
        for(int j = (i * g.eCount) / partitionCount; j < ((i + 1) * g.eCount) / partitionCount; j++)
            eG.at(i).emplace_back(g.eList.at(j));
    }

    //Init subGs parameters
    auto templateBlankVV = std::vector<VertexValueType>(g.vCount, VertexValueType());
    for(int i = 0; i < g.vCount; i++)
    {
        templateBlankVV.at(i).state = g.verticesValue.at(i).state;
        templateBlankVV.at(i).opbit = g.verticesValue.at(i).opbit;
        templateBlankVV.at(i).parent = g.verticesValue.at(i).parent;
        templateBlankVV.at(i).startTime = g.verticesValue.at(i).startTime;
        templateBlankVV.at(i).endTime = g.verticesValue.at(i).endTime;
        templateBlankVV.at(i).needGenToken = g.verticesValue.at(i).needGenToken;
        templateBlankVV.at(i).vNextTokenMSGTo = g.verticesValue.at(i).vNextTokenMSGTo;
    }

    for(int i = 0; i < partitionCount; i++)
        res.emplace_back(Graph<VertexValueType>(g.vList, eG.at(i), templateBlankVV));

    for(auto &subG : res)
    {
        //Scan edges in each subgraph and collect info
        /*
         * for edge (a, b):
         *     add pair (b, MARK_UNVISITED) as (vid, mark) into a's vState priority queue ordered by vid
         *     add pair (a, MARK_UNVISITED) as (vid, mark) into b's vState priority queue ordered by vid
        */
        auto pqVector = std::vector<std::priority_queue<std::pair<bool, std::pair<int, char>>, std::vector<std::pair<bool, std::pair<int, char>>>, cmp>>(g.vCount, std::priority_queue<std::pair<bool, std::pair<int, char>>, std::vector<std::pair<bool, std::pair<int, char>>>, cmp>());
        for(const auto &e : subG.eList)
        {
            auto srcVV = std::pair<bool, std::pair<int, char>>(true, std::pair<int, char>(e.dst, MARK_UNVISITED));
            pqVector.at(e.src).push(srcVV);
            auto dstVV = std::pair<bool, std::pair<int, char>>(false, std::pair<int, char>(e.src, MARK_UNVISITED));
            pqVector.at(e.dst).push(dstVV);
        }

        //For every vertex (for example i), pull sorted vState pairs from pq and push them into g.verticesValue.at(i).vStateList
        //The order of verticesValue.vStateList in subgraph can be ensured

        //To ensure that for there is only one mark for vertex b in vertex a
        //Also, the order of pq ensures that for marks generated by edges between a and b, a->b must be in front of a<-b in pq if both of them exists.
        for(int i = 0; i < subG.vCount; i++)
        {
            int pqPreV = -1;

            while(!pqVector.at(i).empty())
            {
                if(pqPreV != pqVector.at(i).top().second.first)
                {
                    pqPreV = pqVector.at(i).top().second.first;
                    subG.verticesValue.at(i).vStateList.emplace_back(pqVector.at(i).top());
                    subG.verticesValue.at(i).relatedVCount++;
                }
                pqVector.at(i).pop();
            }
        }
    }

    //Test
    /*
    for(int i = 0; i < res.size(); i++)
    {
        for(int j = 0; j < res.at(i).eCount; j++)
            std::cout << res.at(i).eList.at(j).src << " -> "<< res.at(i).eList.at(j).dst << std::endl;
        std::cout << std::endl;

        for(int j = 0; j < res.at(i).vCount; j++)
        {
            for(int k = 0; k < res.at(i).verticesValue.at(j).relatedVCount; k++)
            {
                std::cout << j << " ";
                if(res.at(i).verticesValue.at(j).vStateList.at(k).first) std::cout << "->";
                else std::cout << "<-";
                std::cout << " " << res.at(i).verticesValue.at(j).vStateList.at(k).second.first;
                std::cout << std::endl;
            }
        }
        std::cout << std::endl;
    }
     */

    //Copy vState from global graph into corresponding subgraph.verticesValue.vStateList
    int subGCount = partitionCount;
    int *subGIndex = new int [subGCount];

    for(int i = 0; i < g.vCount; i++)
    {
        for(int j = 0; j < subGCount; j++) subGIndex[j] = 0;
        for(int j = 0; j < g.verticesValue.at(i).relatedVCount; j++)
        {
            const auto &vV = g.verticesValue.at(i).vStateList.at(j);
            for(int k = 0; k < subGCount; k++)
            {
                if(res.at(k).verticesValue.at(i).relatedVCount > subGIndex[k])
                {
                    while (res.at(k).verticesValue.at(i).vStateList.at(subGIndex[k]).second.first < vV.second.first)
                        subGIndex[k]++;
                    if (res.at(k).verticesValue.at(i).vStateList.at(subGIndex[k]).second.first == vV.second.first)
                    {
                        res.at(k).verticesValue.at(i).vStateList.at(subGIndex[k]).second.second = vV.second.second;
                        subGIndex[k]++;
                    }
                }
            }
        }
    }

    //Calculate searchDownwardVCount
    for(int i = 0; i < res.size(); i++)
    {
        for(int j = 0; j < res.at(i).vCount; j++)
        {
            for(auto &vState : res.at(i).verticesValue.at(j).vStateList)
            {
                if(vState.first && vState.second.second == MARK_UNVISITED)
                    res.at(i).verticesValue.at(j).searchDownwardVCount++;
            }
        }
    }

    //Test
    /*
    for(int i = 0; i < res.size(); i++)
    {
        for(int j = 0; j < res.at(i).eCount; j++)
            std::cout << res.at(i).eList.at(j).src << " -> "<< res.at(i).eList.at(j).dst << std::endl;
        std::cout << std::endl;

        for(int j = 0; j < res.at(i).vCount; j++)
        {
            std::cout << j << ": ";
            if(res.at(i).verticesValue.at(j).opbit & OP_BROADCAST)
                std::cout << "BROADCAST ";
            if(res.at(i).verticesValue.at(j).opbit & OP_MSG_FROM_SEARCH)
                std::cout << "SEARCH MSG " << res.at(i).verticesValue.at(j).vNextTokenMSGTo << " ";
            if(res.at(i).verticesValue.at(j).opbit & OP_MSG_DOWNWARD)
                std::cout << "DOWNWARD ";
            std::cout << std::endl;

            for(int k = 0; k < res.at(i).verticesValue.at(j).relatedVCount; k++)
            {
                std::cout << j << " ";
                if(res.at(i).verticesValue.at(j).vStateList.at(k).first) std::cout << "->";
                else std::cout << "<-";
                std::cout << " " << res.at(i).verticesValue.at(j).vStateList.at(k).second.first << ": ";
                //std::cout << std::endl;
                if(res.at(i).verticesValue.at(j).vStateList.at(k).second.second == MARK_UNVISITED)
                    std::cout << "UNVISITED ";
                if(res.at(i).verticesValue.at(j).vStateList.at(k).second.second == MARK_VISITED)
                    std::cout << "VISITED ";
                if(res.at(i).verticesValue.at(j).vStateList.at(k).second.second == MARK_PARENT)
                    std::cout << "PARENT ";
                if(res.at(i).verticesValue.at(j).vStateList.at(k).second.second == MARK_SON)
                    std::cout << "SON ";
                std::cout << std::endl;
            }
        }
        std::cout << std::endl;
    }
    */

    return res;
}

template <typename VertexValueType, typename MessageValueType>
int DDFS<VertexValueType, MessageValueType>::search(int vid, int numOfInitV, const int *initVSet, Vertex *vSet, VertexValueType *vValues, int &avCount)
{
    bool chk = false;
    for(auto &vState : vValues[vid].vStateList)
    {
        if(vState.first && vState.second.first != vid && vState.second.second == MARK_UNVISITED)
        {
            chk = true;
            vState.second.second = MARK_SON;
            vValues[vid].searchDownwardVCount--;
            vValues[vid].opbit |= OP_MSG_FROM_SEARCH;
            vValues[vid].opbit |= OP_MSG_DOWNWARD;

            //Vertex which will send msg will be activated
            if(!vSet[vid].isActive)
                avCount++;
            vSet[vid].isActive = true;
            return vState.second.first;
        }
    }

    if(!chk)
    {
        if(vid == initVSet[0]) return -1;
        else
        {
            if(!vSet[vid].isActive)
                avCount++;
            vSet[vid].isActive = true;
            vValues[vid].opbit |= OP_MSG_FROM_SEARCH;
            return vValues[vid].parent;

            //There should be some approach more efficient
            /*
            for(auto &vState : vValues[vid].vStateList)
            {
                if(vState.second.second == MARK_PARENT)
                {
                    //Vertex which will send msg will be activated
                    if(!vSet[vid].isActive)
                        avCount++;
                    vSet[vid].isActive = true;
                    vValues[vid].opbit |= OP_MSG_FROM_SEARCH;
                    return vState.second.first;
                }
            }
             */

        }
    }

    return -1;
}

template<typename VertexValueType, typename MessageValueType>
void DDFS<VertexValueType, MessageValueType>::ApplyStep(Graph<VertexValueType> &g, const std::vector<int> &initVSet,
                                                        std::set<int> &activeVertices)
{
    //Test
    for(int i = 0; i < g.vCount; i++)
        std::cout << g.verticesValue.at(i).searchDownwardVCount << " ";
    std::cout << std::endl;

    auto mSet = MessageSet<MessageValueType>();

    mSet.mSet.clear();
    MSGGenMerge(g, initVSet, activeVertices, mSet);

    //Test
    std::cout << "MGenMerge:" << clock() << std::endl;
    //Test end

    activeVertices.clear();
    MSGApply(g, initVSet, activeVertices, mSet);

    //Test
    std::cout << "Apply:" << clock() << std::endl;
    //Test end
}

template<typename VertexValueType, typename MessageValueType>
void DDFS<VertexValueType, MessageValueType>::Apply(Graph<VertexValueType> &g, const std::vector<int> &initVList)
{
    //Init the Graph
    std::set<int> activeVertices = std::set<int>();
    //auto mGenSet = MessageSet<MessageValueType>();
    //auto mMergedSet = MessageSet<MessageValueType>();

    Init(g.vCount, g.eCount, initVList.size());

    GraphInit(g, activeVertices, initVList);

    Deploy(g.vCount, g.eCount, initVList.size());

    while(activeVertices.size() > 0)
        ApplyStep(g, initVList, activeVertices);

    Free();
}

template<typename VertexValueType, typename MessageValueType>
void DDFS<VertexValueType, MessageValueType>::ApplyD(Graph<VertexValueType> &g, const std::vector<int> &initVList,
                                                     int partitionCount)
{
    //Init the Graph
    std::set<int> activeVertices = std::set<int>();

    std::vector<std::set<int>> AVSet = std::vector<std::set<int>>();
    for(int i = 0; i < partitionCount; i++) AVSet.push_back(std::set<int>());
    //auto mGenSetSet = std::vector<MessageSet<MessageValueType>>();
    //for(int i = 0; i < partitionCount; i++) mGenSetSet.push_back(MessageSet<MessageValueType>());
    //auto mMergedSetSet = std::vector<MessageSet<MessageValueType>>();
    //for(int i = 0; i < partitionCount; i++) mMergedSetSet.push_back(MessageSet<MessageValueType>());

    Init(g.vCount, g.eCount, initVList.size());

    GraphInit(g, activeVertices, initVList);

    Deploy(g.vCount, g.eCount, initVList.size());

    int iterCount = 0;

    while(activeVertices.size() > 0)
    {
        //Test
        std::cout << ++iterCount << ":" << clock() << std::endl;
        //Test end

        //Test
        for(int i = 0; i < g.vCount; i++)
            std::cout << g.verticesValue.at(i).searchDownwardVCount << " ";
        std::cout << std::endl;

        auto subGraphSet = this->DivideGraphByEdge(g, partitionCount);

        for(int i = 0; i < partitionCount; i++)
        {
            AVSet.at(i).clear();
            AVSet.at(i) = activeVertices;
        }

        //Test
        std::cout << "GDivide:" << clock() << std::endl;
        //Test end

        for(int i = 0; i < partitionCount; i++)
            ApplyStep(subGraphSet.at(i), initVList, AVSet.at(i));

        activeVertices.clear();
        MergeGraph(g, subGraphSet, activeVertices, AVSet, initVList);
        //Test
        std::cout << "GMerge:" << clock() << std::endl;
        //Test end
    }

    Free();

    //Test
    std::cout << "end" << ":" << clock() << std::endl;
    //Test end
}
