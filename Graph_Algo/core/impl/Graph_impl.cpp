//
// Created by Thoh Testarossa on 2019-05-25.
//

#include "../Graph.cpp"
#include "../../algo/LabelPropagation/LabelPropagation.h"
#include "../../algo/DDFS/DDFS.h"

template class Graph<double>;
template class Graph<int>;
template class Graph<std::pair<double, double>>;
template class Graph<LPA_Value>;
template class Graph<DFSValue>;