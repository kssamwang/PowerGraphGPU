//
// Created by liqi on 19-5-27.
//

#include "../UtilClient.cpp"
#include "../../algo/LabelPropagation/LabelPropagation.h"
#include "../../algo/PageRank/PageRank.h"
#include "../../algo/ConnectedComponent/ConnectedComponent.h"

template class UtilClient<double, double>;
template class UtilClient<std::pair<double, double>, PRA_MSG>;
template class UtilClient<LPA_Value, LPA_MSG>;
template class UtilClient<int, int>;
