//
// Created by Thoh Testarossa on 2019-04-05.
//

#include "../UtilServer.cpp"

#include "../../algo/BellmanFord/BellmanFordGPU.cpp"
#include "../../algo/ConnectedComponent/ConnectedComponentGPU.cpp"
#include "../../algo/PageRank/PageRankGPU.cpp"
#include "../../algo/LabelPropagation/LabelPropagationGPU.cpp"

template class UtilServer<BellmanFordGPU<double, double>, double, double>;
template class UtilServer<ConnectedComponentGPU<int, int>, int, int>;
template class UtilServer<LabelPropagationGPU<LPA_Value, LPA_MSG>, LPA_Value, LPA_MSG>;
template class UtilServer<PageRankGPU<std::pair<double, double>, PRA_MSG>, std::pair<double, double>, PRA_MSG>;
