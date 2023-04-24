//
// Created by Thoh Testarossa on 2019-04-05.
//

#include "../UtilServer.cpp"

#include "../../algo/BellmanFord/BellmanFord.cpp"
#include "../../algo/LabelPropagation/LabelPropagation.cpp"
#include "../../algo/PageRank/PageRank.cpp"
#include "../../algo/JumpIteration/JumpIteration.cpp"
#include "../../algo/ConnectedComponent/ConnectedComponent.cpp"

template class UtilServer<BellmanFord<double, double>, double, double>;
template class UtilServer<LabelPropagation<LPA_Value, LPA_MSG>, LPA_Value, LPA_MSG>;
template class UtilServer<PageRank<std::pair<double, double>, PRA_MSG>, std::pair<double, double>, PRA_MSG>;
template class UtilServer<JumpIteration<double, double>, double, double>;
template class UtilServer<ConnectedComponent<int, int>, int, int>;